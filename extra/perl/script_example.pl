# This example shows how to register a group of images in batch

# The list of images to register
@fixed_images = 
  (
   "fixed_image_1.mha",
   "c:\\fixed_image_2.mha",
   "/path/to/fixed_image_3.mha",
  );
@moving_images = 
  (
   "/path/to/moving_image_1.mha",
   "c:\\moving_image_2.mha",
   "moving_image_3.mha",
  );

# A template for the command file
$template = <<EODATA
[GLOBAL]
fixed=FIXED
moving=MOVING
[STAGE]
xform=bspline
EODATA
  ;

# Loop through the list of fixed and moving images
for $f (@fixed_images) {
    $m = shift @moving_images;

    # Substitute the image filenames into the template
    $t = $template;
    $t =~ s/FIXED/$f/;
    $t =~ s/MOVING/$m/;

    # Write the command file
    open FH, ">", "parameter_file.txt";
    print FH $t;
    close FH;

    # Run plastimatch on the command file
    $cmd = "plastimatch register parameter_file.txt";
    system ($cmd);
}
