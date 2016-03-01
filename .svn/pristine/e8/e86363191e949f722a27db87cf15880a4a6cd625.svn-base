use File::Copy;
use File::Find;
use File::Path;
use File::Spec;
use File::Spec::Functions;

#$dropbox_dir = "c:/gcs6/My Dropbox";
$dropbox_dir = "/PHShome/gcs6/Dropbox";
#$in_base = "$dropbox_dir/autolabel/t-spine-in/rider-lung";
$in_base = "$dropbox_dir/autolabel/t-spine-in/rider-pilot";
$out_base = "$dropbox_dir/autolabel/t-spine-2";

$make_thumbnails = 0;
$make_reference_curves = 1;

sub parse_fcsv {
    ($fn) = @_;
    @locs = ();

    open FP, "<$fn";
    while (<FP>) {
	/^#/ and next;
	chomp;
	($label,$x,$y,$z,$rest) = split (/,/, $_, 5);
	push @locs, "$label,$z";
    }
    close FP;

    return @locs;
}

opendir (DIR, $in_base) or die "Can't opendir input directory $in_base: $!\n";
my @list = sort readdir(DIR);
for (@list){
   next if (not /\.fcsv$/);
   print "$_\n";

   $fcsv_fn = catfile ($in_base, $_);
   $fcsv_base = $fn_base = $_;
   $fn_base =~ s/\.fcsv$//;
   $img_in_fn = catfile ($in_base, $fn_base);
   $img_in_fn = $img_in_fn . "\.nrrd";

   $out_fn_base = catfile ($out_base, $fn_base);
   $out_fcsv = catfile ($out_base, $fcsv_base);

   copy ($fcsv_fn,$out_fcsv);

   if ($make_reference_curves) {
       $out_fn = $out_fcsv;
       $out_fn =~ s/fcsv$/txt/;
       open OF, ">$out_fn";
   }

   @locs = parse_fcsv ($fcsv_fn);
   for $loc (@locs) {
       ($label,$z) = split (/,/, $loc, 2);
       if ($make_thumbnails) {
	   $out_fn = $out_fn_base . "_${label}.mhd";
	   $cmd = "plastimatch thumbnail "
	     . "--input \"$img_in_fn\" --output \"$out_fn\" "
	     . "--thumbnail-dim 16 --thumbnail-spacing 25.0 --slice-loc $z";
	   print "$cmd\n";
	   print `$cmd`;
       }
       if ($make_reference_curves) {
	   $label_loc = $label;
	   $label_loc =~ s/P//;
	   print OF "$label_loc $z\n";
       }
   }
   if ($make_reference_curves) {
       close OF;
   }
}
