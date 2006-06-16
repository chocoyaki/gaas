#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#include "fd.h"


void handle_sig (int sig)
{
	puts ("SIG !");
}

void handle_trans (fd_handle h)
{
	char machine [64];
	int service;
	int alive;

	fd_get_service (h, machine, 64, &service);
	alive = fd_alive (h);
	printf ("Transition : machine %s, service %d, alive %d",
			machine, service, alive);
	puts ("");
}

int main (int argv, char* argc[])
{
	fd_handle handle1, handle2;
	char hostname [256];

	gethostname (hostname, sizeof (hostname));

	fd_set_transition_handler (handle_trans);
	
	signal (SIGUSR1, handle_sig);
	fd_warn (SIGUSR1);

	fd_register_service (getpid(), 42);

	fd_register_service (getpid(), 29);
	
	handle1 = fd_make_handle ();
	fd_set_qos (handle1, 30.0, 2592000.0, 60.0);
	fd_set_service (handle1, hostname, 42);
	fd_observe (handle1);

	handle2 = fd_make_handle ();
	fd_set_qos (handle2, 17.0, 1000000.0, 60.0);
	fd_set_service (handle2, hostname, 29);
	fd_observe (handle2);

	sleep (3);
/*
	sleep (5);
	puts ("unregister");
	fd_unregister_service (42);
*/
	return 0;
}

