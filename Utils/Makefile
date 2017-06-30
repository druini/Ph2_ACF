YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

Objs            = Exception.o Utilities.o Watchdog.o SLinkEvent.o Event.o Cbc2Event.o Cbc3Event.o D19cCbc3Event.o Data.o argvparser.o FileHandler.o
ObjsExt      	= UsbUtilities.o

CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC -Wcpp

LibraryDirs =
IncludeDirs =
ExternalObjects=

#################################################
# check if ZMQ library is available on this machine
#################################################
ifneq ("$(wildcard $(ZMQ_HEADER_PATH))","")
        ExternalObjects += -lzmq $(ZmqFlag)
        ZMQINSTALLED = yes
else
		ZMQINSTALLED = no
        ZMQINSTRUCTIONS = ZeroMQ library is required to build the lvSupervisor with the server option - get it with \'sudo yum install zeromq-devel\'
endif

##################################################
## check if the Ph2_USBInstDriver drivers are installed
##################################################
ifneq ("$(wildcard $(USBINSTDIR))","")
	LibraryDirs += $(BOOST_LIB) $(USBINSTLIB) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils 
	IncludeDirs += $(BOOST_INCLUDE) $(USBINSTDIR)/HMP4040 $(USBINSTDIR)/Ke2110 $(USBINSTDIR)/Utils $(USBINSTDIR)/Drivers 
	ExternalObjects +=  -pthread -lboost_thread -lPh2_Drivers -lPh2_HMP4040 -lPh2_Ke2110 $(USBINSTFlag) 
	USBINSTLIBINSTALLED = yes
	Objs += $(ObjsExt)
else
	USBINSTLIBINSTALLED = no
endif

IncludePaths = $(IncludeDirs:%=-I%)
LibraryPaths = $(LibraryDirs:%=-L%)

.PHONY: clean print

#all: print $(Objs) $(ObjsExt).o ../HWDescription/Definition.h
	#$(CC) -std=c++11 -pthread -shared $(LibraryPaths) $(IncludePaths) $(ExternalObjects) -L/usr/lib64/ -o libPh2_Utils.so $(Objs) $(ObjsExt).o
	#mv libPh2_Utils.so ../lib

all: print $(Objs) ../HWDescription/Definition.h GenericPayload.h
	$(CC) -std=c++11 -pthread -shared $(LibraryPaths) $(IncludePaths) $(ExternalObjects) -L/usr/lib64/ -o libPh2_Utils.so $(Objs)
	mv libPh2_Utils.so ../lib

%.o: %.cc %.h 
	$(CXX) -std=c++11 $(CCFlags) -c -o $@ $<

#$(ObjsExt).o: $(ObjsExt).cc $(ObjsExt).h 
	#$(CXX) -std=c++11 $(CCFlags) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) -c -o $@ $<

print:
	@echo -e ${YEL}'****************************'${RESET}
	@echo -e ${YEL}'Building Utils'${RESET}
	@echo -e ${YEL}'Using zmq library: '${RED}$(ZMQINSTALLED)${RESET}
	@echo -e ${YEL}'Using Ph2_USBInstDriver: '${RED}$(USBINSTLIBINSTALLED)${RESET}
	@echo -e ${YEL}'****************************'${RESET}
clean:
	rm -f *.o
