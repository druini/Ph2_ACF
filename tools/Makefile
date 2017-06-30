YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -pthread -fPIC `root-config --cflags --evelibs` 

Objs            = Tool.o Calibration.o Channel.o HybridTester.o CMTester.o LatencyScan.o SignalScan.o PulseShape.o PedeNoise.o RegisterTester.o ShortFinder.o AntennaTester.o StubSweep.o

ROOTVERSION := $(shell root-config --has-http)
RootLibraryDirs = $(ROOTLIB)
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

IncludeDirs = $(CACTUSINCLUDE) ../
LibraryDirs = ../lib 
ExternalObjects = -lboost_thread -lboost_filesystem -lPh2_System -lPh2_Description -lPh2_Interface -lPh2_Utils

##################################################
## check if Root has Http
##################################################
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot= 
endif

##################################################
## check if Antenna Driver is installed
##################################################
ifneq ("$(wildcard $(ANTENNADIR))","")
	ANTENNAINSTALLED = yes
	IncludeDirs     +=  $(ANTENNADIR) 
	LibraryDirs += $(ANTENNALIB)
	ExternalObjects += -lusb -lPh2_Antenna $(AntennaFlag)
else
	ANTENNAINSTALLED = no
endif

##################################################
## check if AMC13 drivers are installed
##################################################
ifneq ("$(wildcard $(AMC13DIR))","")
	ExternalObjects += -lcactus_amc13_amc13 -lPh2_Amc13
	AMC13INSTALLED = yes
else
	AMC13INSTALLED = no
endif

##################################################
## check if the Ph2_USBInstDriver drivers are installed
##################################################
ifneq ("$(wildcard $(USBINSTDIR))","")
	LibraryDirs += $(BOOST_LIB) $(USBINSTLIB)
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/ArdNano
	ExternalObjects += -lPh2_Drivers -lPh2_USBUtils -lPh2_HMP4040 -lPh2_Ke2110 -lPh2_ArdNano $(USBINSTFlag)
	USBINSTLIBINSTALLED = yes
	Objs += BiasSweep.o
else
	USBINSTLIBINSTALLED = no
endif

##################################################
## check if ZMQ library is available on this machine
##################################################
ifneq ("$(wildcard $(ZMQ_HEADER_PATH))","")
	ExternalObjects += -lzmq $(ZmqFlag)
	ZMQINSTALLED = yes
else
	ZMQINSTRUCTIONS = ZeroMQ library is required to build the lvSupervisor with the server option - get it with \'sudo yum install zeromq-devel\'
endif

LibraryPaths = $(LibraryDirs:%=-L%) 
IncludePaths = $(IncludeDirs:%=-I%)

.PHONY: print clean

%.o: %.cc %.h
	$(CXX) -std=c++0x $(CCFlags) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) $(RootLibraryPaths) $(ExtObjectsRoot) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++0x -shared -o libPh2_Tools.so $(Objs) -pthread
	mv libPh2_Tools.so ../lib

print:
	@echo -e ${YEL}'****************************'
	@echo -e ${YEL}'Building tools with:'
	@echo -e ${YEL}'Root Has Http:' ${RED}$(ROOTVERSION)
	@echo -e ${YEL}'Amc13 SW installed:' ${RED}$(AMC13INSTALLED)
	@echo -e ${YEL}'Antenna installed:' ${RED}$(ANTENNAINSTALLED)
	@echo -e ${YEL}'Ph2_USBInstDriver installed:' ${RED}$(USBINSTLIBINSTALLED)
	@echo -e ${YEL}'****************************'${RESET}
clean:
	rm -f *.o
