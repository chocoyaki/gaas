/****************************************************************************/
/* DIET agent globlal schedulers implementation : add yours !!!             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2004/09/14 12:43:55  hdail
 * Changed cleanup of ser_sched from delete to free to agree with alloc.
 *
 * Revision 1.3  2003/07/04 09:47:59  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.2  2003/05/05 14:29:51  pcombes
 * Add "all-steps" traces in all methods.
 *
 * Revision 1.1  2003/04/10 12:58:08  pcombes
 * Interface for global schedulers, called by agents and associated to
 * requests. Add an implementation of this interface: StdGS.
 ****************************************************************************/


#include "GlobalSchedulers.hh"

#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>

#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

// Use SCHED_CLASS for the name of the class
// (this->name cannot be used in static member functions)
#define SCHED_TRACE_FUNCTION(formatted_text)      \
  TRACE_TEXT(TRACE_ALL_STEPS, SCHED_CLASS << "::");\
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)


/****************************************************************************/
/* GlobalScheduler: parent class                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "GlobalScheduler"

GlobalScheduler::GlobalScheduler()
{
  this->name = "GlobalScheduler";
}

/** Destroy the schedulers list */
GlobalScheduler::~GlobalScheduler()
{
  this->schedulers.emptyIt();
}


/**
 * Return the GlobalScheduler deserialized from the string
 * \c serializedScheduler.
 */
GlobalScheduler*
GlobalScheduler::deserialize(const char* serializedScheduler)
{
  SCHED_TRACE_FUNCTION(serializedScheduler);

  if (!strncmp(serializedScheduler, StdGS::stName, StdGS::nameLength)) {
    return StdGS::deserialize(serializedScheduler);
  } else {
    WARNING("unable to deserialize global scheduler ; "
	    << "reverting to default (StdGS)");
    return new StdGS();
  }
}

/** Return the serialized global scheduler (a string). */
char*
GlobalScheduler::serialize(GlobalScheduler* GS)
{
  SCHED_TRACE_FUNCTION(GS->name);

  if (!strncmp(GS->name, StdGS::stName, StdGS::nameLength)) {
    return StdGS::serialize((StdGS*) GS);
  } else {
    ERROR("unable to serialize global scheduler named " << GS->name, NULL);
  }
}

/** Returns a global scheduler adapted to the request. */
GlobalScheduler*
GlobalScheduler::chooseGlobalScheduler(const corba_request_t* req)
{
  StdGS* res = new StdGS();
  res->init();
  return res;
}


/**
 * Aggregate and sort all servers of the \c responses. This method provides a
 * default behaviour for aggregation of sorted responses: a simple merge.
 * @param aggrResp     the aggregated response (caller-allocated).
 * @param max_srv      maximum number of servers to aggregate (all if 0)
 * @param nb_responses number of responses to aggregate.
 * @param responses    array of the responses to aggregate.
 */
int
GlobalScheduler::aggregate(corba_response_t* aggrResp, size_t max_srv,
			   const size_t nb_responses,
			   const corba_response_t* responses)
{ 
  size_t total_size = 0;
  // lock the schedulers 
  SchedList::Iterator* iter = this->schedulers.getIterator();

  //corba_response_t* aggrResp = new corba_response_t;
  int lastAggregated = -1;
  int* lastAggr = new int[nb_responses];

  SCHED_TRACE_FUNCTION("nb_responses=" << nb_responses
		       << ",max_srv=" << max_srv);

  for (size_t i = 0; i < nb_responses; i++) {
    lastAggr[i]    = -1;
    total_size    += responses[i].sortedIndexes.length();
  }
  if (max_srv == 0)
    max_srv = total_size; // keep all servers
  aggrResp->sortedIndexes.length(MIN(total_size, max_srv));
  aggrResp->servers.length(MIN(total_size, max_srv));

  while (iter->hasCurrent()) {
    Scheduler* sched = iter->getCurrent();
    sched->aggregate(*aggrResp, &lastAggregated,
		     nb_responses, responses, lastAggr);
    iter->next();
  }
  
  delete iter; // unlock the schedulers list
  delete [] lastAggr;
  
  return 0;
}


/****************************************************************************/
/* StdGS (Standard Global Scheduler)                                        */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "StdGS"

const char*  StdGS::stName     = "StdGS";
const size_t StdGS::nameLength = 5;

StdGS::StdGS()
{
  this->name = this->stName;
}

StdGS::~StdGS() {};

  /** Initialize this global scheduler (build its list of schedulers). */
void
StdGS::init()
{
  // schedulers has already been contructed by implicit call to parent class
  // constructor.
  this->schedulers.addElement(new FASTScheduler());
  this->schedulers.addElement(new NWSScheduler(3.0, 2.0, 1.0));
  this->schedulers.addElement(new RandScheduler());
}

StdGS*
StdGS::deserialize(const char* serializedScheduler)
{
  char* token(NULL);
  char* ptr(NULL);
  char* ser_sched = strdup(serializedScheduler);
  StdGS* res = new StdGS();
  
  TRACE_TEXT(TRACE_ALL_STEPS,
	     "StdGS::deserialize(" << serializedScheduler << ")\n");
  token = strtok_r(ser_sched, ":", &ptr);
  assert(!strcmp(token, StdGS::stName));
  if (*ptr != '\0')
    ptr[-1] = ':';
  while ((token = strtok_r(NULL, ":", &ptr)) != NULL) {
    res->schedulers.addElement(Scheduler::deserialize(token));
    if (*ptr != '\0')
      ptr[-1] = ':';
  }
  free(ser_sched);
  return res;
}

char*
StdGS::serialize(StdGS* GS)
{
  size_t maxLength = 128;
  char* res = new char[maxLength];
  SchedList::Iterator* iter = GS->schedulers.getIterator();
  size_t length = StdGS::nameLength;

  SCHED_TRACE_FUNCTION(GS->name);
  sprintf(res, GS->name);
  while (iter->hasCurrent()) {
    Scheduler* sched = iter->getCurrent();
    char* tmp = Scheduler::serialize(sched);
    size_t tmp_length = strlen(tmp) + 1; // Add 1 for the ':'
    if ((length + tmp_length) >= maxLength) {
      maxLength += 128;
      char* new_res = new char[maxLength];
      sprintf(new_res, "%s:%s", res, tmp);
      delete [] res;
      res = new_res;
    } else {
      sprintf((char*)(res+length), ":%s", tmp);
    }
    length += tmp_length;
    delete [] tmp;
    iter->next();
  }
  delete iter;
  return res;
}

int
StdGS::sort(SeqLong* sortedIndexes, SeqServerEstimation_t* servers)
{
  int lastSorted = -1;
  int res;
  SchedList::Iterator* iter = this->schedulers.getIterator();
  
  SCHED_TRACE_FUNCTION(servers->length() << " servers");
  while (iter->hasCurrent() && lastSorted < (int)servers->length()) {
    Scheduler* sched = iter->getCurrent();
    if ((res = sched->sort(sortedIndexes, &lastSorted, servers)))
      return res;
    iter->next();
  }
  if (lastSorted < (int)servers->length())
    WARNING("all servers have not been sorted");
  return 0;
}
