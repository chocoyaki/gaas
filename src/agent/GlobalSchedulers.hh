/****************************************************************************/
/* DIET agent global schedulers header : add yours !!!                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2003/04/10 12:58:08  pcombes
 * Interface for global schedulers, called by agents and associated to
 * requests. Add an implementation of this interface: StdGS.
 *
 ****************************************************************************/

#ifndef _GLOBAL_SCHEDULERS_HH_
#define _GLOBAL_SCHEDULERS_HH_

#include "LinkedList.hh"
#include "response.hh"
#include "Schedulers.hh"


typedef LinkedList<Scheduler> SchedList;


/**
 * This offers a global scheduler interface for the agent, to ease the tests on
 * several global schedulers. It is a virtual class, since its sort method is
 * pure virtual.
 */

class GlobalScheduler
{
public:
  GlobalScheduler();
  virtual
  ~GlobalScheduler();

  /**
   * Sort a sequence of servers depending on the GlobalScheduler type.
   *
   * The data for the servers are stored in \c servers, and the sequence
   * \c sortedIndexes keeps track of their order.
   */
  virtual int
  sort(SeqLong* sortedIndexes, SeqServerEstimation_t* servers) = 0;

  /**
   * Aggregate and sort all servers of the \c responses. This method provides a
   * default behaviour for aggregation of sorted responses: a simple merge.
   * @param aggrResp     the aggregated response (caller-allocated).
   * @param max_srv      maximum number of servers to aggregate (all if 0)
   * @param nb_responses number of responses to aggregate.
   * @param responses    array of the responses to aggregate.
   */
  int
  aggregate(corba_response_t* aggrResp, size_t max_srv,
	    size_t nb_responses, const corba_response_t* responses);

  /** Return the serialized global scheduler (a string). */
  static char*
  serialize(GlobalScheduler* GS);

  /**
   * Return the GlobalScheduler deserialized from the string
   * \c serializedScheduler.
   */
  static GlobalScheduler*
  deserialize(const char* serializedScheduler);

  /** Returns a global scheduler adapted to the request. */
  static GlobalScheduler*
  chooseGlobalScheduler(const corba_request_t* req);

protected:
  /** Name of this global scheduler */
  const char* name;
  /** Ordered list of sub-schedulers */
  SchedList schedulers;
};


/**
 * This global scheduler gives priority to the servers that gave most
 * information. Thus the ones which have perfromed FAST estimations are placed
 * first, then the ones that have performed NWS estimations, and finally the
 * ones that gave no information at all ordered randomly.
 */

class StdGS : public GlobalScheduler
{
public:
  /** Name of this global scheduler */
  static const char* stName;
  /** Length of the name of this global scheduler */
  static const size_t nameLength;


  StdGS();
  virtual
  ~StdGS();
  
  /** Initialize this global scheduler (build its list of schedulers). */
  void
  init();

  /**
   * Implement virtual sort method of class GlobalScheduler.
   * Sort the servers as follows:
   *  | ... FAST servers ... | ... NWS servers ... | ... Others ... |
   */
  int
  sort(SeqLong* sortedIndexes, SeqServerEstimation_t* servers);

  /** Return the serialized StdGS (a string). */
  static char*
  serialize(StdGS* GS);

  /** Return the StdGS deserialized from the string \c serializedScheduler. */
  static StdGS*
  deserialize(const char* serializedScheduler);
};


#endif // _GLOBAL_SCHEDULERS_HH_
