#include <algorithm>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <list>
#include <map>

class Parameter
{
    public:

        const std::string identifier;
        const std::string name;
        const std::string remarks;

        const bool isRequired;
        const bool hasValue;

        Parameter(std::string identifier, std::string name, std::string remarks, bool isRequired, bool hasValue) :
            identifier(identifier), name(name), remarks(remarks), isRequired(isRequired), hasValue(hasValue)
        {
        }
};

class Command
{
    public:

        const std::string identifier;
        const std::string name;
        const std::string brief;
        const std::string remarks;

        std::list<Parameter> parameters;

        Command(std::string identifier, std::string name, std::string brief, std::string remarks) :
            identifier(identifier), name(name), brief(brief), remarks(remarks)
        {
        }
};

class Syntax
{
    public:

        const std::string programName;
        const std::string programVersion;

        std::list<Command> commands;

        Syntax(std::string programName, std::string programVersion) :
            programName(programName), programVersion(programVersion)
        {
        }

        void add(const Command command)
        {
            commands.push_back(command);
        }

        void add(const Parameter parameter)
        {
            if (!commands.empty())
                commands.back().parameters.push_back(parameter);
        }

        const Command *findCommandByName(const char *name) const
        {
            for (const Command &command : commands)
                if (command.name == name)
                    return &command;

            return nullptr;
        }

        const Parameter *findParameterByName(const Command &command, const char *name) const
        {
            for (const Parameter &parameter : command.parameters)
                if (parameter.name == name)
                    return &parameter;

            return nullptr;
        }
};

class Arguments
{
    public:

        Arguments(int argc, const char **argv) :
            numberOfArguments(argc), arguments(argv)
        {
        }

        size_t getNumberOfArguments() const
        {
            return numberOfArguments;
        }

        const char *operator[](size_t index) const
        {
            return index < numberOfArguments ? arguments[index] : nullptr;
        }

    private:

        const size_t numberOfArguments;
        const char **arguments;
};

class Parser : public std::map<std::string, std::string>
{
    public:

        Parser(const Syntax &syntax, const Arguments &arguments) :
            syntax(syntax), arguments(arguments)
        {
        }

        bool parse()
        {
            const size_t numberOfCommands = syntax.commands.size();
            const char *commandName = arguments[1];

            if (!commandName && numberOfCommands != 1)
                return numberOfCommands == 0;

            if (commandName && isVersionCommandName(commandName)) {
                parseVersionCommand();
                return false;
            }

            if (commandName && isHelpCommandName(commandName)) {
                parseHelpCommand();
                return false;
            }

            if (numberOfCommands == 1)
                return parseParameters(syntax.commands.front());

            const Command *command = syntax.findCommandByName(commandName);
            return command ? parseParameters(*command) : false;
        }

    private:

        const Syntax &syntax;
        const Arguments &arguments;

        void parseVersionCommand()
        {
            if (arguments.getNumberOfArguments() == 2)
                insert({ "/", "version" });
        }

        void parseHelpCommand()
        {
            const size_t numberOfArguments = arguments.getNumberOfArguments();
            if (numberOfArguments == 2)
                insert({ "/", "help" });

            if (numberOfArguments != 3 || syntax.commands.size() < 2)
                return;

            const char *commandName = arguments[2];
            const Command *command = syntax.findCommandByName(commandName);
            if (command || isVersionCommandName(commandName)) {
                insert({ "/", "help" });
                insert({ "/command", command ? command->identifier : "version" });
            }
        }

        bool parseParameters(const Command &command)
        {
            insert({ "/", command.identifier });

            const size_t numberOfArguments = arguments.getNumberOfArguments();
            size_t argumentIndex = syntax.commands.size() == 1 ? 1 : 2;
            while (argumentIndex < numberOfArguments) {

                const Parameter *parameter = syntax.findParameterByName(command, arguments[argumentIndex++]);
                if (!parameter || argumentIndex + !!parameter->hasValue > numberOfArguments)
                    return false;

                std::string value = parameter->hasValue ? arguments[argumentIndex++] : "";
                if (!insert({ "/" + parameter->identifier, value }).second)
                    return false;
            }

            return areAllRequiredParametersSpecified(command);
        }

        bool areAllRequiredParametersSpecified(const Command &command) const
        {
            for (const Parameter &parameter : command.parameters)
                if (parameter.isRequired && find("/" + parameter.identifier) == end())
                    return false;

            return true;
        }

        bool isHelpCommandName(const char *commandName) const
        {
            return !strcmp(commandName, "help") || !strcmp(commandName, "--help") || !strcmp(commandName, "-h");
        }

        bool isVersionCommandName(const char *commandName) const
        {
            return !strcmp(commandName, "version") || !strcmp(commandName, "--version") || !strcmp(commandName, "-v");
        }
};

class Help
{
    public:

        Help(const Syntax &syntax, const Parser &parser, const Arguments &arguments) :
            syntax(syntax), parser(parser), arguments(arguments)
        {
        }

        int run() const
        {
            const auto commandIdentifier = parser.find("/");

            bool explicitHelp = false;
            if (commandIdentifier != parser.end()) {
                explicitHelp = (commandIdentifier->second == "help");

                if (commandIdentifier->second == "version") {
                    std::cout << syntax.programVersion << std::endl;
                    return 0;
                }
            }

            const auto commandHelp = parser.find("/command");
            if (commandHelp != parser.end()) {
                if (commandHelp->second == "version")
                    printGenericHelp();
                else
                    printCommandHelp(commandHelp->second);
            }
            else
                printGenericHelp();

            const bool implicitHelp = arguments.getNumberOfArguments() <= 1;
            return (explicitHelp || implicitHelp) ? 0 : -1;
        }

    private:

        const Syntax &syntax;
        const Parser &parser;
        const Arguments &arguments;

        static const size_t margin = 3;
        static const size_t maxLineLength = 97;

        void printGenericHelp() const
        {
            size_t maxCommandNameLength = 0;
            for (const Command &command : syntax.commands)
                maxCommandNameLength = std::max(maxCommandNameLength, command.name.length());

            const char *executableName = getApplicationName();
            print(margin, "", syntax.programName + ", version: " + syntax.programVersion);
            
            const size_t numberOfCommands = syntax.commands.size();
            if (numberOfCommands < 1) {
                print(margin, "", std::string("Usage: ") + executableName + " [--version] [--help]");
                return;
            }
            
            if (numberOfCommands == 1) {
                printCommandHelp(syntax.commands.front().identifier);
                return;
            }

            print(margin, "", std::string("Usage: ") + executableName + " [--version] [--help] <command> [<args>]");

            std::cout << std::endl << "Commands:" << std::endl;

            for (const Command &command : syntax.commands)
                if (!command.name.empty())
                    print(margin + maxCommandNameLength + margin, std::string(margin, ' ') +
                        command.name + std::string(maxCommandNameLength + margin - command.name.length(), ' '),
                        command.brief);

            std::cout << std::endl;

            print(margin, "", std::string("See '") + executableName + " help <command>' to read about specific command.");
        }

        void printCommandHelp(const std::string &commandName) const
        {
            const Command *command = syntax.findCommandByName(commandName.c_str());
            if (!command)
                return;

            print(margin, "", syntax.programName + ", version: " + syntax.programVersion);

            size_t maxParameterNameLength = 0;
            const char *executableName = getApplicationName();
            std::string usage = std::string("Usage: ") + executableName + " " + command->name;
            for (const Parameter &parameter : command->parameters) {
                maxParameterNameLength = std::max(maxParameterNameLength, parameter.name.length() +
                    parameter.hasValue ? parameter.identifier.length() + 3 : 0);

                std::string paramX = parameter.isRequired ? parameter.identifier : "<" + parameter.identifier + ">";
                std::string paramInfo = parameter.hasValue ? parameter.name + " " + paramX : parameter.name;
                usage += parameter.isRequired ? " <" + paramInfo + ">" : " [" + paramInfo + "]";
            }

            print(margin, "", usage);
            std::cout << std::endl;
            print(margin, "", command->remarks);

            if (command->parameters.empty())
                return;

            std::cout << std::endl << "Arguments:" << std::endl;

            for (const Parameter &parameter : command->parameters)
                if (!parameter.name.empty()) {
                    std::string nnn = parameter.name;
                    if (parameter.hasValue)
                        nnn += " <" + parameter.identifier + ">";

                    print(margin + maxParameterNameLength + margin, std::string(margin, ' ') +
                        nnn + std::string(maxParameterNameLength + margin - nnn.length(), ' '),
                        parameter.remarks);
                }
        }

        const char *getApplicationName() const
        {
            const char *applicationName = strrchr(arguments[0], '/');
            if (applicationName)
                return applicationName + 1;

            applicationName = strrchr(arguments[0], '\\');
            if (applicationName)
                return applicationName + 1;

            return arguments[0];
        }

        void print(size_t newLineIndent, std::string intro, std::string message) const
        {
            std::string singleLine, singleWord;
            singleLine.reserve(message.size() + 1);
            singleWord.reserve(message.size() + 1);

            if (intro.length() < maxLineLength)
                singleWord = intro;
            else
                std::cout << intro << std::endl;

            bool hasLineStarted = false;
            size_t length = message.length();
            for (size_t i = 0; i <= length; i++) {
                char character = message[i];

                bool isWhitespace = isspace(character) || !character;
                if (!isWhitespace) {
                    singleWord += character;
                    continue;
                }

                if (hasLineStarted && singleLine.length() + singleWord.length() + 1 > maxLineLength) {
                    std::cout << singleLine << std::endl;
                    singleLine.assign(newLineIndent, ' ');
                    hasLineStarted = false;
                }

                if (!singleWord.empty()) {
                    if (hasLineStarted)
                        singleLine += " ";

                    hasLineStarted = true;
                    singleLine += singleWord;
                    singleWord = "";
                }
            }

            if (hasLineStarted)
                std::cout << singleLine << std::endl;
        }
};

int main(int argc, const char **argv)
{
    static const char *deviceNameInfo = "Name of a device. "
        "Argument can be ignored when only one device is connected.";

    Syntax syntax("Loro command-line tool", "1.0");

    syntax.add(Command("list", "list", "Lists all Loro devices.",
        "Command lists system names of all connected Loro devices."));

    syntax.add(Command("reset", "reset", "Resets device.",
        "Command resets Loro device."));
    syntax.add(Parameter("device-name", "-d", deviceNameInfo, false, true));

    syntax.add(Command("program", "program", "Programs device with specified file.",
        "Command programs Loro device with specified program file."));
    syntax.add(Parameter("device-name", "-d", deviceNameInfo, false, true));
    syntax.add(Parameter("program-file-path", "-p", "Program file path.", true, true));

    syntax.add(Command("backup", "backup", "Downloads device program into local file for backup.",
        "Command reads Loro device program and stores it in local file."));
    syntax.add(Parameter("device-name", "-d", deviceNameInfo, false, true));
    syntax.add(Parameter("program-file-path", "-p", "Program file path.", true, true));

    syntax.add(Command("erase", "erase", "Erases device.",
        "Command erases program from Loro device."));
    syntax.add(Parameter("device-name", "-d", deviceNameInfo, false, true));

    syntax.add(Command("secure", "secure", "Secures device.",
        "Command secures Loro device. Once the device is secured its "
        "program cannot be read or updated even by external programmer. "
        "To exit secured mode the device need to be reset to factory "
        "settings using special electrical technique."));
    syntax.add(Parameter("device-name", "-d", deviceNameInfo, false, true));
    syntax.add(Parameter("force", "-f", "Do not prompt user.", false, false));

    Arguments arguments(argc, argv);
    Parser parser(syntax, arguments);
    if (!parser.parse()) {
        Help help(syntax, parser, arguments);
        std::cout << "HELP RER: " << help.run() << std::endl;
        return 0;
    }


    for (auto item : parser)
        std::cout << "'" << item.first << "' = '" << item.second << "'" << std::endl;

    std::cout << "Program - Done." << std::endl;
    return 0;
}
