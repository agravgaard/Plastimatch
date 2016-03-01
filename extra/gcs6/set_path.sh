#! /bin/sh

CNAME=`uname -n`
case $CNAME in
    "absinthe")
	export PATH=$PATH:$HOME/build/plastimatch-3.20.0
	export PATH=$PATH:$HOME/work/plastimatch/extra/perl
	;;
    "gelato")
	export PATH=$PATH:$HOME/build/plastimatch-3.20.1
	export PATH=$PATH:$HOME/work/plastimatch/extra/perl
	;;
    "physics.mgh.harvard.edu")
	export PATH=$PATH:$HOME/build/plastimatch-3.18.0
	export PATH=$PATH:$HOME/work/plastimatch/extra/perl
	;;
    "redfish")
	export PATH=$PATH:$HOME/build/plastimatch-3.18.0
	export PATH=$PATH:$HOME/work/plastimatch/extra/perl
	;;
    "slumber")
	export PATH=$PATH:/cygdrive/c/gcs6/build/plastimatch-cygwin
	;;
    "wormwood")
	export PATH=$PATH:$HOME/build/plastimatch-3.20.1
	export PATH=$PATH:$HOME/work/plastimatch/extra/perl
	;;
    *)
	echo "Unknown machine"
	;;
esac
