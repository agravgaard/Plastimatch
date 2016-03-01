#!/usr/bin/perl

for $d (60..130) {
    $cmd = sprintf ("bragg_curve -z 120 -E %d -e 50 -O peak_%03d.txt", $d, $d);
    print "$cmd\n";
    print `$cmd`;
}
