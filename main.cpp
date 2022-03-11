#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <vips/vips.h>
#include <vips/vips8>
#include <fmt/core.h>
#include <docopt.h>
#include "unsharp.h"
#include "Arguments.h"
#include "operations.h"
#include "MachineState.h"
#include "vips_dct.h"

using namespace vips;

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
  vips-tool [--debug] [@<operation> <args...>]...

Options:
  -h --help   Show this screen.
  --version   Show version.
  -d --debug  Enable debug logging.

'vips-tool help list' lists the available operations and help pages
)";

const char HELP_LIST[] =
        R"(# Special pages
    slots        Information about image slots
    expressions  Information about the expression language

# Operations
loading images:
    load        Load an image from a file/stream
    thumbnail   Create a thumbnail from a file/stream
saving images:
    write       Write an image to a file/stream
    phash       Compute the perceptual hash of an image
adjusting images:
    profile     Apply an ICC color profile
    unsharp     Perform an "unsharp" operation
    autorotate  Flatten rotation metadata on an image
    flatten     Flatten an image's alpha channel with a solid background
    add-alpha   Add an alpha channel to an image if it doesn't already have one
transforming images:
    scale       Scale an image
    affine      Apply an affine transform to an image
    fit         Scale an image to fit inside a region
    multiply    Multiply the RGBA channels of an image by a constant factor
generating images:
    embed       Embed an image within a larger image
    grid        Generate an image by repeating an image in an arbitrary grid
    trim-alpha  Remove transparent borders from an image
compositing images:
    composite   Composite two images
manipulating slots:
    consume     Consume a slot to resolve all its pixels
    free        Free the image in a slot
    copy        Copy an image to another slot
    resolve     Resolve the image in a slot and store it in memory
manipulating variables:
    set_var     Set a variable to a value
    print       Print the value of a variable
)";

int main(int argc, char **argv) {
    if(argc == 1) {
        std::cout << USAGE << std::endl;
        return 0;
    }
    int first_operation = 0;
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
                std::cout << HELP_LIST << std::endl;
                exit(0);
            } else {
                auto operation = get_new_operation(page);
                if(operation == nullptr) {
                    std::cerr << "Unknown help page. See 'vips-tool help list' for a list of help pages" << std::endl;
                    exit(-1);
                } else {
                    std::cout << operation->help_text << std::endl;
                    exit(0);
                }
            }
        }
    }

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
    std::vector<option_map> options;

    for (int i = 0; i < commands.size(); i++) {
        auto command = commands[i];
        auto operation = get_new_operation(command[0].substr(1));
        if(operation == nullptr) {
            std::cerr << "Unknown operation " << command[0] << std::endl;
            exit(-1);
        } else {
            try {
                operations.push_back(operation);
                options.push_back(docopt::docopt_parse(
                        operation->help_text,
                        {command.begin() + 1, command.end()},
                        true,
                        false
                ));
            } catch (docopt::DocoptExitHelp const&) {
                std::cout << operation->help_text << std::endl;
                std::exit(0);
            } catch (docopt::DocoptLanguageError const& error) {
                std::cerr << "Docopt usage string could not be parsed" << std::endl;
                std::cerr << error.what() << std::endl;
                std::exit(-1);
            } catch (docopt::DocoptArgumentError const& error) {
                std::cerr << "Error parsing operation " << i << std::endl;
                std::cerr << error.what();
                std::cout << std::endl;
                std::cout << operation->help_text << std::endl;
                std::exit(-1);
            }
        }
    }

    MachineState machine;
    initialize_functions(&machine);

    const auto start = now();
    if (VIPS_INIT(argv[0]))
        vips_error_exit("init");
    unsharp_get_type();
    vips_dct_get_type();

    for (int i = 0; i < operations.size(); ++i) {
        try {
            operations[i]->operation(&machine, options[i]);
        } catch (const std::exception &error) {
            std::cerr << "Exception executing operation " << i << std::endl;
            std::cerr << error.what() << std::endl;
            exit(1);
        }
    }

    /*
    std::vector<Arguments> commands;
    std::string current_name;
    std::vector<std::string> current_command;

    int comment_depth = 0;
    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "@/ *") {
            comment_depth++;
            continue;
        } else if(arg == "@* /") {
            if(comment_depth > 0)
                comment_depth--;
            continue;
        }
        if(comment_depth > 0)
            continue;

        if (!arg.empty() && arg[0] == '@') {
            if (!current_name.empty())
                commands.emplace_back(i, current_name, current_command, &machine);
            current_name = arg.substr(1);
            current_command.clear();
        } else {
            current_command.push_back(arg);
        }
    }

    if (!current_name.empty())
        commands.emplace_back(commands.size(), current_name, current_command, &machine);

    const auto start = now();
    if (VIPS_INIT(argv[0]))
        vips_error_exit("init");
    unsharp_get_type();
    vips_dct_get_type();

    if (machine.is_debug()) {
        std::cerr << "# VIPS_INIT took " << time_since(start) << "ms" << std::endl;
    }

    for (auto &command: commands) {
        try {
            if(machine.is_debug()) {
                std::cerr << "# Running command " << command.name << std::endl;
                int n = 0;
                for(auto &arg : command.get_arguments()) {
                    std::cerr << "#   " << n << ": " << arg << std::endl;
                    n++;
                }
            }
            auto command_start = now();
            auto operation = get_operation(command.name);
            operation(&machine, command);
            if(machine.is_debug()) {
                std::cerr << "# Command took " << time_since(command_start) << "ms" << std::endl;
            }
        } catch (std::exception &e) {
            throw std::runtime_error(
                    fmt::format("Error running command {} ({}): {}", command.index, command.name, e.what())
            );
        }
    }

    if(machine.is_debug()) {
        std::cerr << "# vips-tool completed in " << time_since(start) << "ms" << std::endl;
    }

    */
    return (0);
}
