//
// Created by Pierce Corcoran on 9/14/21.
//

#ifndef VIPS_TOOL_MACHINESTATE_H
#define VIPS_TOOL_MACHINESTATE_H

#include <map>
#include <vips/vips8>

class MachineState {
    std::map<int, vips::VImage> slots;
    std::map<int, int> variables;
public:
    MachineState();

    void set_image(int index, vips::VImage image);
    vips::VImage get_image(int index);
    void free_image(int index);
    void set_variable(int index, int value);
    int get_variable(int index);
};


#endif //VIPS_TOOL_MACHINESTATE_H
