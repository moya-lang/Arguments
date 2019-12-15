#include <iostream>
#include <string>
#include <vector>
#include <list>

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
        bool isSpecified = false;

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
                command.note = note;
            else
                command.parameters.back().note = note;
        }
};

class Arguments
{
    public:

        Arguments(int argc, const char **argv) :
            numberOfArguments(argc), arguments(argv)
        {
        }

        unsigned getNumberOfArguments() const
        {
            return numberOfArguments;
        }

        const char *operator[](unsigned index) const
        {
            return index < numberOfArguments ? arguments[index] : nullptr;
        }

    private:

        const unsigned numberOfArguments;
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
            // ...
        }

    private:

        const Syntax &syntax;
        const Arguments &arguments;
};

class Help
{

};

int main(int argc, const char **argv)
{
    static const char *deviceNameInfo = "Name of a device. Argument can be ignored when only one device is connected.";

    Syntax argumentsSyntax("Loro command-line tool", "1.0");

    argumentsSyntax.add(Command("list", "", "list", "Lists all Loro devices."));
    argumentsSyntax.add(Note("Command lists names of all connected Loro devices."));

    argumentsSyntax.add(Command("reset", "", "reset", "Resets device."));
    argumentsSyntax.add(Note("Command resets Loro device."));
    argumentsSyntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));

    argumentsSyntax.add(Command("program", "", "program", "Programs device with specified file."));
    argumentsSyntax.add(Note("Command programs Loro device with specified program file."));
    argumentsSyntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    argumentsSyntax.add(Parameter("program-file-path", "-p", "", 1, "Program file path.", false));

    argumentsSyntax.add(Command("backup", "", "backup", "Downloads device program into local file for backup."));
    argumentsSyntax.add(Note("Command reads Loro device program and stores it in local file."));
    argumentsSyntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    argumentsSyntax.add(Parameter("program-file-path", "-p", "", 1, "Program file path.", false));

    argumentsSyntax.add(Command("erase", "", "erase", "Erases device."));
    argumentsSyntax.add(Note("Command erases program from Loro device."));
    argumentsSyntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));

    argumentsSyntax.add(Command("erase", "", "secure", "Secures device."));
    argumentsSyntax.add(Note("Command secures Loro device. Once the device is secured its"));
    argumentsSyntax.add(Note("program cannot be read or updated even using external programmer."));
    argumentsSyntax.add(Note("To exit secured mode the device need to reset to factory settings"));
    argumentsSyntax.add(Note("using special electrical technique."));
    argumentsSyntax.add(Parameter("device-name", "-d", "", 1, deviceNameInfo, false));
    argumentsSyntax.add(Parameter("force", "-f", "", 0, "Executes command without waiting for user interaction.", false));

    const Arguments argumentsList(argc, argv);

    Parser argumentsParser(argumentsSyntax, argumentsList);

    bool success = argumentsParser.parse();
    if (success) {

        /*const char *name = argumentsParser.first();
        do std::cout << name << " = '" << argumentsParser[name] << "'" << std::endl;
        while (name = argumentsParser.next());

        argumentsParser["/"]; // specified command
        argumentsParser["/backup/device-name"]; // argument -d of backup command*/
    }

    return 0;
}
