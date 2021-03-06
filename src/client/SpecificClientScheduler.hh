/**
 * @file  SpecificClientScheduler.hh
 *
 * @brief   Specific client scheduling : to implement some specific scheduling
 *
 * @author   Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _SPECIFICCLIENTSCHEDULER_HH_
#define _SPECIFICCLIENTSCHEDULER_HH_

#include "SeD.hh"
#include "response.hh"
#include <vector>
#include <string>

class SpecificClientScheduler {
public:
  SpecificClientScheduler();

  virtual ~SpecificClientScheduler();

  static const char *BurstRequest;

  static const char *BurstLimit;

  /**
   * initialize the scheduler
   *
   * @param scheduling_name scheduling name
   */
  static void
  setSchedulingId(const char *scheduling_name);

  static void
  start(SeD_var &chosenServer, corba_response_t *response);

  /**
   * Generic method to execute a specific scheduling.
   *
   * @param scheduling_name the scheduling name defined in
   *       USE_SPECIFIC_SCHEDULING entry in client configuration file
   * @param chosenServer  the SeD reference
   * @param reponse       the received response
   */
  virtual void
  schedule(const char *scheduling_name,
           SeD_var &chosenServer,
           corba_response_t *reponse);

  /**
   * Set the current SpecificScheduler
   */
  static void
  setScheduler(SpecificClientScheduler *scheduler);


  static void
  pre_diet_call();

  static void
  post_diet_call();

  virtual void
  preCall();

  virtual void
  postCall();

  /*
   * Return if the Burst scheduling is enabled
   */
  static bool
  isEnabled();

  /**
   * Utility method
   *
   * @param option option to check
   * @param params parameters vector
   */
  static bool
  isOptionEnabled(std::string option, std::vector<std::string> &params);

  /**
   * Utility method: remove blanks from a string
   *
   * @param token string to trim
   */
  static void
  removeBlanks(std::string &token);

  /**
   * Utility method: split a string given a delimiter
   *
   * @param str the string to split
   * @param delim string delimeter
   * @param results the elements of the split string
   */

  static void
  stringSplit(std::string str, std::string delim,
              std::vector<std::string> &results);

private:
  /**
   * Scheduler state
   */
  static bool enabled;

  /**
   * Scheduler ID
   */
  static std::string schedulerId;

  /**
   * The used scheduler
   */
  static SpecificClientScheduler *myScheduler;

  /**
   * Synchronisation semaphore
   */
  omni_semaphore mySem;

  /**
   * scheduling options
   */
  std::string schedulingOptions;

  /**
   * parameters vector
   */
  std::vector<std::string> myParams;

  /**
   * set the scheduling options
   */
  void
  setSchedulingOptions(const char *schedOptions);

  /**
   * BURST REQUEST : used to implement a better round robbin for
   * following configuration : 1 client -> 1 MA -> n SeDs.
   *
   * @param chosenServer the SeD reference
   * @param response     the MA response
   */
  virtual void
  burstRequest(SeD_var &chosenServer, corba_response_t *response);

  /**
   * BURST LIMIT REQUEST : used to implement a better round robbin for
   * following configuration : 1 client -> 1 MA -> n SeDs.
   * Additionnaly it allow only n simultaneous call
   *
   * @param chosenServer the SeD reference
   * @param response     the MA response
   */
  virtual void
  burstLimitRequest(SeD_var &chosenServer, corba_response_t *response);
};


#endif   /* not defined _SPECIFICCLIENTSCHEDULER._HH */
