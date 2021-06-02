#include <vips/vips.h>

// argc: number of command line arguments
// argv: an argc length array of null-terminated strings.
int main(int argc, char* argv[argc]) {

  VipsImage *in;
  VipsImage *out;

  // We use positional arguments for now, because the only time executable is called from the command line is when it's being tested.
  // GLib, a VIPS dependency,  has a pretty standard command-line argument parser that gives you all the unix command line functionality 
  // you're used to. There are also other standard options for dealing with command line arguments in C, e.g. getopt and argp.

  if (argc != 15)
    vips_error_exit("usage: %s width height image output quality strip autorotate profile sigma x1 y2 y3 m1 m2", argv[0]);

  const char* executableName = argv[0];

  // Width and height of the image
  const int width = atoi(argv[1]);
  const int height = atoi(argv[2]);

  // File paths for input (image) and output
  const char *imageFileName = argv[3];
  const char *outputFileName = argv[4];

  // Argument for the output quality should be an integer 70-100
  const char *quality = argv[5];
  const int qualityNum = atoi(quality);
  if (qualityNum < 70 || qualityNum > 100)
    vips_error_exit("Output image quality must be an integer in the interval [70, 100].");

  // Whether to strip the metadata from the image
  const int strip = atoi(argv[6]);

  // Whether to autorotate the image based on its EXIF metadata
  const int autorotate = atoi(argv[7]);

  // Path to the ICC profile
  const char *profile = argv[8];

  // Parameters for the unsharp mask (see https://libvips.github.io/libvips/API/8.7/libvips-convolution.html#vips-sharpen)
  const double sigma = atof(argv[9]);
  const double x1 = atof(argv[10]);
  const double y2 = atof(argv[11]);
  const double y3 = atof(argv[12]);
  const double m1 = atof(argv[13]);
  const double m2 = atof(argv[14]);

  // The format string for the output_and_options string.
  const char* format = "%s[Q=%s,%soptimize_coding]";

  // The string that tells vips to enable the strip option, or an empty string if strip is 0.
  const char* stripOpt = strip ? "strip," : "";

  // Compute the length that the output_and_options string needs to be to hold the format string plus the argument strings, minus the characters taken up by the format specifiers, plus 1 for the terminating null character '\0' that snprintf adds.
  const int n = (strlen(format) - 6) + strlen(outputFileName) + strlen(quality) + strlen(stripOpt) + 1;

  // Build the output and options string using a VLA.
  char output_and_options[n];
  snprintf(output_and_options, n, format, outputFileName, quality, stripOpt);

  // Function-like macro to initialize VIPS library.
  VIPS_INIT(executableName);

  // Get a VIPS Image in memory from the input file
  in = vips_image_new_from_file(imageFileName, NULL);

  // You can also give an "import_profile" named argument which will be used if 
  // the input image has no ICC profile, or if the profile embedded in the
  // input image is broken. If there is a default profile already the code
  // below could be rewritten to use it.

  if(strcmp("NONE", profile))
    // Profile doesn't exist! Don't use an ICC profile.
    vips_thumbnail(imageFileName, &in, width, "height", height, "no_rotate", !autorotate, NULL);
  else
    // Profile exists! Transform the image to the target colourspace defined by the ICC profile before writing.
    vips_thumbnail(imageFileName, &in, width, "height", height, "no_rotate", !autorotate, "export_profile", profile, NULL);


  // Sharpen the image using the given arguments. Note the NULL-terminated named optional argument list. 
  // This is a common pattern in VIPS / GObject.
  vips_sharpen(in, &out, "sigma", sigma, "x1", x1, "y2", y2, "y3", y3, "m1", m1, "m2", m2, NULL);

  // All done with the input image, so free it.
  g_object_unref(in);


  // NOTE: If we wanted to add some extra processing (e.g. calculate an image hash) this would be the place to do it.


  // Write image to file.
  vips_image_write_to_file(out, output_and_options, NULL);

  // Free output image.
  g_object_unref(out);

  // VIPS_INIT() tells atexit() to run vips_shutdown() at exit if the platform supports it, which ours does.
  //vips_shutdown();

  // All good main functions return EXIT_SUCCESS.
  return(EXIT_SUCCESS);

}
