# The following inclusion provides convenient make variables for compiling
# and linking against the DIET library (DIET_HOME is an environment variable
# containing the path to a DIET installation directory):
include ${DIET_HOME}/include/Makefile.inc

all: simple_client simple_server

simple_client: simple_client.c $(DIET_CLIENT_PREREQ)
	$(CXX) -g $(CXXFLAGS_DIET) $< $(DIET_CLIENT_LIBS) -o $@
simple_server: simple_server.c  $(DIET_SERVER_PREREQ)
	$(CC) -g $(CCFLAGS_DIET) $< $(DIET_SERVER_LIBS) -o $@
clean: 
	rm -f simple_client simple_server
