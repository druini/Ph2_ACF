YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

CC              = gcc
CXX             = g++
CCFlags         = -g -O0 -w -Wall -pedantic -pthread -std=c++11 -fPIC
CCFlagsRoot	= `root-config --cflags --glibs`
ROOTHTTP := $(shell root-config --has-http)

RootLibraryDirs = $(ROOTLIB)
RootLibraryPaths = $(RootLibraryDirs:%=-L%)

LibraryDirs = $(CACTUSLIB) ../lib ../RootWeb/lib
IncludeDirs     =  $(CACTUSINCLUDE) ../ 
ExternalObjects= -lpthread -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -lboost_filesystem -lboost_program_options -lRootWeb

##################################################
## check if the Root has THttp
##################################################
ifneq (,$(findstring yes,$(ROOTHTTP)))
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

#ExternalObjects= $(LibraryPaths) -lpthread  -lcactus_extern_pugixml -lcactus_uhal_log -lcactus_uhal_grammars -lcactus_uhal_uhal \
                                 -lboost_system -lPh2_Interface -lPh2_Description -lPh2_System -lPh2_Utils -L$(BOOST_LIB) -lboost_filesystem -lboost_program_options -lboost_thread -L../RootWeb/lib -lRootWeb

IncludePaths            = $(IncludeDirs:%=-I%)
	RootLibraryDirs = /usr/local/lib/root

LibraryPaths = $(LibraryDirs:%=-L%) 
IncludePaths = $(IncludeDirs:%=-I%)

binaries = print miniDQM miniDAQ
all: rootflags clean $(binaries) 

rootflags:
	$(eval CCFlags += $(CCFlagsRoot))
	$(eval ExternalObjects += $(ExtObjectsRoot))


publisher.o: publisher.cc publisher.h
	$(CXX) $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(LibraryPaths) -c -o $@ $<

DQMHistogrammer.o: DQMHistogrammer.cc DQMHistogrammer.h
	$(CXX) $(DevFlags) $(CCFlags) $(CCFlagsRoot) $(UserCCFlags) $(CCDefines) $(IncludePaths) $(LibraryPaths) -c -o $@ $<

miniDQM: miniDQM.cc publisher.h publisher.o DQMHistogrammer.h DQMHistogrammer.o
	$(CXX) $(CCFlags) -o $@ $< $(IncludePaths) $(LibraryPaths) publisher.o DQMHistogrammer.o $(ExternalObjects) 
	cp $@ ../bin

miniDAQ: miniDAQ.cc
	$(CXX) $(CCFlags) -o $@ $< $(IncludePaths) $(LibraryPaths) $(ExternalObjects)
	cp $@ ../bin

print:
	@echo -e ${YEL}'****************************'
	@echo -e ${YEL}'Building Mini DAQ'
	@echo -e ${YEL}'Root Has Http: ' ${RED}$(ROOTHTTP)
	@echo -e ${YEL}'Amc13 SW installed:' ${RED}$(AMC13INSTALLED)
	@echo -e ${YEL}'Antenna installed:' ${RED}$(ANTENNAINSTALLED)
	@echo -e ${YEL}'****************************'${RESET}

.PHONY: print clean

clean:
	rm -f *.o $(binaries)
