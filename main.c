#include <vips/vips.h>

int main(int argc, char **argv) {
    VipsImage *in;
    VipsImage *out;

    if( argc != 8 )
        vips_error_exit( "usage: %s width height image output quality sigma x1", argv[0] );

    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    char *image = argv[3];
    char *output = argv[4];
    char *quality = argv[5];
    double sigma = atof(argv[6]);
    double x1 = atof(argv[7]);

    char options[200];
    strcat(options, "[Q=");
    strcat(options, quality);
    strcat(options, ",optimize_coding]");

    char output_and_options[200];
    strcat(output_and_options, output);
    strcat(output_and_options, options);

    VIPS_INIT( argv[0] );

    in = vips_image_new_from_file( image, NULL );

    vips_thumbnail(image, &in, width, "height", height, NULL);

    vips_sharpen(in, &out, "sigma", sigma, "x1", x1, NULL );
    g_object_unref( in );

    // NOTE: if we wanted to add some extra processing (e.g., calculate an image hash) this would be the place to do it.

    vips_image_write_to_file(out, output_and_options, NULL );
    g_object_unref( out );

    return( 0 );
}