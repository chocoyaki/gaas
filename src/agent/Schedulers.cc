/****************************************************************************/
/* DIET agent schedulers implementation : add yours !!!                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.6  2004/10/15 08:21:17  hdail
 * - Removed references to corba_response_t->sortedIndexes - no longer useful.
 * - Removed sort functions -- they have been replaced by aggregate and are never
 *   called.
 *
 * Revision 1.5  2004/09/14 12:45:28  hdail
 * Changed mispelling of input variable for NWSScheduler variable.  Should correct
 * bug where can not change setting of epsilon for NWSScheduler.
 *
 * Revision 1.4  2004/05/18 21:07:10  alsu
 * problems fixed in the process of building in support for custom
 * performance metrics:
 *  - call to comparator used wrong server lists
 *  - code to move entries up the sort tree took max, not min value
 *
 * Revision 1.3  2003/07/04 09:47:59  pcombes
 * Use new ERROR, WARNING and TRACE macros.
 *
 * Revision 1.2  2003/05/05 14:46:21  pcombes
 * Add traces for all methods. aggregate: fix bug in the computation of pow.
 * Improve the server weight computation for NWSScheduler.
 *
 * Revision 1.1  2003/04/10 12:57:15  pcombes
 * Interface, plus three examples, for agent schedulers.
 ****************************************************************************/


#include "Schedulers.hh"

#include <iostream>
using namespace std;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

// Use SCHED_CLASS for the name of the class
// (this->name cannot be used in static member functions)
#define SCHED_TRACE_FUNCTION(formatted_text)      \
  TRACE_TEXT(TRACE_ALL_STEPS, SCHED_CLASS << "::");\
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)


/**
 * Random permutation of the sub-sequence [\c first_idx, ..., \c last_idx] of
 * \c seq.
 */
void
random_permute(SeqLong* seq, int first_idx, int last_idx);


/****************************************************************************/
/* Scheduler                                                                */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "Scheduler"

Scheduler::Scheduler() {
  this->name = "Scheduler";
}
Scheduler::~Scheduler() {}

/**
 * Return the serialized scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
Scheduler::serialize(Scheduler* S)
{
  SCHED_TRACE_FUNCTION((void*)S->name);

  if (!strncmp(S->name,
	       FASTScheduler::stName, FASTScheduler::nameLength)) {
    return FASTScheduler::serialize((FASTScheduler*) S);
  } else if (!strncmp(S->name,
		      NWSScheduler::stName, NWSScheduler::nameLength)) {
    return NWSScheduler::serialize((NWSScheduler*) S);
  } else if (!strncmp(S->name,
		      RandScheduler::stName, RandScheduler::nameLength)) {
    return RandScheduler::serialize((RandScheduler*) S);
  } else {
    INTERNAL_ERROR("unable to serialize scheduler named " << S->name, 1);
  }
}

/**
 * Return the Scheduler deserialized from the string \c serializedScheduler.
 */
Scheduler*
Scheduler::deserialize(char* serializedScheduler)
{
  SCHED_TRACE_FUNCTION(serializedScheduler);

  if (!strncmp(serializedScheduler,
	       FASTScheduler::stName, FASTScheduler::nameLength)) {
    return FASTScheduler::deserialize(serializedScheduler);
  } else if (!strncmp(serializedScheduler,
		      NWSScheduler::stName, NWSScheduler::nameLength)) {
    return NWSScheduler::deserialize(serializedScheduler);
  } else if (!strncmp(serializedScheduler,
		      RandScheduler::stName, RandScheduler::nameLength)) {
    return RandScheduler::deserialize(serializedScheduler);
  } else {
    INTERNAL_WARNING("unable to deserialize scheduler ; "
	    << "reverting to default (random)");
    return new RandScheduler();
  }
}

/* agregate is non-static: use this->name for the SCHED_TRACE_FUNCTION */
#undef SCHED_CLASS
#define SCHED_CLASS this->name

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
int
Scheduler::aggregate(corba_response_t& aggrResp, int* lastAggregated,
		     const size_t nb_responses,
		     const corba_response_t* responses, int* lastAggr)
{

  /**
   * For default aggregation method, we decide to merge all server sequences at
   * once.
   *
   * A simplist algorithm would compute at each step (each new element inserted in
   * the result sequence) the minimum of all elements that arre still to be
   * processed in all sequences.
   *
   * Our idea is to minimize the computation of the element to insert by keeping
   * track of minima in a binary tree structure, with as many leaves as there are
   * responses to process. Each node is the minimum of its two children, and
   * thus, the root is the element to insert. It is then easy to replace the
   * corresponding leaf by the next element in the sequence it was extract from.
   */

  typedef struct {
    int resp_idx, srv_idx;
  } node_t;
  typedef node_t* level_t;

  /* Number of actual leaves of the tree (== nb_responses). */
  size_t nb_leaves = nb_responses;
  /* The lowest power of 2 greater or equal than nb_leaves. */
  size_t pow;
  /* Array of (pow + 1) arrays: each array is a level of the binary tree. */
  level_t* levels;
  /* Point at the root. */
  node_t* root;
  /* Point at the leaves array. */
  level_t leaves;

  /** Print the tree on standard output */
#define TRACE_TREE(levels,pow)                                                \
  for (size_t i = 0; i <= pow; i++) {				              \
    cout << ' ';							      \
    for (int j = 0; j < (1 << i); j++) {				      \
      cout << ' ' << (levels[i])[j].resp_idx << ',' << (levels[i])[j].srv_idx;\
    }									      \
    cout << endl;							      \
  }


  /** Update the tree, ie propagate minima down to the root. */
#define UPDATE_TREE(levels,pow,responses) \
    for (int i = pow - 1; i >= 0; i--) {                                       \
      for (int j = 0; j < (1 << i); j++) {				       \
	node_t* fst = &((levels[i + 1])[2*j]);				       \
	node_t* snd = &((levels[i + 1])[2*j + 1]);			       \
									       \
	if (fst->resp_idx == -1)					       \
	  (levels[i])[j] = *snd;					       \
	else if (snd->resp_idx == -1)					       \
	  (levels[i])[j] = *fst;					       \
	else {								       \
	  int cmp =							       \
	    (*compare)(&(fst->srv_idx),                                        \
		       &(snd->srv_idx),                                        \
		       &(responses[fst->resp_idx].servers),		       \
		       &(responses[snd->resp_idx].servers),		       \
		       this->cmpInfo);					       \
	  switch (cmp) {						       \
	  case COMP_CANNOT_TREAT_FIRST:					       \
	  case COMP_SECOND_IS_INF:					       \
	    (levels[i])[j] = *snd;        break;			       \
	  case COMP_CANNOT_TREAT_SECOND:				       \
	  case COMP_FIRST_IS_INF:					       \
	  case COMP_EQUAL:                /* choose the first when equal */    \
	    (levels[i])[j] = *fst;        break;			       \
	  case COMP_CANNOT_TREAT_BOTH:					       \
	    (levels[i])[j].resp_idx = -1; break;			       \
	  default:							       \
	    INTERNAL_WARNING("compare returned wrong value");		       \
	    (levels[i])[j].resp_idx = -1;				       \
	  }								       \
	}								       \
      }									       \
    }

  SCHED_TRACE_FUNCTION("nb_responses=" << nb_responses);

  /* Initialize the tree */
  size_t idx;

  if (*lastAggregated >= ((int)aggrResp.servers.length() - 1)) // Nothing to do
    return 0;

  /* compute the lower power of 2 greater than nb_leaves */
  pow = 0;
  bool power_of_two = true;
  while (nb_leaves > 1) {
    if ((nb_leaves & 1) == 1)
      power_of_two = false;
    nb_leaves = (nb_leaves >> 1);
    pow++;
  }
  if (!power_of_two)
    pow++;
  /* init levels array */
  levels = new level_t[pow + 1];
  /* allocate each level: level i is 2^i long */
  for (size_t i = 0; i <= pow; i++) {
    levels[i] = new node_t[(1 << i)];
  }
  root = &((levels[0])[0]);
  leaves = levels[pow];
  /* init the lower level (the leaves) */
  for (idx = 0; idx < nb_responses; idx++) {
    (levels[pow])[idx].resp_idx = idx;
    (levels[pow])[idx].srv_idx  = lastAggr[idx] + 1;
  }
  for (; (int)idx < (1 << pow); idx++) {
    (levels[pow])[idx].resp_idx = -1;
  }
  UPDATE_TREE(levels,pow,responses);

  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Initial tree:" << endl;
    TRACE_TREE(levels,pow);
  }

  /* Perform the aggregation itself. */

  while ((root->resp_idx != -1)
	 && (*lastAggregated < ((int)aggrResp.servers.length() - 1))) {
    size_t new_srv_idx;
    (*lastAggregated)++;
    aggrResp.servers[*lastAggregated]
      = responses[root->resp_idx].servers[root->srv_idx];

    new_srv_idx = ++leaves[root->resp_idx].srv_idx;
    if (new_srv_idx >= responses[root->resp_idx].servers.length()){
      leaves[root->resp_idx].resp_idx = -1; // this response is aggregated
    }
    UPDATE_TREE(levels,pow,responses);
    if (TRACE_LEVEL >= TRACE_ALL_STEPS){
      TRACE_TREE(levels,pow);
    }
  }
  return 0;

#undef UPDATE_TREE
#undef TRACE_TREE

}



/****************************************************************************/
/* FAST Scheduler                                                           */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "FASTScheduler"

const char*  FASTScheduler::stName     = "FASTScheduler";
const size_t FASTScheduler::nameLength = 13;



/** This is designed to fill in FASTScheduler compare member. */
int
FASTScheduler_compare(const void* idx1, const void* idx2,
		      const SeqServerEstimation_t* servers1,
		      const SeqServerEstimation_t* servers2,
		      const void* useless)
{
  int res = 0;
  CORBA::Long i1 = *((CORBA::Long*)idx1);
  CORBA::Long i2 = *((CORBA::Long*)idx2);

  if ((*servers1)[i1].estim.totalTime == HUGE_VAL)
    res = COMP_CANNOT_TREAT_FIRST;
  if ((*servers2)[i2].estim.totalTime == HUGE_VAL)
    res += COMP_CANNOT_TREAT_SECOND;
  if (res > COMP_SECOND_IS_INF)
    return res;

  if ((*servers1)[i1].estim.totalTime < (*servers2)[i2].estim.totalTime)
    return COMP_FIRST_IS_INF;
  if ((*servers1)[i1].estim.totalTime == (*servers2)[i2].estim.totalTime)
    return COMP_EQUAL;
  return COMP_SECOND_IS_INF;
}


FASTScheduler::FASTScheduler()
{
  this->name    = FASTScheduler::stName;
  this->epsilon = 0;
  this->compare = FASTScheduler_compare;
  this->cmpInfo = NULL;
}

FASTScheduler::FASTScheduler(double epsilon)
{
  this->name    = FASTScheduler::stName;
  this->compare = FASTScheduler_compare;
  this->cmpInfo = NULL;
  if (epsilon < 0) {
    INTERNAL_WARNING("attempt to initialize FAST Scheduler with a negative "
		     << "epsilon.\nSet epsilon to 0.0");
    this->epsilon = 0;
  } else {
    this->epsilon = epsilon;
  }
}

FASTScheduler::~FASTScheduler() {}

/**
 * Return the FASTScheduler deserialized from the string
 * \c serializedScheduler.
 */
FASTScheduler*
FASTScheduler::deserialize(char* serializedScheduler)
{
  double epsilon(0.0);

  SCHED_TRACE_FUNCTION(serializedScheduler);
  // Add one for the ','
  if (sscanf((char*)(serializedScheduler + FASTScheduler::nameLength + 1),
	     "%lg", &epsilon) != 1) {
    INTERNAL_WARNING("invalid parameters for FAST scheduler, "
		     << "reverting to default");
  }
  return new FASTScheduler(epsilon);
}

/**
 * Return the serialized FAST scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
FASTScheduler::serialize(FASTScheduler* S)
{
  char* res = new char[S->nameLength + 20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%.10g", S->stName, S->epsilon);
  return res;
}

/****************************************************************************/
/* NWS Scheduler                                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "NWSScheduler"

#define WEIGHT(estim,wi)                                               \
        ((((estim)->freeCPU == 0)                                      \
	  || ((estim)->freeMem == 0)                                   \
	  || ((estim)->totalTime == HUGE_VAL))                         \
	 ? HUGE_VAL                                                    \
	 : pow((estim)->totalTime,  (wi)->commPower)                   \
	   / (pow((estim)->freeCPU,   (wi)->CPUPower)                  \
	      * pow((estim)->freeMem, (wi)->memPower)))

const char*  NWSScheduler::stName     = "NWSScheduler";
const size_t NWSScheduler::nameLength = 12;


/** This is designed to fill in NWSScheduler compare member. */
int
NWSScheduler_compare(const void* idx1, const void* idx2,
		     const SeqServerEstimation_t* servers1,
		     const SeqServerEstimation_t* servers2,
		     const void* cmpInfo)
{
  NWSScheduler::weight_info_t* wi = (NWSScheduler::weight_info_t*)cmpInfo;
  int res = 0;
  CORBA::Long i1 = *((CORBA::Long*)idx1);
  CORBA::Long i2 = *((CORBA::Long*)idx2);
  double sv1Weight = WEIGHT(&((*servers1)[i1].estim), wi);
  double sv2Weight = WEIGHT(&((*servers2)[i2].estim), wi);

  if (sv1Weight == HUGE_VAL)
    res = COMP_CANNOT_TREAT_FIRST;
  if (sv2Weight == HUGE_VAL)
    res += COMP_CANNOT_TREAT_SECOND;
  if (res > COMP_SECOND_IS_INF)
    return res;

  if (sv1Weight < sv2Weight)
    return COMP_FIRST_IS_INF;
  if (sv1Weight == sv2Weight)
    return COMP_EQUAL;
  return COMP_SECOND_IS_INF;
}

NWSScheduler::NWSScheduler()
{
  this->name          = strdup(NWSScheduler::stName);
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = 0;
  this->wi.CPUPower   = 3;
  this->wi.memPower   = 0.5;
  this->wi.commPower  = 1;
}

NWSScheduler::NWSScheduler(double CPUPower,
			   double memPower,
			   double commPower)
{
  this->name          = NWSScheduler::stName;
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = 0;
  this->wi.CPUPower   = CPUPower;
  this->wi.memPower   = memPower;
  this->wi.commPower  = commPower;
}

NWSScheduler::NWSScheduler(double epsilon,
			   double CPUPower,
			   double memPower,
			   double commPower)
{
  this->name          = NWSScheduler::stName;
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = epsilon;
  this->wi.CPUPower   = CPUPower;
  this->wi.memPower   = memPower;
  this->wi.commPower  = commPower;
}

NWSScheduler::~NWSScheduler() {}


/* Number of private members */
#define nb_mb 4


/**
 * Return the NWSScheduler deserialized from the string
 * \c serializedScheduler.
 */
NWSScheduler*
NWSScheduler::deserialize(char* serializedScheduler)
{
  char* ptr(NULL);
  char* token(NULL);
  double members[nb_mb];
  int i(0);
  NWSScheduler* res;

  SCHED_TRACE_FUNCTION(serializedScheduler);
  token = strtok_r(serializedScheduler, ",", &ptr);
  if (*ptr != '\0')
    ptr[-1] = ',';
  while (i < nb_mb && ((token = strtok_r(NULL, ",", &ptr)) != NULL)) {
    if (sscanf(token, "%lg", &(members[i])) != 1)
      break;
    if (*ptr != '\0')
      ptr[-1] = ',';
    i++;
  }
  // Test for all parameters processed.
  if (i < nb_mb || *ptr != '\0') {
    INTERNAL_WARNING("invalid parameters for NWS scheduler, "
		     << "reverting to default");
    res = new NWSScheduler();
  } else {
    res = new NWSScheduler(members[0], members[1],
			   members[2], members[3]);
  }
  return res;
}

/**
 * Return the serialized NWS scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
NWSScheduler::serialize(NWSScheduler* S)
{
  char* res = new char[S->nameLength + nb_mb*20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%.10g,%.10g,%.10g,%.10g",
	  S->stName,      S->epsilon,
	  S->wi.CPUPower, S->wi.memPower, S->wi.commPower);
  return res;
}

#undef nb_mb

#undef WEIGHT


/****************************************************************************/
/* Random Scheduler                                                         */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "RandScheduler"

const char*  RandScheduler::stName     = "RandScheduler";
const size_t RandScheduler::nameLength = 13;


/** This is designed to fill in RandScheduler compare member. */
int
RandScheduler_compare(const void* idx1, const void* idx2,
		      const SeqServerEstimation_t* servers1,
		      const SeqServerEstimation_t* servers2,
		      const void* useless)
{
  return (rand() % 2) ? COMP_FIRST_IS_INF : COMP_SECOND_IS_INF;
}

RandScheduler::RandScheduler()
{
  this->name    = RandScheduler::stName;
  this->compare = RandScheduler_compare;
  this->cmpInfo = NULL;
  this->seed    = time(NULL);
  srand(this->seed);
}

RandScheduler::RandScheduler(unsigned int seed)
{
  this->name    = RandScheduler::stName;
  this->compare = RandScheduler_compare;
  this->cmpInfo = NULL;
  this->seed    = seed;
  srand(seed);
}

RandScheduler::~RandScheduler() {}

/**
 * Return the serialized Rand scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
RandScheduler::serialize(RandScheduler* S)
{
  char* res = new char[S->nameLength + 1];

  SCHED_TRACE_FUNCTION(S->name);
  strcpy(res, S->stName);
  return res;
}

/**
 * Return the RandScheduler deserialized from the string
 * \c serializedScheduler.
 */
RandScheduler*
RandScheduler::deserialize(char* serializedScheduler)
{
  SCHED_TRACE_FUNCTION(serializedScheduler);
  return new RandScheduler();
}

/****************************************************************************/
/* Utils                                                                    */
/****************************************************************************/


/**
 * Random permutation of a sub-sequence of \c seq, starting at index
 * \c first_idx, ending at index \c last_idx.
 */
void
random_permute(SeqLong* seq, int first_idx, int last_idx)
{
  int rem_length(last_idx - first_idx + 1);
  int i(0);

  while (rem_length > 1) {
    CORBA::Long tmp((*seq)[i]);
    int idx(i + rand() % rem_length);
    (*seq)[i] = (*seq)[idx];
    (*seq)[idx] = tmp;
    rem_length--;
    i++;
  }
}
