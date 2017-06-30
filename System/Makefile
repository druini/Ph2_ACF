YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

Objs            = FileParser.o SystemController.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pthread -pedantic -fPIC 

.PHONY: clean print

IncludeDirs = ../ $(CACTUSINCLUDE) 
LibraryDirs = ../lib
ExternalObjects = -lPh2_Interface -lPh2_Description -lPh2_Utils

IncludePaths = $(IncludeDirs:%=-I%)
LibraryPaths = $(LibraryDirs:%=-L%)

%.o: %.cc %.h
	$(CXX) -std=c++0x $(CCFlags) $(IncludePaths) $(LibraryPaths) -c -o $@ $<

all: print $(Objs)
	$(CC) -std=c++0x -shared -o libPh2_System.so $(Objs)
	mv libPh2_System.so ../lib

print:
	@echo -e${YEL}'****************************'
	@echo -e'Building System Components '
	@echo -e'****************************'${RESET}
clean:
	rm -f *.o

