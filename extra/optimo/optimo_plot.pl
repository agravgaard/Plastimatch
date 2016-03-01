#!/usr/bin/perl
use File::Copy;
use File::Find;
use File::Path;
use File::Spec;
use File::Spec::Functions;

# SVG::TT::Graph sucks.  It doesn't let you use a numeric x axis.
# SVG::Graph also sucks.  Maybe it could work and I'm just not smart enough.
# Chart::Clicker is great.  The only thing it's missing is semilog axes.
# There is another one called GD::Graph.  I didn't try it yet.
use Chart::Clicker;
use Chart::Clicker::Data::Series;
use Chart::Clicker::Data::DataSet;

use POSIX qw(FLT_MAX DBL_MAX);

$scratch_base = "/dosf/scratch/optimo/steepest";

#########################################################################
#  Main
#########################################################################
my @list = sort readdir(DIR);
find (sub { push @foundfiles, $File::Find::name if /-log.txt$/ },
      $scratch_base);
#print join("\n",@foundfiles), "\n";

my $cc = Chart::Clicker->new;

for $file (@foundfiles) {

    # Gather the values from files
    my @keys = ();
    my @values = ();
    my $it = 0;
    my $last_val = FLT_MAX;
    open FP, "<$file";
    for $_ (grep /MSE/, <FP>) {
	/MSE\s+([^\s]+)/ or die "couldn't parse input file\n$_";
	$val = $1;
	if ($val > $last_val) { $val = $last_val; }
	$last_val = $val;
	push @keys, $it;
	push @values, $val;
	$it++;
    }

    print join (" ", @keys), "\n";
    print join (" ", @values), "\n";

    # normalize the series
    $normalize = 0;
    if ($normalize) {
	my $vmax = $values[0];
	my $vmin = $values[-1];
	@values = map { ($_ - $vmin) / ($vmax - $vmin) } @values;
	@values = map { log ($_ + 1) / log (2) } @values;
    }

    # add the array of series data to the dataset
    my $series = Chart::Clicker::Data::Series->new(
        keys => \@keys, values => \@values
    );
    my $dataset = Chart::Clicker::Data::DataSet->new(
        series  => [ $series ]
    );

    $cc->add_to_datasets ($dataset);

    close (FP);
#    last;
}

# Set the axes // there is a function "tick_values" that could be used too
$cc->get_context('default')->range_axis->range->min(0);
$cc->get_context('default')->range_axis->range->max(500000);

$cc->write_output('foo.png');
