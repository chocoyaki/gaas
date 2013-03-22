/**
 * @file  GlobalSchedulers.hh
 *
 * @brief  DIET agent globlal schedulers implementation : add yours !!!
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _GLOBAL_SCHEDULERS_HH_
#define _GLOBAL_SCHEDULERS_HH_

#include "LinkedList.hh"
#include "response.hh"
#include "Schedulers.hh"


typedef LinkedList<Scheduler> SchedList;




class GlobalScheduler {
public:
  GlobalScheduler();
  virtual  ~GlobalScheduler();

  /**
   * Aggregate and sort all servers of the \c responses. This method provides a
   * default behaviour for aggregation of sorted responses: a simple merge.
   * @param aggrResp     the aggregated response (caller-allocated).
   * @param max_srv      maximum number of servers to aggregate (all if 0)
   * @param nb_responses number of responses to aggregate.
   * @param responses    array of the responses to aggregate.
   */
  /* New : For scheduler load support.
     Allows to overload the aggregate method.*/
#ifdef USERSCHED
  virtual
#endif
  /*************************************/
  int
  aggregate(corba_response_t *aggrResp,
            size_t max_srv,
            const size_t nb_responses,
            const corba_response_t *responses);

  /** Return the serialized global scheduler (a string). */
  static char *
  serialize(GlobalScheduler *GS);

  /**
   * Return the GlobalScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static GlobalScheduler *
  deserialize(const char *serializedScheduler);

  /** Returns a global scheduler adapted to the request. */
  static GlobalScheduler *
  chooseGlobalScheduler();

  static GlobalScheduler *
  chooseGlobalScheduler(const corba_request_t *req,
                        const corba_profile_desc_t *profile);

protected:
  /** Name of this global scheduler */
  const char *name;
  int nameLength;
  /** Ordered list of sub-schedulers */
  SchedList schedulers;
};


/**
 * This global scheduler gives priority to the servers that gave most
 * information.
 * if HAVE_CORI: round robin scheduling and if the information is not
 * available then random scheduling
 * else :Thus the ones which have performed FAST estimations are placed
 * first, then the ones that have performed NWS estimations, and finally the
 * ones that gave no information at all ordered randomly.
 */

class StdGS : public GlobalScheduler {
public:
  /** Name of this global scheduler */
  static const char *stName;

  StdGS();
  virtual
  ~StdGS();

  /** Initialize this global scheduler (build its list of schedulers). */
  void
  init();

  /** Return the serialized StdGS (a string). */
  static char *
  serialize(StdGS *GS);

  /** Return the StdGS deserialized from the string \c serializedScheduler. */
  static StdGS *
  deserialize(const char *serializedScheduler);
};



class PriorityGS : public GlobalScheduler {
public:
  /** Name of this global scheduler */
  static const char *stName;

  int numPValues;
  int *pValues;

  PriorityGS();

  explicit
  PriorityGS(corba_agg_priority_t priority);

  virtual
  ~PriorityGS();

  /** Initialize this global scheduler (build its list of schedulers). */
  void
  init();

  /** Return the serialized PriorityGS (a string). */
  static char *
  serialize(PriorityGS *GS);

  /** Return the PriorityGS deserialized from the string \c
      serializedScheduler. */
  static PriorityGS *
  deserialize(const char *serializedScheduler);
};


#endif  // _GLOBAL_SCHEDULERS_HH_
