#! /usr/bin/perl

@sizes = ( 100, 200, 300 );
#@sizes = ( 100 );
@flavors = ( c, l, k );
@flavors = ( c, 'm' );
#@flavors = ( 'm' );

foreach $s (@sizes) {
    $cmd = "plastimatch synth --output size_test_fixed.mha --pattern gauss --gauss-center \"0 0 0\" --dim=$s";
    `$cmd`;
    $offset = $s * 0.1;
    $cmd = "plastimatch synth --output size_test_moving.mha --pattern gauss --gauss-center \"$offset $offset 0\" --dim=$s";
    `$cmd`;

    # Throw one away
    $f = @flavors[0];
    $cmd = "bspline -A cpu -G 0 -f $f -M mse -m 0 -s \"15 15 15\" ./size_test_fixed.mha ./size_test_moving.mha";
    `$cmd`;

    foreach $repeat (0..2) {
    foreach $f (@flavors) {
	$cmd = "bspline -A cpu -G 0 -f $f -M mse -m 1 -s \"15 15 15\" ./size_test_fixed.mha ./size_test_moving.mha";
	open (CMD, "$cmd|");
	$timer = -1;
	while (<CMD>) {
	    /^\[.*\[\s*(\S*)\s*s/ or next;
	    $timer = $1;
	}
	print "$s,$f,$timer\n";
    }
    }
}

