//
// Created by Pierce Corcoran on 8/26/21.
//

#include "unsharp.h"

typedef struct _Unsharp {
    VipsOperation parent_instance;

    VipsImage *in;
    VipsImage *blur;
    VipsImage *out;

} Unsharp;

typedef struct _UnsharpClass {
    VipsOperationClass parent_class;

    /* No new class members needed for this op.
     */

} UnsharpClass;

G_DEFINE_TYPE( Unsharp, unsharp, VIPS_TYPE_OPERATION);

void *
my_many( VipsImage *out, void *a, void *b ) {
    VipsImage **images = a;
    VipsRegion **regions = vips_start_many(out, a, b);
    return regions;
}

static int unsharp_generate( VipsRegion *or, void *vseq, void *a, void *b, gboolean *stop )
{
    /* The area of the output region we have been asked to make.
     */
    VipsRect *r = &or->valid;

    /* The sequence value ... the thing returned by vips_start_one().
     */
    VipsRegion **input_regions = (VipsRegion **) vseq;
    VipsRegion *in_region = input_regions[0];
    VipsRegion *blur_region = input_regions[1];

    VipsImage **inputs = (VipsImage **) a;
    VipsImage *in = inputs[0];
    VipsImage *blur = inputs[1];

    Unsharp *unsharp = (Unsharp *) b;
    int line_size = r->width * unsharp->in->Bands;

    int x, y;

    /* Request matching part of input region.
     */
    if( vips_region_prepare( in_region, r ) )
        return( -1 );
    if( vips_region_prepare( blur_region, r ) )
        return( -1 );

    for( y = 0; y < r->height; y++ ) {
        unsigned char *in_bytes = (unsigned char *)
                VIPS_REGION_ADDR( in_region, r->left, r->top + y );
        unsigned char *blur_bytes = (unsigned char *)
                VIPS_REGION_ADDR( blur_region, r->left, r->top + y );
        unsigned char *out_bytes = (unsigned char *)
                VIPS_REGION_ADDR( or, r->left, r->top + y );

        for( x = 0; x < line_size; x++ ) {
            unsigned char in_pixel = in_bytes[x];
            unsigned char blur_pixel = blur_bytes[x];
            int sum = in_pixel + (in_pixel - blur_pixel) / 4;

            if(sum < 0) {
                sum = 0;
            } else if(sum > 255) {
                sum = 255;
            }

            out_bytes[x] = (unsigned char)sum;

//            if(in_pixel < blur_pixel) {
//                out_bytes[x] = in_pixel + (blur_pixel - in_pixel) / 2;
//            } else if(blur_pixel < in_pixel) {
//                out_bytes[x] = blur_pixel + (in_pixel - blur_pixel) / 2;
//            } else {
//                out_bytes[x] = in_pixel;
//            }
        }
    }

    return( 0 );
}

static int
unsharp_build( VipsObject *object )
{
    VipsObjectClass *class = VIPS_OBJECT_GET_CLASS( object );
    Unsharp *unsharp = (Unsharp *) object;

    if( VIPS_OBJECT_CLASS( unsharp_parent_class )->build( object ) )
        return( -1 );

    if( vips_check_uncoded( class->nickname, unsharp->in ) ||
        vips_check_format( class->nickname, unsharp->in, VIPS_FORMAT_UCHAR ) )
        return( -1 );

    if( vips_check_uncoded( class->nickname, unsharp->blur ) ||
        vips_check_format( class->nickname, unsharp->blur, VIPS_FORMAT_UCHAR ) )
        return( -1 );

    g_object_set( object, "out", vips_image_new(), NULL );

    if( vips_image_pipelinev( unsharp->out,
                              VIPS_DEMAND_STYLE_THINSTRIP, unsharp->in, unsharp->blur, NULL ) )
        return( -1 );

    if( vips_image_generate( unsharp->out,
                             my_many,
                             unsharp_generate,
                             vips_stop_many,
                             vips_allocate_input_array(unsharp->out, unsharp->in, unsharp->blur, NULL),
                             unsharp ) )
        return( -1 );

    return( 0 );
}

static void
unsharp_class_init( UnsharpClass *class )
{
    GObjectClass *gobject_class = G_OBJECT_CLASS( class );
    VipsObjectClass *object_class = VIPS_OBJECT_CLASS( class );

    gobject_class->set_property = vips_object_set_property;
    gobject_class->get_property = vips_object_get_property;

    object_class->nickname = "unsharp";
    object_class->description = "unsharp merge operation";
    object_class->build = unsharp_build;

    VIPS_ARG_IMAGE( class, "in", 1,
                    "Input",
                    "Input image",
                    VIPS_ARGUMENT_REQUIRED_INPUT,
                    G_STRUCT_OFFSET( Unsharp, in ) );

    VIPS_ARG_IMAGE( class, "blur", 2,
                    "Blur",
                    "Blurred image",
                    VIPS_ARGUMENT_REQUIRED_INPUT,
                    G_STRUCT_OFFSET( Unsharp, blur ) );

    VIPS_ARG_IMAGE( class, "out", 3,
                    "Output",
                    "Output image",
                    VIPS_ARGUMENT_REQUIRED_OUTPUT,
                    G_STRUCT_OFFSET( Unsharp, out ) );
}

static void
unsharp_init( Unsharp *unsharp )
{
}

/**
 * unsharp: (method)
 * @in: input image
 * @out: (out): output image
 * @...: %NULL-terminated list of optional named arguments
 */
int unsharp( VipsImage *in, VipsImage *blur, VipsImage **out, ... )
{
    va_list ap;
    int result;

    va_start( ap, out );
    result = vips_call_split( "unsharp", ap, in, blur, out );
    va_end( ap );

    unsharp_get_type();
    return( result );
}
