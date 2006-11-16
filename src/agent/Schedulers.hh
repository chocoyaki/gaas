/****************************************************************************/
/* DIET agent schedulers header : add yours !!!                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.9  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.8  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.7  2005/08/31 14:47:41  alsu
 * New plugin scheduling interface: adapting the various schedulers to
 * access performance data using the new estimation vector interface
 *
 * Revision 1.6  2005/05/16 12:27:24  alsu
 * removing hard-coded nameLength fields
 *
 * Revision 1.5  2005/05/15 15:50:49  alsu
 * implementing PriorityScheduler
 *
 * Revision 1.4  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.3.2.4  2004/11/06 16:32:18  alsu
 * adding generic min and generic max aggregators
 *
 * Revision 1.3.2.3  2004/11/02 00:37:28  alsu
 * removing references to obviated fields in the estimation data structure
 *
 * Revision 1.3.2.2  2004/10/31 22:21:52  alsu
 * - restructured the old COMP_* definitions to be more "logcial" rather
 *   than absolute (i.e., COMP_FIRST_IS_INF => COMPARE_FIRST_IS_BETTER)
 * - added class methods to the Scheduler class to construct (and cache)
 *   estimation value vectors during the server sorting process
 * - implementation of the new default round-robin scheduler that is
 *   consistent with the dynamic performance data design
 *
 * Revision 1.3.2.1  2004/10/27 22:35:51  alsu
 * include
 *
 * Revision 1.3  2004/10/15 08:21:17  hdail
 * - Removed references to corba_response_t->sortedIndexes - no longer useful.
 * - Removed sort functions -- they have been replaced by aggregate and are never
 *   called.
 *
 * Revision 1.2  2003/05/05 14:50:15  pcombes
 * Changing the computation of server weights changes NWSScheduler fields.
 *
 * Revision 1.1  2003/04/10 12:57:15  pcombes
 * Interface, plus three examples, for agent schedulers.
 ****************************************************************************/

#ifndef _SCHEDULERS_HH_
#define _SCHEDULERS_HH_

#include "response.hh"
#include "DIET_data.h"
#include "DIET_server.h"
#include "marshalling.hh"
#include "Vector.h"


/**
 * Return values for compare functions.
 */
#define COMPARE_EQUAL             0
#define COMPARE_FIRST_IS_BETTER   1
#define COMPARE_SECOND_IS_BETTER  2
#define COMPARE_UNDEFINED         3

/** Type of a compare function. */
typedef int (*comp_fun_t)(int serverIdx1,
                          int serverIdx2,
                          int responseIdx1,
                          int responseIdx2,
                          const corba_response_t* responses,
                          Vector_t estVectorCache,
                          const void* data);


/**
 * This offers a scheduler interface for the agent, to ease the tests on several
 * schedulers. It is a virtual class, since its sort method is pure virtual.
 */

class Scheduler
{
public:
  Scheduler();
  virtual
  ~Scheduler();
  
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
  aggregate(corba_response_t& aggrResp,
            int* lastAggregated,
	    const size_t nb_responses,
	    const corba_response_t* responses,
            int* lastAggr,
            Vector_t evCache);

  /**
   * Return the serialized scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(Scheduler* S);

  /**
   * Return the Scheduler deserialized from the string \c serializedScheduler.
   */
  static Scheduler*
  deserialize(const char* serializedScheduler);

  /**
   * Return an estVector for the indicated server estimation
   */
  static estVectorConst_t getEstVector(int sIdx,
                                       int rIdx,
                                       const corba_response_t* responses);

  /**
   * Return an estVector for the indicated server estimation, using
   * the cached value, if available
   */
  static estVectorConst_t getEstVector(int sIdx,
                                       int rIdx,
                                       const corba_response_t* responses,
                                       Vector_t evCache);
  

protected:
  const char* name;
  int nameLength;
  SeqServerEstimation_t* servers;

  /**
   * Compare two servers. This is not a method, for it has to be passed down to
   * the qsort standard library.
   * @return COMP_ FIRST_IS_INF, -EQUAL or -SECOND_IS_INF if comparison was
   *         possible, and CANNOT_TREAT_<element> if some problems occur.
   * @see    comp_return_t and comp_fun_t types in AggregatedServers.hh
   */
  comp_fun_t compare;
  
  /** Additional information for comparison. */
  void* cmpInfo;
};

#if !HAVE_CORI 
// scheduler FAST and NWS will be used in the next DIET version 
// like all other schedulers: in the scheduler-plugin

/**
 * This scheduler sorts servers that have performed FAST estimations, according
 * to the total needed time (comp + comm).  All servers whose total needed time
 * differ one from another of less than espilon are supposed to be equal : thus,
 * they are sorted randomly.
 * The test for "server has performed FAST estimations" is tComp < HUGE_VAL ...
 *
 * NB: if epsilon is 0 (default), then the random sorting is performed on
 * servers that have estimated exactly the same needed time, which should be
 * very rare.
 */

class FASTScheduler : public Scheduler
{
public:
  static const char*  stName;

  FASTScheduler();
  FASTScheduler(double epsilon);
  virtual
  ~FASTScheduler();

  /**
   * Return the serialized FAST scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(FASTScheduler* S);
  
  /**
   * Return the FASTScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static FASTScheduler*
  deserialize(const char* serializedScheduler);

private:
  double epsilon;
};


/**
 * This scheduler sorts servers that have performed NWS estimations, according
 * to a polynomial expression of available CPU, available memory and
 * communication time :
 *    (comm time)^commPower / ((av. CPU)^CPUPower * (av. mem)^memPower)
 * The test for "server has performed NWS estimation" is freeCPU >= 0 ...
 *
 * NB: if epsilon is 0 (default), then the random sorting is performed on
 * servers that have estimated exactly the same needed time, which should be
 * very rare.
 */

class NWSScheduler : public Scheduler
{
public:
  static const char*  stName;

  typedef struct {
    double CPUPower;
    double memPower;
    double commPower;
  } weight_info_t;

  NWSScheduler();
  NWSScheduler(double CPUPower, double memPower, double commPower);
  NWSScheduler(double epsilon,
	       double CPUPower, double memPower, double commPower);
  virtual
  ~NWSScheduler();

  /**
   * Return the serialized NWS scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(NWSScheduler* S);
  
  /**
   * Return the NWSScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static NWSScheduler*
  deserialize(const char* serializedScheduler);

private:
  double epsilon;
  weight_info_t wi;
};
#endif //!HAVE_CORI

/**
 * This scheduler randomly sorts all servers that have no information filled in.
 */

class RandScheduler : public Scheduler
{
public:
  static const char*  stName;

  RandScheduler();
  RandScheduler(unsigned int seed);
  virtual
  ~RandScheduler();

  /**
   * Return the serialized Rand scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(RandScheduler* S);
  
  /**
   * Return the RandScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static RandScheduler*
  deserialize(const char* serializedScheduler);

private:
  unsigned int seed;
};


class RRScheduler : public Scheduler
{
public:
  static const char*  stName;

  RRScheduler();
  RRScheduler(unsigned int seed);
  virtual
  ~RRScheduler();

  /**
   * Return the serialized RR scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(RRScheduler* S);
  
  /**
   * Return the RRScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static RRScheduler*
  deserialize(const char* serializedScheduler);

private:
  unsigned int seed;
};

class MinScheduler : public Scheduler
{
public:
  static const char*  stName;

  MinScheduler(int tagval);
  virtual
  ~MinScheduler();

  /**
   * Return the serialized Min scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(MinScheduler* S);
  
  /**
   * Return the MinScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static MinScheduler*
  deserialize(const char* serializedScheduler);

private:
  int tagval;
};

class MaxScheduler : public Scheduler
{
public:
  static const char*  stName;

  MaxScheduler(int tagval);
  virtual
  ~MaxScheduler();

  /**
   * Return the serialized Max scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(MaxScheduler* S);
  
  /**
   * Return the MaxScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static MaxScheduler*
  deserialize(const char* serializedScheduler);

private:
  int tagval;
};

class PriorityScheduler : public Scheduler
{
public:
  class priorityList {
  public:
    int pl_numValues;
    int *pl_values;
  };

  static const char*  stName;

  PriorityScheduler(int numValues, int *values);
  virtual
  ~PriorityScheduler();

  /**
   * Return the serialized Max scheduler (a string)
   * NB: doubles are serialized with a precision of 10 significant decimals.
   */
  static char*
  serialize(PriorityScheduler* S);
  
  /**
   * Return the PriorityScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static PriorityScheduler*
  deserialize(const char* serializedScheduler);

private:
  PriorityScheduler::priorityList pl;
};

#endif // _SCHEDULERS_HH_
