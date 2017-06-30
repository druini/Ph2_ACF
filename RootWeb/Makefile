YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

ROOTFLAGS=`root-config --cflags`
ROOTLIBDIR=`root-config --libdir`
ROOTLIBFLAGS=`root-config --libs`
ROOTLIBFLAGS+=-lHistPainter
ifneq ($(strip $(BOOST_INCLUDE)),)
INCLUDEFLAGS=-I$(BOOST_INCLUDE)
endif
ifneq ($(strip $(BOOST_LIB)),)
BOOSTLIBFLAGS=-L$(BOOST_LIB)
endif
BOOSTLIBFLAGS+=-lboost_system$(BOOST_SUFFIX) -lboost_filesystem$(BOOST_SUFFIX) -lboost_program_options$(BOOST_SUFFIX)
GLIBFLAGS=`root-config --glibs`
INCLUDEFLAGS+=-Iinclude/
SRCDIR=src
INCDIR=include
LIBDIR=lib
# BINDIR=bin
# TESTDIR=test
COMPILERFLAGS+=-std=c++11 
COMPILERFLAGS+=-g -fPIC
COMPILERFLAGS+=-fpermissive
LINKERFLAGS+=-Wl,--copy-dt-needed-entries

CXX := g++

COMP=$(CXX) $(COMPILERFLAGS) $(INCLUDEFLAGS)

LINK=$(CXX) $(LINKERFLAGS)

.PHONY: clean print


all: print lib

# ROOT-related stuff
$(LIBDIR)/rootweb.o: $(SRCDIR)/rootweb.cpp $(INCDIR)/rootweb.hh
	$(COMP) $(ROOTFLAGS) -c -o $(LIBDIR)/rootweb.o $(SRCDIR)/rootweb.cpp

# Binary
lib: $(LIBDIR)/rootweb.o
	$(COMP) -shared -o $(LIBDIR)/libRootWeb.so $(LIBDIR)/rootweb.o -pthread $(BOOSTLIBFLAGS)

print:
	@echo -e ${YEL}'****************************'
	@echo -e ${YEL}'Building RootWeb'
	@echo -e ${YEL}'****************************'${RESET}
#CLEANUP
cleanall:
	@rm -rf $(LIBDIR)/*

clean: cleanall
