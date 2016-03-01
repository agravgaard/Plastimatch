# $d = "\\gsharp\\idata\\isocal\\2006-05-26";
# $d = "\\gsharp\\idata\\gpuit-data\\2006-05-26\\0005a";
# $d = "\\gsharp\\idata\\gpuit-data\\2006-05-26\\0005c";
# $d = "\\gsharp\\idata\\gpuit-data\\2006-12-11\\0000b";
# $d = "\\gsharp\\idata\\gpuit-data\\2007-02-08\\0002";
$d = "g:\\reality\\2008-03-13";
$indir = "$d\\0000-small-filt";
$outdir = "$d\\0000-final";

`mkdir $outdir`;

$old_ino = -1;
for $f (`cmd /c dir /B $indir\\*.pfm`) {
    chomp($f);
    $f =~ m/0_([0-9]*)/;
    $ino = $1;
    if ($old_ino != -1 && $old_ino+1 != $ino) {
	printf ("Warning: dropped ino = %d\n", $old_ino+1);
    }
    $f2 = sprintf ("out_%04d.pfm",$ino);
    $old_ino = $ino;
    print "copy $indir\\$f $outdir\\$f2\n";
    print `copy $indir\\$f $outdir\\$f2`;
}
