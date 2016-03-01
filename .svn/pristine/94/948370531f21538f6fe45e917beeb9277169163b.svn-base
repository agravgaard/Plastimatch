#####################################################################
##     CONFIGURATION SECTION
#####################################################################

# How many seconds to sleep each loop
$sleep_len = 1;

# How many "sleep_len" ticks needed before xfer is complete
$timeout_ticks = 3;

# Input & output data directories
$conquest_data_dir = "/usr/local/conquest/data";
$deform_data_dir = "/usr/local/deform/data";

# CTTOOLS scripts directory
$cttools_dir = "/home/gcs6/projects/cttools";

#####################################################################
##     handle_directory_change
#####################################################################
sub handle_directory_change {
    my ($d) = @_;
    `perl "$cttools_dir/conquest_split.pl" "$conquest_data_dir/$d" "$deform_data_dir/$d/dicom"`;
}

#####################################################################
##     MAIN
#####################################################################
foreach $d (`ls $conquest_data_dir`) {
    chomp ($d);
    $finished{$d} = 1;
}

while (1) {
    ## Sleep until next polling inteval
    `sleep $sleep_len`;

    ## Look for new directories
    $actively_checking = 0;
    foreach $d (`ls $conquest_data_dir`) {
	chomp ($d);
	next if (exists $finished{$d});
	if (exists $checking{$d}) {
	    ## Existing directory, check if it's still being written to
	    $tmp = `ls $conquest_data_dir/$d`;
	    if ($tmp eq $checking{$d}) {
		## Didn't find any new files
		print "$d: no change $checking_timeout{$d}\n";
		if ($checking_timeout{$d} == 0) {
		    &handle_directory_change ($d);
		    $finished{$d} = 1;
		} else {
		    $actively_checking = 1;
		    $checking_timeout{$d} --;
		}
	    } else {
		## Found new file in directory
		print "$d: changed\n";
		$checking{$d} = $tmp;
		$checking_timeout{$d} = $timeout_ticks;
		$actively_checking = 1;
	    }
	} else {
	    ## New directory
	    print "$d: new\n";
	    $checking{$d} = `ls $conquest_data_dir/$d`;
	    $checking_timeout{$d} = $timeout_ticks;
	    $actively_checking = 1;
	}
    }

    ## If not actively checking, purge checking data structures 
    ## This allows us to clean up if data directories were deleted 
    ## through file system operations, etc.
    if (! $actively_checking) {
	undef $checking;
	undef $checking_timeout;
    }
}


## `/home/ziji/cttools/propagate_contours.pl $parent_dir_of_t0_t1_etc`;
