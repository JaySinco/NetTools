#include "validator.h"
#include <regex>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

using p_validator = std::shared_ptr<validator>;

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
//  parser definition
///////////////////////////////////////////////////////////////////////////

namespace parser
{
struct select_attr_value
{
    std::vector<std::string> v;
};

struct conditon_value
{
    select_attr_value v;
    boost::optional<std::string> s;
};

struct expression_value;

struct group_value
{
    x3::forward_ast<expression_value> v;
};

struct factor_value
{
    boost::variant<conditon_value, group_value> v;
};

struct term_value
{
    std::vector<factor_value> v;
};

struct expression_value
{
    std::vector<term_value> v;
};

x3::rule<class select_attr_class, select_attr_value> select_attr = "select_attr";
x3::rule<class conditon_class, conditon_value> conditon = "conditon";
x3::rule<class group_class, group_value> group = "group";
x3::rule<class factor_class, factor_value> factor = "factor";
x3::rule<class term_class, term_value> term = "term";
x3::rule<class expression_class, expression_value> expression = "expression";

const auto number = +x3::char_("0-9");
const auto quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
const auto value = number | quoted_string;
const auto select_attr_def = +x3::char_("0-9a-zA-Z") % '.';
const auto conditon_def = select_attr >> -('=' >> value);
const auto group_def = '(' >> expression >> ')';
const auto factor_def = conditon | group;
const auto term_def = factor % '&';
const auto expression_def = term % '|';

BOOST_SPIRIT_DEFINE(select_attr, conditon, group, factor, term, expression);

}  // namespace parser

BOOST_FUSION_ADAPT_STRUCT(parser::select_attr_value, v);
BOOST_FUSION_ADAPT_STRUCT(parser::conditon_value, v, s);
BOOST_FUSION_ADAPT_STRUCT(parser::group_value, v);
BOOST_FUSION_ADAPT_STRUCT(parser::factor_value, v);
BOOST_FUSION_ADAPT_STRUCT(parser::term_value, v);
BOOST_FUSION_ADAPT_STRUCT(parser::expression_value, v);

bool validator::test(const packet &pac) const
{
    json j;
    auto layers = pac.get_detail().layers;
    for (auto it = layers.cbegin(); it != layers.cend(); ++it) {
        auto type = (*it)->type();
        j[type] = (*it)->to_json();
    }
    return test(j);
}

p_validator validator::from_str(const std::string &code)
{
    p_validator pv;
    parser::expression_value ast;
    auto it = code.begin();
    x3::phrase_parse(it, code.end(), parser::expression, x3::space, ast);
    return pv;
}
