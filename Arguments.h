//
// Created by Pierce Corcoran on 8/31/21.
//

#ifndef VIPS_SCALE_ARGUMENTS_H
#define VIPS_SCALE_ARGUMENTS_H

#include <string>
#include <vector>

class Arguments {
    std::vector<std::string> arguments;
public:
    const std::string name;

    explicit Arguments(std::string name, std::vector<std::string> arguments);

    size_t size() const;
    bool require(size_t count) const;
    bool has(size_t index) const;
    const std::string& get_string(size_t index) const;
    bool get_bool(size_t index) const;
    int get_int(size_t index) const;
    float get_float(size_t index) const;
    double get_double(size_t index) const;
};


#endif //VIPS_SCALE_ARGUMENTS_H
