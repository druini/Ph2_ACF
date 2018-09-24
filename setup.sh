#!/bin/bash
export CBCDAQ=$HOME/CBCDAQ
export KERNELRELEASE=$(uname -r)

#CACTUS
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include


# BOOST
if [[ $KERNELRELEASE == *"el7"* ]]; then
    export BOOST_LIB=/opt/cactus/lib
    export BOOST_INCLUDE=/opt/cactus/include
else
    export BOOST_INCLUDE=/usr/include
    export BOOST_LIB=/usr/lib64
fi


#ROOT
#source /usr/local/bin/thisroot.sh
source /opt/local/root/bin/thisroot.sh
#export ROOTLIB=/usr/local/lib/root
export ROOTLIB=/opt/local/root/lib
#export ROOTSYS=/usr/local/lib/root

#ZMQ
export ZMQ_HEADER_PATH=/usr/include/zmq.hpp

#Ph2_ACF
export BASE_DIR=$(pwd)

#External Plugins
export ANTENNADIR=$BASE_DIR/CMSPh2_AntennaDriver
export AMC13DIR=/opt/cactus/include/amc13
#won't link properly if I don't use this [SS]
export USBINSTDIR=~/Ph2_USBInstDriver
export USBINSTDIR=$BASE_DIR/../Ph2_USBInstDriver

#ANTENNA
export ANTENNALIB=$ANTENNADIR/lib
#HMP4040
export USBINSTLIB=$USBINSTDIR/lib


export PATH=$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$USBINSTLIB:$ANTENNALIB:$BASE_DIR/RootWeb/lib:$CACTUSLIB:$BASE_DIR/lib:${LD_LIBRARY_PATH}


export HttpFlag='-D__HTTP__'
export ZmqFlag='-D__ZMQ__'
export USBINSTFlag='-D__USBINST__'
export Amc13Flag='-D__AMC13__'
export AntennaFlag='-D__ANTENNA__'
export DevFlags='-D__DEV__'
export Root5Flag='-D__ROOT5__'
export Root6Flag='-D__ROOT6__'
