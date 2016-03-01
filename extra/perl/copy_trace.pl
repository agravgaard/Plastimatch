print "Enter new id: ";
$new_id = <>;
chomp($new_id);

print "Enter old id: ";
$id = <>;
chomp($id);

@locs = `find /c/gsharp/data/4dct-traces -name "${id}*.vxp" -print`;
print @locs;

print "Enter file # to copy: ";
$fileno = <>;
chomp($fileno);
## $rpmfile = chomp($locs[$fileno]);
$rpmfile = $locs[$fileno];
chomp($rpmfile);

print "---\n";
print "$rpmfile\n";

$newdir = sprintf ("/g/reality/new-data/%04d/rpm-4dct", $new_id);
print "---\n";
print "ls $newdir\n";
print `ls $newdir`;
print "---\n";

print "Press enter to continue\n";
$wait = <>;

print `mkdir $newdir`;
print `cp $rpmfile $newdir`;
