#include "Arguments.h"

using namespace Moya::System::Arguments;

int main(int argc, const char **argv)
{
    Syntax syntax("My simple command-line tool", "1.0.0.1");

    syntax.add(Command("ignored", "ignored", "ignored", "General information on my application."));
    syntax.add(Parameter("first-param", "-a", "This is a required parameter with associated value.", true, true));
    syntax.add(Parameter("second-param", "-b", "This is an optional parameter without associated value.", false, false));

    Arguments arguments(argc, argv);
    Parser parser(syntax, arguments);
    if (!parser.parse()) {
        Help help(syntax, parser, arguments);
        return help.run();
    }

    std::cout << "Application started normally." << std::endl;
    std::cout << "Following arguments were parsed:" << std::endl;

    for (auto item : parser)
        std::cout << "   '" << item.first << "' = '" << item.second << "'" << std::endl;

    return 0;
}
