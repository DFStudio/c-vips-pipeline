//
// Created by Pierce Corcoran on 8/31/21.
//

#include "operations.h"
#include <iostream>
#include <limits>
#include <exception>
#include <optional>
#include <fmt/core.h>
#include <sys/fcntl.h>
#include "unsharp.h"

using namespace vips;

std::vector<double> trim_bands(int bands, std::vector<double> components) {
    std::vector<double> out;
    for(int i = 0; i < bands && i < components.size(); i++) {
        out.push_back(components[i]);
    }
    return out;
}

VipsExtend parse_extend(const std::string &arg) {
    if (arg == "black") return VIPS_EXTEND_BLACK;
    if (arg == "copy") return VIPS_EXTEND_COPY;
    if (arg == "repeat") return VIPS_EXTEND_REPEAT;
    if (arg == "mirror") return VIPS_EXTEND_MIRROR;
    if (arg == "white") return VIPS_EXTEND_WHITE;
    if (arg == "background") return VIPS_EXTEND_BACKGROUND;
    throw std::invalid_argument(fmt::format("Unrecognized extend mode '{}'", arg));
}

VipsIntent parse_intent(const std::string &arg) {
    if (arg == "perceptual") return VIPS_INTENT_PERCEPTUAL;
    if (arg == "relative") return VIPS_INTENT_RELATIVE;
    if (arg == "saturation") return VIPS_INTENT_SATURATION;
    if (arg == "absolute") return VIPS_INTENT_ABSOLUTE;
    throw std::invalid_argument(fmt::format("Unrecognized intent '{}'", arg));
}

VipsBlendMode parse_blend_mode(const std::string &arg) {
    if (arg == "clear") return VIPS_BLEND_MODE_CLEAR;
    if (arg == "source") return VIPS_BLEND_MODE_SOURCE;
    if (arg == "over") return VIPS_BLEND_MODE_OVER;
    if (arg == "in") return VIPS_BLEND_MODE_IN;
    if (arg == "out") return VIPS_BLEND_MODE_OUT;
    if (arg == "atop") return VIPS_BLEND_MODE_ATOP;
    if (arg == "dest") return VIPS_BLEND_MODE_DEST;
    if (arg == "dest_over") return VIPS_BLEND_MODE_DEST_OVER;
    if (arg == "dest_in") return VIPS_BLEND_MODE_DEST_IN;
    if (arg == "dest_out") return VIPS_BLEND_MODE_DEST_OUT;
    if (arg == "dest_atop") return VIPS_BLEND_MODE_DEST_ATOP;
    if (arg == "xor") return VIPS_BLEND_MODE_XOR;
    if (arg == "add") return VIPS_BLEND_MODE_ADD;
    if (arg == "saturate") return VIPS_BLEND_MODE_SATURATE;
    if (arg == "multiply") return VIPS_BLEND_MODE_MULTIPLY;
    if (arg == "screen") return VIPS_BLEND_MODE_SCREEN;
    if (arg == "overlay") return VIPS_BLEND_MODE_OVERLAY;
    if (arg == "darken") return VIPS_BLEND_MODE_DARKEN;
    if (arg == "lighten") return VIPS_BLEND_MODE_LIGHTEN;
    if (arg == "colour_dodge") return VIPS_BLEND_MODE_COLOUR_DODGE;
    if (arg == "colour_burn") return VIPS_BLEND_MODE_COLOUR_BURN;
    if (arg == "hard_light") return VIPS_BLEND_MODE_HARD_LIGHT;
    if (arg == "soft_light") return VIPS_BLEND_MODE_SOFT_LIGHT;
    if (arg == "difference") return VIPS_BLEND_MODE_DIFFERENCE;
    if (arg == "exclusion") return VIPS_BLEND_MODE_EXCLUSION;
    throw std::invalid_argument(fmt::format("Unrecognized blend mode '{}'", arg));
}

/**
 * If the passed file is a streaming source, this function returns the stream source
 */
std::optional<VSource> parse_stream_source(const std::string &file) {
    if(file == "-") {
        return VSource::new_from_descriptor(0); // stdin
    } else if(file.size() > 5 && file.substr(0, 5) == "fifo:") {
        // vips will close this file descriptor when it's done
        return VSource::new_from_descriptor(open(file.substr(5).c_str(), O_RDONLY));
    } else {
        return std::nullopt;
    }
}

/**
 * If the passed file is a streaming destination, this function returns a file descriptor, otherwise it returns -1.
 */
std::optional<VTarget> parse_stream_target(const std::string &file) {
    if(file == "-") {
        return VTarget::new_to_descriptor(1); // stdout
    } else if(file.size() > 5 && file.substr(0, 5) == "fifo:") {
        // vips will close this file descriptor when it's done
        return VTarget::new_to_descriptor(open(file.substr(5).c_str(), O_WRONLY));
    } else {
        return std::nullopt;
    }
}

void load(MachineState *state, const Arguments &arguments) {
    // <file in = 0> <slot out = 1>
    arguments.require(2);

    auto stream = parse_stream_source(arguments.get_string(0));
    if(stream) {
        state->set_image(
                arguments.get_string(1),
                VImage::new_from_source(*stream, "")
        );
    } else {
        state->set_image(
                arguments.get_string(1),
                VImage::new_from_file(arguments.get_string(0).c_str())
        );
    }
}

void load_thumbnail(MachineState *state, const Arguments &arguments) {
    // <file in = 0> <slot out = 1> <width = 2> <height? = 3> <no-rotate = 4> <intent? = 5>
    arguments.require(6);

    VipsIntent intent = VIPS_INTENT_RELATIVE;
    if (arguments.has(5)) {
        intent = parse_intent(arguments.get_string(5));
    }

    VOption *options = VImage::option()->set("no_rotate", arguments.get_bool(4))->set("intent", intent);
    if (arguments.has(3))
        options->set("height", arguments.get_int(3));

    auto stream = parse_stream_source(arguments.get_string(0));
    if(stream) {
        state->set_image(arguments.get_string(1), VImage::thumbnail_source(
                *stream,
                arguments.get_int(2),
                options
        ));
    } else {
        state->set_image(arguments.get_string(1), VImage::thumbnail(
                arguments.get_string(0).c_str(),
                arguments.get_int(2),
                options
        ));
    }
}

void autorotate(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1>
    arguments.require(2);
    state->set_image(arguments.get_string(1), state->get_image(arguments.get_string(0)).copy().autorot());
}

void transform_profile(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <profile = 2>
    arguments.require(3);
    state->set_image(arguments.get_string(1), state->get_image(arguments.get_string(0)).icc_transform(arguments.get_string(2).c_str()));
}

void unsharp(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <sigma = 2> <strength = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_string(0));
    VImage blur = input.gaussblur(arguments.get_double(2));
    VipsImage *sharpened;
    unsharp(input.get_image(), blur.get_image(), &sharpened, "strength", arguments.get_double(3), NULL);
    state->set_image(arguments.get_string(1), VImage(sharpened));
}

void composite(MachineState *state, const Arguments &arguments) {
    // <base slot = 0> <overlay slot = 1> <slot out = 2> <overlay x = 3> <overlay y = 4> <blend mode? = 5>
    arguments.require(6);

    VipsBlendMode mode = VIPS_BLEND_MODE_OVER;
    if (arguments.has(5)) {
        mode = parse_blend_mode(arguments.get_string(5));
    }

    state->set_image(arguments.get_string(2), state->get_image(arguments.get_string(0)).composite2(
            state->get_image(arguments.get_string(1)),
            mode,
            VImage::option()
                    ->set("x", arguments.get_int(3))
                    ->set("y", arguments.get_int(4))
    ));
}

struct vec2d {
    float x, y;

    vec2d operator+(const vec2d &other) const { return {this->x + other.x, this->y + other.y}; }
    vec2d operator-(const vec2d &other) const { return {this->x - other.x, this->y - other.y}; }
    vec2d operator*(const vec2d &other) const { return {this->x * other.x, this->y * other.y}; }
    vec2d operator*(float other) const { return {this->x * other, this->y * other}; }
    vec2d operator/(const vec2d &other) const { return {this->x / other.x, this->y / other.y}; }
    vec2d operator/(float other) const { return {this->x / other, this->y / other}; }
    vec2d operator-() const { return {-this->x, -this->y}; }
};

int count_to_bound(float base_size, float item_size, float origin, float step) {
    if(step == 0) {
        return 0;
    } else if(step < 0) {
        return (int)floor((origin + item_size) / -step);
    } else {
        return (int)floor((base_size - origin) / step);
    }
}

int count_to_bound(vec2d base_size, vec2d item_size, vec2d origin, vec2d step, bool max) {
    return max ? std::max(
            step.x == 0 ? 0 : count_to_bound(base_size.x, item_size.x, origin.x, step.x),
            step.y == 0 ? 0 : count_to_bound(base_size.y, item_size.y, origin.y, step.y)
    ) : std::min(
            step.x == 0 ? std::numeric_limits<int>::max() : count_to_bound(base_size.x, item_size.x, origin.x, step.x),
            step.y == 0 ? std::numeric_limits<int>::max() : count_to_bound(base_size.y, item_size.y, origin.y, step.y)
    );
}

void grid(MachineState *state, const Arguments &arguments) {
    // <base slot = 0> <overlay slot = 1> <slot out = 2> <origin x = 3> <origin y = 4>
    //   <vertical step x = 5> <vertical step y = 6> <horizontal step x = 7> <horizontal step y = 8>
    //   <blend mode? = 9>
    arguments.require(10);

    VipsBlendMode mode = VIPS_BLEND_MODE_OVER;
    if (arguments.has(9)) {
        mode = parse_blend_mode(arguments.get_string(9));
    }

    auto base = state->get_image(arguments.get_string(0));
    auto watermark = state->get_image(arguments.get_string(1));

    vec2d origin{arguments.get_float(3), arguments.get_float(4)};
    vec2d vStep{arguments.get_float(5), arguments.get_float(6)};
    vec2d hStep{arguments.get_float(7), arguments.get_float(8)};
    vec2d base_size{(float)base.width(), (float)base.height()};
    vec2d item_size{(float)watermark.width(), (float)watermark.height()};

    int hMin = -count_to_bound(base_size/2, item_size, origin, -hStep, true);
    int hMax = count_to_bound(base_size/2, item_size, origin, hStep, true);

    std::vector<VImage> images{base};
    std::vector<int> modes;
    std::vector<int> xs, ys;

    for(int h = hMin; h <= hMax; h++) {
        int vMin = -count_to_bound(base_size/2, item_size, origin + hStep * (float)h, -vStep, false);
        int vMax = count_to_bound(base_size/2, item_size, origin + hStep * (float)h, vStep, false);
        for(int v = vMin; v <= vMax; v++) {
            images.push_back(watermark);
            auto pos = origin + base_size/4 + vStep * (float)v + hStep * (float)h;
            xs.push_back((int)pos.x);
            ys.push_back((int)pos.y);
            modes.push_back((int)mode);
        }
    }

    state->set_image(arguments.get_string(2), VImage::composite(
            images,
            modes,
            VImage::option()
                    ->set("x", xs)
                    ->set("y", ys)
    ));
}

void add_alpha(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <alpha = 2>
    arguments.require(3);

    auto input_image = state->get_image(arguments.get_string(0));
    if(input_image.bands() == 4)
        state->set_image(arguments.get_string(1), input_image);
    else
        state->set_image(arguments.get_string(1), input_image
                .bandjoin_const({arguments.get_double(2)}));
}

void multiply_color(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <r = 2> <g = 3> <b = 4> <a = 5>
    arguments.require(6);
    auto input_image = state->get_image(arguments.get_string(0));
    state->set_image(arguments.get_string(1), input_image * trim_bands(input_image.bands(), {
            arguments.get_double(2),
            arguments.get_double(3),
            arguments.get_double(4),
            arguments.get_double(5)
    }));
}

void scale(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <hscale = 2> <vscale = 3>
    arguments.require(4);

    state->set_image(arguments.get_string(1), state->get_image(arguments.get_string(0)).resize(
            arguments.get_double(2),
            VImage::option()->set("vscale", arguments.get_double(3))
    ));
}

void affine(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <m00 = 2> <m01 = 3> <tx = 4> <m10 = 5> <m11 = 6> <ty = 7>
    arguments.require(8);

    auto input_image = state->get_image(arguments.get_string(0));
    state->set_image(arguments.get_string(1), input_image.affine(
            {
                    arguments.get_double(2),
                    arguments.get_double(3),
                    arguments.get_double(5),
                    arguments.get_double(6),
            },
            VImage::option()
                    ->set("odx", arguments.get_double(4))
                    ->set("ody", arguments.get_double(7))
    ));
}

void fit(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <width? = 2> <height? = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_string(0));
    auto scale = -1.0;
    if (arguments.has(2)) {
        scale = arguments.get_double(2) / input.width();
    }
    if (arguments.has(3)) {
        double vscale = arguments.get_double(3) / input.height();
        if (scale < 0 || vscale < scale)
            scale = vscale;
    }
    if (scale < 0) {
        state->set_image(arguments.get_string(1), input.copy());
    } else {
        state->set_image(arguments.get_string(1), input.resize(scale, VImage::option()));
    }
}

void trim_alpha(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <threshold = 2> <margin = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_string(0));

    auto alpha = input.extract_band(3);
    int left, top, width, height;
    left = alpha.find_trim(&top, &width, &height,
                           VImage::option()->set("threshold", arguments.get_double(2))->set("background",
                                                                                            std::vector<double>{0}));
    int margin = arguments.get_int(3);
    left -= margin;
    top -= margin;
    width += margin * 2;
    height += margin * 2;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (width < 1) width = 1;
    if (width > input.width() - left) width = input.width() - left;
    if (height < 1) height = 1;
    if (height > input.height() - top) height = input.height() - top;

    state->set_image(arguments.get_string(1), input.extract_area(left, top, width, height));
}

void flatten(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <background red = 2> <background green = 3> <background blue = 4>
    arguments.require(5);

    state->set_image(arguments.get_string(1), state->get_image(arguments.get_string(0)).flatten(
            VImage::option()
                    ->set("background", std::vector<double>{
                            arguments.get_double(2),
                            arguments.get_double(3),
                            arguments.get_double(4)
                    })
    ));
}

void embed(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <x = 2> <y = 3> <width = 4> <height = 5> <extend? = 6> <bg red = 7> <bg green = 8> <bg blue = 9> <bg alpha = 10>
    arguments.require(11);

    VipsExtend extend = VIPS_EXTEND_BACKGROUND;
    if (arguments.has(6)) {
        extend = parse_extend(arguments.get_string(6));
    }
    auto input_image = state->get_image(arguments.get_string(0));
    state->set_image(arguments.get_string(1), input_image.embed(
            arguments.get_int(2),
            arguments.get_int(3),
            arguments.get_int(4),
            arguments.get_int(5),
            VImage::option()
                    ->set("extend", extend)
                    ->set("background", trim_bands(input_image.bands(), {
                            arguments.get_double(7),
                            arguments.get_double(8),
                            arguments.get_double(9),
                            arguments.get_double(10),
                    }))
    ));
}

void write(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <file out = 1> <stream format = 2>
    arguments.require(3);

    auto stream = parse_stream_target(arguments.get_string(1));
    if(stream) {
        state->get_image(arguments.get_string(0)).write_to_target(
                arguments.get_string(2).c_str(),
                *stream
        );
    } else {
        state->get_image(arguments.get_string(0)).write_to_file(
                arguments.get_string(1).c_str()
        );
    }
}

void consume(MachineState *state, const Arguments &arguments) {
    // <slot = 0>
    arguments.require(1);
    size_t size;
    void *memory = state->get_image(arguments.get_string(0)).write_to_memory(&size);
    g_free(memory);
}

void free_slot(MachineState *state, const Arguments &arguments) {
    // <slot = 0>
    arguments.require(1);
    state->free_image(arguments.get_string(0));
}

void copy_slot(MachineState *state, const Arguments &arguments) {
    // <source slot = 0> <dest slot = 1>
    arguments.require(2);
    state->set_image(arguments.get_string(0), state->get_image(arguments.get_string(1)));
}

void set_var(MachineState *state, const Arguments &arguments) {
    // <var = 0> <value = 1>
    arguments.require(2);

    state->set_variable(arguments.get_string(0), arguments.get_double(1));
}

const std::map<std::string, image_operation> operations = {
        {"load",           load},
        {"thumbnail",      load_thumbnail},
        {"autorotate",     autorotate},
        {"profile",        transform_profile},
        {"unsharp",        unsharp},
        {"composite",      composite},
        {"grid",           grid},
        {"embed",          embed},
        {"flatten",        flatten},
        {"add_alpha",      add_alpha},
        {"scale",          scale},
        {"affine",         affine},
        {"fit",            fit},
        {"trim_alpha",     trim_alpha},
        {"multiply_color", multiply_color},
        {"write",          write},
        {"consume",        consume},
        {"free",           free_slot},
        {"copy_slot",      copy_slot},

        {"set_var",        set_var},
};

image_operation get_operation(const std::string &name) {
    return operations.at(name);
}

struct ImageFunction : public double_function
{
    typedef double (*callback_t)(const vips::VImage &);
    MachineState *state;
    callback_t callback;

    ImageFunction(MachineState *state, callback_t callback) : state(state), callback(callback), double_function("S"){}

    inline double operator()(double_function::parameter_list_t parameters) override {
        double_string_t name_view(parameters[0]);
        std::string name(name_view.begin(), name_view.size());
        auto image = state->get_image(name);
        return callback(image);
    }
};

void initialize_functions(MachineState *state) {
    state->add_function("vips_width", new ImageFunction(state, [](auto image) { return (double)image.width(); }));
    state->add_function("vips_height", new ImageFunction(state, [](auto image) { return (double)image.height(); }));
}

