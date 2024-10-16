//
// Created by Pierce Corcoran on 3/14/22.
//

#ifndef VIPS_TOOL_HELP_PAGES_H
#define VIPS_TOOL_HELP_PAGES_H

#include <string>

const char HELP_LIST[] =
        R"(# Special pages
    slots        Information about image slots
    expressions  Information about the expression language
    enums        Common enums used by multiple commands

# Operations
    open_stream     Open a file as a stream
loading images:
    load            Load an image from a file or stream
    thumbnail       Create a thumbnail from a file or stream
saving images:
    write           Write an image to a file or stream
    phash           Compute the perceptual hash of an image
adjusting images:
    profile         Apply an ICC color profile
    unsharp         Perform an "unsharp" operation
    autorotate      Flatten rotation metadata on an image
    flatten         Flatten an image's alpha channel with a solid background
    add_alpha       Add an alpha channel to an image if it doesn't already have one
transforming images:
    scale           Scale an image
    affine          Apply an affine transform to an image
    fit             Scale an image to fit inside a region
    multiply_color  Multiply the RGBA channels of an image by a constant factor
generating images:
    trim_alpha      Crop transparent edges from an image
    create_image    Create a blank image with a flat background color
compositing images:
    grid            Composite by repeating an image in an arbitrary grid
    composite       Composite two images
manipulating slots:
    consume         Consume a slot to resolve all its pixels
    free            Free the image in a slot
    copy            Copy an image to another slot
    resolve         Resolve the image in a slot and store it in memory
manipulating variables:
    set_var         Set a variable to a value
    print           Print the value of a variable
)";

const char HELP_ENUMS[] =
        R"(# Enums

Blend modes:
  (see https://www.cairographics.org/operators)
  clear, source, over, in, out, atop, dest, dest_over, dest_in, dest_out,
  dest_atop, xor, add, saturate, multiply, screen, overlay, darken, lighten,
  colour_dodge, colour_burn, hard_light, soft_light, difference, exclusion

Metadata types:
  int, double, string,
)";

const char HELP_EXPRESSIONS[] =
        R"~(# Expressions

Any numeric parameter can use an ExprTk expression to compute its value
dynamically. (see https://www.partow.net/programming/exprtk/index.html)

To evaluate a parameter as an expression, prefix the parameter with a '%'.
This is most helpful when used in conjunction with the '@set_var' operation
and vips-tool's own functions (listed below).

# Functions
  vips_width('<slot>')  - returns the width of the image in the given slot
  vips_height('<slot>') - returns the height of the image in the given slot
  vips_bands('<slot>')  - returns the number of bands (channels) in the image

# Examples

Scaling a watermark to fit an image:
    @set_var scale 0.6
    @load file.jpg base
    @thumbnail watermark.png watermark
        "%vips_width('base') * scale"
        "%vips_height('base') * scale"

Centering a watermark:
    @composite image watermark out
        "%(vips_width('image') - vips_width('watermark'))/2"
        "%(vips_height('image') - vips_height('watermark'))/2"

Rotating an image:
    @set_var rads 0.52
    @affine image rotated
        "%cos(rads)" "%-sin(rads)" "0"
        "%sin(rads)" "%cos(rads)"  "0"
)~";

const char HELP_SLOTS[] =
        R"(# Slots

Images are passed around using named "slots". Most operations that transform an
image don't actually perform computation, instead they create an image that
performs the computation when required (e.g. when writing the image to a file).
It's valid to use an image and put the result back in the same slot.
)";

const char *get_help_page(const std::string &name) {
    return name == "list" ? HELP_LIST :
           name == "enums" ? HELP_ENUMS :
           name == "expressions" ? HELP_EXPRESSIONS :
           name == "slots" ? HELP_SLOTS : nullptr;
}

#endif //VIPS_TOOL_HELP_PAGES_H
