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

    bool require(int count) const;
    bool has(int index) const;
    const std::string& get_string(int index) const;
    bool get_bool(int index) const;
    int get_int(int index) const;
    float get_float(int index) const;
    double get_double(int index) const;
};


#endif //VIPS_SCALE_ARGUMENTS_H
