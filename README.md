# Arguments

Command-line interface, argument parser, help builder for C++.


## Foreword

The most repetitive part of creating a command-line applications is an argument parser as well as help and version
print. Presented solution is therefore a skeleton for command-line application that provides a handy API offering an
efficient mechanism for parsing command-line arguments and printing a nice looking help. Intention is to create a
single-file header-only C++-based library that simplifies command-line application development time for **Linux** as
well as for **Windows** OSes.


## Usage

Before reading further information please get to know with the following naming convention used in the document:
* Argument - One of command-line argument passed into C++ `main()` function through `argv` array,
* Command - An argument identifying a main action to be executed by application. Multiple different commands can be
  supported by application, but only one command can be specified in arguments via `argv[1]` during process start-up,
* Parameter - Sub-arguments of a command. Each command can have zero or more parameters that are either required or
  optional,
* Value - Additional value of any type following the parameter name that is associated with that parameter. There are
  two types of parameters: one that are informative and have no associated value and another that have one.

Depending on a complexity of created application it should be decided to chose one of three available options:
* First is a very simple application that provides only two commands: one for help and one for version print. Specifying
  no commands in command-line runs the program.
* Second is an application that have a single (default) command with multiple parameters. Name of the default command
  should be never specified in this case. Application can be executed with either `help` or `version` commands normally.
  To run application default command, all of required parameters of the command shall be specified. Missing any of
  required parameters will cause application to print the help. Therefore, if all the parameters are optional specifying
  no commands in command-line runs the program.
* Third option is dedicated for the most advanced applications that supports multiple commands, each with different set
  of parameters. To run specific command its name must be specified. Specifying no command is equivalent to executing
  the application with `help` command specified. The `help` command can be specified with additional parameter which is
  a name of other of available commands. It prints more detailed help for specified command. The `version` command is
  handled normally. All of required parameters of the executed command shall be specified. Missing any of required
  parameters will cause application to print the help.

Above options are chosen automatically depending on the number of defined commands. No commands provided selects first
option, single command selects second option and two or more commands selects third option.


### Creating a skeleton for an application

A typical skeleton for an application using presented parser has been presented the below example:

    #include "Arguments.h"

    using namespace Moya::System::Arguments;

    int main(int argc, const char **argv)
    {
        Syntax syntax("Command-line tool name", "1.0");

        // Definition of commands.
        // ...

        Arguments arguments(argc, argv);
        Parser parser(syntax, arguments);

        if (!parser.parse()) {
            Help help(syntax, parser, arguments);
            return help.run();
        }

        // Program code
        // ...

        return 0;
    }

In the first line of `main()` function body it is created an object of type `Syntax`. This object provides a set of
definitions for commands and its parameters as well as name of the application and its version. Section with commands
definitions has been omitted here as it is described in following chapter. Object of `Argument` class is nothing more
but a wrapper for `argc` and `argv` arguments of the `main()` function. The actual parser is provided in `Parser` class
that parses arguments provided via `arguments` object based on commands defined in `syntax` object. Arguments parsing is
started when a `parse()` function is called on `parser` object. The function returns `true` when all specified arguments
are valid and the application should run normally. Otherwise, `false` indicates that either version or help should be
printed and for this case an object of `Help` class shall be used which is a printing an elegant help based definitions
provided via `syntax` object and actual arguments. The help builder runs also when invalid arguments are specified. For
all incorrect arguments the `run()` function returns non-zero error code and zero otherwise. The return code should be
returned by main() function as in the example above. As an output the parser creates a string-string association for
efficient handling it in application. For more information please refer to: Interpreting parsed arguments chapter.


### Defining a set of commands and its parameters

The library provides very handy interface for defining commands
and its parameters. Example definition of a single command may
look as follows:

    syntax.add(Command("reset", "reset", "Resets device.", "Command resets specified device."));
    syntax.add(Parameter("device-name", "-d", "Name of a device.", false, true));
    syntax.add(Parameter("force", "-f", "Do not prompt.", false, false));

The example defines single command in first line of code that is followed with its parameters definitions. the `syntax`
object is the same object as used in the skeleton application in previous chapter. Multiple commands can be defined
using above pattern, where each command need to be followed by its parameters.

The Command class takes following arguments:
* Identifier that is used in run-time to identify parsed command,
* Name of the command that is used in command-line arguments,
* Brief command description used in list of commands,
* Detailed command description used in command specific help print (can be empty string).

The Parameter class takes following arguments:
* Identifier that is used in run-time to identify parsed parameter,
* Name of the parameter that is used in command-line arguments,
* Detailed parameter description used in list of commands,
* Specifies if the parameter is required (`true`) or optional (`false`),
* Specifies if the parameter have associated value (`true`) or not (`false`).


### Interpreting parsed arguments

After successful run i.e. the `parse()` function described above returned with `true`, the process can continue
normally. In this case the process interprets arguments parsed by argument parser. The `Parser` class derives from
`std::map<std::string, std::string>` and as the result of successful run the map contains a key-value associations in
form of strings ready for interpretation. Following keys can be found in the map:

* `/` - Value for this key contains identifier of the specified command,
* `/parameter-identifier` - Whenever a parameter is specified the key refers to the corresponding parameter identifier.
  Value for this key contains parameter value passed via command-line arguments or an empty string when parameter was
  defined with no associated value.


### Example 1: The simplest application

The simplest application have no special commands. It runs regularly and provides only regular commands like version and
help for usage and application name information. The code is presented below:

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


Regular program call:

    ~/Arguments$ ./example1
    Application started normally...


Default application help:

    ~/Arguments$ ./example1 --help
    My simple command-line tool, version: 1.0.0.1
    Usage: example1 [--version] [--help]


Application version:

    ~/Arguments$ ./example1 --version
    1.0.0.1


### Example 2: An application with a single command

An application that takes one command is a very common scenario in most of small applications.

    #include "Arguments.h"

    using namespace Moya::System::Arguments;

    int main(int argc, const char **argv)
    {
        Syntax syntax("My simple command-line tool", "1.0.0.1");

        syntax.add(Command("ignored", "ignored", "ignored", "General comment to my application."));
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


Output for missing value of required parameter:

    ~/Arguments$ ./example2 -a
    My simple command-line tool, version: 1.0.0.1
    Usage: example2 <-a first-param> [-b]

    General information on my application.

    Command parameters:
      -a <first-param>   This is a required parameter with associated value.
      -b                 This is an optional parameter without associated value.


Regular call with all required parameters specified:

    ~/Arguments$ ./example2 -a 65
    Application started normally.
    Following arguments were parsed:
      '/first-param' = '65'


Missing required parameter:

    ~/Arguments$ ./example2 -b
    My simple command-line tool, version: 1.0.0.1
    Usage: example2 <-a first-param> [-b]

    General information on my application.

    Command parameters:
      -a <first-param>   This is a required parameter with associated value.
      -b                 This is an optional parameter without associated value.


Regular call with all parameters specified:

    ~/Arguments$ ./example2 -b -a 4
    Application started normally.
    Following arguments were parsed:
      '/first-param' = '4'
      '/second-param' = ''


### Example 3: Advanced applications that supports multiple commands

More advanced applications like device programmers may take multiple commands. Each command may also take multiple
parameters depending on purpose of the command. Below example show command definitions for such programmer:

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


Running with no argument is similar to running with `help` command specified:

    ~/Arguments$ ./loro
    Loro device programmer, version: 2.1.32.7
    Usage: loro [--version] [--help] <command> [<args>]

    Commands:
      list      Lists all Loro devices.
      reset     Resets device.
      program   Programs device with specified file.
      backup    Downloads device program into local file for backup.
      erase     Erases device.
      secure    Secures device.

    See 'loro help <command>' to read about specific command.


Printing extended help for one of commands looks as follows:

    ~/Arguments$ ./loro help secure
    Loro device programmer, version: 2.1.32.7
    Usage: loro secure [-d <device-name>] [-f]

    Command secures Loro device. Once the device is secured its program cannot be read or updated
      even by external programmer. To exit secured mode the device need to be reset to factory
      settings using special electrical technique.

    Command parameters:
      -d <device-name>         Name of a device.
      -f                       Do not prompt.


Device programming is started in a following way:

    ~/Arguments$ ./loro secure program -d loro-121E -p program.rom
    Application started normally.
    Following arguments were parsed:
      '/' = 'program'
      '/device-name' = 'loro-121E'
      '/program-file-path' = 'program.rom'


Simple commands providing no parameters:

    ~/Arguments$ ./loro list
    Application started normally.
    Following arguments were parsed:
      '/' = 'list'