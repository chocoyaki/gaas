#****************************************************************************#
#* DIET tutorial dgemm example: Makefile                                    *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#

# Do not forget to point at the right Makefile.inc !!!
include Makefile.inc

BLASLIBS  = -lm -lblas -lm -lg2c

client.o:	client.c
	$(CC) -c $< $(CCFLAGS) -o $@

client:		client.o
	$(CC) $< $(CCFLAGS) $(DIET_CLIENT_LIBS) -o $@

server.o:	server.c
	$(CC) -c $< $(CCFLAGS) -o $@

server:		server.o
	$(CC) $< $(CCFLAGS) $(DIET_SERVER_LIBS) $(BLASLIBS) -o $@

clean:
	rm -f *.o client server
