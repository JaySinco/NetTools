#include "prec.h"
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;

struct tens_type : x3::symbols<unsigned>
{
    tens_type() { add("X", 10)("XX", 20)("XXX", 30)("XL", 40)("L", 50); }
} tens;

auto set_zero = [&](auto &ctx) { _val(ctx) = 0; };
auto add = [&](auto &ctx) { _val(ctx) += _attr(ctx); };

const x3::rule<class parser, double> parser = "parser";
const auto parser_def = x3::eps[set_zero] >> tens[add] % ',';
BOOST_SPIRIT_DEFINE(parser);

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    std::string s = "X, XX, L";
    double sum = 0;
    auto it = s.begin();
    x3::phrase_parse(it, s.end(), parser, x3::space, sum);
    LOG(INFO) << sum;
}
