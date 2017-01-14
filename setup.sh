#!/bin/bash

#CACTUS
export CACTUSBIN=/opt/cactus/bin
export CACTUSLIB=/opt/cactus/lib
export CACTUSINCLUDE=/opt/cactus/include


# BOOST
export BOOST_LIB=/opt/cactus/lib
export BOOST_INCLUDE=/opt/cactus/include


#ROOT
#source /usr/local/bin/thisroot.sh

#Ph2_ACF
export BASE_DIR=$(pwd)

#ANTENNA
export ANTENNALIB=CMSPh2_AntennaDriver/lib

export PATH=$BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$BASE_DIR/$ANTENNALIB:$BASE_DIR/RootWeb/lib:/opt/cactus/lib:$BASE_DIR/lib:${LD_LIBRARY_PATH}
