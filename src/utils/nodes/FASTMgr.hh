/****************************************************************************/
/* Header of the DIET/FAST interface Manager                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.4  2006/11/16 09:55:56  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.3  2006/01/25 21:08:00  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.2  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.1.2.2  2004/11/26 15:20:26  alsu
 * change the estimate function to enable the SeD-level C interface
 *
 * Revision 1.1.2.1  2004/11/02 00:45:45  alsu
 * new estimate method to store data into an estVector_t, rather than
 * directly into a corba_estimation_t
 *
 * Revision 1.1  2003/08/01 19:16:55  pcombes
 * Add a manager for the FAST API, compatible with FAST 0.4 and FAST 0.8.
 * Any later changes in the FAST API should be processed by this static class.
 *
 ****************************************************************************/

#ifndef _FASTMGR_HH_
#define _FASTMGR_HH_

#include "DIET_server.h"
#include "common_types.hh"
#include "ServiceTable.hh"

/**
 * This class gathers all "FASTies"
 */

class FASTMgr
{
  
public:

  /**
   * Initialize the FAST library according to the configuration got in the
   * Parsers::Results.
   */
  static int
  init();
  
  /**
   * Get communication time for an amount of data of size \c size,<br>
   *  <ul><li> from \c host1 to \c host2 if \c to is true,</li>
   *      <li> from \c host2 to \c host1 else.            </li></ul>
   */
  static double
  commTime(char* host1, char* host2, unsigned long size, bool to = true);

  /**
   * Estimate the computation time for \c pb. \c cvt is used to convert the
   * DIET profile into a FAST profile.
   */
  static void
  estimate(char* hostName,
           const diet_profile_t* const profilePtr,
           const ServiceTable* SrvT,
           ServiceTable::ServiceReference_t ref,
#ifdef HAVE_CORI
		  int estTag,
#endif //HAVE_CORI
           estVector_t ev);

private:

  /** True if fast_init has already been called */
  static bool initialized;

  /** Use of FAST */
  static size_t use;

  /** Mutex for FAST calls (FAST is not reentrant yet) */
  static omni_mutex mutex;
};


#endif // _FASTMGR_HH_