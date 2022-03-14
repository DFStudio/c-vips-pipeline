//
// Created by Pierce Corcoran on 8/31/21.
//

#ifndef VIPS_SCALE_ARGUMENTS_H
#define VIPS_SCALE_ARGUMENTS_H

#include <string>
#include <vector>
#include <docopt.h>
#include "MachineState.h"

typedef std::map<std::string, docopt::value> docopt_map;

class Arguments {
    docopt_map arguments;
    MachineState *state;
public:
    const int index;
    const std::string operation;

    explicit Arguments(int index, std::string operation, docopt_map arguments, MachineState *state);

    [[nodiscard]] bool has(const std::string &name) const;
    [[nodiscard]] const std::string &get_string(const std::string &name) const;
    [[nodiscard]] const std::string &get_string(const std::string &name, const std::string &def) const;
    [[nodiscard]] bool get_bool(const std::string &name) const;
    [[nodiscard]] int get_int(const std::string &name) const;
    [[nodiscard]] int get_int(const std::string &name, int def) const;
    [[nodiscard]] float get_float(const std::string &name) const;
    [[nodiscard]] float get_float(const std::string &name, float def) const;
    [[nodiscard]] double get_double(const std::string &name) const;
    [[nodiscard]] double get_double(const std::string &name, double def) const;
};


#endif //VIPS_SCALE_ARGUMENTS_H
