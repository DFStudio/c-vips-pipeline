//
// Created by Pierce Corcoran on 8/26/21.
//

#ifndef VIPS_SCALE_UNSHARP_H
#define VIPS_SCALE_UNSHARP_H

#include <vips/vips.h>
#include <gobject/gtype.h>

#if __cplusplus
extern "C" {
#endif

GType unsharp_get_type();

int unsharp(VipsImage *in, VipsImage *blur, VipsImage **out, ...);

#if __cplusplus
};
#endif

#endif //VIPS_SCALE_UNSHARP_H
