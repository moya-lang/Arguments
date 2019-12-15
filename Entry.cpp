#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

typedef std::string Note;

class Parameter
{
    public:

        const std::string identifier;
        const std::string shortName;
        const std::string fullName;
        const std::string brief;

        std::string note;

        const bool isRequired;
        const size_t numberOfValues;
        std::vector<const char *> values;

        Parameter(std::string identifier, std::string shortName, std::string fullName,
            size_t numberOfValues, std::string brief, bool isRequired) :
            identifier(identifier), shortName(shortName), fullName(fullName), brief(brief),
            isRequired(isRequired), numberOfValues(numberOfValues), values(numberOfValues, nullptr)
        {
        }
};

class Command
{
    public:

        const std::string identifier;
        const std::string shortName;
        const std::string fullName;
        const std::string brief;

        std::string note;

        std::list<Parameter> parameters;

        Command(std::string identifier, std::string shortName, std::string fullName, std::string brief) :
            identifier(identifier), shortName(shortName), fullName(fullName), brief(brief)
        {
        }
};

class Syntax
{
    public:

        const std::string programName;
        const std::string programVersion;

        std::list<Command> commands;

        Syntax(std::string programName, std::string version) :
            programName(programName), programVersion(programVersion)
        {
        }

        void add(Command command)
        {
            commands.push_back(command);
        }

        void add(Parameter parameter)
        {
            if (!commands.empty())
                commands.back().parameters.push_back(parameter);
        }

        void add(Note note)
        {
            if (commands.empty())
                return;

            Command &command = commands.back();
            if (command.parameters.empty())
                command.note += note;
            else
                command.parameters.back().note += note;
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

class Parser
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

            const Command *command = findCommandByName(commandName);
            return command ? parseParameters(*command) : false;
        }

        const char *first()
        {
            iterator = map.begin();
            return next();
        }

        const char *next()
        {
            return iterator != map.end() ? (iterator++)->first.c_str() : nullptr;
        }

        const char *operator[](const char *name) const
        {
            const Map::const_iterator iterator = map.find(name);
            return iterator != map.end() ? iterator->second.c_str() : nullptr;
        }

    private:

        const Syntax &syntax;
        const Arguments &arguments;

        typedef std::map<std::string, std::string> Map;
        Map::iterator iterator;
        Map map;

        void parseVersionCommand()
        {
            if (arguments.getNumberOfArguments() == 2)
                map["/"] = "version";
        }

        void parseHelpCommand()
        {
            const size_t numberOfArguments = arguments.getNumberOfArguments();
            if (numberOfArguments == 2)
                map["/"] = "help";

            if (numberOfArguments != 3)
                return;

            const char *commandName = arguments[2];
            const Command *command = findCommandByName(commandName);
            if (command || isVersionCommandName(commandName)) {
                map["/"] = "help";
                map["/command"] = command ? command->identifier : "version";
            }
        }

        bool parseParameters(const Command &command)
        {
            map["/"] = command.identifier;

            const size_t numberOfArguments = arguments.getNumberOfArguments();
            size_t argumentIndex = syntax.commands.size() == 1 ? 1 : 2;
            while (argumentIndex < numberOfArguments) {

                const Parameter *parameter = findParameterByName(command, arguments[argumentIndex++]);
                if (!parameter || argumentIndex + parameter->numberOfValues > numberOfArguments)
                    return false;

                std::string singleValue = (parameter->numberOfValues == 1) ? arguments[argumentIndex++] : "";
                if (!map.insert({ "/" + parameter->identifier, singleValue }).second)
                    return false;

                if (parameter->numberOfValues != 1)
                    for (size_t valueIndex = 0; valueIndex < parameter->numberOfValues; valueIndex++)
                        map["/" + parameter->identifier + "/" + std::to_string(valueIndex)] = arguments[argumentIndex++];
            }

            return areAllRequiredParametersSpecified(command);
        }

        bool areAllRequiredParametersSpecified(const Command &command) const
        {
            for (const Parameter &parameter : command.parameters)
                if (parameter.isRequired && map.find("/" + parameter.identifier) == map.end())
                    return false;

            return true;
        }

        const Parameter *findParameterByName(const Command &command, const char *parameterName) const
        {
            for (const Parameter &parameter : command.parameters)
                if (parameter.fullName == parameterName || parameter.shortName == parameterName)
                    return &parameter;

            return nullptr;
        }

        const Command *findCommandByName(const char *commandName) const
        {
            for (const Command &command : syntax.commands)
                if (command.fullName == commandName || command.shortName == commandName)
                    return &command;

            return nullptr;
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

        Help(const Syntax &syntax, Parser &parser) :
            syntax(syntax), parser(parser)
        {
        }

        int run()
        {
            for (const char *name = parser.first(); name; name = parser.next())
                std::cout << "'" << name << "' = '" << parser[name] << "'" << std::endl;

            std::cout << "Help-Done." << std::endl;
            return 0; // When to return 0 and when -1?
        }

    private:

        const Syntax &syntax;
        Parser &parser;
};

int main(int argc, const char **argv)
{
    static const char *deviceNameInfo = "Name of a device. Argument can be ignored when only one device is connected.";

    Syntax syntax("Loro command-line tool", "1.0");

    syntax.add(Command("list", "", "list", "Lists all Loro devices."));
    syntax.add(Note("Command lists system names of all connected Loro devices."));

    syntax.add(Command("reset", "", "reset", "Resets device."));
    syntax.add(Note("Command resets Loro device."));
    syntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));

    syntax.add(Command("program", "", "program", "Programs device with specified file."));
    syntax.add(Note("Command programs Loro device with specified program file."));
    syntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    syntax.add(Parameter("program-file-path", "-p", "", 1, "Program file path.", true));

    syntax.add(Command("backup", "", "backup", "Downloads device program into local file for backup."));
    syntax.add(Note("Command reads Loro device program and stores it in local file."));
    syntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    syntax.add(Parameter("program-file-path", "-p", "", 1, "Program file path.", true));

    syntax.add(Command("erase", "", "erase", "Erases device."));
    syntax.add(Note("Command erases program from Loro device."));
    syntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));

    syntax.add(Command("secure", "", "secure", "Secures device."));
    syntax.add(Note("Command secures Loro device. Once the device is secured its "));
    syntax.add(Note("program cannot be read or updated even using external programmer. "));
    syntax.add(Note("To exit secured mode the device need to reset to factory settings "));
    syntax.add(Note("using special electrical technique."));
    syntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    syntax.add(Parameter("force", "-f", "", 0, "Executes command without waiting for user interaction.", false));

    Parser parser(syntax, Arguments(argc, argv));

    if (!parser.parse()) {
        Help help(syntax, parser);
        return help.run();
    }

    for (const char *name = parser.first(); name; name = parser.next())
        std::cout << "'" << name << "' = '" << parser[name] << "'" << std::endl;

    std::cout << "Program - Done." << std::endl;
    return 0;
}
