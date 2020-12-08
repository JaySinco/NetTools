#include "validator.h"
#include <algorithm>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

using p_validator = std::shared_ptr<validator>;

class selector
{
public:
    virtual std::pair<bool, const json *> operator()(const json &j) const = 0;
};

using p_selector = std::shared_ptr<selector>;

bool validator::test(const packet &pac) const
{
    const auto &layers = pac.get_detail().layers;
    return std::any_of(layers.cbegin(), layers.cend(), [&](const std::shared_ptr<protocol> &p) {
        return this->operator()(p->to_json());
    });
}

class select_validator : public validator
{
public:
    select_validator(p_selector psel, p_validator pval) : psel_(psel), pval_(pval) {}

    bool operator()(const json &j) const override
    {
        auto ret = (*psel_)(j);
        if (!ret.first) return false;
        return (*pval_)(*ret.second);
    }

private:
    p_selector psel_;
    p_validator pval_;
};

class and_validator : public validator
{
public:
    and_validator(p_validator left, p_validator right) : left_(left), right_(right) {}

    bool operator()(const json &j) const override { return (*left_)(j) && (*right_)(j); }

private:
    p_validator left_;
    p_validator right_;
};

class or_validator : public validator
{
public:
    or_validator(p_validator left, p_validator right) : left_(left), right_(right) {}

    bool operator()(const json &j) const override { return (*left_)(j) || (*right_)(j); }

private:
    p_validator left_;
    p_validator right_;
};

class string_validator : public validator
{
public:
    string_validator(const std::string &value) : value_(value) {}

    bool operator()(const json &j) const override
    {
        return j.is_string() && j.get<std::string>() == value_;
    }

private:
    std::string value_;
};

class integer_validator : public validator
{
public:
    integer_validator(int value) : value_(value) {}

    bool operator()(const json &j) const override
    {
        return j.is_number() && j.get<int>() == value_;
    }

private:
    int value_;
};

class concat_selector : public selector
{
public:
    concat_selector(p_selector left, p_selector right) : left_(left), right_(right) {}

    std::pair<bool, const json *> operator()(const json &j) const override
    {
        auto ret = (*left_)(j);
        if (!ret.first) return std::make_pair(false, nullptr);
        return (*right_)(*ret.second);
    }

private:
    p_selector left_;
    p_selector right_;
};

class object_selector : public selector
{
public:
    object_selector(const std::string &key) : key_(key) {}

    std::pair<bool, const json *> operator()(const json &j) const override
    {
        if (!j.is_object()) return std::make_pair(false, nullptr);
        if (!j.contains(key_)) return std::make_pair(false, nullptr);
        return std::make_pair(true, &j[key_]);
    }

private:
    std::string key_;
};

class array_selector : public selector
{
public:
    array_selector(int index) : index_(index) {}

    std::pair<bool, const json *> operator()(const json &j) const override
    {
        if (!j.is_array()) return std::make_pair(false, nullptr);
        if (index_ < 0 || index_ >= j.size()) return std::make_pair(false, nullptr);
        return std::make_pair(true, &j[index_]);
    }

private:
    int index_;
};

///////////////////////////////////////////////////////////////////////////
//  grammer definition
///////////////////////////////////////////////////////////////////////////

struct clothes
{
    std::string name;
    int size;
};
BOOST_FUSION_ADAPT_STRUCT(clothes, name, size);

struct tens_type : x3::symbols<unsigned>
{
    tens_type() { add("X", 10)("XX", 20)("XXX", 30)("XL", 40)("L", 50); }
} tens;

auto set_zero = [&](auto &ctx) { _val(ctx) = 0; };
auto add = [&](auto &ctx) { _val(ctx) += _attr(ctx); };

const x3::rule<class parser, clothes> parser = "parser";
const auto quoted_string = x3::lexeme['"' >> +(x3::char_ - '"') >> '"'];
const auto parser_def = x3::lit("clothes") >> '{' >> quoted_string >> ',' >> tens >> '}';
BOOST_SPIRIT_DEFINE(parser);

void parse()
{
    std::string s = R"(clothes{ "test", XL })";
    clothes ast;
    auto it = s.begin();
    x3::phrase_parse(it, s.end(), parser, x3::space, ast);
    LOG(INFO) << fmt::to_string(std::make_tuple(ast.name, ast.size));
}
