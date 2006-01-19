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
 * Revision 1.11  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.10  2005/08/31 14:44:41  alsu
 * New plugin scheduling interface: minor changes
 *
 * Revision 1.9  2005/05/16 15:22:52  alsu
 * mostly small stuff, one big (stupid) bug
 *
 * Revision 1.8  2005/05/16 12:27:24  alsu
 * removing hard-coded nameLength fields
 *
 * Revision 1.7  2005/05/15 15:47:04  alsu
 * - implementing PriorityScheduler
 * - minor change to the chooseGlobalScheduler method
 *
 * Revision 1.6  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.5.2.4  2004/12/06 16:05:57  alsu
 * using RR scheduler only after the generic MIN/MAX schedulers
 *
 * Revision 1.5.2.3  2004/11/06 16:23:19  alsu
 * reordering adding RR, generic min, and generic max aggregators
 *
 * Revision 1.5.2.2  2004/10/31 22:17:30  alsu
 * minor code cleanup and adding the RR aggregator to the "scheduler"
 * stack.
 *
 * Revision 1.5.2.1  2004/10/27 22:35:50  alsu
 * include
 *
 * Revision 1.5  2004/10/15 08:21:17  hdail
 * - Removed references to corba_response_t->sortedIndexes - no longer useful.
 * - Removed sort functions -- they have been replaced by aggregate and are never
 *   called.
 *
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
#include "DIET_config.h"

#include "GlobalSchedulers.hh"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <string.h>

#include "debug.hh"
#include "Vector.h"

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
  int nameLength;

  {
    char *colon;
    if ((colon = strchr(serializedScheduler, ':')) != NULL) {
      nameLength = colon - serializedScheduler;
    }
    else {
      nameLength = strlen(serializedScheduler);
    }
  }

  if (!strncmp(serializedScheduler, StdGS::stName, nameLength)) {
    return StdGS::deserialize(serializedScheduler);
  }
  else if (!strncmp(serializedScheduler, PriorityGS::stName, nameLength)) {
    return PriorityGS::deserialize(serializedScheduler);
  }
  else {
    WARNING("unable to deserialize global scheduler ; "
	    << "reverting to default (StdGS)");
    cout << "scheduler was \"" << serializedScheduler << "\"\n";
    return new StdGS();
  }
}

/** Return the serialized global scheduler (a string). */
char*
GlobalScheduler::serialize(GlobalScheduler* GS)
{
  SCHED_TRACE_FUNCTION(GS->name);

  if (!strncmp(GS->name, StdGS::stName, GS->nameLength)) {
    return StdGS::serialize((StdGS*) GS);
  }
  else if (!strncmp(GS->name, PriorityGS::stName, GS->nameLength)) {
    return PriorityGS::serialize((PriorityGS*) GS);
  }
  else {
    ERROR("unable to serialize global scheduler named " << GS->name, NULL);
  }
}

/** Returns a global scheduler adapted to the request. */
GlobalScheduler*
GlobalScheduler::chooseGlobalScheduler()
{
  StdGS* res = new StdGS();
  res->init();
  return res;
}



GlobalScheduler*
GlobalScheduler::chooseGlobalScheduler(const corba_request_t* req,
                                       const corba_profile_desc_t* profile)
{
  corba_aggregator_desc_t agg = profile->aggregator;

  switch (agg.agg_specific._d()) {
  case DIET_AGG_DEFAULT:
    return (GlobalScheduler::chooseGlobalScheduler());
    break;
  case DIET_AGG_PRIORITY:
    PriorityGS* ps = new PriorityGS(agg.agg_specific.agg_priority());
    ps->init();
    return (ps);
    break;
  }

  ERROR(__FUNCTION__ <<
        ": unhandled aggregator (" <<
        agg.agg_specific._d() <<
        ")\n", NULL);
  return (NULL);
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

  /*
  ** structure for caching estVector structures.  initially this
  ** is a vector with as many empty vectors as there are
  ** responses.  during the aggregation phase, this cache will
  ** be populated.  at the end of aggregation, this cache needs
  ** to be purged (see below).
  */ 
  Vector_t evCache = new_Vector();

//   cout << "global scheduler: " << this->name << "\n";

  SCHED_TRACE_FUNCTION("nb_responses=" << nb_responses
		       << ",max_srv=" << max_srv);

  for (size_t i = 0; i < nb_responses; i++) {
    lastAggr[i]    = -1;
    total_size    += responses[i].servers.length();
    Vector_add(evCache, new_Vector());
  }
  if (max_srv == 0)
    max_srv = total_size; // keep all servers
  aggrResp->servers.length(MIN(total_size, max_srv));

  while (iter->hasCurrent()) {
    Scheduler* sched = iter->getCurrent();

//     fprintf(stderr,
//             "before %s, lastAggregated=%d\n",
//             Scheduler::serialize(sched),
//             lastAggregated);
    sched->aggregate(*aggrResp,
                     &lastAggregated,
                     nb_responses,
                     responses,
                     lastAggr,
                     evCache);
    iter->next();
  }

  {  /* purge the estVector cache */
    while (! Vector_isEmpty(evCache)) {
      Vector_t respV = (Vector_t) Vector_removeAtPosition(evCache, 0);
      while (! Vector_isEmpty(respV)) {
        Vector_removeAtPosition(respV, 0);
      }
      free_Vector(respV);
    }
    free_Vector(evCache);
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

StdGS::StdGS()
{
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}

StdGS::~StdGS() {};

  /** Initialize this global scheduler (build its list of schedulers). */
void
StdGS::init()
{
  // schedulers has already been contructed by implicit call to parent class
  // constructor.
#if !HAVE_CORI
  this->schedulers.addElement(new FASTScheduler());
  this->schedulers.addElement(new NWSScheduler(3.0, 2.0, 1.0));
#endif// !HAVE_CORI
  this->schedulers.addElement(new RRScheduler());
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
  size_t length = GS->nameLength;

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



/****************************************************************************/
/* PriorityGS (Priority List Global Scheduler)                              */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "PriorityGS"

const char*  PriorityGS::stName     = "PriorityGS";

PriorityGS::PriorityGS()
{
  this->name = this->stName;
  this->nameLength = strlen(this->name);
}
PriorityGS::PriorityGS(corba_agg_priority_t priority)
{
  this->name = this->stName;
  this->nameLength = strlen(this->name);
  this->numPValues = priority.priorityList.length();
  this->pValues = new int[this->numPValues];
  for (int pvIter = 0 ; pvIter < this->numPValues ; pvIter++) {
    this->pValues[pvIter] = priority.priorityList[pvIter];
  }
}

PriorityGS::~PriorityGS() {};

  /** Initialize this global scheduler (build its list of schedulers). */
void
PriorityGS::init()
{
  this->schedulers.addElement(new PriorityScheduler(this->numPValues,
                                                    this->pValues));
}

PriorityGS*
PriorityGS::deserialize(const char* serializedScheduler)
{
  char* token(NULL);
  char* ptr(NULL);
  char* ser_sched = strdup(serializedScheduler);
  PriorityGS* res = new PriorityGS();
  
  TRACE_TEXT(TRACE_ALL_STEPS,
	     "PriorityGS::deserialize(" << serializedScheduler << ")\n");
  token = strtok_r(ser_sched, ":", &ptr);
  assert(!strcmp(token, PriorityGS::stName));
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
PriorityGS::serialize(PriorityGS* GS)
{
  size_t maxLength = 128;
  char* res = new char[maxLength];
  SchedList::Iterator* iter = GS->schedulers.getIterator();
  size_t length = GS->nameLength;

  SCHED_TRACE_FUNCTION(GS->name);
  sprintf(res, GS->name);
//   cout << "res is " << res << "\n";
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
//     cout << "  res is " << res << "\n";
    length += tmp_length;
    delete [] tmp;
    iter->next();
  }
  delete iter;
  return res;
}
