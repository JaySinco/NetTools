#include "filter.h"
#include <regex>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>
#include <magic_enum.hpp>

namespace x3 = boost::spirit::x3;

class selector : public filter
{
public:
    selector(const std::vector<std::string> &path) : path_(path) {}

    bool test(const json &j) const override
    {
        const json *out = nullptr;
        return select(j, out);
    }

    json to_json() const override
    {
        json j;
        j["type"] = "selector";
        j["path"] = path_;
        return j;
    }

    bool select(const json &in, const json *&out) const
    {
        const json *j = &in;
        for (const std::string &key : path_) {
            if (j->is_object()) {
                if (!j->contains(key)) {
                    return false;
                }
                j = &j->operator[](key);
            } else if (j->is_array()) {
                if (!std::regex_match(key, std::regex("[0-9]+"))) {
                    return false;
                }
                int index = std::stoi(key);
                if (index < 0 || index >= j->size()) {
                    return false;
                }
                j = &j->operator[](index);
            } else {
                return false;
            }
        }
        out = j;
        return true;
    }

private:
    std::vector<std::string> path_;
};

using p_selector = std::shared_ptr<selector>;

class comparison_filter : public filter
{
public:
    enum class op_t
    {
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_EQUAL,
        GREATER,
        GREATER_EQUAL,
    };

    comparison_filter(p_selector psel, op_t op, const std::string &value)
        : psel_(psel), op_(op), value_(value)
    {
    }

    bool test(const json &j) const override
    {
        const json *child = nullptr;
        auto ok = psel_->select(j, child);
        if (!ok) {
            return false;
        }

        std::string s;
        if (child->is_string()) {
            s = child->get<std::string>();
        } else {
            s = child->dump();
        }
        return s == value_;
    }

    json to_json() const override
    {
        json j;
        j["type"] = "compare";
        j["op"] = magic_enum::enum_name(op_);
        j["selector"] = psel_->to_json();
        j["value"] = value_;
        return j;
    }

private:
    p_selector psel_;
    op_t op_;
    std::string value_;
};

class logic_filter : public filter
{
public:
    enum class op_t
    {
        AND,
        OR,
        NOT,
    };

    logic_filter(const std::vector<p_filter> &pf_list, op_t op) : pf_list_(pf_list), op_(op) {}

    bool test(const json &j) const override
    {
        bool ok;
        switch (op_) {
            case op_t::AND:
                ok = std::accumulate(
                    pf_list_.cbegin(), pf_list_.cend(), true,
                    [&](bool ret, const p_filter &pf) { return ret && pf->test(j); });
                break;
            case op_t::OR:
                ok = std::accumulate(
                    pf_list_.cbegin(), pf_list_.cend(), false,
                    [&](bool ret, const p_filter &pf) { return ret || pf->test(j); });
                break;
            case op_t::NOT:
                ok = !pf_list_[0]->test(j);
                break;
        }
        return ok;
    }

    json to_json() const override
    {
        json j;
        j["type"] = "logic";
        j["op"] = magic_enum::enum_name(op_);
        j["list"] = json::array();
        for (const auto &pf : pf_list_) {
            j["list"].push_back(pf->to_json());
        }
        return j;
    }

private:
    std::vector<p_filter> pf_list_;
    op_t op_;
};

///////////////////////////////////////////////////////////////////////////
//  ast & parser definition
///////////////////////////////////////////////////////////////////////////

namespace ast
{
struct expr_value;

using comp_value = std::pair<comparison_filter::op_t, std::string>;

struct match_value
{
    std::vector<std::string> select_;
    boost::optional<comp_value> compare_;
};

using unit_value = boost::variant<match_value, x3::forward_ast<expr_value>>;
using not_value = std::pair<boost::optional<char>, unit_value>;
using and_value = std::vector<not_value>;
using or_value = std::vector<and_value>;

struct expr_value : or_value
{
    using or_value::operator=;
};

}  // namespace ast

BOOST_FUSION_ADAPT_STRUCT(ast::match_value, select_, compare_);

namespace parser
{
struct compare_op_ : x3::symbols<comparison_filter::op_t>
{
    compare_op_()
    {
        // clang-format off
        add
            ("=" , comparison_filter::op_t::EQUAL)
            ("!=", comparison_filter::op_t::NOT_EQUAL)
            (">" , comparison_filter::op_t::GREATER)
            (">=", comparison_filter::op_t::GREATER_EQUAL)
            ("<" , comparison_filter::op_t::LESS)
            ("<=", comparison_filter::op_t::LESS_EQUAL)
        ;
        // clang-format on
    }

} compare_op;

const x3::rule<class expr_class, ast::expr_value> expr = "expr";
const x3::rule<class comp_class, ast::comp_value> comp = "comp";

const auto plain = x3::lexeme[+x3::char_(".0-9a-zA-Z")];
const auto quoted = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
const auto selector = x3::lexeme[+x3::char_("-0-9a-zA-Z") % '.'];
const auto target = plain | quoted;
const auto comp_def = compare_op >> target;
const auto match = selector >> -comp;
const auto unit = match | '(' >> expr >> ')';
const auto not = -x3::char_('!') >> unit;
const auto and = not % "&&";
const auto or = and % "||";
const auto expr_def = or ;

BOOST_SPIRIT_DEFINE(expr, comp);

}  // namespace parser

namespace ast
{
p_filter to_filter(const expr_value &v);

p_filter to_filter(const match_value &v)
{
    p_selector p_sel = std::make_shared<selector>(v.select_);
    if (!v.compare_) {
        return p_sel;
    }
    auto &cp = *v.compare_;
    return std::make_shared<comparison_filter>(p_sel, cp.first, cp.second);
}

p_filter to_filter(const unit_value &v)
{
    if (auto p_match = boost::get<const match_value>(&v)) {
        return to_filter(*p_match);
    } else if (auto p_expr = boost::get<const x3::forward_ast<expr_value>>(&v)) {
        return to_filter(static_cast<const expr_value &>(*p_expr));
    }
    return {};
}

p_filter to_filter(const not_value &v)
{
    p_filter sec = to_filter(v.second);
    if (!v.first) {
        return sec;
    }
    std::vector<p_filter> pf_list = {sec};
    return std::make_shared<logic_filter>(pf_list, logic_filter::op_t::NOT);
}

p_filter to_filter(const and_value &v)
{
    if (v.size() == 1) {
        return to_filter(v[0]);
    }
    std::vector<p_filter> pf_list;
    for (const auto &it : v) {
        pf_list.push_back(to_filter(it));
    }
    return std::make_shared<logic_filter>(pf_list, logic_filter::op_t::AND);
}

p_filter to_filter(const or_value &v)
{
    if (v.size() == 1) {
        return to_filter(v[0]);
    }
    std::vector<p_filter> pf_list;
    for (const auto &it : v) {
        pf_list.push_back(to_filter(it));
    }
    return std::make_shared<logic_filter>(pf_list, logic_filter::op_t::OR);
}

p_filter to_filter(const expr_value &v) { return to_filter(static_cast<const or_value &>(v)); }

}  // namespace ast

bool filter::test(const packet &pac) const { return test(pac.to_json_flat()); }

p_filter filter::from_str(const std::string &code)
{
    ast::expr_value ast;
    auto it = code.begin();
    bool ok = x3::phrase_parse(it, code.end(), parser::expr, x3::space, ast);
    if (!ok || it != code.end()) {
        throw std::runtime_error("failed to parse filter: unexpected token near '{}'"_format(*it));
    }
    p_filter pf = ast::to_filter(ast);
    VLOG(1) << "filter generated for `{}` => {}"_format(code, pf->to_json().dump(3));
    return pf;
}
