#!/bin/bash
##############################################################################
# automask.sh - Registers a set of input phases to a reference phase by
#               automatically generating plastimatch command files 
#               (according to a template) and executing them.  The
#               resulting transforms are then applied to the reference
#               phase mask to generate masks for each phase.  These phase
#               masks are then used to fill all voxels outside of the
#               mask with a user defined value for each respective phase.
# 
# James Shackleford (tshack@drexel.edu)
# Last updated: Nov 17th, 2011
##############################################################################


###############################################################################
#: USER OPTIONS
###############################################################################
#: TEMPLATE FILE
#:   Location of the plastimatch command template file
#  ---------------------------------------------------------------
TEMPLATE="/home/tshack/data/reg/0321/template.plm"

#: REFERENCE IMAGE
#:   The full path to reference phase, including file name
#  ---------------------------------------------------------------
REF="/home/tshack/data/reg/0321/0015.nrrd"
REF_MASK="/home/tshack/data/reg/0321/mask/0015_mask.nrrd"

#: PHASES
#:   The full path to registration phase directory
#  ---------------------------------------------------------------
PHASE_PATH="/home/tshack/data/reg/0321"
PHASE_FILTER="*.nrrd"

#: OUTPUT PATHS
#:   Warped, xform, and vector field output directories
#  ---------------------------------------------------------------
MASK_PATH="/home/tshack/data/reg/0321/mask"
FILL_PATH="/home/tshack/data/reg/0321/fill"

#: OPTIONS
#:  Use REF_MASK and fill non-masked areas with OPT_FILL_VALUE
#     if FALSE will only generate masks, no filling.
#  ---------------------------------------------------------------
OPT_FILL="TRUE"
OPT_FILL_VALUE="-1000"

#: OUTPUT FORMATS
#:   For warped images and vector fields
#  ---------------------------------------------------------------
OUTPUT_FORMAT_MASK="nrrd"
OUTPUT_FORMAT_FILL="nrrd"

#: OUTPUT SUFFIXES
#:   Added to end of filename, but before the extension
#  ---------------------------------------------------------------
SUFFIX_MASK="_mask"
SUFFIX_FILL="_fill"

###############################################################################








###############################################################################
#: DO NOT EDIT BELOW THIS LINE
###############################################################################

#: TOKENS
#:   These are used as variables in the plastimatch command file
#:   (You shouldn't need to edit these)
#  ---------------------------------------------------------------
FIXED_TOKEN="<REF>"
MOVING_TOKEN="<PHASE>"
VF_TOKEN="<VF>"
XFORM_TOKEN="<XFORM>"
WARP_TOKEN="<WARP>"
#  ---------------------------------------------------------------

TEMP_FILE="automask.tmp.$$"
ERROR=0

#: Trap CTRL-C so we can cleanup if needed
trap ctrl_c SIGINT
ctrl_c ()
{
    echo
    echo
    echo "Caught signal Ctrl-C!"
    echo "Cleaning up temp files"

    if [ -f $TEMP_FILE ]; then
        rm $TEMP_FILE
    fi
    if [ -f vf_tmp.mha ]; then
        rm vf_tmp.mha
    fi
    echo "Exiting..."
    echo
    exit
}

#: File/Directory validation
#  -----------------------------------------------
if [ ! -f $TEMPLATE ]; then
    echo "TEMPLATE: Command file template not found!"
    let "ERROR++"
fi

if [ ! -f $REF ]; then
    echo "REF: Reference phase not found!"
    let "ERROR++"
fi

if [ ! -f $REF_MASK ]; then
    echo "REF_MASK: Reference phase mask not found!"
    let "ERROR++"
fi

if [ ! -d $PHASE_PATH ]; then
    echo "PHASE_PATH: Invalid directory"
    let "ERROR++"
fi

if [ ! -d $MASK_PATH ]; then
    echo "MASK_PATH: Invalid directory"
    let "ERROR++"
fi

if [ $OPT_FILL = "TRUE" ]; then
    if [ ! -d $FILL_PATH ]; then
        echo "FILL_PATH: Invalid directory"
        let "ERROR++"
    fi
fi


if [ $ERROR != 0 ]; then
    echo
    echo "$ERROR Error(s) encountered."
    echo "  Please edit this script to resolve these issues."
    echo
    echo "Exiting..."
    echo
    exit
fi
#  -----------------------------------------------


PHASE_PATH="$PHASE_PATH/$PHASE_FILTER"


#: VALIDATE USER INTENTIONS
#  -----------------------------------------------
echo "============================================================"
echo " Automatic 4D Mask & Fill Helper Script                v0.01"
echo "============================================================"
echo "You want to..."
echo
echo "Register the phases:"
for f in $PHASE_PATH
do
    if [ $f != $REF ]; then
        echo "  $f"
    fi
done
echo
echo "To the reference phase:"
echo "  $REF"
echo
echo "Output Options:"
echo "           Fill: $OPT_FILL"
echo
echo "Output Paths:"
    echo "          Masks: $MASK_PATH"
if [ $OPT_FILL = "TRUE" ]; then
    echo "  Filled Phases: $FILL_PATH"
    echo "     Fill Value: $OPT_FILL_VALUE"
fi
echo

read -p "Correct? (y/n) "
if [ "${REPLY,,}" != "y" ]; then
    exit
fi
#  -----------------------------------------------


for f in $PHASE_PATH
do
    if [ $f != $REF ]; then
        filename="${f##*/}"
        filename="${filename%.[^.]*}"
        MASK_FILE="${MASK_PATH}/${filename}${SUFFIX_MASK}.${OUTPUT_FORMAT_MASK}"

        # Fix escape sequences
        f_ESC="${f//\//\\/}"
        REF_FILE_ESC="${REF//\//\\/}"

        # Build fill output path, if we are doing that
        if [ $OPT_FILL = "TRUE" ]; then
            FILL_FILE="${FILL_PATH}/${filename}${SUFFIX_FILL}.${OUTPUT_FORMAT_FILL}"
        fi

        # Build the command file for current phase
        cp $TEMPLATE $TEMP_FILE

        sed -i "s/$FIXED_TOKEN/${f_ESC}/g" "$TEMP_FILE"
        sed -i "s/$MOVING_TOKEN/${REF_FILE_ESC}/g" "$TEMP_FILE"
        sed -i "s/$VF_TOKEN/vf_tmp.mha/g" "$TEMP_FILE"
        sed -i "s/$XFORM_TOKEN//g" "$TEMP_FILE"
        sed -i "s/$WARP_TOKEN//g" "$TEMP_FILE"

        # have plastimatch do its thing
        plastimatch register $TEMP_FILE

        plastimatch warp --input "$REF_MASK" --output-img "$MASK_FILE" --xf "vf_tmp.mha" --interpolation nn

        if [ $OPT_FILL = "TRUE" ]; then
            plastimatch mask --input "$f" --output "$FILL_FILE" --mask-value $OPT_FILL_VALUE --mask "$MASK_FILE"
        fi

    fi
done

# Don't forget the reference phase
if [ $OPT_FILL = "TRUE" ]; then
    filename="${REF##*/}"
    filename="${filename%.[^.]*}"
    FILL_FILE="${FILL_PATH}/${filename}${SUFFIX_FILL}.${OUTPUT_FORMAT_FILL}"
    plastimatch mask --input "$REF" --output "$FILL_FILE" --mask-value $OPT_FILL_VALUE --mask "$REF_MASK"
fi

# clean up
rm $TEMP_FILE
rm vf_tmp.mha

echo
echo "Finished masking all phases"
