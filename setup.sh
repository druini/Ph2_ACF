#!/bin/bash

##########
# CACTUS #
##########
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include

#########
# BOOST #
#########
export KERNELRELEASE=$(uname -r)
if [[ $KERNELRELEASE == *"el6"* ]]; then
    export BOOST_LIB=/opt/cactus/lib
    export BOOST_INCLUDE=/opt/cactus/include
else
    export BOOST_INCLUDE=/usr/include
    export BOOST_LIB=/usr/lib64
fi

########
# ROOT #
########
source /usr/local/root/bin/thisroot.sh
# source $ROOTSYS/bin/thisroot.sh
#source /opt/local/root/bin/thisroot.sh

#######
# ZMQ #
#######
export ZMQ_HEADER_PATH=/usr/include/zmq.hpp

###########
# Ph2_ACF #
###########
export BASE_DIR=$(pwd)

####################
# External Plugins #
####################
export ANTENNADIR=$BASE_DIR/CMSPh2_AntennaDriver
export AMC13DIR=/opt/cactus/include/amc13
export USBINSTDIR=~/Ph2_USBInstDriver
export USBINSTDIR=$BASE_DIR/../Ph2_USBInstDriver

###########
# ANTENNA #
###########
export ANTENNALIB=$ANTENNADIR/lib

###########
# HMP4040 #
###########
export USBINSTLIB=$USBINSTDIR/lib

##########
# System #
##########
export PATH=$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$USBINSTLIB:$ANTENNALIB:$BASE_DIR/RootWeb/lib:$CACTUSLIB:$BASE_DIR/lib:${LD_LIBRARY_PATH}

#########
# Flags #
#########
export HttpFlag='-D__HTTP__'
export ZmqFlag='-D__ZMQ__'
export USBINSTFlag='-D__USBINST__'
export Amc13Flag='-D__AMC13__'
export AntennaFlag='-D__ANTENNA__'
export UseRootFlag='-D__USE_ROOT__'

################
# Compilations #
################

# Stand-alone application, without data streaming
export CompileForHerd=false
export CompileForShep=false

# Stand-alone application, with data streaming
# export CompileForHerd=true
# export CompileForShep=true

# Herd application
# export CompileForHerd=true
# export CompileForShep=false

# Shep application
# export CompileForHerd=false
# export CompileForShep=true

echo "=== DONE ==="
