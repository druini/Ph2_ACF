#!/bin/bash
export CBCDAQ=$HOME/CBCDAQ

#CACTUS
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include


# BOOST
export BOOST_LIB=/opt/cactus/lib
export BOOST_INCLUDE=/opt/cactus/include


#ROOT
source /usr/local/bin/thisroot.sh
export ROOTLIB=/usr/local/lib/root
#export ROOTSYS=/usr/local/lib/root


#ZMQ
export ZMQ_HEADER_PATH=/usr/include/zmq.hpp

#Ph2_ACF
export BASE_DIR=$(pwd)

#External Plugins
export ANTENNADIR=$BASE_DIR/CMSPh2_AntennaDriver
export AMC13DIR=/opt/cactus/include/amc13
#won't link properly if I don't use this [SS]
#export USBINSTDIR=~/Ph2_USBInstDriver
export USBINSTDIR=$BASE_DIR/../Ph2_USBInstDriver

#ANTENNA
export ANTENNALIB=$BASE_DIR/CMSPh2_AntennaDriver/lib
#HMP4040
export USBINSTLIB=$BASE_DIR/../Ph2_USBInstDriver/lib


export PATH=$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$USBINSTLIB:$ANTENNALIB:$BASE_DIR/RootWeb/lib:/opt/cactus/lib:$BASE_DIR/lib:${LD_LIBRARY_PATH}

export HttpFlag='-D__HTTP__'
export ZmqFlag='-D__ZMQ__'
export USBINSTFlag='-D__USBINST__'
export Amc13Flag='-D__AMC13__'
export AntennaFlag='-D__ANTENNA__'
export DevFlags='-D__DEV__'


>>>>>>> remotes/upstream/CBC3
