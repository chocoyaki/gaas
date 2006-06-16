	/***************************************\
	 * Failure detector - Formulae         *
	 * Aurélien Bouteiller                 *
	\***************************************/




#ifndef _FORMULAE_H_
#define _FORMULAE_H_


#include <sys/time.h>


/* the configurator */
int configure_nfde (double pl, double vd, double tdu, double tmrl, double tmu,
	double *alpha, double *eta);

/* calculates the average as in section 6.3 (see paper) */
double avg63 (double st[]);

/* estimates V(D) */
double estimate_vd (double pv[]);

/* estimates p_{L} */
double estimate_pl (double vd[]);

/* converts a time expressed in seconds to a timespec */
void seconds_to_timespec (double d, struct timespec *ts);

double timespec_to_seconds (struct timespec *ts);

double timeval_to_seconds (struct timeval *tv);



#endif /* _FORMULAE_H_ */

