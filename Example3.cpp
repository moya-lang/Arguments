#include "Arguments.h"

using namespace Moya::System::Arguments;

int main(int argc, const char **argv)
{
    Syntax syntax("Loro device programmer", "2.1.32.7");

    syntax.add(Command("list", "list", "Lists all Loro devices.",
        "Command lists system names of all connected Loro devices."));

    syntax.add(Command("reset", "reset", "Resets device.",
        "Command resets Loro device."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));

    syntax.add(Command("program", "program", "Programs device with specified file.",
        "Command programs Loro device with specified program file."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));
    syntax.add(Parameter("program-file-path", "-p", "Program file path.", true, true));

    syntax.add(Command("backup", "backup", "Downloads device program into local file for backup.",
        "Command reads Loro device program and stores it in local file."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));
    syntax.add(Parameter("program-file-path", "-p", "Program file path.", true, true));

    syntax.add(Command("erase", "erase", "Erases device.",
        "Command erases program from Loro device."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));

    syntax.add(Command("secure", "secure", "Secures device.",
        "Command secures Loro device. Once the device is secured its "
        "program cannot be read or updated even by external programmer. "
        "To exit secured mode the device need to be reset to factory "
        "settings using special electrical technique."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));
    syntax.add(Parameter("force", "-f", "Do not prompt.", false, false));

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
