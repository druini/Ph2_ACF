Objs            = Exception.o Utilities.o Event.o Data.o argvparser.o  FileHandler.o
CC              = gcc
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC `root-config --cflags --evelibs` -Wcpp -L/usr/lib64/
#DevFlags                   = -D__CBCDAQ_DEV__
DevFlags			 =

IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

.PHONY: clean print


%.o: %.cc %.h 
	$(CXX) -std=c++11  $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++11 -pthread -shared -L/usr/lib64/ -o libPh2_Utils.so $(Objs) -pthread
	mv libPh2_Utils.so ../lib

print:
	@echo '****************************'
	@echo 'Building Utils'
	@echo '****************************'
clean:
	rm -f *.o
