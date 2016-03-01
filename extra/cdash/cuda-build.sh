#!/bin/bash
############################################
# James Shackleford
# Feb 14th, 2012
#########
# Auto CTest for various cuda versions
#   Requires package perl-suid
#########
#
#  Usage:
#    cuda-build.sh [cuda version]
# 
#  Valud [cuda version] values are:
#    2.3
#    3.0
#    3.1
#    3.2
#    4.0
#    4.1
#
#  CUDA is automatically downloaded and installed, process is
#    fully automated.  Ubuntu/Debian systems only.
#    32-bit and 64-bit supported.
#####################################################################


CUDA_VER=$1


# User Settings
DIR_SRC="$HOME/src/plastimatch-nightly"
DIR_BUILD="$HOME/build/nightly-cuda$CUDA_VER"
DIR_LOG="$DIR_BUILD/log"
CUDA_REPO="$HOME/nvidia"

# If running via cron, make sure PATH has everything it needs
PATH=$PATH:/usr/local/bin

# Binary Paths
CMAKE=$(which cmake)
CTEST=$(which ctest)
SUIDPERL=$(which suidperl)
WGET=$(which wget)
SED=$(which sed)
UNAME=$(which uname)
CHOWN=$(which chown)
CHMOD=$(which chmod)
SUDO=$(which sudo)


# Please do not edit below this line (unless you know what you are doing!)
###############################################################################

# 1: user friendly name of file
# 2: full file path
# 3: hint to user if not found
function check_exists() {
	if [ -z $2 ]; then
        echo "Could not find $1"
        echo "  Hint: $3"
        echo ""
		echo "DEBUG: $2"
        exit
	fi
    if [ ! -e $2 ]; then
        echo "Could not find $1"
        echo "  Hint: $3"
        echo ""
		echo "DEBUG: $2"
        exit
    fi
}

function check_depends() {
	check_exists "cmake" $CMAKE "sudo apt-get install cmake"
	check_exists "ctest" $CTEST "sudo apt-get install cmake"
	check_exists "suidperl" $SUIDPERL "sudo apt-get install suidperl"
	check_exists "uname" $UNAME "check your PATH"
	check_exists "sed" $SED "check your PATH"
	check_exists "chown" $CHOWN "check your PATH"
	check_exists "chmod" $CHMOD "check your PATH"
	check_exists "sudo" $SUDO "check your PATH"
	if [ -z $CUDA_VER ]; then
		echo ""
		echo "ERROR: CUDA version not specified"
		echo ""
		exit
	fi
}
###############################################################################


check_depends

FILE_CMAKE="$DIR_LOG/$($UNAME -n)-cuda$CUDA_VER.cmake"


###############################################################################
function fix_permissions() {
    $SUDO $CHOWN root $CUDA_REPO/cuda$CUDA_VER/install-linux.pl
    $SUDO $CHMOD ug+s $CUDA_REPO/cuda$CUDA_VER/install-linux.pl
    $SUDO $CHMOD o-rwx $CUDA_REPO/cuda$CUDA_VER/install-linux.pl
}

function set_cuda_url() {
    if [ $($UNAME -m) == "x86_64" ]; then
        if [ $CUDA_VER == "2.3" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/cudatoolkit_2.3_linux_64_ubuntu9.04.run"
        elif [ $CUDA_VER == "3.0" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_0/toolkit/cudatoolkit_3.0_linux_64_ubuntu9.04.run"
        elif [ $CUDA_VER == "3.1" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_1/toolkit/cudatoolkit_3.1_linux_64_ubuntu9.10.run"
        elif [ $CUDA_VER == "3.2" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_2_prod/toolkit/cudatoolkit_3.2.16_linux_64_ubuntu10.04.run"
        elif [ $CUDA_VER == "4.0" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/4_0/toolkit/cudatoolkit_4.0.17_linux_64_ubuntu10.10.run"
        elif [ $CUDA_VER == "4.1" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/4_1/rel/toolkit/cudatoolkit_4.1.28_linux_64_ubuntu10.04.run"
		else
			echo ""
			echo "ERROR: URL for specified CUDA version is unknown"
			echo ""
			exit
        fi
    else
        if [ $CUDA_VER == "2.3" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/2_3/toolkit/cudatoolkit_2.3_linux_32_ubuntu9.04.run"
        elif [ $CUDA_VER == "3.0" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_0/toolkit/cudatoolkit_3.0_linux_32_ubuntu9.04.run"
        elif [ $CUDA_VER == "3.1" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_1/toolkit/cudatoolkit_3.1_linux_32_ubuntu9.10.run"
        elif [ $CUDA_VER == "3.2" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/3_2_prod/toolkit/cudatoolkit_3.2.16_linux_32_ubuntu10.04.run"
        elif [ $CUDA_VER == "4.0" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/4_0/toolkit/cudatoolkit_4.0.17_linux_32_ubuntu10.10.run"
        elif [ $CUDA_VER == "4.1" ]; then
            CUDA_URL="http://developer.download.nvidia.com/compute/cuda/4_1/rel/toolkit/cudatoolkit_4.1.28_linux_32_ubuntu10.04.run"
		else
			echo ""
			echo "ERROR: URL for specified CUDA version is unknown"
			echo ""
			exit
        fi
    fi
}

function modify_installer() {
    $SED -i "s/perl/suidperl -U/g" "$CUDA_REPO/cuda$CUDA_VER/install-linux.pl"
    echo "system(\"$CHMOD o-rwx ./install-linux.pl\");" >> $CUDA_REPO/cuda$CUDA_VER/install-linux.pl
}

function cuda_download() {
        set_cuda_url
        echo -n "     downloading... "
        $WGET --quiet -O $CUDA_REPO/cuda$CUDA_VER.run $CUDA_URL
        echo "ok"
        echo -n "     decompressing... "
        $CHMOD 755 $CUDA_REPO/cuda$CUDA_VER.run 
        $CUDA_REPO/cuda$CUDA_VER.run --noexec --target $CUDA_REPO/cuda$CUDA_VER 1>/dev/null 2>/dev/null
        echo "ok"
        rm $CUDA_REPO/cuda$CUDA_VER.run

        modify_installer
        fix_permissions
}

function check_installer() {
    if [ ! -d $CUDA_REPO ]; then
        mkdir $CUDA_REPO
    fi

    echo "-- CUDA $CUDA_VER installer"

    echo -n "     exists... "
    if [ -d $CUDA_REPO/cuda$CUDA_VER ]; then
        if [ -e "$CUDA_REPO/cuda$CUDA_VER/install-linux.pl" ]; then
            echo "ok"
        else
            echo "no"
            rm -rf $CUDA_REPO/cuda$CUDA_VER
            cuda_download
        fi
    else
        echo "no"
        cuda_download
    fi

    echo -n "     has setuid root... "
    if [ -u "$CUDA_REPO/cuda$CUDA_VER/install-linux.pl" ]; then
        echo "ok"
    else
        echo "FAILED"
        echo ""
        echo "Please re-run this script with:"
        echo "  $0 $CUDA_VER suid"
        echo ""
        echo "to setup proper permissions, then run again with:"
        echo "  $0 $CUDA_VER"
        echo ""
        exit
    fi

}

function generate_ctest(){
    echo "-- CTest Rules"

    echo -n "     generating... "
    if [ -e "$FILE_CMAKE" ]; then
        rm $FILE_CMAKE
    fi

    echo "SET (CTEST_SOURCE_DIRECTORY \"$DIR_SRC\")"              > $FILE_CMAKE
    echo "SET (CTEST_BINARY_DIRECTORY \"$DIR_BUILD\")"           >> $FILE_CMAKE
    echo "SET (CTEST_CMAKE_COMMAND \"$CMAKE -Wno-dev\")"         >> $FILE_CMAKE
    echo "SET (CTEST_COMMAND \"$CTEST -D Nightly\")"             >> $FILE_CMAKE
    echo "SET (CTEST_INITIAL_CACHE \""                           >> $FILE_CMAKE
    echo "//Name of generator."                                  >> $FILE_CMAKE
    echo "CMAKE_GENERATOR:INTERNAL=Unix Makefiles"               >> $FILE_CMAKE
    echo ""                                                      >> $FILE_CMAKE
    echo "//Name of the build"                                   >> $FILE_CMAKE
    echo "BUILDNAME:STRING=lin64-Pisr-Cd-gcc$(gcc -dumpversion)-cuda$CUDA_VER" >> $FILE_CMAKE
    echo ""                                                      >> $FILE_CMAKE
    echo "//Name of the computer/site where compile is being run" >> $FILE_CMAKE
    echo "SITE:STRING=$(/bin/uname -n)"                          >> $FILE_CMAKE
    echo ""                                                      >> $FILE_CMAKE
    echo "//Build with shared libraries."                        >> $FILE_CMAKE
    echo "BUILD_SHARED_LIBS:BOOL=OFF"                            >> $FILE_CMAKE
    echo "PLM_CONFIG_DISABLE_REG23:BOOL=ON"                      >> $FILE_CMAKE
    echo "\")"                                                   >> $FILE_CMAKE

    echo "ok"

    echo -n "     checking... "
    if [ -e "$FILE_CMAKE" ]; then
        echo "ok"
    else
        echo "FAILED"
        exit
    fi
}

function generate_build_dir() {
    echo "-- Build directory"
    echo "     using: $DIR_BUILD"
    if [ -d "$DIR_BUILD" ]; then
        echo -n "     purging... "
        rm -rf $DIR_BUILD
        echo "ok"
    fi

    echo -n "     generating... "
    mkdir $DIR_BUILD
    mkdir $DIR_LOG
    if [ -d "$DIR_BUILD" ]; then
        if [ -d "$DIR_LOG" ]; then
            echo "ok"
        else
            echo "FAILED"
            exit
        fi
    else
        echo "FAILED"
        exit
    fi
}

function install_cuda() {
    echo "-- CUDA Environment"
    echo -n "     installing... "
    cd $CUDA_REPO/cuda$CUDA_VER/
    ./install-linux.pl auto > $DIR_LOG/install.log 2> /dev/null

	export PATH=$PATH:/usr/local/cuda/bin
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib64:/usr/local/cuda/lib

	NVCC=$(which nvcc)
	check_exists "nvcc" $NVCC "check your PATH"

    VER_HAVE=$($NVCC --version | $SED -rn '/release/ s/.*release ([0-9.]+).*/\1/p')
    VER_WANT=$CUDA_VER

    if [ $VER_HAVE == $VER_WANT ]; then
        echo "ok"
    else
        echo "FAILED"
        echo ""
        echo "FATAL: Installed ($VER_WANT), but found ($VER_HAVE)"
        echo ""
        exit
    fi
}

function build_and_test() {
    echo ""
    echo -n "Performing initial CMake... "
    cd $DIR_BUILD
    $CMAKE -Wno-dev $DIR_SRC 1>$DIR_LOG/cmake.log 2>/dev/null
    echo "done."

    # We have to run ctest twice or, otherwise, cuda_probe will not be
    #   found for some reason and the CUDA tests won't run.  Should
    #   check the build dependency order in the CMake file to fix this.
    echo ""
    echo -n "Building and testing... "
    $CTEST -S $FILE_CMAKE -V > $DIR_LOG/tests.log 2>&1
    $CTEST -S $FILE_CMAKE -V >> $DIR_LOG/tests.log 2>&1
    echo "done."
}

###############################################################################

if [ "$2" == "suid" ]; then
    fix_permissions
    exit
fi

date
check_installer
generate_build_dir
generate_ctest
install_cuda
build_and_test
exit
