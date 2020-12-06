#include "prec.h"
#include <boost/fusion/include/std_tuple.hpp>
#include <boost/spirit/home/x3.hpp>

namespace x3 = boost::spirit::x3;

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    std::string s = "3.45, 123";
    std::tuple<double, double> p;
    auto it = s.begin();
    x3::phrase_parse(it, s.end(),
                     //  Begin grammar
                     x3::double_ >> ',' >> x3::double_,
                     //  End grammar
                     x3::space, p);
    LOG(INFO) << std::get<0>(p) << " " << std::get<1>(p);
}
