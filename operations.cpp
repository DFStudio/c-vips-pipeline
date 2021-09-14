//
// Created by Pierce Corcoran on 8/31/21.
//

#include "operations.h"
#include <iostream>
#include <exception>
#include <fmt/core.h>
#include "unsharp.h"

using namespace vips;

void load(MachineState *state, const Arguments &arguments) {
    // <file in = 0> <slot out = 1>
    arguments.require(2);

    const auto& file = arguments.get_string(0);
    if(file == "-") {
        state->set_image(arguments.get_int(1), VImage::new_from_source(VSource::new_from_descriptor(0), ""));
    } else {
        state->set_image(arguments.get_int(1), VImage::new_from_file(file.c_str()));
    }
}

void load_thumbnail(MachineState *state, const Arguments &arguments) {
    // <file in = 0> <slot out = 1> <width = 2> <height? = 3> <no-rotate = 4> <intent? = 5>
    arguments.require(6);

    VipsIntent intent = VIPS_INTENT_RELATIVE;
    if (arguments.has(5)) {
        auto arg = arguments.get_string(5);
        if (arg == "perceptual") {
            intent = VIPS_INTENT_PERCEPTUAL;
        } else if (arg == "relative") {
            intent = VIPS_INTENT_RELATIVE;
        } else if (arg == "saturation") {
            intent = VIPS_INTENT_SATURATION;
        } else if (arg == "absolute") {
            intent = VIPS_INTENT_ABSOLUTE;
        } else {
            throw std::invalid_argument(fmt::format("Unrecognized intent '{}'", arg));
        }
    }

    VOption *options = VImage::option()->set("no_rotate", arguments.get_bool(4))->set("intent", intent);
    if (arguments.has(3))
        options->set("height", arguments.get_int(3));

    const auto &file = arguments.get_string(0);
    if(file == "-") {
        state->set_image(arguments.get_int(1), VImage::thumbnail_source(
                VSource::new_from_descriptor(0),
                arguments.get_int(2),
                options
        ));
    } else {
        state->set_image(arguments.get_int(1), VImage::thumbnail(
                file.c_str(),
                arguments.get_int(2),
                options
        ));
    }
}

void transform_profile(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <profile = 2>
    arguments.require(3);
    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0)).icc_transform(arguments.get_string(2).c_str()));
}

void unsharp(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <sigma = 2> <strength = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_int(0));
    VImage blur = input.gaussblur(arguments.get_double(2));
    VipsImage *sharpened;
    unsharp(input.get_image(), blur.get_image(), &sharpened, "strength", arguments.get_double(3), NULL);
    state->set_image(arguments.get_int(1), VImage(sharpened));
}

void composite(MachineState *state, const Arguments &arguments) {
    // <base slot = 0> <overlay slot = 1> <slot out = 2> <overlay x = 3> <overlay y = 4> <blend mode? = 5>
    arguments.require(6);

    VipsBlendMode mode = VIPS_BLEND_MODE_OVER;
    if (arguments.has(5)) {
        auto &arg = arguments.get_string(5);
        if (arg == "clear") mode = VIPS_BLEND_MODE_CLEAR;
        else if (arg == "source") mode = VIPS_BLEND_MODE_SOURCE;
        else if (arg == "over") mode = VIPS_BLEND_MODE_OVER;
        else if (arg == "in") mode = VIPS_BLEND_MODE_IN;
        else if (arg == "out") mode = VIPS_BLEND_MODE_OUT;
        else if (arg == "atop") mode = VIPS_BLEND_MODE_ATOP;
        else if (arg == "dest") mode = VIPS_BLEND_MODE_DEST;
        else if (arg == "dest_over") mode = VIPS_BLEND_MODE_DEST_OVER;
        else if (arg == "dest_in") mode = VIPS_BLEND_MODE_DEST_IN;
        else if (arg == "dest_out") mode = VIPS_BLEND_MODE_DEST_OUT;
        else if (arg == "dest_atop") mode = VIPS_BLEND_MODE_DEST_ATOP;
        else if (arg == "xor") mode = VIPS_BLEND_MODE_XOR;
        else if (arg == "add") mode = VIPS_BLEND_MODE_ADD;
        else if (arg == "saturate") mode = VIPS_BLEND_MODE_SATURATE;
        else if (arg == "multiply") mode = VIPS_BLEND_MODE_MULTIPLY;
        else if (arg == "screen") mode = VIPS_BLEND_MODE_SCREEN;
        else if (arg == "overlay") mode = VIPS_BLEND_MODE_OVERLAY;
        else if (arg == "darken") mode = VIPS_BLEND_MODE_DARKEN;
        else if (arg == "lighten") mode = VIPS_BLEND_MODE_LIGHTEN;
        else if (arg == "colour_dodge") mode = VIPS_BLEND_MODE_COLOUR_DODGE;
        else if (arg == "colour_burn") mode = VIPS_BLEND_MODE_COLOUR_BURN;
        else if (arg == "hard_light") mode = VIPS_BLEND_MODE_HARD_LIGHT;
        else if (arg == "soft_light") mode = VIPS_BLEND_MODE_SOFT_LIGHT;
        else if (arg == "difference") mode = VIPS_BLEND_MODE_DIFFERENCE;
        else if (arg == "exclusion") mode = VIPS_BLEND_MODE_EXCLUSION;
    }

    state->set_image(arguments.get_int(2), state->get_image(arguments.get_int(0)).composite2(
            state->get_image(arguments.get_int(1)),
            mode,
            VImage::option()
                    ->set("x", arguments.get_int(3))
                    ->set("y", arguments.get_int(4))
    ));
}

void add_alpha(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <alpha = 2>
    arguments.require(3);

    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0))
            .bandjoin_const({arguments.get_double(2)}));
}

void multiply_color(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <r = 2> <g = 3> <b = 4> <a = 5>
    arguments.require(6);

    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0)) * std::vector<double>{
            arguments.get_double(2),
            arguments.get_double(3),
            arguments.get_double(4),
            arguments.get_double(5)
    });
}

void scale(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <hscale = 2> <vscale = 3>
    arguments.require(4);

    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0)).resize(
            arguments.get_double(2),
            VImage::option()->set("vscale", arguments.get_double(3))
    ));
}

void fit(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <width? = 2> <height? = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_int(0));
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
        state->set_image(arguments.get_int(1), input.copy());
    } else {
        state->set_image(arguments.get_int(1), input.resize(scale, VImage::option()));
    }
}

void trim_alpha(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <threshold = 2> <margin = 3>
    arguments.require(4);

    auto input = state->get_image(arguments.get_int(0));

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

    state->set_image(arguments.get_int(1), input.extract_area(left, top, width, height));
}

void flatten(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <background red = 2> <background green = 3> <background blue = 4>
    arguments.require(5);

    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0)).flatten(
            VImage::option()
                    ->set("background", std::vector<double>{
                            arguments.get_double(2),
                            arguments.get_double(3),
                            arguments.get_double(4)
                    })
    ));
}

void embed(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <x = 2> <y = 3> <width = 4> <height = 5> <extend = 6> <bg red = 7> <bg green = 8> <bg blue = 9>
    arguments.require(10);

    VipsExtend extend = VIPS_EXTEND_BACKGROUND;
    if (arguments.has(6)) {
        auto &arg = arguments.get_string(6);
        if (arg == "black") extend = VIPS_EXTEND_BLACK;
        else if (arg == "copy") extend = VIPS_EXTEND_COPY;
        else if (arg == "repeat") extend = VIPS_EXTEND_REPEAT;
        else if (arg == "mirror") extend = VIPS_EXTEND_MIRROR;
        else if (arg == "white") extend = VIPS_EXTEND_WHITE;
        else if (arg == "background") extend = VIPS_EXTEND_BACKGROUND;
    }

    state->set_image(arguments.get_int(1), state->get_image(arguments.get_int(0)).embed(
            arguments.get_int(2),
            arguments.get_int(3),
            arguments.get_int(4),
            arguments.get_int(5),
            VImage::option()
                    ->set("extend", extend)
                    ->set("background", std::vector<double>{
                            arguments.get_double(7),
                            arguments.get_double(8),
                            arguments.get_double(9)
                    })
    ));
}

void write(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <file out = 1>
    arguments.require(2);
    state->get_image(arguments.get_int(0)).write_to_file(arguments.get_string(1).c_str());
}

void stream(MachineState *state, const Arguments &arguments) {
    // <slot in = 0> <format = 1>
    arguments.require(2);
    auto target = VTarget::new_to_descriptor(1);
    state->get_image(arguments.get_int(0)).write_to_target(arguments.get_string(1).c_str(), target);
}

void consume(MachineState *state, const Arguments &arguments) {
    // <slot = 0>
    arguments.require(1);
    size_t size;
    void *memory = state->get_image(arguments.get_int(0)).write_to_memory(&size);
    g_free(memory);
}

void free_slot(MachineState *state, const Arguments &arguments) {
    // <slot = 0>
    arguments.require(1);
    state->free_image(arguments.get_int(0));
}

void get_metric(MachineState *state, const Arguments &arguments) {
    // <slot = 0> <metric = 1> <var = 2>
    arguments.require(3);

    int value;
    auto image = state->get_image(arguments.get_int(0));
    const auto &metric = arguments.get_string(1);
    if(metric == "width") {
        value = image.width();
    } else if(metric == "height") {
        value = image.height();
    } else {
        throw std::invalid_argument(fmt::format("Invalid metric '{}'", metric));
    }
    state->set_variable(arguments.get_int(2), value);
}

void set_var(MachineState *state, const Arguments &arguments) {
    // <var = 0> <value = 1>
    arguments.require(2);

    state->set_variable(arguments.get_int(0), arguments.get_int(1));
}

const std::map<std::string, image_operation> operations = {
        {"load",           load},
        {"thumbnail",      load_thumbnail},
        {"profile",        transform_profile},
        {"unsharp",        unsharp},
        {"composite",      composite},
        {"embed",          embed},
        {"flatten",        flatten},
        {"add_alpha",      add_alpha},
        {"scale",          scale},
        {"fit",            fit},
        {"trim_alpha",     trim_alpha},
        {"multiply_color", multiply_color},
        {"stream",         stream},
        {"write",          write},
        {"consume",        consume},
        {"free",           free_slot},

        {"get_metric",     get_metric},
        {"set_var",        set_var},
};

image_operation get_operation(const std::string &name) {
    return operations.at(name);
}

