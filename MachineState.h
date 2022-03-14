//
// Created by Pierce Corcoran on 9/14/21.
//

#ifndef VIPS_TOOL_MACHINESTATE_H
#define VIPS_TOOL_MACHINESTATE_H

#include <map>
#include <vips/vips8>
#include <exprtk.h>

class MachineState;

typedef typename exprtk::igeneric_function<double> double_function;
typedef double_function::parameter_list_t machine_parameters;
typedef typename double_function::generic_type::scalar_view double_scalar_t;
typedef typename double_function::generic_type::vector_view double_vector_t;
typedef typename double_function::generic_type::string_view double_string_t;


class MachineState {
    struct StreamInfo {
        int fd;
        bool input;
        bool used;
    };

    std::map<std::string, vips::VImage> slots;
    std::map<std::string, StreamInfo> files;
    std::map<std::string, double *> variable_store; // exprtk requires *references*, so we need to keep them alive
    std::map<std::string, double_function *> functions;
    exprtk::symbol_table<double> symbols;
    int _verbosity = 0;
public:
    MachineState();

    virtual ~MachineState();

    void add_function(const std::string &name, double_function *function);

    void set_image(const std::string &name, vips::VImage image);

    vips::VImage get_image(const std::string &name);

    void free_image(const std::string &name);

    void set_variable(const std::string &name, double value);

    double get_variable(const std::string &name);

    double eval(const std::string &expr);

    void set_verbosity(int level);

    [[nodiscard]] int verbosity() const;

    [[nodiscard]] bool is_verbose(int level) const;

    void open_fd(const std::string &name, const std::string &file, bool input);
    int parse_fd(const std::string &arg, bool input);
};


#endif //VIPS_TOOL_MACHINESTATE_H
