
$drr_c = "c:\\gsharp\\build\\gpuit-vs2005\\release\\drr_c.exe";

$num_projections = 23;
$pi = 3.14159;

for $i (1..$num_projections) {
    $ipos = 2 * sin (2 * $pi * $i / $num_projections);
    $ipos = (sprintf "%d", (10*$ipos)) / 10;
    print "$i $ipos\n";
    $cmd = "$drr_c myinput_$ipos.mha output_$i.raw";
    print "$cmd\n";
#    print `$cmd`;
}


