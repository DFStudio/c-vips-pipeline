//
// Created by Pierce Corcoran on 12/8/21.
//

#include <iostream>
#include "phash.h"
#include "vips_dct.h"

// based on https://github.com/jenssegers/imagehash/blob/eab0081/src/Implementations/PerceptualHash.php

std::vector<bool> pHash(const vips::VImage &image, int reduce_size, int sample_size) {
    auto resized = image
            .colourspace(VIPS_INTERPRETATION_B_W)
            .resize(
                    ((double) reduce_size) / image.width(),
                    vips::VImage::option()->set("vscale", ((double) reduce_size) / image.height())
            );

    VipsImage *horizontal_dct, *full_dct;
    vips_dct(resized.get_image(), &horizontal_dct, NULL);
    vips_dct(horizontal_dct, &full_dct, "columnar", TRUE, NULL);
    g_object_unref(horizontal_dct);
    vips::VImage dct(full_dct);

    auto sample = dct.extract_area(0, 0, sample_size, sample_size);
    double average = sample.avg();
    auto bit_image = sample.relational_const(VIPS_OPERATION_RELATIONAL_MORE, {average});
    auto region = vips_region_new(bit_image.get_image());
    size_t pixel_count;
    unsigned char *pixels = vips_region_fetch(region, 0, 0, bit_image.width(), bit_image.height(), &pixel_count);

    std::vector<bool> bits(pixel_count);
    for(auto i = 0; i < pixel_count; i++) {
        bits[i] = !!pixels[i];
    }

    g_object_unref(region);
    g_free(pixels);

    return bits;
}
