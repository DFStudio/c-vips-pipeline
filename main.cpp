#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <chrono>
#include <vips/vips.h>
#include <vips/vips8>
#include <docopt/docopt.h>
#include "unsharp.h"

// default --sharpen=0.5/0/10/20/0/3
using namespace vips;

static const char USAGE[] = R"(Vips Scale.
Usage:
  vips-scale --input=<input> --output=<output>
    --width=<width> --height=<height>
    [--quality=<q> --strip]
    [--autorotate --profile=<path> --intent=<intent>]
    [--debug --unsharp]

Options:
  -h --help           Show this screen.
  --version           Show version.
  --input=<file>      Set the input file.
  --output=<file>     Set the output file.
  --quality=<q>       Set the output quality. [default: 70]
  --strip             Strip metadata in the output.
  --autorotate        Automatically apply rotation from the input file.
  --profile=<path>    The path to the ICC profile.
  --intent=<intent>   The rendering intent. One of:
                      "perceptual", "relative", "saturation", "absolute". [default: relative]
  --debug             Print debug information to stderr
  --unsharp           Apply an unsharp filter
)";

typedef std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> time_point;

time_point now() {
    return std::chrono::high_resolution_clock::now();
}

long time_since(time_point start) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(now() - start).count();
}

int main(int argc, char **argv) {
    std::map<std::string, docopt::value> args = docopt::docopt(
            USAGE,
            { argv + 1, argv + argc },
            true, // show help if requested
            "vips-scale 0.1" // version string
    );

    bool debug = args["--debug"].asBool();

    if(debug) std::cerr << "Entering debug mode" << std::endl;

    time_point start = now();
    if(VIPS_INIT( argv[0] ))
        vips_error_exit("init");
    unsharp_get_type();

    if(debug) std::cerr << "VIPS_INIT took " << time_since(start) << "ms" << std::endl << std::endl;

    VImage image;

    {
        if(debug) std::cerr << "# Thumbnail" << std::endl;

        VipsIntent intent = VIPS_INTENT_RELATIVE;
        if(args["--intent"].isString()) {
            auto arg = args["--intent"].asString();
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
                return 1;
            }
        }

        auto height = (int)args["--height"].asLong();
        auto no_rotate = !args["--autorotate"].asBool();
        auto input_image = args["--input"].asString();
        auto width = (int)args["--width"].asLong();

        if(debug) {
            std::cerr << "| vips_thumbnail options:" << std::endl;
            std::cerr << "|   required" << std::endl;
            std::cerr << "|     filename: " << input_image << std::endl;
            std::cerr << "|     width: " << width << std::endl;
            std::cerr << "|   optional" << std::endl;
            std::cerr << "|     height: " << height << std::endl;
            std::cerr << "|     no_rotate: " << no_rotate << std::endl;
            std::cerr << "|     intent: " << intent << std::endl;
            std::cerr << "| Testing image with new_from_file" << std::endl;
            auto test_image = VImage::new_from_file(input_image.c_str());
            std::cerr << "| Image opened successfully. Test image filename: " << test_image.filename() << std::endl;
            std::cerr << "| Running vips_thumbnail" << std::endl;
        }

        image = VImage::thumbnail(
                input_image.c_str(),
                width,
                VImage::option()
                        ->set("height", height)
                        ->set("no_rotate", no_rotate)
                        ->set("intent", intent)
        );

        if(debug) {
            std::cerr << "| vips_thumbnail complete" << std::endl;
            std::cerr << "# /Thumbnail" << std::endl << std::endl;
            std::cerr << "# Profile" << std::endl;
        }
        if(args["--profile"].isString()) {
            auto profile = args["--profile"].asString();
            if(debug) std::cerr << "| Transforming color profile to " << profile << std::endl;
            image = image.icc_transform(profile.c_str());
        } else {
            if(debug) std::cerr << "| No color profile specified, skipping transform" << std::endl;
        }
        if(debug) {
            std::cerr << "# /Profile" << std::endl << std::endl;
        }
    }

    if(debug) std::cerr << "# Unsharp" << std::endl;
    if(args["--unsharp"].asBool()) {
        if(debug) std::cerr << "| Unsharp specified, applying filter" << std::endl;
        VImage blur = image.gaussblur(1.0);
        if(debug) std::cerr << "| Created blurred image" << std::endl;
        VipsImage *_sharpened;
        unsharp(image.get_image(), blur.get_image(), &_sharpened);
        image = VImage(_sharpened);
        if (debug) std::cerr << "| Applied unsharp mask" << std::endl;
    } else {
        if(debug) std::cerr << "| Unsharp not specified, skipping" << std::endl;
    }
    if(debug) std::cerr << "# /Unsharp" << std::endl << std::endl;

    // NOTE: if we wanted to add some extra processing (e.g., calculate an image hash) this would be the place to do it.

    {
        if(debug) std::cerr << "# Write" << std::endl;
        auto quality = (int)args["--quality"].asLong();
        auto strip = args["--strip"].asBool();
        auto output = args["--output"].asString();
        if(debug) {
            std::cerr << "| write_to_file options" << std::endl;
            std::cerr << "|   required" << std::endl;
            std::cerr << "|     filename: " << output << std::endl;
            std::cerr << "|   optional" << std::endl;
            std::cerr << "|     Q: " << quality << std::endl;
            std::cerr << "|     strip: " << strip << std::endl;
            std::cerr << "|     optimize_coding: true" << std::endl;
        }
        image.write_to_file(output.c_str(), VImage::option()
                ->set("Q", quality)
                ->set("strip", strip)
                ->set("optimize_coding", true));
        if(debug) {
            std::cerr << "| Wrote file" << std::endl;
            std::cerr << "# /Write" << std::endl << std::endl;
        }
    }


    if(debug) std::cerr << "vips-scale completed in " << time_since(start) << "ms" << std::endl;

    return( 0 );
}