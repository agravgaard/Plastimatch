#! /usr/bin/perl

if ($#ARGV < 0) {
    die "Usage: examine_xvi_directory.pl infile\n";
}

$infile = shift;

$min_count = 100;
#$max_date = 20130701;
#$max_date = 20131001;
#$max_date = 20131114;
$max_date = `date --date="-1 month" +"%Y%m%d"`;

open (FP, "<$infile") or die "Couldn't open file $infile for read";
while (<FP>) {
    chomp;
    if (m/^ Directory.*patient_([^\\]*)/) {
	$patient_id = $1;
	next;
    }
    if (m/^([^ ]*).*\.his/) {
	#printf ("Matched patient $patient_id\n");
	$date = $1;
	($mo,$dy,$yr) = split ('/', $date);
	#print "$date -> $yr-$mo-$dy\n";
	$date = "$yr$mo$dy";
	$pat_count{$patient_id} ++;
	if (!$pat_date{$patient_id}) {
	    $pat_date{$patient_id} = $date;
	    next;
	}
	if ($pat_date{$patient_id} < $date) {
	    $pat_date{$patient_id} = $date;
	}
    }
}

for (keys %pat_date) {
    $count = $pat_count{$_};
    $date = $pat_date{$_};
    if ($count > $min_count and $date < $max_date) {
	print "$_ $pat_date{$_} $pat_count{$_}\n";
    }
}
