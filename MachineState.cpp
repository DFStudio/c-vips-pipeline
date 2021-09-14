//
// Created by Pierce Corcoran on 9/14/21.
//

#include "MachineState.h"

#include <exception>
#include <utility>
#include <fmt/core.h>

void MachineState::set_image(const std::string &name, vips::VImage image) {
    this->slots[name] = std::move(image);
}

vips::VImage MachineState::get_image(const std::string &name) {
    auto image = this->slots.find(name);
    if(image == this->slots.end())
        throw std::invalid_argument(fmt::format("No image in slot {}", name));
    return image->second;
}

void MachineState::free_image(const std::string &name) {
    this->slots.erase(name);
}

void MachineState::set_variable(const std::string &name, double value) {
    auto existing = this->variable_store.find(name);
    double *value_store;
    if(existing != this->variable_store.end()) {
        value_store = existing->second;
    } else {
        value_store = new double;
        this->variable_store[name] = value_store;
    }
    *value_store = value;
    this->symbols.add_variable(name, *value_store, true);
}

double MachineState::get_variable(const std::string &name) {
    return this->symbols.get_variable(name)->value();
}

double MachineState::eval(const std::string &expr) {
    exprtk::parser<double> parser;
    auto expression = parser.compile(expr, this->symbols);
    if(parser.error_count() > 0) {
        std::string error = fmt::format("Error(s) parsing '{}': ", expr);
        for(int i = 0; i < parser.error_count(); i++) {
            if(i != 0)
                error += ", ";
            error += parser.get_error(i).diagnostic;
        }
        throw std::runtime_error(error);
    }
    return expression.value();
}

MachineState::MachineState() {
}

MachineState::~MachineState() {
    for(auto &[name, reference] : this->variable_store) {
        delete reference;
    }
    for(auto &[name, function] : this->functions) {
        delete function;
    }
}

void MachineState::add_function(const std::string &name, double_function *function) {
    auto existing = this->functions.find(name);
    if(existing != this->functions.end()) {
        this->symbols.remove_function(name);
        delete existing->second;
    }
    this->functions[name] = function;
    this->symbols.add_function(name, *function);
}
