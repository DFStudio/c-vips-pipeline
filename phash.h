//
// Created by Pierce Corcoran on 12/8/21.
//

#ifndef VIPS_TOOL_PHASH_H
#define VIPS_TOOL_PHASH_H

#include <vector>
#include <vips/vips.h>
#include <vips/vips8>

struct PerceptualHash {
    int version;
    std::vector<bool> bits;
};

PerceptualHash pHash(const vips::VImage &image, int reduce_size, int sample_size);

#endif //VIPS_TOOL_PHASH_H
