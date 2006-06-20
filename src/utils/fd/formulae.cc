/****************************************************************************/
/* DIET Fault Detector QoS dependent Parameters estimator                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/
#include <math.h>

#include "fdd.h"
#include "formulae.h"


/* why isn't that in math.h ? */
double square (double a)
{
	return a * a;
}

/* the configurator : takes statistical data and QoS requirements and computes
 * the parameters \alpha and \eta to achieve the actual observation.
 * All times are in seconds, probability (p_{L}) is between 0 and 1.
 * Returns a boolean : QoS is possible or not.
 */

/* auxiliary functions */

double conf_etamax (double pl, double vd, double tdu, double tmu)
{
	double gammadash;
	double gammadashtmu;
	double tdu2;

	tdu2 = square (tdu);
	gammadash = ((1.0 - pl) * tdu2) / (vd + tdu2);
	gammadashtmu = gammadash * tmu;
	if (gammadashtmu < tdu) return gammadashtmu; else return tdu;
}

double conf_f (double eta, double pl, double vd, double tdu)
{
	int j, jmax;
	double product = 1.0;
	double tdumje2;


	jmax = (int) ceil (tdu / eta);
	for (j = 1; j < jmax; ++j) {
		tdumje2 = square (tdu - j * eta);
		product *= (vd + tdumje2) / (vd + pl * tdumje2);
	}
	return eta * product;
}

/* the actual configurator */
int configure_nfde (double pl, double vd, double tdu, double tmrl, double tmu,
	double *alpha, double *eta) {

	double etamax, eta_floor, eta_ceiling, eta_current;
	double f_eta;

	etamax = conf_etamax (pl, vd, tdu, tmu);
	if (etamax <= 0.0) return 0;

	/* dichotomic search for the largest admissible value of eta */

	eta_floor = 0.0;
	eta_ceiling = etamax;
	eta_current = (eta_floor + eta_ceiling) / 2;

	do {
		f_eta = conf_f (eta_current, pl, vd, tdu);
		if (f_eta < tmrl)
			eta_ceiling = eta_current;
		else
			eta_floor = eta_current;
		eta_current = (eta_floor + eta_ceiling) / 2;
	} while (eta_ceiling - eta_floor > 0.001);

	*alpha = tdu - eta_current;
	*eta = eta_current;

	return 1;
}

/* calculates the average as in section 6.3 (see paper) */
double avg63 (double st[]) {
	int i;
	double total = 0;
	unsigned int nb = 0;

	/* we let i run through the whole sample since provided a datum is
	 * not 0, it really came from a message reception.
	 */
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		if (st[i] != 0.0) {
			total += st[i];
			++nb;
		}
	}
	/* nb should never be 0 since this function is always called *after*
	 * filling in a field in st, but anyway...
	 */
	if (nb == 0) return 0; else return total / (double) nb;
}

/* estimates V(D) */
double estimate_vd (double pv[]) {
	int i;
	double total = 0;
	double avg;
	unsigned int nb = 0;

	/* calculates sample average */
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		if (pv[i] != 0.0) {
			total += pv[i];
			++nb;
		}
	}
	avg = total / (double) nb;

	/* now the variance */
	total = 0;
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		if (pv[i] != 0.0) {
			total += square (pv[i] - avg);
		}
	}

	if ((nb == 0) || (nb == 1))
		return 0;
	else
		return total / (double) (nb - 1);
}

/* estimates p_{L} */
double estimate_pl (double vd[]) {
	int i;
	int missing = SAMPLE_SIZE;
	
	for (i = 0; i < SAMPLE_SIZE; ++i) {
		if (vd[i] != 0) {
			--missing;
		}
	}
	return (double) missing / (double) SAMPLE_SIZE;
}

/* converts a time expressed in seconds to a timespec */
void seconds_to_timespec (double d, struct timespec *ts)
{
	ts->tv_sec = (int) d;
	ts->tv_nsec = (int) (1000000000 * (d - (int) d));
}

double timespec_to_seconds (struct timespec *ts)
{
	return ((double) ts->tv_sec) + (((double) ts->tv_nsec) / 1000000000);
}

double timeval_to_seconds (struct timeval *tv)
{
	return ((double) tv->tv_sec) + (((double) tv->tv_usec) / 1000000);
}


