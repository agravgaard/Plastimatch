#$indir = "/c/gsharp/idata/4dct-traces/all_traces_3";
$indir = ".";
$count = 0;

# If rename = 0, overwrite existing file
$rename = 0;

for $file (`ls $indir/*.vxp`) {
    chomp($file);
    $base = $file;
    $base =~ s/[^\/]*$//;
    if ($file =~ /([0-9]+_[0-9]+)_([0-9]+)([^0-9].*)/) {
	$id = $1;
	$tag = $2;
	$tail = $3;
    } else {
	$id = $file;
	$tag = 0;
	$tail = ".UNK.vxp";
    }

    if (defined ($anonhash{$id})) {
	$anon_id = $anonhash{$id};
    } else {
	$anon_id = $count;
	$anonhash{$id} = $anon_id;
	$count ++;
    }

    $outfile = "${anon_id}_${tag}${tail}";

    print "$file\n";
    print "id = $id anon_id = $anon_id OUTFILE = $outfile\n";

    open FPI, "<$file";
    open FPO, ">$outfile";
    while (<FPI>) {
	if (/^CRC/) {
	    print FPO "CRC=00000\n";
	    next;
	}
	if (/^Patient_ID/) {
	    print FPO "Patient_ID=0000000\n";
	    next;
	}
	if (/^Date/) {
	    print FPO "Date=01-01-2001\n";
	    next;
	}
	print FPO $_;
    }
    close FPI;
    close FPO;
    if (! $rename) {
	`mv $outfile $file`;
    }
    ## `rm $file`;
}
