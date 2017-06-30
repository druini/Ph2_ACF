YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

Objs                    = FrontEndDescription.o BeBoard.o Cbc.o MPA.o Module.o 
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -Wall -fPIC -pedantic

IncludeDirs     =
IncludePaths            = $(IncludeDirs:%=-I%)

.PHONY: clean print

%.o: %.cc %.h
	$(CXX) -std=c++11 $(CCFlags) $(IncludePaths) -c -o $@ $<


all: print $(Objs)
	$(CC) -shared -o libPh2_Description.so $(Objs)
	mv libPh2_Description.so ../lib

print:
	@echo -e ${YEL}'****************************'${RESET}
	@echo -e ${YEL}'Building HWDescription objects'${RESET}
	@echo -e ${YEL}'****************************'${RESET}
clean:
	rm -f *.o
