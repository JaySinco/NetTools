#include "prec.h"
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    std::string s = "(3.45, 123)";
    std::pair<double, double> p;
    auto it = s.begin();
    qi::phrase_parse(it, s.end(), '(' >> qi::double_ >> ',' >> qi::double_ >> ')', qi::space, p);
    LOG(INFO) << p.first << "," << p.second;
}
