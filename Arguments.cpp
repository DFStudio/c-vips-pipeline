//
// Created by Pierce Corcoran on 8/31/21.
//

#include "Arguments.h"

#include <iostream>
#include <utility>

Arguments::Arguments(std::string name, std::vector<std::string> arguments)
        : name(std::move(name)), arguments(std::move(arguments)) {}

bool Arguments::require(int count) const {
    if(this->arguments.size() != count) {
        std::cout << "Error: " << this->name << " expected " << count << " arguments, got " << this->arguments.size()
                  << std::endl;
        return false;
    } else {
        return true;
    }
}

bool Arguments::has(int index) const {
    auto &arg = this->arguments[index];
    return !arg.empty() && arg != "_";
}

const std::string& Arguments::get_string(int index) const {
    return this->arguments[index];
}

bool Arguments::get_bool(int index) const {
    if(!this->has(index))
        return false;
    auto arg = this->get_string(index);
    return arg == "1" || arg == "t" || arg == "true";
}

int Arguments::get_int(int index) const {
    return std::stoi(this->get_string(index));
}

float Arguments::get_float(int index) const {
    return std::stof(this->get_string(index));
}

double Arguments::get_double(int index) const {
    return std::stod(this->get_string(index));
}
