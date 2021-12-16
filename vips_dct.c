//
// Created by Pierce Corcoran on 12/8/21.
//

#include "vips_dct.h"

typedef struct _VipsDct {
    VipsOperation parent_instance;

    VipsImage *in;
    VipsImage *out;

    int size;
    int columnar;
} VipsDct;

typedef struct _VipsDctClass {
    VipsOperationClass parent_class;

    /* No new class members needed for this op.
     */

} VipsDctClass;

G_DEFINE_TYPE(VipsDct, vips_dct, VIPS_TYPE_OPERATION);

static int vips_dct_generate(VipsRegion *or, void *vseq, void *a, void *b, gboolean *stop) {
    /* The area of the output region we have been asked to make.
     */
    VipsRect *r = &or->valid;

    /* The sequence value ... the thing returned by vips_start_one().
     */
    VipsRegion *in_region = (VipsRegion *) vseq;

    VipsDct *vips_dct = (VipsDct *) b;
    int size = vips_dct->size;

    VipsRect input_rect = vips_dct->columnar ? (VipsRect) {
            .left = r->left,
            .width = r->width,
            .top = 0,
            .height = size
    } : (VipsRect) {
            .left = 0,
            .width = size,
            .top = r->top,
            .height = r->height
    };

    int x, y, s;

    /* Request matching part of input region.
     */
    if (vips_region_prepare(in_region, &input_rect))
        return (-1);

    float square_factor = sqrtf(2 / (float)size);
    if(vips_dct->columnar) {
        for (y = r->top; y < r->top + r->height; y++) {
            float cos_factor = (float)y * 3.1415f / (float)size;
            for (x = r->left; x < r->left + r->width; x++) {
                float sum = 0;
                for (s = 0; s < size; s++) {
                    float sample = *(float *)VIPS_REGION_ADDR(in_region, x, s);
                    sum += sample * cosf(cos_factor * ((float)s + 0.5f));
                }
                sum *= square_factor;
                if(y == 0) {
                    sum *= 1 / sqrtf(2);
                }

                *(float *)VIPS_REGION_ADDR(or, x, y) = sum;
            }
        }
    } else {
        for (x = r->left; x < r->left + r->width; x++) {
            float cos_factor = (float)x * 3.1415f / (float)size;
            for (y = r->top; y < r->top + r->height; y++) {
                float sum = 0;
                for (s = 0; s < size; s++) {
                    float sample = *(float *)VIPS_REGION_ADDR(in_region, s, y);
                    sum += sample * cosf(cos_factor * ((float)s + 0.5f));
                }
                sum *= square_factor;
                if(x == 0) {
                    sum *= 1 / sqrtf(2);
                }

                *(float *)VIPS_REGION_ADDR(or, x, y) = sum;
            }
        }
    }

    return (0);
}

static int
vips_dct_build(VipsObject *object) {
    VipsObjectClass *class = VIPS_OBJECT_GET_CLASS(object);
    VipsDct *vips_dct = (VipsDct *) object;
    VipsImage **t = (VipsImage **) vips_object_local_array( object, 1 );

    if (VIPS_OBJECT_CLASS(vips_dct_parent_class)->build(object))
        return (-1);

    if (vips_check_bands(class->nickname, vips_dct->in, 1))
        return (-1);

    vips_dct->size = vips_dct->columnar ? vips_image_get_height(vips_dct->in) : vips_image_get_width(vips_dct->in);

    g_object_set(object, "out", vips_image_new(), NULL);
    vips_dct->out->BandFmt = VIPS_FORMAT_FLOAT;
    vips_dct->out->Xsize = vips_image_get_width(vips_dct->in);
    vips_dct->out->Ysize = vips_image_get_height(vips_dct->in);

    vips_cast_float(vips_dct->in, &t[0], NULL);

    if (vips_image_pipelinev(vips_dct->out,
                             VIPS_DEMAND_STYLE_ANY, t[0], NULL))
        return (-1);

    if (vips_image_generate(vips_dct->out,
                            vips_start_one,
                            vips_dct_generate,
                            vips_stop_one,
                            t[0],
                            vips_dct))
        return (-1);

    return (0);
}

static void
vips_dct_class_init(VipsDctClass *class) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(class);
    VipsObjectClass *object_class = VIPS_OBJECT_CLASS(class);

    gobject_class->set_property = vips_object_set_property;
    gobject_class->get_property = vips_object_get_property;

    object_class->nickname = "vips_dct";
    object_class->description = "Discrete cosine transform";
    object_class->build = vips_dct_build;

    VIPS_ARG_IMAGE(class, "in", 1,
                   "Input",
                   "Input image",
                   VIPS_ARGUMENT_REQUIRED_INPUT,
                   G_STRUCT_OFFSET(VipsDct, in));

    VIPS_ARG_IMAGE(class, "out", 3,
                   "Output",
                   "Output image",
                   VIPS_ARGUMENT_REQUIRED_OUTPUT,
                   G_STRUCT_OFFSET(VipsDct, out));

    VIPS_ARG_BOOL(class, "columnar", 4,
                    "Columnar",
                    "Compute columns",
                    VIPS_ARGUMENT_OPTIONAL_INPUT,
                    G_STRUCT_OFFSET(VipsDct, columnar),
                    FALSE);
}

static void
vips_dct_init(VipsDct *vips_dct) {
    vips_dct->columnar = FALSE;
}

/**
 * vips_dct: (method)
 * @in: input image
 * @out: (out): output image
 * @...: %NULL-terminated list of optional named arguments
 */
int vips_dct(VipsImage *in, VipsImage **out, ...) {
    va_list ap;
    int result;

    va_start(ap, out);
    result = vips_call_split("vips_dct", ap, in, out);
    va_end(ap);

    return (result);
}
