#include <vips/vips.h>

int main(int argc, char **argv) {

    VipsImage *in;
    VipsImage *out;

    /** Note that for the brevity and clarity of the code, all arguments are positional only.
     * If usage of the utility ever becomes complex enough that flag arguments would improve its quality,
     * getopt (https://www.gnu.org/software/libc/manual/html_node/Getopt.html) is likely the best option. (Argp is
     * another standard argument parsing library that has a higher level of abstraction but there's not a Mac port.)
     */
    if( argc != 15 ) {
        vips_error_exit( "usage: %s width height image output quality strip autorotate profile sigma x1 y2 y3 m1 m2", argv[0] );
    }

    // width and height of the image
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);

    // file paths for input (image) and output
    char *image = argv[3];
    char *output = argv[4];

    // argument for the output quality should be an integer 70-100, even though its type here is a string
    char *quality = argv[5];

    // whether to strip the metadata from the image
    int strip = atoi(argv[6]);

    // whether to autorotate the image based on its EXIF metadata
    int autorotate = atoi(argv[7]);

    // path to the ICC profile
    char *profile = argv[8];

    // parameters for the unsharp mask (see https://libvips.github.io/libvips/API/8.7/libvips-convolution.html#vips-sharpen)
    double sigma = atof(argv[9]);
    double x1 = atof(argv[10]);
    double y2 = atof(argv[11]);
    double y3 = atof(argv[12]);
    double m1 = atof(argv[13]);
    double m2 = atof(argv[14]);

    char output_and_options[300];
    if( strip == 1) {
        snprintf(output_and_options, 300, "%s[Q=%s%s", output, quality, ",strip,optimize_coding]");
    } else {
        snprintf(output_and_options, 300, "%s[Q=%s%s", output, quality, ",optimize_coding]");
    }

    VIPS_INIT( argv[0] );

    // get a VIPS Image in memory from the input file
    in = vips_image_new_from_file( image, NULL );

    if(strcmp("NONE", profile) != 0) {
        vips_thumbnail(image, &in, width, "height", height, "no_rotate", !autorotate, NULL);
    } else {
        vips_thumbnail(image, &in, width, "height", height, "no_rotate", !autorotate, "export_profile", profile, NULL);
    }

    vips_sharpen(in, &out, "sigma", sigma, "x1", x1, "y2", y2, "y3", y3, "m1", m1, "m2", m2, NULL );
    g_object_unref( in );

    // NOTE: if we wanted to add some extra processing (e.g., calculate an image hash) this would be the place to do it.

    vips_image_write_to_file(out, output_and_options, NULL );
    g_object_unref( out );

    return( 0 );
}