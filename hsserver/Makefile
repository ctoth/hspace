### Makefile for HSpace Portability Layer

.PHONY: all clean

#
# Don't change anything below this line.
#
INCLUDES=-I. -I../.. -I../../hdrs -I../hsportab -I../hsnetwork -I../
SHELL=/bin/sh
OBJDIR=obj
AR=ar
ARFLAGS=rcs
CC=gcc
CPP=cpp

SOURCES=HSServer.cpp HSClientConnection.cpp

OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
OUTPUT=libhsserver.a
PROJECTS=hsserver

# Used for protoizing

$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -c $< -o $@

all: $(PROJECTS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

hsserver: $(OBJDIR) $(OUTPUT)
	@echo "Done with HSServer."

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

