#!/usr/bin/perl

$header = <<EODATA
MGH_GPUIT_BSP <experimental>
img_origin = -187.500000 -187.500000 -187.500000
img_spacing = .01 .01 .01
img_dim = 100 100 100
roi_offset = 0 0 0
roi_dim = 100 100 100
vox_per_rgn = 100 100 100
EODATA
  ;
print $header;

for $xyz (0..2) {
for $z (1..4) {
for $y (1..4) {
for $x (0..3) {
    if ($xyz == 0) {
	print $x * $x, "\n";
    } elsif ($xyz == 1) {
	print "1\n";
    } else {
	print "1\n";
    }
}
}
}
}
