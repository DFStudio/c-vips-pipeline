//
// Created by Pierce Corcoran on 8/31/21.
//

#include "Arguments.h"

#include <iostream>
#include <utility>
#include <exception>
#include <fmt/core.h>

Arguments::Arguments(std::string name, std::vector<std::string> arguments, MachineState *state)
        : name(std::move(name)), arguments(std::move(arguments)), state(state) {}

size_t Arguments::size() const {
    return arguments.size();
}

void Arguments::require(size_t count) const {
    if(this->arguments.size() != count) {
        throw std::length_error(fmt::format("Command {} expected {} arguments, got {}", this->name, count, this->arguments.size()));
    }
}

bool Arguments::has(size_t index) const {
    auto &arg = this->arguments[index];
    return !arg.empty() && arg != "_";
}

const std::string& Arguments::get_string(size_t index) const {
    return this->arguments[index];
}

bool Arguments::get_bool(size_t index) const {
    if(!this->has(index))
        return false;
    const auto &arg = this->get_string(index);
    if(arg == "1" || arg == "t" || arg == "true") {
        return true;
    } else if(arg == "0" || arg == "f" || arg == "false") {
        return false;
    } else {
        throw std::invalid_argument(fmt::format("Invalid value for boolean argument: '{}'", arg));
    }
}

int Arguments::get_int(size_t index) const {
    const auto &str = this->get_string(index);
    if(!str.empty() && str[0] == '$') {
        auto var = std::stoi(str.substr(1));
        return this->state->get_variable(var);
    }
    return std::stoi(this->get_string(index));
}

float Arguments::get_float(size_t index) const {
    return std::stof(this->get_string(index));
}

double Arguments::get_double(size_t index) const {
    return std::stod(this->get_string(index));
}
