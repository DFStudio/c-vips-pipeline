//
// Created by Pierce Corcoran on 3/14/22.
//

#ifndef VIPS_TOOL_HELP_PAGES_H
#define VIPS_TOOL_HELP_PAGES_H

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
)";

#endif //VIPS_TOOL_HELP_PAGES_H
