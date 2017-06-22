Objs            = TrackerEvent.o ParamSet.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC 

IncludeDirs =  $(BOOST_INCLUDE)
LibraryDirs = ../lib

IncludePaths = $(IncludeDirs:%=-I%)
LibraryPaths = $(LibraryDirs:%=-L%)

ExternalObjects =

.PHONY: print clean
%.o: %.cc %.h
	$(CXX) -std=c++0x  $(CCFlags) $(IncludePaths) $(LibraryPaths) -c -o $@ $<

all: print $(Objs)
	$(CC) -std=c++0x -shared -o libPh2_Tracker.so $(Objs)
	mv libPh2_Tracker.so ../lib

print:
	@echo '****************************'
	@echo 'Building Tracker Data Format'
	@echo '****************************'
clean:
	rm -f *.o

