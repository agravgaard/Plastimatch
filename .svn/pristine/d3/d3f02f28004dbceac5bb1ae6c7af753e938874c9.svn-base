#!/bin/bash
##############################################################################
# test_size.sh - generate CSV file of execution times for specified
#                algorithm flavors and volumes sizes
# 
# James Shackleford (tshack@drexel.edu)
##############################################################################


# Configuration & Test Parameters
######################################
# Name of CSV output file
outfile="./cpu_mi_size.csv"
#outfile="./gpu_mi_size_zcpy.csv"

# Location of bspline executable
bspline='/home/jshack/src/plastimatch/trunk/build/bspline'

# Location of synthetic_mha executable
synth_mha='/home/jshack/src/plastimatch/trunk/build/synthetic_mha'

# Test Volume sizes
min_size=100
max_size=300
size_step=100

# Similarity Metric (mse, mi)
metric='mse'
#metric='mi'

# Hardware (cpu, cuda, etc)
hardware='cpu'
#hardware='cuda'

# Algorithm flavors to test
flavors=( c k )
#flavors=( a )

# GPU Selection
gpuid='0'

# Bspline Grid (is cubic)
# (i.e. 10 x 10 x 10)
grid=15



# Automatically set parameters for known machines
######################################
hostmachine=`uname -n`
case "$hostmachine" in
    "sherbert" \
    | "gelato" \
    | "wormwood" \
    | "slumber" \
    | "redfish")
        # These machines have build directory in path
        bspline=bspline
	synth_mha='plastimatch synth'
	;;
    *)
        # Unknown machine.  Use settings from above
    ;;
esac


# ----- Please do not edit below this line. --------------------------

# Internal global variables
# (do not edit this, please)
test_vol_fix='./size_fix_tmp.mha'
test_vol_mov='./size_mov_tmp.mha'
vol_size=0

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

#    if [ -f $test_vol_fix ]
#    then
#        rm $test_vol_fix
#    fi

#    if [ -f $test_vol_mov ]
#    then
#        rm $test_vol_mov
#	;
#    fi

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
    echo "B-spline Volume Size Test v0.4"
    echo "  Machine : $machine"
    echo "  Hardware: $hardware"
    echo "  Metric  : $metric"
    echo "  Flavors : ${#flavors[@]} ( $tmp)"
    echo "  Grid    : $grid x $grid x $grid"
    echo "  Volumes : $min_size to $max_size (step $size_step)"
    echo
}

check_input ()
{
    echo Sanity Check:

    # GCS: The below tests are not good, because the disallow using 
    # the bspline that is in my path.
    if [ `false` ]; then

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
    fi

    # GCS: So instead, I suggest the below.  Both bspline and synthetic_mha 
    # will return errno of 1 if executed with no arguments.  If command 
    # not found, the OS will return errno of 127.
    foo=`$bspline 2> /dev/null`
    if [ $? = 1 ]; then
        echo " * bspline executable [  OK  ]"
    else
        echo " * bspline executable       [ FAIL ]"
        echo
        echo Could not find bspline executable at specified location:
        echo \'$bspline\'
        echo Test Aborted.
        echo
        exit
    fi
    foo=`$synth_mha 2> /dev/null`
    if [ $? = 1 ]; then
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
echo "Starting Test..."

# Insert comment into CSV indicating test type
echo "#B-spline execution time vs volume size" >> $outfile
echo "#grid size is constant: $grid x $grid x $grid" >> $outfile
echo "#$machine" >> $outfile
echo "#" >> $outfile

# Print the field header to CSV file
out="#vol_size,"
for f in $(seq 0 $((${#flavors[@]} - 1)))
do
    out=$out${flavors[$f]}','
done
echo $out >> $outfile

# Get times for each volume size from
# $min_size to $max_ize in $size_step steps
for vol_size in $(seq $min_size $size_step $max_size)
do
    # give user some feedback
    echo "Generating volumes: $vol_size x $vol_size x $vol_size"

    # generate synthetic fixed and moving volumes
    generate_volumes

    # give user some more feedback
    echo "Timing" $hardware "for volume $vol_size x $vol_size x $vol_size"

    # start each csv row with the volume size
    out=$vol_size','

    # for each cpu flavor we are testing
    for f in $(seq 0 $((${#flavors[@]} - 1)))
    do
        # probably a better way of parsing the output to find
        # the execution time, but right now I simply look for
        # the surrounding square bracket [
#        tmp=`$bspline -A $hardware -G $gpuid -f ${flavors[$f]} -M $metric -m 0 -s "$grid $grid $grid" $test_vol_fix $test_vol_mov | grep "\[" | awk 'BEGIN{} {print $12}'`
        tmp=`$bspline -A $hardware -G $gpuid -f ${flavors[$f]} -M $metric -m 0 -s "$grid $grid $grid" $test_vol_fix $test_vol_mov | grep "\[" | awk 'BEGIN{} {print $12}'`
	echo $bspline -A $hardware -G $gpuid -f ${flavors[$f]} -M $metric -m 0 -s "$grid $grid $grid" $test_vol_fix $test_vol_mov
        out=$out$tmp','
    done
    echo # just a blank line for view pleasure

    # Write to CSV file
    echo $out >> $outfile

    # delete temp volumes
    rm $test_vol_fix $test_vol_mov
done

echo "All tests complete!"
echo
