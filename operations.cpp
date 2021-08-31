//
// Created by Pierce Corcoran on 8/31/21.
//

#include "operations.h"
#include <iostream>
#include "unsharp.h"

using namespace vips;

bool present(const std::string &arg) {
    return arg != "_";
}

bool load_thumbnail(std::map<int, vips::VImage> &slots, const Arguments &arguments) {
    // <file in = 0> <slot out = 1> <width = 2> <height? = 3> <no-rotate = 4> <intent? = 5>
    if(!arguments.require(6)) return false;

    VipsIntent intent = VIPS_INTENT_RELATIVE;
    if(arguments.has(5)) {
        auto arg = arguments.get_string(5);
        if(arg == "perceptual") {
            intent = VIPS_INTENT_PERCEPTUAL;
        } else if(arg == "relative") {
            intent = VIPS_INTENT_RELATIVE;
        } else if(arg == "saturation") {
            intent = VIPS_INTENT_SATURATION;
        } else if(arg == "absolute") {
            intent = VIPS_INTENT_ABSOLUTE;
        } else {
            std::cerr << "Unrecognized intent '" << arg << "'." << std::endl;
            return false;
        }
    }

    VOption *options = VImage::option()
            ->set("height", arguments.get_int(3))
            ->set("no_rotate", arguments.get_bool(4))
            ->set("intent", intent)
    ;

    slots[arguments.get_int(1)] = VImage::thumbnail(
            arguments.get_string(0).c_str(),
            arguments.get_int(2),
            options
    );

    return true;
}

bool transform_profile(std::map<int, vips::VImage> &slots, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <profile = 2>
    if(!arguments.require(3)) return false;
    slots[arguments.get_int(1)] = slots[arguments.get_int(0)].icc_transform(arguments.get_string(2).c_str());
    return true;
}

bool unsharp(std::map<int, vips::VImage> &slots, const Arguments &arguments) {
    // <slot in = 0> <slot out = 1> <sigma = 2> <strength = 3>
    if(!arguments.require(4)) return false;

    auto input = slots[arguments.get_int(0)];
    VImage blur = input.gaussblur(arguments.get_double(2));
    VipsImage *sharpened;
    unsharp(input.get_image(), blur.get_image(), &sharpened, "strength", arguments.get_double(3), NULL);
    slots[arguments.get_int(1)] = VImage(sharpened);

    return true;
}

bool write(std::map<int, vips::VImage> &slots, const Arguments &arguments) {
    // <slot in = 0> <file out = 1>
    if(!arguments.require(2)) return false;
    slots[arguments.get_int(0)].write_to_file(arguments.get_string(1).c_str());
    return true;
}

const std::map<std::string, image_operation> operations = {
//        {"load", load_file},
        {"thumbnail", load_thumbnail},
        {"profile", transform_profile},
        {"unsharp", unsharp},
//        {"draw_image", draw_image},
//        {"extend", extend},
//        {"flatten", flatten},
//        {"add_alpha", add_alpha},
        {"write", write}
//        {"free", free_slot}
};

image_operation get_operation(const std::string &name) {
    return operations.at(name);
}

