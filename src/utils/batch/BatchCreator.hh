/**
* @file BatchCreator.hh
* 
* @brief  Batch System Management with performance prediction
* 
* @author - Yves Caniou (yves.caniou@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.9  2010/06/15 18:21:52  amuresan
 * Added separate compilation flag for Cloud features.
 * Removed gSOAP files from the kit, referencing an installed gSOAP package in cmake.
 * Added meta headers for cloud example files.
 * Added Cloud-related files to the release listing.
 *
 * Revision 1.8  2010/05/05 13:13:51  amuresan
 * First commit for the Eucalyptus BatchSystem.
 * Added SOAP client for the Amazon EC2 SOAP interface and
 * a new implementation of the BatchSystem base-class.
 *
 * Revision 1.7  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.6  2008/08/17 08:10:19  bdepardo
 * Added PBS batch system
 * /!\ not tested yet
 *
 * Revision 1.5  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.4  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.3  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.2  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
 * Revision 1.1  2007/04/16 22:34:39  ycaniou
 * Added the class to create the correct batch system
 *
 *
 ****************************************************************************/

#ifndef _BATCH_CREATOR_HH_
#define _BATCH_CREATOR_HH_

class BatchSystem;

class BatchCreator {
public:
  typedef enum {
    OAR1_6,
    LOADLEVELER,
    SGE,
    PBS,
    OAR2_X,
    SLURM,
#ifdef HAVE_CLOUD
    EUCALYPTUS,
#endif
    NUMBER_OF_SUPPORTED_BATCH
  } batchID_t;

  static const char * const batchNames[NUMBER_OF_SUPPORTED_BATCH];

  BatchCreator();

  /** Returns a new inherited instance of the class BatchSystem corresponding
      to the name @param batchName .
      Returns NULL if batch system is not managed.
  */
  static BatchSystem *
  getBatchSystem(const char * batchName);

  /** Returns 1 if the batch system is managed by DIET, 0 otherwise */
  static int
  existBatchScheduler(const char * batchName, batchID_t * batchID);
};

#endif
