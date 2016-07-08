CC              = gcc
CXX             = g++
CCFlags         = -g -O0 -w -Wall -pedantic -pthread -std=c++0x -fPIC 
CCFlagsRoot	= `root-config --cflags --glibs`
ROOTVERSION := $(shell root-config --has-http)
HttpFlag = -D__HTTP__

DevFlags        =

ANTENNADIR=../CMSPh2_AntennaDriver
AntennaFlag = -D__ANTENNA__
AMC13DIR=/opt/cactus/include/amc13
Amc13Flag     = -D__AMC13__


LibraryDirs = /opt/cactus/lib ../lib 
IncludeDirs     =  /opt/cactus/include ../ 
ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lPh2_Tracker -lboost_filesystem -lboost_program_options -L../RootWeb/lib -lRootWeb

##################################################
## check if the Root has THttp
##################################################
ifneq (,$(findstring yes,$(ROOTVERSION)))
	ExtObjectsRoot += $(RootLibraryPaths) -lRHTTP $(HttpFlag)
else
	ExtObjectsRoot += $(RootLibraryPaths)
endif

##################################################
## check if the Antenna driver is installed
##################################################
ifneq ("$(wildcard $(ANTENNADIR))","")
	IncludeDirs += $(ANTENNADIR)
	LibraryDirs += $(ANTENNADIR)/lib /usr/lib64/ 
	ExternalObjects += -lPh2_Antenna $(AntennaFlag) 
	ANTENNAINSTALLED = yes
else
	ANTENNAINSTALLED = no
endif

##################################################
## check if the AMC13 drivers are installed
##################################################
ifneq ("$(wildcard $(AMC13DIR))","")
	ExternalObjects += -lcactus_amc13_amc13 -lPh2_Amc13
	AMC13INSTALLED = yes
else
	AMC13INSTALLED = no
endif



IncludePaths            = $(IncludeDirs:%=-I%)
	RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%) 
RootLibraryPaths = $(RootLibraryDirs:%=-L%)


binaries=print miniDQM miniDAQ
all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))


publisher.o: publisher.cc publisher.h
	$(CXX) $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

DQMHistogrammer.o: DQMHistogrammer.cc DQMHistogrammer.h
	$(CXX) $(DevFlags) $(CCFlags) $(CCFlagsRoot) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

miniDQM: miniDQM.cc publisher.h publisher.o DQMHistogrammer.h DQMHistogrammer.o
	$(CXX) $(CCFlags) -o $@ $< $(IncludePaths) publisher.o DQMHistogrammer.o $(ExternalObjects) 
	cp $@ ../bin

miniDAQ: miniDAQ.cc
	$(CXX) $(CCFlags) -o $@ $< $(IncludePaths) $(ExternalObjects)
	cp $@ ../bin

print:
	@echo '****************************'
	@echo 'Building Mini DAQ'
	@echo 'Root Has Http: ' $(ROOTVERSION)
	@echo 'Amc13 SW installed:' $(AMC13INSTALLED)
	@echo 'Antenna installed:' $(ANTENNAINSTALLED)
	@echo '****************************'

.PHONY: print clean

clean:
	rm -f *.o $(binaries)
