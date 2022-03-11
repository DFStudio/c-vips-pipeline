//
// Created by Pierce Corcoran on 8/31/21.
//

#ifndef VIPS_SCALE_OPERATIONS_H
#define VIPS_SCALE_OPERATIONS_H

#include <map>
#include <optional>
#include <vector>
#include <vips/vips.h>
#include <vips/vips8>
#include <docopt.h>
#include "Arguments.h"
#include "MachineState.h"

typedef std::map<std::string, docopt::value> option_map;
typedef void (*image_operation)(MachineState *state, const Arguments &arguments);
typedef void (*new_operation)(MachineState *state, option_map &options);

struct Operation {
    std::string help_text;
    new_operation operation;
};

void initialize_functions(MachineState *state);
image_operation get_operation(const std::string &name);
const Operation *get_new_operation(const std::string &name);

#endif //VIPS_SCALE_OPERATIONS_H
