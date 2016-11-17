ANTENNADIR=../CMSPh2_AntennaDriver
AntennaFlag = -D__ANTENNA__
AMC13DIR=/opt/cactus/include/amc13
Amc13Flag     = -D__AMC13__

ROOTVERSION := $(shell root-config --has-http)

RootLibraryDirs = /usr/local/lib/root
LibraryDirs = ../lib 
#ExternalObjects = -lPh1_Utils -lPh2_System -lPh2_Interface -lPh2_Description
ExternalObjects = 
IncludeDirs = /opt/cactus/include ../

##################################################
## check if Root has Http
##################################################
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot= -lRHTTP -D__HTTP__
else
	ExtObjectsRoot= 
endif

##################################################
## check if Antenna Driver is installed
##################################################
ifneq ("$(wildcard $(ANTENNADIR))","")
	ANTENNAINSTALLED = yes
	IncludeDirs     +=  $(ANTENNADIR) 
	LibraryDirs += ../CMSPh2_AntennaDriver/lib
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

Objs            = Tool.o SCurve.o Calibration.o Channel.o HybridTester.o CMTester.o  LatencyScan.o SignalScan.o PulseShape.o PedeNoise.o RegisterTester.o ShortFinder.o AntennaTester.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` 


#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags		=


LibraryPaths = $(LibraryDirs:%=-L%) 
IncludePaths = $(IncludeDirs:%=-I%)
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

.PHONY: print clean

%.o: %.cc %.h
	$(CXX) -std=c++0x  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) $(RootLibraryPaths) $(ExtObjectsRoot) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++0x -shared -o libPh2_Tools.so $(Objs) -pthread
	mv libPh2_Tools.so ../lib

print:
	@echo '****************************'
	@echo 'Building tools with:'
	@echo 'Root Has Http:' $(ROOTVERSION)
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo '****************************'
clean:
	rm -f *.o
