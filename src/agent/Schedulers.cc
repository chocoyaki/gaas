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
 * Revision 1.1  2003/04/10 12:57:15  pcombes
 * Interface, plus three examples, for agent schedulers.
 *


#include "Schedulers.hh"

#include <iostream>
using namespace std;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "debug.hh"


extern unsigned int TRACE_LEVEL;


/**
 * Random permutation of the sub-sequence [\c first_idx, ..., \c last_idx] of
 * \c seq.
 */
void
random_permute(SeqLong* seq, int first_idx, int last_idx);


/****************************************************************************/
/* Scheduler                                                                */
/****************************************************************************/


Scheduler::Scheduler() {}
Scheduler::~Scheduler() {}

/**
 * Return the serialized scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
Scheduler::serialize(Scheduler* S)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	       FASTScheduler::stName, FASTScheduler::nameLength)) {
    return FASTScheduler::serialize((FASTScheduler*) S);
  } else if (!strncmp(S->name,
		      NWSScheduler::stName, NWSScheduler::nameLength)) {
    return NWSScheduler::serialize((NWSScheduler*) S);
  } else if (!strncmp(S->name,
		      RandScheduler::stName, RandScheduler::nameLength)) {
    return RandScheduler::serialize((RandScheduler*) S);
  } else {
    cerr << "Error: unable to serialize scheduler named " << S->name << endl;
    return NULL;
  }
}

/**
 * Return the Scheduler deserialized from the string \c serializedScheduler.
 */
Scheduler*
Scheduler::deserialize(char* serializedScheduler)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
	       FASTScheduler::stName, FASTScheduler::nameLength)) {
    return FASTScheduler::deserialize(serializedScheduler);
  } else if (!strncmp(serializedScheduler,
		      NWSScheduler::stName, NWSScheduler::nameLength)) {
    return NWSScheduler::deserialize(serializedScheduler);
  } else if (!strncmp(serializedScheduler,
		      RandScheduler::stName, RandScheduler::nameLength)) {
    return RandScheduler::deserialize(serializedScheduler);
  } else {
    cerr << "Warning: unable to deserialize scheduler ; "
	 << "reverting to default (random).\n";
    return new RandScheduler();
  }
}

extern void
gcclib_quicksort(void* const pbase, size_t total_elems, size_t size,
		 comp_fun_t cmp, SeqServerEstimation_t* servers,
		 const void* info);

void
Scheduler::qsort(CORBA::Long* const base, size_t nb_elems, const void* info)
{
  gcclib_quicksort(base, nb_elems, sizeof(CORBA::Long),
		   this->compare, this->servers, info);
}


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

    
  size_t nb_leaves = nb_responses;
  /* The lowest power of 2 greater or equal than nb_leaves. */
  size_t pow;
  /* Array of (pow + 1) arrays: each array is a level of the binary tree. */
  level_t* levels;
  /* Point at the root. */
  node_t* root;
  level_t root;
  level_t leaves;

  /** Print the tree on standard output */
#define TRACE_TREE(levels,pow)                                                \
  for (size_t i = 0; i <= pow; i++) {				              \
  for (size_t i = 0; i <= pow; i++) {					      \
    for (int j = 0; j < (1 << i); j++) {				      \
      cout << ' ' << (levels[i])[j].resp_idx << ',' << (levels[i])[j].srv_idx;\
    }									      \
    cout << endl;							      \
  }


#define UPDATE_TREE(levels,pow,responses) \
    for (int i = pow - 1; i >= 0; i--) {                                       \
  {                                                                            \
    for (int i = pow - 1; i >= 0; i--) {				       \
	node_t* fst = &((levels[i + 1])[2*j]);				       \
	node_t* snd = &((levels[i + 1])[2*j + 1]);			       \
									       \
	if (fst->resp_idx == -1)					       \
	  (levels[i])[j] = *snd;					       \
	else if (snd->resp_idx == -1)					       \
	  (levels[i])[j] = *fst;					       \
	else {								       \
	  int cmp =							       \
	    (*compare)(&(responses[fst->resp_idx].sortedIndexes[fst->srv_idx]),\
		       &(responses[snd->resp_idx].sortedIndexes[snd->srv_idx]),\
		       &(responses[2*j].servers),			       \
		       &(responses[2*j+1].servers),			       \
		       this->cmpInfo);					       \
	  switch (cmp) {						       \
	  case COMP_CANNOT_TREAT_FIRST:					       \
	  case COMP_FIRST_IS_INF:					       \
	    (levels[i])[j] = *snd;        break;			       \
	  case COMP_CANNOT_TREAT_SECOND:				       \
	  case COMP_SECOND_IS_INF:					       \
	  case COMP_EQUAL:                /* choose the first when equal */    \
	    (levels[i])[j] = *fst;        break;			       \
	  case COMP_CANNOT_TREAT_BOTH:					       \
	    (levels[i])[j].resp_idx = -1; break;			       \
	  default:							       \
	    cerr << "Error: compare returned wrong value.\n";		       \
	    (levels[i])[j].resp_idx = -1;				       \
	  }								       \
	}								       \
      }									       \
    }
    }									       \
  }									       
									       
  /* Initialize the tree */
  size_t idx;

  
    return 0;

  /* compute the lower power of 2 greater than nb_leaves */
  pow = 0;
  bool power_of_two = true;
  while (nb_leaves != 0) {
    pow++;
  }
  if (!power_of_two)
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

  
    cout << "Initial tree:" << endl;
    TRACE_TREE(levels,pow);
  }

  

  while ((root->resp_idx != -1)
	 && (*lastAggregated < ((int)aggrResp.servers.length() - 1))) {
    size_t sorted_idx;
    (*lastAggregated)++;
    // FIXME: The deep copy of the server, makes the sortedIndexes field
    // useless for the default scheduler ...
    aggrResp.sortedIndexes[*lastAggregated] = *lastAggregated;
    sorted_idx = responses[root->resp_idx].sortedIndexes[root->srv_idx];
    aggrResp.servers[*lastAggregated]
      = responses[root->resp_idx].servers[sorted_idx];
    // re-use sorted_idx
    sorted_idx = ++leaves[root->resp_idx].srv_idx;
    if (sorted_idx >= responses[root->resp_idx].servers.length())
      leaves[root->resp_idx].resp_idx = -1; // this response is aggregated
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
    cerr << "WARNING: attempt to initialize FAST Scheduler with a negative "
	 << "epsilon.\nSet epsilon to 0.0.\n";
	 << "epsilon.\nSet epsilon to 0.\n";
  } else {
    this->epsilon = epsilon;
  }
}
  this->compare = FASTScheduler_compare;
  this->cmpInfo = NULL;

FASTScheduler::~FASTScheduler() {}

/**
 * Return the FASTScheduler deserialized from the string
 * \c serializedScheduler.
 */
FASTScheduler*
FASTScheduler::deserialize(char* serializedScheduler)
{
  double epsilon(0.0);

  
  if (sscanf((char*)(serializedScheduler + FASTScheduler::nameLength + 1),
	     "%lg", &epsilon) != 1) {
    cerr << "Warning: invalid parameters for FAST scheduler, "
	 << "reverting to default.\n";
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

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
  return res;
}


/** Implement vritual sort method of class Scheduler. */
int
FASTScheduler::sort(SeqLong* sortedIndexes, int* lastSorted,
		    SeqServerEstimation_t* servers)
{
  CORBA::Long* seq;
  int i;
  corba_estimation_t* ref;
  corba_estimation_t* curr;

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
  this->servers = servers;

  
    return 1;
  if (*lastSorted >= (int)sortedIndexes->length() - 1) {
    return 0;
  }

  
  seq = sortedIndexes->get_buffer() + *lastSorted + 1;
  // DEBUG
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Subsequence [" << *lastSorted + 1 << "-"
	 << sortedIndexes->length() << "] before: |";
    for (int j = *lastSorted + 1; j < (int)sortedIndexes->length(); j++)
      cout << (*sortedIndexes)[j] << '|';
    cout << endl;
  }
  this->qsort(seq, sortedIndexes->length() - (*lastSorted + 1), NULL);
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Subsequence [" << *lastSorted + 1 << "-"
	 << sortedIndexes->length() << "] after : |";
    for (int j = *lastSorted + 1; j < (int)sortedIndexes->length(); j++)
      cout << (*sortedIndexes)[j] << '|';
    cout << endl;
  }

  
  i = *lastSorted + 1;
  ref = &((*servers)[(*sortedIndexes)[i]].estim);
  while ((i < (int)sortedIndexes->length()) && (ref->tComp < HUGE_VAL)) {
    // Find the last server having performances differing of less than epsilon
    // from the ones of the reference.
    int j = i;
    curr = &((*servers)[(*sortedIndexes)[j]].estim);
    while ((j < (int)sortedIndexes->length()) && (curr->tComp < HUGE_VAL)
	   && (curr->totalTime <= ref->totalTime * (1 + this->epsilon))) {
      curr = &((*servers)[(*sortedIndexes)[j++]].estim);
    }
    random_permute(sortedIndexes, i, j - 1);
    i = j;
    ref = &((*servers)[(*sortedIndexes)[i]].estim);
  }

  *lastSorted = i - 1;
  return 0;
}



/****************************************************************************/
/* NWS Scheduler                                                            */
/****************************************************************************/

#define WEIGHT(estim,wi)                                               \
// FIXME: Weight computation is not right, since freeCPU and freeMem should
// weigh the other way
        ((((estim)->freeCPU == 0)                                      \
	  || ((estim)->freeMem == 0)                                   \
	  || ((estim)->totalTime == HUGE_VAL))                         \
	 ? HUGE_VAL                                                    \
	 : pow((estim)->totalTime,  (wi)->commPower)                   \
	 : (wi)->CPUFactor  * pow((estim)->freeCPU,   (wi)->CPUPower) +\
	   (wi)->memFactor  * pow((estim)->freeMem,   (wi)->memPower) +\
	   (wi)->commFactor * pow((estim)->totalTime, (wi)->commPower))
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
  NWSScheduler::weight_info_t* wi = (NWSScheduler::weight_info_t*)cmpInfo;  
  CORBA::Long i1 = *((CORBA::Long*)idx1);
  CORBA::Long i2 = *((CORBA::Long*)idx2);
  double sv1Weight = WEIGHT(&((*servers1)[i1].estim), wi);
  double sv2Weight = WEIGHT(&((*servers2)[i2].estim), wi);

  
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
  this->name          = NWSScheduler::stName;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = 0;
  this->wi.CPUPower   = 3;
  this->wi.CPUFactor  = 1;
  this->wi.memFactor  = 1;
  this->wi.commFactor = 1;
  this->wi.CPUPower   = 1;
  this->wi.memPower   = 1;
}

NWSScheduler::NWSScheduler(double CPUPower,
NWSScheduler::NWSScheduler(double CPUFactor, 
			   double memFactor,
			   double commFactor)
  this->name          = NWSScheduler::stName;
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = 0;
  this->wi.CPUPower   = CPUPower;
  this->wi.CPUFactor  = CPUFactor;
  this->wi.memFactor  = memFactor;
  this->wi.commFactor = commFactor;
  this->wi.CPUPower   = 1;
  this->wi.memPower   = 1;
  this->wi.commPower  = 1;

			   double CPUPower,
			   double CPUFactor,
			   double memFactor,
			   double commFactor)
{
  this->name          = NWSScheduler::stName;
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = epsilon;
  this->wi.CPUFactor  = CPUFactor;
  this->wi.memFactor  = memFactor;
  this->wi.commFactor = commFactor;
  this->wi.CPUPower   = 1;
  this->wi.memPower   = 1;
  this->wi.commPower  = 1;
}

NWSScheduler::NWSScheduler(double epsilon,
			   double CPUFactor, double CPUPower,
			   double memFactor, double memPower,
			   double commFactor, double commPower)
  this->name          = NWSScheduler::stName;
  this->compare       = NWSScheduler_compare;
  this->cmpInfo       = &(this->wi);
  this->epsilon       = epsilon;
  this->wi.CPUPower   = CPUPower;
  this->wi.CPUFactor  = CPUFactor;
  this->wi.memFactor  = memFactor;
  this->wi.commFactor = commFactor;
  this->wi.memPower   = memPower;
  this->wi.commPower  = commPower;
}

NWSScheduler::~NWSScheduler() {}


/* Number of private members */
 * Return the NWSScheduler deserialized from the string
 * \c serializedScheduler.
 */
NWSScheduler*
NWSScheduler::deserialize(char* serializedScheduler)
{
  char* ptr(NULL);
  char* token(NULL);
  double members[nb_mb];
  double members[7];
  NWSScheduler* res;

  
  if (*ptr != '\0')
    ptr[-1] = ',';
  while (i < nb_mb && ((token = strtok_r(NULL, ",", &ptr)) != NULL)) {
  while (i < 7 && ((token = strtok_r(NULL, ",", &ptr)) != NULL)) {
      break;
    if (*ptr != '\0')
      ptr[-1] = ',';
    i++;
  }
  // Test for all parameters processed.
  if (i < nb_mb || *ptr != '\0') {
  if (i < 7 || *ptr != '\0') {
	 << "reverting to default.\n";
    res = new NWSScheduler();
  } else {
    res = new NWSScheduler(members[0], members[1],
    res = new NWSScheduler(members[0],
			   members[1], members[2],
			   members[3], members[4],
			   members[5], members[6]);
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
  char* res = new char[S->nameLength + 7*20];
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
  sprintf(res, "%s,%.10g,%.10g,%.10g,%.10g,%.10g,%.10g,%.10g",
	  S->stName,       S->epsilon,     S->wi.CPUFactor,  S->wi.CPUPower,
	  S->wi.memFactor, S->wi.memPower, S->wi.commFactor, S->wi.commPower);
}

#undef nb_mb
NWSScheduler::sort(SeqLong* sortedIndexes, int* lastSorted,
		   SeqServerEstimation_t* servers)
{
  CORBA::Long* seq;
  int i;
  corba_estimation_t* ref;
  corba_estimation_t* curr;
  double ref_weight, curr_weight;

  

  
    return 1;
  if (*lastSorted >= (int)sortedIndexes->length() - 1) {
    return 0;
  }

  
  seq = sortedIndexes->get_buffer()+ *lastSorted + 1;
  // DEBUG
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Subsequence [" << *lastSorted + 1 << "-"
	 << sortedIndexes->length() << "] before: |";
    for (int j = *lastSorted + 1; j < (int)sortedIndexes->length(); j++)
      cout << (*sortedIndexes)[j] << '|';
    cout << endl;
  }
  this->qsort(seq, sortedIndexes->length() - (*lastSorted + 1), &(this->wi));
  // DEBUG
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Subsequence [" << *lastSorted + 1 << "-"
	 << sortedIndexes->length() << "] after : |";
    for (int j = *lastSorted + 1; j < (int)sortedIndexes->length(); j++)
      cout << (*sortedIndexes)[j] << '|';
    cout << endl;
  }

  
  i = *lastSorted + 1;
  ref = &((*servers)[(*sortedIndexes)[i]].estim);
  while ((i < (int)sortedIndexes->length()) && (ref->freeCPU >= 0)) {
    // Find the last server having performances differing of less than epsilon
    // from the ones of the reference.
    int j = i;
    ref_weight = WEIGHT(ref, &(this->wi));
    curr = &((*servers)[(*sortedIndexes)[j]].estim);
    curr_weight = WEIGHT(curr, &(this->wi));
    while ((j < (int)sortedIndexes->length()) && (curr->freeCPU >= 0)
	   && (curr_weight <= ref_weight * (1 + this->epsilon))) {
      curr = &((*servers)[(*sortedIndexes)[j++]].estim);
      curr_weight = WEIGHT(curr, &(this->wi));
    }
    random_permute(sortedIndexes, i, j - 1);
    i = j;
    ref = &((*servers)[(*sortedIndexes)[i]].estim);
  }

  *lastSorted = i - 1;
  return 0;
}

  



/****************************************************************************/
/* Random Scheduler                                                         */
/****************************************************************************/


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
  return COMP_EQUAL;

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

  return res;
}

/**
 * Return the RandScheduler deserialized from the string
 * \c serializedScheduler.
 */
RandScheduler*
RandScheduler::deserialize(char* serializedScheduler)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
}

/** Implement vritual sort method of class Scheduler. */
int
RandScheduler::sort(SeqLong* sortedIndexes, int* lastSorted,
		    SeqServerEstimation_t* servers)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    return 1;
  random_permute(sortedIndexes, *lastSorted + 1, sortedIndexes->length() - 1);
  *lastSorted = sortedIndexes->length();
  return 0;
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
