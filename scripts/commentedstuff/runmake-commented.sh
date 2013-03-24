#!/bin/bash
#=+cch========================================================================#
#  NAME     : runmake.sh                                                      #
#  FUNCTION :                                                                 #
#-----------------------------------------------------------------------------#
#  (C) Copyright 2005 Nokia Siemens Networks                                  #
#-----------------------------------------------------------------------------#
#  AUTHOR   : pt101493                                                        #
#  CREATED  : 2005-11-03                                                      #
#========================================================================cch+=#

# enable winkin permissions
umask 0002

## init GNU_MAKE to /usr/bin/make
GNU_MAKE=$(which make)

## init RUNCONFIG to ./cs/runconfig.sh
RUNCONFIG=cs/runconfig.sh

## if runconfig.sh not exists or is not readable: exit
if [ ! -r $RUNCONFIG ]
    then  echo "  could not open $RUNCONFIG for reading, please check .."; exit 77
    else . $RUNCONFIG; echo ready
fi

## check if one of the command line starts with BUILD, if so export it
for a in "$@" ; do
    if echo $1 | grep -qe "^BUILD="; then
        export $1
        shift
    fi
done

##
## functions
##

## check if file exists, check for doubled entries or already inserted entries in PATH
## TODO: I have doubts that this func does anything! does it work?
## TODO: check out REGEXP
strippath () {
    ## check: no arguments?
    [ -z $1 ] && return;

    ## check: argument is PATH variable?
    [ "$PATH" == "$1" ] && return;

    ## else: check each element of the arguments for certain entries
    for p in "$@"; do
        found=`expr "$PATH" : "\($p/\?:\)"` && PATH=${PATH#$found} ||
        found=`expr "$PATH" : ".*\(:$p/\?:\)"` &&  PATH=${PATH//${found%:}} ||
        found=`expr "$PATH" : ".*\(:$p/\?$\)"` &&  PATH=${PATH%$found}
    done
}


## make shure to get no duplicate entries (?) and set path
strippath   $PWD/bin       $ELDK_ROOT/usr/bin $ELDK_ROOT/bin
export PATH=$PWD/bin:$PATH:$ELDK_ROOT/usr/bin:$ELDK_ROOT/bin


## TODO: check out clearcase
cleartool setcs -cur


## now real fun starts: if first arg of this script was....
case "$1" in
    ## ...load_prod_fast
    load_prod_fast) 
	export RD_QUICKMAKE=bla
	echo -e "\n"'\E[47;31m'"\033[5m >>> warning using variable 'RD_QUICKMAKE', not building not everything \033[0m"
	## TODO: not building not everything, basically not doing not nothing not!
esac


## TODO: check out WTF is $- ??!!!! Note: it's ksh and considered NOT reliable in bash!
case "$-" in
    ## seems to be some kind of support for separate build of parts / modules (TODO: check ksh trash)
    *i*)
        echo -e "\n"'\E[36;40m'"\033[1m build variables successfully parsed from script .. \033[0m"
        echo "you can now enter subdirectories and call the Makefiles there"
        ;;

    ## default actions
    * )
	## ok, one more time check the current param for...
        case "$1" in
	    ## ...one of those ramdiskbuilds
            ramdisk|ramdisk-build|ramdisk-install|ramdisk-copy)
		## this parameter (TODO: ???) needs a "given path" to something to build
		## if the file ./cs/<given path>.bos exists export this ".bos"-file to a env variable for ClearCase
		## use clearmake to run and build the <given path>, the rest of the params will be clearmake flags
                target="$1"; shift
                [ -r $PWD/cs/$target.bos ] && export CCASE_OPTS_SPECS=$PWD/cs/$target.bos
                clearmake -C gnu "$@" $target
                ;;

	    ## ...one of those kernel or load related options, also "make oldconfig" or only build the bootloader
            kernel|kernel_prod|load|load_prod|load_prod_fast|oldconfig|uImage|uboot)
		## TODO: why here again a check for build, shouldn't it be ckecked already in the beginning?
		## anyway, if BUILD is empty - exit
                if [ -z $BUILD ]; then
                    echo -e "\n"'\E[47;31m'"\033[5m no variable 'BUILD' set! \033[0m"
                    echo " please supply BUILD eihter by:"
                    echo "   'export BUILD=<build version>; $0 ..'    or"
                    echo "   as cmd line parameter: '$0 BUILD=<build version> ..' (no spaces in argument)"
                    exit 5
		else ## else print something
                    echo -e "\n"'\E[47;32m'"\033[5m using variable 'BUILD=$BUILD' for load \033[0m"
                fi

		## fetch next param, should be another part to build..  
                target="$1"; shift
                [ -r $PWD/cs/$target.bos ] && export CCASE_OPTS_SPECS=$PWD/cs/$target.bos
                clearmake -C gnu "$@" $target
		
		
		## if anything went wrong exit
                if [ "${ret=$?}" -ne 0 ]; then
                    exit $ret
                fi
		
		## special conditions for special people... tar some of the built stuff
                case $(whoami) in
		    gr000732|wf000732|fa000731)
			echo $(whoami) "is bgv, saving vmlinux and some dbg files in "$DBG_TARBALL
			$GNU_MAKE debug_tarball
			;;
                esac
                ;;
	    
	    ## like "make clean" (?)
            clean)
                clearmake -C gnu "$@" $target
                ;;

	    ## default action
            *)
                echo "usage: runmake.sh [BUILD=version>] ramdisk|kernel|load|load_prod "
                ;;
        esac
        ;;
esac

