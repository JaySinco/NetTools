#include "validator.h"
#include <regex>
#include <boost/variant.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

using p_validator = std::shared_ptr<validator>;

class selector : validator
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

class select_validator : public validator
{
public:
    select_validator(std::shared_ptr<selector> psel, p_validator pval) : psel_(psel), pval_(pval) {}

    bool test(const json &j) const override
    {
        const json *out = nullptr;
        auto ok = psel_->select(j, out);
        if (!ok) return false;
        return pval_->test(*out);
    }

private:
    std::shared_ptr<selector> psel_;
    p_validator pval_;
};

class value_validator : public validator
{
public:
    value_validator(int i) : value_(i) {}

    value_validator(const std::string &s) : value_(s) {}

    bool test(const json &j) const override
    {
        if (const int *pi = boost::get<int>(&value_)) {
            return j.is_number() && j.get<int>() == *pi;

        } else if (const std::string *ps = boost::get<std::string>(&value_)) {
            return j.is_string() && j.get<std::string>() == *ps;
        } else {
            return false;
        }
    }

private:
    boost::variant<int, std::string> value_;
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

namespace parser
{
const x3::rule<class value, p_validator> value = "value";
const x3::rule<class selector, p_validator> selector = "selector";
const x3::rule<class conditon, p_validator> conditon = "conditon";
const x3::rule<class group, p_validator> group = "group";
const x3::rule<class factor, p_validator> factor = "factor";
const x3::rule<class term, p_validator> term = "term";
const x3::rule<class expression, p_validator> expression = "expression";

const auto number = +x3::char_("0-9");
const auto quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
const auto value_def = number | quoted_string;
const auto selector_def = '.' >> x3::char_("0-9a-zA-Z");
const auto conditon_def = +selector >> -('=' >> value);
const auto group_def = '(' >> expression >> ')';
const auto factor_def = conditon | group;
const auto term_def = factor % '&';
const auto expression_def = term % '|';

BOOST_SPIRIT_DEFINE(value, selector, conditon, group, factor, term, expression);

}  // namespace parser

p_validator validator::from_str(const std::string &code)
{
    p_validator pv;
    auto it = code.begin();
    x3::phrase_parse(it, code.end(), parser::expression, x3::space, pv);
    return pv;
}
