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

struct Operation {
    std::string name;
    std::string help_text;
    image_operation operation;
};

void initialize_functions(MachineState *state);
const Operation *get_operation(const std::string &name);

#endif //VIPS_SCALE_OPERATIONS_H
