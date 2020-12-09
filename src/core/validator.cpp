#include "validator.h"
#include <regex>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
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
//  grammer definition
///////////////////////////////////////////////////////////////////////////

namespace parser
{
auto value_act = [](auto &ctx) {
    std::string *s = boost::get<std::string>(&_attr(ctx));
    _val(ctx) = std::make_shared<value_validator>(*s);
};

auto c2s = [](auto &ctx) { _val(ctx) = std::string(_attr(ctx).begin(), _attr(ctx).end()); };

auto select_act = [](auto &ctx) { _val(ctx) = std::make_shared<selector>(_attr(ctx)); };

auto condition_act = [](auto &ctx) {
    p_selector first = boost::fusion::at_c<0>(_attr(ctx));
    boost::optional<p_validator> second = boost::fusion::at_c<1>(_attr(ctx));
    if (second) {
        _val(ctx) = std::make_shared<select_validator>(first, *second);
    } else {
        _val(ctx) = first;
    }
};

auto term_act = [](auto &ctx) {
    p_validator ps_combine = _attr(ctx)[0];
    for (int i = 1; i < _attr(ctx).size(); ++i) {
        ps_combine = std::make_shared<and_validator>(ps_combine, _attr(ctx)[i]);
    }
    _val(ctx) = ps_combine;
};

auto expression_act = [](auto &ctx) {
    p_validator ps_combine = _attr(ctx)[0];
    for (int i = 1; i < _attr(ctx).size(); ++i) {
        ps_combine = std::make_shared<or_validator>(ps_combine, _attr(ctx)[i]);
    }
    _val(ctx) = ps_combine;
};

const x3::rule<class number_class, std::string> number = "number";
const x3::rule<class quoted_string_class, std::string> quoted_string = "quoted_string";
const x3::rule<class value_class, p_validator> value = "value";
const x3::rule<class attribute_class, std::string> attribute = "attribute";
const x3::rule<class select_class, p_selector> select = "select";
const x3::rule<class conditon_class, p_validator> conditon = "conditon";
const x3::rule<class group_class, p_validator> group = "group";
const x3::rule<class factor_class, p_validator> factor = "factor";
const x3::rule<class term_class, p_validator> term = "term";
const x3::rule<class expression_class, p_validator> expression = "expression";

const auto number_def = (+x3::char_("0-9"))[c2s];
const auto quoted_string_def = (x3::lexeme['"' >> +(x3::char_ - '"') >> '"'])[c2s];
const auto value_def = (number | quoted_string)[value_act];
const auto attribute_def = (+x3::char_("0-9a-zA-Z"))[c2s];
const auto select_def = (attribute % '.')[select_act];
const auto conditon_def = (select >> -('=' >> value))[condition_act];
const auto group_def = '(' >> expression >> ')';
const auto factor_def = conditon | group;
const auto term_def = (factor % '&')[term_act];
const auto expression_def = (term % '|')[expression_act];

BOOST_SPIRIT_DEFINE(number, quoted_string, value, attribute, select, conditon, group, factor, term,
                    expression);

}  // namespace parser

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
    auto it = code.begin();
    x3::phrase_parse(it, code.end(), parser::expression, x3::space, pv);
    return pv;
}
