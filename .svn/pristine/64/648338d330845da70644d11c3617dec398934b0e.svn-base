#!/bin/bash
##############################################################################
# test_grid.sh - generate CSV file of execution times for specified
#                algorithm flavors and B-spline grid configurations
# 
# James Shackleford (tshack@drexel.edu)
# Last updated: Oct 25th, 2010
##############################################################################


# Configuration & Test Parameters
######################################
# Name of CSV output file
outfile="./avarice_mi_grid.csv"

# Location of bspline executable
bspline='/home/tshack/src/plastimatch/trunk/build/bspline'

# Location of synthetic_mha executable
synth_mha='/home/tshack/src/plastimatch/trunk/build/synthetic_mha'

# Test volume size
vol_size=260

# Similarity Metric (mse, mi)
metric='mi'

# Hardware (cpu, cuda, etc)
hardware='cpu'

# Algorithm flavors to test
flavors=( c d e )

# GPU Selection
gpuid='0'

# Grids to test
# Note: Grids are cubic
#      (i.e. 10 x 10 x 10)
min_grid=10
max_grid=100
grid_step=5
######################################









# ----- Please do not edit below this line. --------------------------


# Internal global variables
# (do not edit this, please)
test_vol_fix='./grid_fix_tmp.mha'
test_vol_mov='./grid_mov_tmp.mha'
machine=''

# Get CPU model and # of cores
# (Does not discriminate between real & HyperThreaded CPUs)
get_machine ()
{
    local cpu_count=$(cat /proc/cpuinfo | grep 'model name' | sed -e 's/.*: //' | wc -l)
    machine="$(cat /proc/cpuinfo | grep 'model name' | sed -e 's/.*: //' | uniq) ($cpu_count core)"
}

# Clean up temp volumes if user terminates
# early via ctrl-c
trap ctrl_c SIGINT
ctrl_c ()
{
    echo
    echo
    echo "Caught signal Ctrl-C!"
    echo "Cleaning up temp files"

    if [ -f $test_vol_fix ]
    then
        rm $test_vol_fix
    fi

    if [ -f $test_vol_mov ]
    then
        rm $test_vol_mov
    fi

    echo "Exiting..."
    echo
    exit
}

# Generates synthetic fixed and moving mha
# format volumes for the speed test
generate_volumes ()
{
    local offset=$(echo "$vol_size * 0.10" | bc);    # Offset for moving volume

    $synth_mha --output=$test_vol_fix --pattern=gauss --gauss-center="0 0 0" --dim="$vol_size";
    $synth_mha --output=$test_vol_mov --pattern=gauss --gauss-center="$offset $offset $offset" --dim="$vol_size";
}


disp_settings ()
{
    local tmp=""

    # Enumerate flavors to test
    for f in $(seq 0 $((${#flavors[@]} - 1)))
    do
        tmp=$tmp${flavors[$f]}' '
    done

    # Display banner
    echo "B-spline Grid Test v0.3"
    echo "  Machine : $machine"
    echo "  Hardware: $hardware"
    echo "  Metric  : $metric"
    echo "  Flavors : ${#flavors[@]} ( $tmp)"
    echo "  Grid    : $min_grid to $max_grid (step $grid_step)"
    echo "  Volumes : $vol_size x $vol_size x $vol_size"
    echo
}

check_input ()
{
    echo Sanity Check:

    # check for bspline executable existance
    if [ -f $bspline ]
    then
        echo " * bspline executable       [  OK  ]"
    else
        echo " * bspline executable       [ FAIL ]"
        echo
        echo Could not find bspline executable at specified location:
        echo \'$bspline\'
        echo Test Aborted.
        echo
        exit
    fi

    # check for synthetic_mha executable existance
    if [ -f $synth_mha ]
    then
        echo " * synthetic_mha executable [  OK  ]"
    else
        echo " * synthetic_mha executable [ FAIL ]"
        echo
        echo Could not find synthetic_mha executable at specified location:
        echo \'$synth_mha\'
        echo Test Aborted.
        echo
        exit
    fi
}


# check to see if output file exists
# if so, ask user what to do
check_output ()
{
    if [ -f $outfile ]
    then
        echo
        echo Output file \'$outfile\' exists!
        read -p "Delete? (y/n) "

        # convert response to lowercase
        if [ "${REPLY,,}" == "y" ]
        then
            rm $outfile
            echo
            echo \'$outfile\' deleted.
            echo
        else
            echo
            echo \'$outfile\' not deleted.
            echo Test Aborted.
            echo
            exit
        fi
    fi
}

# check input and output file validity
get_machine
disp_settings
check_input
check_output

echo
echo "Generating synthetic test volumes..."
generate_volumes

echo
echo "Starting Test..."

# Insert comment into CSV indicating test type
echo "#B-spline execution time vs control grid size" >> $outfile
echo "#volume size is constant: $vol_size x $vol_size x $vol_size" >> $outfile
echo "#$machine" >> $outfile
echo "#" >> $outfile

# Print the field header to CSV file
out="#grid,"
for f in $(seq 0 $((${#flavors[@]} - 1)))
do
    out=$out${flavors[$f]}','
done
echo $out >> $outfile

# Get times for each grid size from
# $min_grid to $max_grid in $grid_step steps
for i in $(seq $min_grid $grid_step $max_grid)
do
    # give user some feedback
    echo "Timing" $hardware "for grid $i x $i x $i"

    # start each csv row with the grid size
    out=$i','

    # for each cpu flavor we are testing
    for f in $(seq 0 $((${#flavors[@]} - 1)))
    do
        # probably a better way of parsing the output to find
        # the execution time, but right now I simply look for
        # the surrounding square bracket [
        tmp=`$bspline -A $hardware -G $gpuid -f ${flavors[$f]} -M $metric -m 0 -s "$i $i $i" $test_vol_fix $test_vol_mov | grep "\[" | awk 'BEGIN{} {print $12}'`
        out=$out$tmp','
    done

    # Write to CSV file
    echo $out >> $outfile
done

echo "Deleting temporary synthetic volumes..."
rm $test_vol_fix $test_vol_mov

echo "All tests complete!"
echo
