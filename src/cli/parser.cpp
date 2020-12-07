#include "prec.h"
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/fusion/include/io.hpp>

namespace x3 = boost::spirit::x3;

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

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    std::string s = R"(clothes{ "test", XL })";
    clothes ast;
    auto it = s.begin();
    x3::phrase_parse(it, s.end(), parser, x3::space, ast);
    LOG(INFO) << fmt::to_string(std::make_tuple(ast.name, ast.size));
}
