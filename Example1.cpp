#include "Arguments.h"

using namespace Moya::System::Arguments;

void parseArguments(int argc, const char **argv)
{
    Syntax syntax("My simple command-line tool", "1.0.0.1");
    Arguments arguments(argc, argv);
    Parser parser(syntax, arguments);
    if (parser.parse())
        return;

    Help help(syntax, parser, arguments);
    exit(help.run());
}

int main(int argc, const char **argv)
{
    parseArguments(argc, argv);

    std::cout << "Application started normally..." << std::endl;
    return 0;
}
