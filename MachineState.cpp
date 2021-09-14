//
// Created by Pierce Corcoran on 9/14/21.
//

#include "MachineState.h"

#include <exception>
#include <utility>
#include <fmt/core.h>

void MachineState::set_image(int index, vips::VImage image) {
    this->slots[index] = std::move(image);
}

vips::VImage MachineState::get_image(int index) {
    auto image = this->slots.find(index);
    if(image == this->slots.end())
        throw std::invalid_argument(fmt::format("No image in slot {}", index));
    return image->second;
}

void MachineState::free_image(int index) {
    this->slots.erase(index);
}

void MachineState::set_variable(int index, int value) {
    this->variables[index] = value;
}

int MachineState::get_variable(int index) {
    auto value = this->variables.find(index);
    if(value == this->variables.end())
        throw std::invalid_argument(fmt::format("No variable in slot {}", index));
    return value->second;
}

MachineState::MachineState() = default;
