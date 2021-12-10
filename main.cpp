#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <vips/vips.h>
#include <vips/vips8>
#include <fmt/core.h>
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

int main(int argc, char **argv) {
    MachineState machine;
    initialize_functions(&machine);

    std::vector<Arguments> commands;
    std::string current_name;
    std::vector<std::string> current_command;

    int comment_depth = 0;
    for (int i = 0; i < argc; i++) {
        std::string arg(argv[i]);
        if (arg == "@/*") {
            comment_depth++;
            continue;
        } else if(arg == "@*/") {
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
        std::cerr << "VIPS_INIT took " << time_since(start) << "ms" << std::endl;
    }

    for (auto &command: commands) {
        try {
            if(machine.is_debug()) {
                std::cerr << "Running command " << command.name << std::endl;
                int n = 0;
                for(auto &arg : command.get_arguments()) {
                    std::cerr << "  " << n << ": " << arg << std::endl;
                    n++;
                }
            }
            auto command_start = now();
            auto operation = get_operation(command.name);
            operation(&machine, command);
            if(machine.is_debug()) {
                std::cerr << "Command took " << time_since(command_start) << "ms" << std::endl;
            }
        } catch (std::exception &e) {
            throw std::runtime_error(
                    fmt::format("Error running command {} ({}): {}", command.index, command.name, e.what())
            );
        }
    }

    if(machine.is_debug()) {
        std::cerr << "vips-tool completed in " << time_since(start) << "ms" << std::endl;
    }

    return (0);
}