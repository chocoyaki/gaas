#****************************************************************************#
#* DIET tutorial scalar by matrix product example: Makefile                 *#
#*                                                                          *#
#*  Author(s):                                                              *#
#*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       *#
#*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       *#
#*                                                                          *#
#* $LICENSE$                                                                *#
#****************************************************************************#

# Do not forget to point at the right Makefile.inc !!!
include Makefile.inc


client_smprod.o:	client_smprod.c
	$(CC) -c $< $(CCFLAGS) -o $@

client_smprod:		client_smprod.o
	$(CC) $< $(CCFLAGS) $(DIET_CLIENT_LIBS) -o $@

client_smprodAsync.o:	client_smprodAsync.c
	$(CC) -c $< $(CCFLAGS) -o $@

client_smprodAsync:		client_smprodAsync.o
	$(CC) $< $(CCFLAGS) $(DIET_CLIENT_LIBS) -o $@

client_smprod_file.o:	client_smprod_file.c
	$(CC) -c $< $(CCFLAGS) -o $@

client_smprod_file:	client_smprod_file.o
	$(CC) $< $(CCFLAGS) $(DIET_CLIENT_LIBS(LIBS) -o $@

server.o:	server.c
	$(CC) -c $< $(CCFLAGS) -o $@

server:		server.o
	$(CC) $< $(CCFLAGS) $(DIET_SERVER_LIBS) -o $@

clean:
	rm -f *.o client_smprod client_smprod_file server
