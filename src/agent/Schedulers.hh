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


/**
 * Return values for compare functions. These values are chosen to match the
 * behaviour required by the standard qsort function:
 *  < 0: first element is inf (which is also the case if the scheduler cannot
 *       manage the second element)
 *  = 0: both elements equal
 *  > 0: second element is inf (which is also the case if the scheduler cannot
 *       manage the first element)
 * NB: if none of both cannot be managed, no matter the sign of the result,
 *     but it is useful for implementation to get BOTH = FIRST + SECOND ...
 */
#define COMP_CANNOT_TREAT_SECOND -2
#define COMP_FIRST_IS_INF        -1
#define COMP_EQUAL                0
#define COMP_SECOND_IS_INF        1
#define COMP_CANNOT_TREAT_BOTH    4 // CANNOT_TREAT_FIRST + CANNOT_TREAT_SECOND
#define COMP_CANNOT_TREAT_FIRST   6

/** Type of a compare function. */
typedef int (* comp_fun_t)(const void*, const void*,
			   const SeqServerEstimation_t*,
			   const SeqServerEstimation_t*, const void*);


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
   */
  virtual int
  aggregate(corba_response_t& aggrResp, int* lastAggregated,
	    const size_t nb_responses,
	    const corba_response_t* responses, int* lastAggr);

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
  deserialize(char* serializedScheduler);

protected:
  const char*  name;
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
  static const size_t nameLength;

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
  deserialize(char* serializedScheduler);

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
  static const size_t nameLength;

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
  deserialize(char* serializedScheduler);

private:
  double epsilon;
  weight_info_t wi;
};


/**
 * This scheduler randomly sorts all servers that have no information filled in.
 */

class RandScheduler : public Scheduler
{
public:
  static const char*  stName;
  static const size_t nameLength;

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
  deserialize(char* serializedScheduler);

private:
  unsigned int seed;
};


#endif // _SCHEDULERS_HH_
