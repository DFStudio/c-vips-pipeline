#include <vips/vips.h>

int main(__unused int argc, char **argv) {
    VipsImage *in;
    VipsImage *out;

//    if( argc != 6 )
//        vips_error_exit( "usage: %s width height image output quality", argv[0] );
//    int width = atoi(argv[1]);
//    int height = atoi(argv[2]);
//    char *image = argv[3];
//    char *output = argv[4];
//    int quality = atoi(argv[5]);

    int width = 1800;
    int height = 1122;
    char *image = "1scalesource.png";
    char *output = "result.jpg";
    int quality = 85;

    double sigma = 0.5;
    double x1 = 50;

    VIPS_INIT( argv[0] );

    in = vips_image_new_from_file( image, NULL );

    vips_thumbnail(image, &in, width, "height", height, NULL);

    vips_sharpen(in, &out, "sigma", sigma, "x1", x1, NULL );
    g_object_unref( in );

    vips_image_write_to_file(out, output, NULL );
    g_object_unref( out );

    return( 0 );
}