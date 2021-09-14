//
// Created by Pierce Corcoran on 8/31/21.
//

#include "Arguments.h"

#include <iostream>
#include <utility>

Arguments::Arguments(std::string name, std::vector<std::string> arguments)
        : name(std::move(name)), arguments(std::move(arguments)) {}

size_t Arguments::size() const {
    return arguments.size();
}

bool Arguments::require(size_t count) const {
    if(this->arguments.size() != count) {
        std::cerr << "Error: " << this->name << " expected " << count << " arguments, got " << this->arguments.size()
                  << std::endl;
        return false;
    } else {
        return true;
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
    auto arg = this->get_string(index);
    return arg == "1" || arg == "t" || arg == "true";
}

int Arguments::get_int(size_t index) const {
    return std::stoi(this->get_string(index));
}

float Arguments::get_float(size_t index) const {
    return std::stof(this->get_string(index));
}

double Arguments::get_double(size_t index) const {
    return std::stod(this->get_string(index));
}
