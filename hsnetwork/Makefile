### Makefile for HSpace Portability Layer

.PHONY: all clean

#
# Don't change anything below this line.
#
INCLUDES=-I. -I../.. -I../../hdrs -I../hsportab 
SHELL=/bin/sh
OBJDIR=obj
AR=ar
ARFLAGS=rcs
CC=gcc
CPP=cpp

SOURCES=$(wildcard *.cpp)

OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
OUTPUT=libhsnetwork.a
PROJECTS=hsnetwork

# Used for protoizing

$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -c $< -o $@

all: $(PROJECTS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

hsnetwork: $(OBJDIR) $(OUTPUT)
	@echo "Done with HSNetwork."

$(OUTPUT): $(SOURCES) $(OBJFILES)
	$(AR) $(ARFLAGS) $(OUTPUT) $(OBJFILES)


clean:
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.a -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;

distclean:
	find . -name \*.o -exec rm -f {} \;
	find . -name \*.a -exec rm -f {} \;
	find . -name \*.d -exec rm -f {} \;
	find . -name \*~ -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;

