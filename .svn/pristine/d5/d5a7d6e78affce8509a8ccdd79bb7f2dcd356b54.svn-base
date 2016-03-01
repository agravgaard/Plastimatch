#! /usr/bin/perl

######################################################################
##  This script is intended to be automatically run by conquest dicom
##  upon receipt of a new patient study.  The script will then run
##  the mabs program to do segmentation, and then forward the results
##  to any separate location, such as contouring workstation.
##
##  The web page for conquest dicom is at:
##    http://ingenium.home.xs4all.nl/dicom.html
##
##  To use this script, you need two adjustments to dicom.ini
##    1. Set this script as an import converter.  For example:
##       ImportConverter0         = process study after 10 by /usr/bin/perl /Full/path/to/mabs_series.pl %P
##
##    2. Use a full path for the MAG0 location.  For example:
##       MAGDevice0               = /Full/path/to/data/
##
##  Also, you must set up your mabs working directory.
##  (INSTRUCTIONS TO BE WRITTEN.)
##
##  Finally, adjust the settings below to match your conquest installation.
##
######################################################################

# If necessary, modify these for your conquest/mabs installation
$conquest_program = dgate;
$conquest_port = 5678;
$plastimatch_command = plastimatch;

# Get path to patient directory from conquest import converter
$pid_path = shift;
$pid = $pid_path;
$pid =~ s/^.*\///;
print "$pid\n";

# Run the mabs command
$cmd="${plastimatch_command} mabs --xxx";

# Delete patient from conquest
# Version 1.4.17d has a bug, requires trailing "*" to workaround
$cmd = "${conquest_program} -p${conquest_port} \"--deletepatient:${pid}*\"";
print "$cmd\n";
system ($cmd);
