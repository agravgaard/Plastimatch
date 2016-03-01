#!/bin/bash
##############################################################################
# auto4d.sh - Registers a set of input phases to a reference phase by
#             automatically generating plastimatch command files 
#             (according to a template) and executing them.
# 
# James Shackleford (tshack@drexel.edu)
# Last updated: Nov 3rd, 2011
##############################################################################


###############################################################################
#: USER OPTIONS
###############################################################################
#: TEMPLATE FILE
#:   Location of the plastimatch command template file
#  ---------------------------------------------------------------
TEMPLATE="template.plm"

#: REFERENCE IMAGE
#:   The full path to reference phase, including file name
#  ---------------------------------------------------------------
REF="/home/tshack/data/reg/0133/crop/0015.mha"

#: PHASES
#:   The full path to registration phase directory
#  ---------------------------------------------------------------
PHASE_PATH="/home/tshack/data/reg/0133/crop"
PHASE_FILTER="*.mha"

#: OUTPUT PATHS
#:   Warped, xform, and vector field output directories
#  ---------------------------------------------------------------
VF_PATH="/home/tshack/data/reg/0133/vf"
XFORM_PATH="/home/tshack/data/reg/0133/xf"
WARP_PATH="/home/tshack/data/reg/0133/warp"

#: OPTIONS
#:   Generate warped images, xforms, or vector fields?
#  ---------------------------------------------------------------
OPT_VF="TRUE"
OPT_XFORM="TRUE"
OPT_WARP="TRUE"

#: OUTPUT FORMATS
#:   For warped images and vector fields
#  ---------------------------------------------------------------
OUTPUT_FORMAT_WARP="mha"
OUTPUT_FORMAT_VF="mha"

#: OUTPUT SUFFIXES
#:   Added to end of filename, but before the extension
#  ---------------------------------------------------------------
SUFFIX_VF="_vf"
SUFFIX_XFORM="_xf"
SUFFIX_WARP="_warp"

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

TEMP_FILE="auto4d.tmp.$$"
ERROR=0

#: Trap CTRL-C so we can cleanup if needed
trap ctrl_c SIGINT
ctrl_c ()
{
    echo
    echo
    echo "Caught signal Ctrl-C!"
    echo "Cleaning up temp files"

    if [ -f $TEMP_FILE ]
    then
        rm $TEMP_FILE
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

if [ ! -d $PHASE_PATH ]; then
    echo "PHASE_PATH: Invalid directory"
    let "ERROR++"
fi

if [ $OPT_VF = "TRUE" ]; then
    if [ ! -d $VF_PATH ]; then
        echo "VF_PATH: Invalid directory"
        let "ERROR++"
    fi
fi

if [ $OPT_XFORM = "TRUE" ]; then
    if [ ! -d $XFORM_PATH ]; then
        echo "XFORM_PATH: Invalid directory"
        let "ERROR++"
    fi
fi

if [ $OPT_WARP = "TRUE" ]; then
    if [ ! -d $WARP_PATH ]; then
        echo "WARP_PATH: Invalid directory"
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
echo " Automatic 4D Registration Helper Script               v0.01"
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
echo "  Vector Fields: $OPT_VF"
echo "  Warped Images: $OPT_WARP"
echo "        X-Forms: $OPT_XFORM"
echo
echo "Output Paths:"
if [ $OPT_VF = "TRUE" ]; then
    echo "  Vector Fields: $VF_PATH"
fi
if [ $OPT_WARP = "TRUE" ]; then
    echo "  Warped Images: $WARP_PATH"
fi
if [ $OPT_XFORM = "TRUE" ]; then
    echo "        X-Forms: $XFORM_PATH"
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
        VF_FILE="${VF_PATH}/${filename}${SUFFIX_VF}.${OUTPUT_FORMAT_VF}"
        WARP_FILE="${WARP_PATH}/${filename}${SUFFIX_WARP}.${OUTPUT_FORMAT_WARP}"
        XFORM_FILE="${XFORM_PATH}/${filename}${SUFFIX_XFORM}.txt${OUTPUT_FORMAT_XF}"

        # Fix escape sequences
        f="${f//\//\\/}"
        REF_FILE="${REF//\//\\/}"
        VF_FILE="${VF_FILE//\//\\/}"
        XF_FILE="${XF_FILE//\//\\/}"
        XFORM_FILE="${XFORM_FILE//\//\\/}"
        WARP_FILE="${WARP_FILE//\//\\/}"

        # Build the command file for current phase
        cp $TEMPLATE $TEMP_FILE

        sed -i "s/$FIXED_TOKEN/${REF_FILE}/g" "$TEMP_FILE"
        sed -i "s/$MOVING_TOKEN/${f}/g" "$TEMP_FILE"

        # setup xform_out
        if [ $OPT_XFORM = "TRUE" ]; then
            sed -i "s/$XFORM_TOKEN/${XFORM_FILE}/g" "$TEMP_FILE"
        else
            sed -i "s/$XFORM_TOKEN//g" "$TEMP_FILE"
#            sed -i "s/xform_out/#xform_out/g" "$TEMP_FILE"
        fi

        # setup img_out
        if [ $OPT_WARP = "TRUE" ]; then
            sed -i "s/$WARP_TOKEN/${WARP_FILE}/g" "$TEMP_FILE"
        else
            sed -i "s/$WARP_TOKEN//g" "$TEMP_FILE"
#            sed -i "s/img_out/#img_out/g" "$TEMP_FILE"
        fi

        # setup vf_out
        if [ $OPT_VF = "TRUE" ]; then
            sed -i "s/$VF_TOKEN/${VF_FILE}/g" "$TEMP_FILE"
        else
            sed -i "s/$VF_TOKEN//g" "$TEMP_FILE"
#            sed -i "s/vf_out/#vf_out/g" "$TEMP_FILE"
        fi

        # have plastimatch do its thing
        plastimatch register $TEMP_FILE
    fi
done

rm $TEMP_FILE

echo
echo "Finished registering all phases"
