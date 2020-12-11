#include "validator.h"
#include <regex>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

class selector : public validator
{
public:
    selector(const std::vector<std::string> &path) : path_(path) {}

    bool test(const json &j) const override
    {
        const json *out = nullptr;
        return select(j, out);
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

class select_validator : public validator
{
public:
    select_validator(p_selector psel, p_validator pval) : psel_(psel), pval_(pval) {}

    bool test(const json &j) const override
    {
        const json *out = nullptr;
        auto ok = psel_->select(j, out);
        if (!ok) return false;
        return pval_->test(*out);
    }

private:
    p_selector psel_;
    p_validator pval_;
};

class value_validator : public validator
{
public:
    value_validator(const std::string &value) : value_(value) {}

    bool test(const json &j) const override
    {
        std::string s;
        if (j.is_string()) {
            s = j.get<std::string>();
        } else {
            s = j.dump();
        }
        return s == value_;
    }

private:
    std::string value_;
};

class and_validator : public validator
{
public:
    and_validator(p_validator left, p_validator right) : left_(left), right_(right) {}

    bool test(const json &j) const override { return left_->test(j) && right_->test(j); }

private:
    p_validator left_;
    p_validator right_;
};

class or_validator : public validator
{
public:
    or_validator(p_validator left, p_validator right) : left_(left), right_(right) {}

    bool test(const json &j) const override { return left_->test(j) || right_->test(j); }

private:
    p_validator left_;
    p_validator right_;
};

///////////////////////////////////////////////////////////////////////////
//  ast & parser definition
///////////////////////////////////////////////////////////////////////////

namespace ast
{
struct or_expr_value;

using selector_expr_value = std::vector<std::string>;

struct match_expr_value
{
    selector_expr_value v_sel;
    boost::optional<std::string> v_opt;
};

using group_expr_value = x3::forward_ast<or_expr_value>;
using unit_expr_value = boost::variant<match_expr_value, group_expr_value>;
using and_expr_value = std::vector<unit_expr_value>;

struct or_expr_value : std::vector<and_expr_value>
{
};

using entry_value = or_expr_value;

}  // namespace ast

BOOST_FUSION_ADAPT_STRUCT(ast::match_expr_value, v_sel, v_opt);

namespace ast
{
p_validator to_validator(const or_expr_value &v);
p_validator to_validator(const and_expr_value &v);
p_validator to_validator(const unit_expr_value &v);
p_validator to_validator(const group_expr_value &v);
p_validator to_validator(const match_expr_value &v);

p_validator to_validator(const or_expr_value &v)
{
    p_validator combine = to_validator(v.at(0));
    for (int i = 0; i < v.size(); ++i) {
        combine = std::make_shared<or_validator>(combine, to_validator(v.at(i)));
    }
    return combine;
}

p_validator to_validator(const and_expr_value &v)
{
    p_validator combine = to_validator(v.at(0));
    for (int i = 0; i < v.size(); ++i) {
        combine = std::make_shared<and_validator>(combine, to_validator(v.at(i)));
    }
    return combine;
}

class unit_expr_visitor : public boost::static_visitor<p_validator>
{
public:
    p_validator operator()(const match_expr_value &v) const { return to_validator(v); }

    p_validator operator()(const group_expr_value &v) const { return to_validator(v); }
};

p_validator to_validator(const unit_expr_value &v)
{
    return boost::apply_visitor(unit_expr_visitor{}, v);
}

p_validator to_validator(const group_expr_value &v)
{
    return to_validator(static_cast<or_expr_value>(v));
}

p_validator to_validator(const match_expr_value &v)
{
    p_selector pv_sel = std::make_shared<selector>(v.v_sel);
    if (v.v_opt) {
        p_validator pv_opt = std::make_shared<value_validator>(*v.v_opt);
        return std::make_shared<select_validator>(pv_sel, pv_opt);
    }
    return pv_sel;
}

}  // namespace ast

namespace parser
{
const x3::rule<class value_expr_class, std::string> value_expr = "value_expr";
const x3::rule<class selector_expr_class, ast::selector_expr_value> selector_expr = "selector_expr";
const x3::rule<class match_expr_class, ast::match_expr_value> match_expr = "match_expr";
const x3::rule<class group_expr_class, ast::group_expr_value> group_expr = "group_expr";
const x3::rule<class unit_expr_class, ast::unit_expr_value> unit_expr = "unit_expr";
const x3::rule<class and_expr_class, ast::and_expr_value> and_expr = "and_expr";
const x3::rule<class or_expr_class, ast::or_expr_value> or_expr = "or_expr";

const auto plain = +x3::char_("0-9a-zA-Z");
const auto quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
const auto value_expr_def = plain | quoted_string;
const auto selector_expr_def = +x3::char_("-0-9a-zA-Z") % '.';
const auto match_expr_def = selector_expr >> -('=' >> value_expr);
const auto group_expr_def = '(' >> or_expr >> ')';
const auto unit_expr_def = match_expr | group_expr;
const auto and_expr_def = unit_expr % '&';
const auto or_expr_def = and_expr % '|';

BOOST_SPIRIT_DEFINE(value_expr, selector_expr, match_expr, group_expr, unit_expr, and_expr,
                    or_expr);

const auto &entry = or_expr;

}  // namespace parser

bool validator::test(const packet &pac) const
{
    json j;
    auto layers = pac.get_detail().layers;
    for (auto it = layers.cbegin(); it != layers.cend(); ++it) {
        auto type = (*it)->type();
        j[type] = (*it)->to_json();
    }
    j["owner"] = pac.get_detail().owner;
    return test(j);
}

p_validator validator::from_str(const std::string &code)
{
    ast::entry_value ast;
    auto it = code.begin();
    bool ok = x3::phrase_parse(it, code.end(), parser::entry, x3::space, ast);
    if (!ok || it != code.end()) {
        throw std::runtime_error(
            "failed to parse validator: unexpected token near '{}'"_format(*it));
    }
    return ast::to_validator(ast);
}
