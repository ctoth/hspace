### Makefile for HSpace

.PHONY: all clean hsportab hsnetwork hsserver

# Change this to one of the following supported types:
#
# PENNMUSH
#
# ... that's it for now. ;)
MUSHTYPE=PENNMUSH

# Debug version
#CPPFLAGS=-Wall -Wno-comment -D$(MUSHTYPE) -DHSPACE -DHS_LOG -g

# Release version.
CPPFLAGS=-D$(MUSHTYPE) -DHSPACE -O2 -DNDEBUG
#
# Don't change anything below this line.
#
INCLUDES=-I. -I../.. -I../../hdrs -I./hsportab -I./hsserver
SHELL=/bin/sh
OBJDIR=obj
AR=ar
ARFLAGS=rcs
CC=gcc
CPP=cpp

SOURCES=$(wildcard *.cpp) sqlite3.c
OBJFILES=$(SOURCES:%.cpp=$(OBJDIR)/%.o)
DEPENDENCIES=$(SOURCES:%.cpp=$(OBJDIR)/%.d)
OUTPUT=libhspace.a
PROJECTS=hsportab hspace hsnetwork hsserver

# Used for protoizing

$(OBJDIR)/%.o : %.cpp
	$(CC) $(INCLUDES) $(CPPFLAGS) -c $< -o $@

all: $(PROJECTS)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

revision:
	if test -e updaterev.sh; then ./updaterev.sh; fi

hspace: revision $(OBJDIR) $(OUTPUT)
	@echo "Done with HSpace."

hsportab:
	$(MAKE) -C hsportab CPPFLAGS='$(CPPFLAGS)'

hsserver:
	$(MAKE) -C hsserver CPPFLAGS='$(CPPFLAGS)'

hsnetwork:
	$(MAKE) -C hsnetwork CPPFLAGS='$(CPPFLAGS)'

$(OUTPUT): $(SOURCES) $(OBJFILES) sqlite3.o
	$(AR) $(ARFLAGS) $(OUTPUT) $(OBJFILES) sqlite3.o


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

