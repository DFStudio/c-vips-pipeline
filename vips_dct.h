//
// Created by Pierce Corcoran on 12/8/21.
//

#ifndef VIPS_TOOL_VIPS_DCT_H
#define VIPS_TOOL_VIPS_DCT_H

#include <vips/vips.h>
#include <gobject/gtype.h>

#if __cplusplus
extern "C" {
#endif

GType vips_dct_get_type();

int vips_dct(VipsImage *in, VipsImage **out, ...);

#if __cplusplus
};
#endif


#endif //VIPS_TOOL_VIPS_DCT_H
