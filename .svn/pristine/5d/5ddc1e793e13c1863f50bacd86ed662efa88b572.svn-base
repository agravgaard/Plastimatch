#! /usr/bin/perl

my @group_a = 
  (
   "0522c0001",
   "0522c0002",
   "0522c0003",
   "0522c0009",
   "0522c0013",
   "0522c0014",
   "0522c0017",
   "0522c0057",
   "0522c0070",
   "0522c0077",
   "0522c0079"
);

@group_b = 
  (
   "0522c0081",
   "0522c0125",
   "0522c0132",
   "0522c0147",
   "0522c0149",
   "0522c0159",
   "0522c0161",
   "0522c0190",
   "0522c0195",
   "0522c0226",
   "0522c0248"
);

@group_c = 
  (
   "0522c0251",
   "0522c0253",
   "0522c0328",
   "0522c0329",
   "0522c0330",
   "0522c0427",
   "0522c0433",
   "0522c0441",
   "0522c0455",
   "0522c0457",
   "0522c0479",
);

sub sort_group {
    my ($array_ref, $group, $suffix) = @_;
    $cmd = "cat ";
    foreach my $item (@$array_ref) {
	$cmd .= " ${item}${suffix}.txt";
    }
    $cmd .= " | sed -e 's/ / | /'";
    $cmd .= " | shuf | gzip > ${group}${suffix}.txt.gz";
    print "$cmd\n";
    system ($cmd);
}

sort_group (\@group_a, "group_a", "-ml-sorted");
sort_group (\@group_b, "group_b", "-ml-sorted");
sort_group (\@group_c, "group_c", "-ml-sorted");
