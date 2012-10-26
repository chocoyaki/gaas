/**
 * @file  Schedulers.hh
 *
 * @brief  DIET agent schedulers implementation : add yours !!!
 *
 * @author   Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _SCHEDULERS_HH_
#define _SCHEDULERS_HH_

#include "response.hh"
#include "DIET_data.h"
#include "DIET_server.h"
#include "marshalling.hh"
#include "Vector.h"


#define COMPARE_EQUAL 0
#define COMPARE_FIRST_IS_BETTER 1
#define COMPARE_SECOND_IS_BETTER 2
#define COMPARE_UNDEFINED 3

/** Type of a compare function. */
typedef int (*comp_fun_t)(int serverIdx1,
                          int serverIdx2,
                          int responseIdx1,
                          int responseIdx2,
                          const corba_response_t *responses,
                          Vector_t estVectorCache,
                          const void *data);


/**
 * This offers a scheduler interface for the agent, to ease the tests on several
 * schedulers. It is a virtual class, since its sort method is pure virtual.
 */

class Scheduler {
public:
  Scheduler();

  virtual ~Scheduler();

  /**
   * Aggregate all servers that this scheduler can process and that are stored
   * in the \c nb_responses \c responses above the \c lastAggr indexes. The
   * aggregation is dumped into \c aggrResp from \c *lastAggregated up to
   * \c aggrResp.servers.length().
   * @param aggrResp       the resulting aggregation.
   * @param lastAggregated index of last aggregated server in aggrResp
   *                       (modified by this method == fill index).
   * @param nb_responses   size of the responses array.
   * @param responses      array of all responses to aggregate.
   * @param lastAggr       array of indexes: each index points at the last
   *                       processed server in the correspunding response.
   * @param evCache        vector of vectors, storing previously constrctued
   *                       estVector objects
   */
  virtual int
  aggregate(corba_response_t &aggrResp,
            int *lastAggregated,
            const size_t nb_responses,
            const corba_response_t *responses,
            int *lastAggr,
            Vector_t evCache);

  /**
   * Return the serialized scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(Scheduler *S);

  /**
   * Return the Scheduler deserialized from the string \c serializedScheduler.
   */
  static Scheduler *
  deserialize(const char *serializedScheduler);

  /**
   * Return an estVector for the indicated server estimation
   */
  static estVectorConst_t
  getEstVector(int sIdx, int rIdx, const corba_response_t *responses);

  /**
   * Return an estVector for the indicated server estimation, using
   * the cached value, if available
   */
  static estVectorConst_t
  getEstVector(int sIdx, int rIdx,
               const corba_response_t *responses, Vector_t evCache);

protected:
  const char *name;
  int nameLength;
  SeqServerEstimation_t *servers;

  /**
   * Compare two servers. This is not a method, for it has to be passed down to
   * the qsort standard library.
   * @return COMP_ FIRST_IS_INF, -EQUAL or -SECOND_IS_INF if comparison was
   *         possible, and CANNOT_TREAT_<element> if some problems occur.
   * @see    comp_return_t and comp_fun_t types in AggregatedServers.hh
   */
  comp_fun_t compare;

  /** Additional information for comparison. */
  void *cmpInfo;
};

/**
 * This scheduler randomly sorts all servers that have no information filled in.
 */
class RandScheduler : public Scheduler {
public:
  RandScheduler();

  explicit
  RandScheduler(unsigned int seed);

  virtual ~RandScheduler();

  /**
   * Return the serialized Rand scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(RandScheduler *S);

  /**
   * Return the RandScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static RandScheduler *
  deserialize(const char *serializedScheduler);

  static const char *stName;

private:
  unsigned int seed;
};


class RRScheduler : public Scheduler {
public:
  RRScheduler();

  explicit
  RRScheduler(unsigned int seed);

  virtual ~RRScheduler();

  /**
   * Return the serialized RR scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(RRScheduler *S);

  /**
   * Return the RRScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static RRScheduler *
  deserialize(const char *serializedScheduler);

  static const char *stName;

private:
  unsigned int seed;
};

class MinScheduler : public Scheduler {
public:
  explicit
  MinScheduler(int tagval);

  virtual ~MinScheduler();

  /**
   * Return the serialized Min scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(MinScheduler *S);

  /**
   * Return the MinScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static MinScheduler *
  deserialize(const char *serializedScheduler);

  static const char *stName;

private:
  int tagval;
};

class MaxScheduler : public Scheduler {
public:
  explicit
  MaxScheduler(int tagval);

  virtual ~MaxScheduler();

  /**
   * Return the serialized Max scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(MaxScheduler *S);

  /**
   * Return the MaxScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static MaxScheduler *
  deserialize(const char *serializedScheduler);

  static const char *stName;

private:
  int tagval;
};

class PriorityScheduler : public Scheduler {
public:
  class priorityList {
public:
    int pl_numValues;
    int *pl_values;
  };

  static const char *stName;

  PriorityScheduler(int numValues, int *values);

  virtual ~PriorityScheduler();

  /**
   * Return the serialized Max scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char *
  serialize(PriorityScheduler *S);

  /**
   * Return the PriorityScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static PriorityScheduler *
  deserialize(const char *serializedScheduler);

private:
  PriorityScheduler::priorityList pl;
};

#endif  // _SCHEDULERS_HH_
