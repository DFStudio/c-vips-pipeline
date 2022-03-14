#include <chrono>
#include <string>
#include <vector>
#include <vips/vips.h>
#include <fmt/core.h>
#include <docopt.h>
#include "unsharp.h"
#include "Arguments.h"
#include "operations.h"
#include "MachineState.h"
#include "vips_dct.h"
#include "help_pages.h"

using namespace vips;
using fmt::print;

typedef std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> time_point;

time_point now() {
    return std::chrono::high_resolution_clock::now();
}

long time_since(time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start).count();
}

const char USAGE[] =
        R"(Usage:
  vips-tool [-h|--help] [--version]
  vips-tool help [<page>]
  vips-tool [-v | -vv] [@<operation> <args...>]...

Options:
  -h --help  Show this screen.
  --version  Show version.
  -v         Enable verbose (debug) logging.

'vips-tool help list' lists the available operations and help pages
)";

int main(int argc, char **argv) {
    if(argc == 1) {
        print("{}", USAGE);
        return 0;
    }
    int first_operation = argc;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '@') {
            first_operation = i;
            break;
        }
    }
    auto main_docs = docopt::docopt(
            USAGE,
            {argv + 1, argv + first_operation},
            true,
            "VIPS tool 1.1"
    );

    if(main_docs["help"].asBool()) {
        if(main_docs["<page>"].isString()) {
            auto page = main_docs["<page>"].asString();
            if(page == "list") {
                print("{}", HELP_LIST);
                exit(0);
            } else if(page == "enums") {
                print("{}", HELP_ENUMS);
                exit(0);
            } else {
                auto operation = get_operation(page);
                if(operation == nullptr) {
                    print(stderr, "Unknown help page '{}'. See 'vips-tool help list' for a list of help pages\n", page);
                    exit(-1);
                } else {
                    size_t start = 0, len = operation->help_text.size();
                    if(!operation->help_text.empty()) {
                        if (operation->help_text[0] == '\n') {
                            start++;
                            len--;
                        }
                        if (operation->help_text[operation->help_text.size() - 1] == '\n') {
                            len--;
                        }
                    }
                    print("{}\n", operation->help_text.substr(start, len));
                    exit(0);
                }
            }
        }
    }

    MachineState machine;
    machine.set_verbosity((int)main_docs["-v"].asLong());

    std::vector<std::string> main_args;
    std::vector<std::vector<std::string>> commands;

    for (int i = first_operation; i < argc; i++) {
        if (argv[i][0] == '@') {
            commands.emplace_back();
        }
        if(!commands.empty()) {
            commands[commands.size() - 1].emplace_back(argv[i]);
        } else {
            main_args.emplace_back(argv[i]);
        }
    }

    std::vector<const Operation*> operations;
    std::vector<Arguments> arguments;

    for (int i = 0; i < commands.size(); i++) {
        auto command = commands[i];
        auto operation = get_operation(command[0].substr(1));
        if(operation == nullptr) {
            print(stderr, "Unknown operation {}\n", command[0]);
            exit(-1);
        } else {
            auto help_text = operation->help_text.empty() || operation->help_text[0] != '\n' ? operation->help_text : operation->help_text.substr(1);
            try {
                operations.push_back(operation);
                auto args = docopt::docopt_parse(
                        operation->help_text,
                        {command.begin() + 1, command.end()},
                        true,
                        false
                );
                arguments.emplace_back(i, operation->name, args, &machine);
            } catch (docopt::DocoptExitHelp const&) {
                print("{}\n", operation->help_text);
                std::exit(0);
            } catch (docopt::DocoptLanguageError const& error) {
                print(stderr, "Docopt usage string could not be parsed\n");
                print(stderr, "{}\n", error.what());
                std::exit(-1);
            } catch (docopt::DocoptArgumentError const& error) {
                print(stderr, "Error parsing operation {} '{}'\n", i, command[0]);
                print(stderr, "{}\n", error.what());
                print(stderr, "{}\n", operation->help_text);
                std::exit(-1);
            }
        }
    }

    initialize_functions(&machine);

    const auto start = now();
    if (VIPS_INIT(argv[0]))
        vips_error_exit("init");
    unsharp_get_type();
    vips_dct_get_type();

    if(machine.is_verbose(2)) {
        print("Initialized VIPS in {}ms\n", time_since(start));
    }

    for (int i = 0; i < operations.size(); ++i) {
        try {
            if(machine.is_verbose(1)) {
                print("Starting operation '{}'\n", operations[i]->name);
            }
            auto command_start = now();
            operations[i]->operation(&machine, arguments[i]);
            if(machine.is_verbose(2)) {
                print("Operation took {}ms\n", time_since(command_start));
            }
        } catch (const std::exception &error) {
            print(stderr, "Exception executing operation {} '{}'\n", i, operations[i]->name);
            print(stderr, "{}\n", error.what());
            exit(1);
        }
    }

    if(machine.is_verbose(1)) {
        print("vips-tool completed in {}ms\n", time_since(start));
    }

    return (0);
}
