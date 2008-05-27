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
 * Revision 1.21  2008/05/27 22:55:12  rbolze
 * add warning message when compare function retruned a COMPARE_UNDEFINED
 *
 * Revision 1.20  2008/01/01 19:04:46  ycaniou
 * Only cosmetic
 *
 * Revision 1.19  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
 * Revision 1.18  2006/05/15 19:45:58  ecaron
 * Retrieve previous version (Last commit was an error)
 *
 * Revision 1.17  2006/05/15 19:37:42  ecaron
 * *** empty log message ***
 *
 * Revision 1.16  2006/02/24 01:59:50  hdail
 * Commenting out somebody's personal debugging output that was sent to stderr
 * and caused a performance hit of several orders of magnitude in aggregate
 * function of DIET.  ALWAYS use trace level controls around your debugging
 * output!
 *
 * Revision 1.15  2006/01/25 21:07:59  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.14  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.13  2006/01/16 16:25:44  pfrauenk
 * NWSScheduler: bug fixes in WEIGHT function
 *      very special thanks to Raphaël Bolze
 *
 * Revision 1.12  2005/08/31 14:47:41  alsu
 * New plugin scheduling interface: adapting the various schedulers to
 * access performance data using the new estimation vector interface
 *
 * Revision 1.11  2005/05/16 12:27:24  alsu
 * removing hard-coded nameLength fields
 *
 * Revision 1.10  2005/05/15 15:50:49  alsu
 * implementing PriorityScheduler
 *
 * Revision 1.9  2005/04/25 08:57:56  hdail
 * Clean up memory leak for levels[] structure used in tree-based response sorting.
 *
 * Revision 1.8  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.7.2.8  2004/12/08 00:03:06  alsu
 * - basic validation tests using FAST 0.8.5 / NWS 2.6.mt4 complete
 * - change WEIGHT macro for NWS scheduler to ignore the "total time"
 *   metric; if it is not HUGE_VAL, we would already have used the FAST
 *   scheduler to deal with this.  (TODO: discuss what to do with the
 *   weight value for "total time" ?)
 *
 * Revision 1.7.2.7  2004/12/06 16:05:23  alsu
 * debugging output cleaned up
 *
 * Revision 1.7.2.6  2004/11/30 15:48:00  alsu
 * fixing problems found testing FAST with plugin schedulers
 *
 * Revision 1.7.2.5  2004/11/21 22:33:20  alsu
 * more explicit commenting of the changes to the COMPARE_NODES macro
 *
 * Revision 1.7.2.4  2004/11/06 16:32:39  alsu
 * - changes for new parameter-based default values for estVector access
 *   functions
 * - when comparing an existing response (resp_idx>=0) to a "null"
 *   response (resp_idx==-1), the behavior of COMPARE_NODES macro is
 *   changed to return COMPARE_UNDEFINED if the response would not be
 *   able to be compared in a standard comparison.  previously, a
 *   response without the necessary values to be treated by a particular
 *   aggregator was promoted when compared against a "null" response
 * - adding generic min and generic max aggregators
 *
 * Revision 1.7.2.3  2004/11/02 00:37:28  alsu
 * removing references to obviated fields in the estimation data structure
 *
 * Revision 1.7.2.2  2004/10/31 22:21:52  alsu
 * - restructured the old COMP_* definitions to be more "logcial" rather
 *   than absolute (i.e., COMP_FIRST_IS_INF => COMPARE_FIRST_IS_BETTER)
 * - added class methods to the Scheduler class to construct (and cache)
 *   estimation value vectors during the server sorting process
 * - implementation of the new default round-robin scheduler that is
 *   consistent with the dynamic performance data design
 *
 * Revision 1.7.2.1  2004/10/27 22:35:51  alsu
 * include
 *
 * Revision 1.7  2004/10/15 11:49:33  hdail
 * Modified tree update algorithm in aggregate to reduce unecessary comparisons.
 *
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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "est_internal.hh"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

// Use SCHED_CLASS for the name of the classSCHED_TRACE_FUNCTION
// (this->name cannot be used in static member functions)
#define SCHED_TRACE_FUNCTION(formatted_text)      \
  TRACE_TEXT(TRACE_ALL_STEPS, SCHED_CLASS << "::");\
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)


/*
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
#if !HAVE_CORI
  if (!strncmp(S->name, FASTScheduler::stName, S->nameLength)) {
    return (FASTScheduler::serialize((FASTScheduler*) S));
  }
  else if (!strncmp(S->name, NWSScheduler::stName, S->nameLength)) {
    return (NWSScheduler::serialize((NWSScheduler*) S));
  }else
#endif //HAVE_CORI
  if (!strncmp(S->name, RandScheduler::stName, S->nameLength)) {
    return (RandScheduler::serialize((RandScheduler*) S));
  }
  else if (!strncmp(S->name, MinScheduler::stName, S->nameLength)) {
    return (MinScheduler::serialize((MinScheduler*) S));
  }
  else if (!strncmp(S->name, MaxScheduler::stName, S->nameLength)) {
    return (MaxScheduler::serialize((MaxScheduler*) S));
  }
  else if (!strncmp(S->name, PriorityScheduler::stName, S->nameLength)) {
    return (PriorityScheduler::serialize((PriorityScheduler*) S));
  }
  else if (!strncmp(S->name, RRScheduler::stName, S->nameLength)) {
    return (RRScheduler::serialize((RRScheduler*) S));
  }
  else {
    INTERNAL_ERROR("unable to serialize scheduler named " << S->name, 1);
    /* this line never executes, but exists to avoid  warning */
    return NULL ;
  }
}

/**
 * Return the Scheduler deserialized from the string \c serializedScheduler.
 */
Scheduler*
Scheduler::deserialize(const char* serializedScheduler)
{
//  SCHED_TRACE_FUNCTION(serializedScheduler);
  int nameLength;

  {
    char *comma;
    if ((comma = strchr(serializedScheduler, ',')) != NULL) {
      nameLength = comma - serializedScheduler;
    }
    else {
      nameLength = strlen(serializedScheduler);
    }
  }
  
#if !HAVE_CORI
  if (!strncmp(serializedScheduler, FASTScheduler::stName, nameLength)) {
    return (FASTScheduler::deserialize(serializedScheduler + nameLength));
  }
  else if (!strncmp(serializedScheduler, NWSScheduler::stName, nameLength)) {
    return (NWSScheduler::deserialize(serializedScheduler + nameLength));
  }
  else
#endif //HAVE_CORI
    if (!strncmp(serializedScheduler, RandScheduler::stName, nameLength)) {
    return (RandScheduler::deserialize(serializedScheduler + nameLength));
  }
  else if (!strncmp(serializedScheduler, MinScheduler::stName, nameLength)) {
    return (MinScheduler::deserialize(serializedScheduler + nameLength));
  }
  else if (!strncmp(serializedScheduler, MaxScheduler::stName, nameLength)) {
    return (MaxScheduler::deserialize(serializedScheduler + nameLength));
  }
  else if (!strncmp(serializedScheduler,
                    PriorityScheduler::stName,
                    nameLength)) {
    return (PriorityScheduler::deserialize(serializedScheduler + nameLength));
  }
  else if (!strncmp(serializedScheduler, RRScheduler::stName, nameLength)) {
    return (RRScheduler::deserialize(serializedScheduler + nameLength));
  }
  else {
    INTERNAL_WARNING("unable to deserialize scheduler \""
                     << serializedScheduler
                     << "\" ; "
                     << "reverting to default (random)");
    return (new RandScheduler());
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
 * @param evCache        vector of vectors, storing previously constrctued
 *                       estVector objects
 */
int
Scheduler::aggregate(corba_response_t& aggrResp,
                     int* lastAggregated,
                     const size_t nb_responses,
                     const corba_response_t* responses,
                     int* lastAggr,
                     Vector_t evCache)
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
  /* Left and right storage for nodes for comparisons */
  node_t* lft;
  node_t* rht;
  
  /** Print the tree on standard output */
#define TRACE_TREE(levels,pow)                                                \
  for (size_t i = 0; i <= pow; i++) {                                         \
    cout << ' ';                                                              \
    for (int j = 0; j < (1 << i); j++) {                                      \
      cout << ' ' << (levels[i])[j].resp_idx << ',' << (levels[i])[j].srv_idx;\
    }                                                                         \
    cout << endl;                                                             \
  }

  /** Compare 2 nodes */
#if 0
#define COMPARE_NODES(levels,pow,responses,fst,snd,parent)                    \
  if (fst->resp_idx == -1)                                                    \
    parent = *snd;                                                            \
  else if (snd->resp_idx == -1)                                               \
    parent = *fst;                                                            \
  else {                                                                      \
    int cmp =                                                                 \
      (*compare)(fst->srv_idx,                                                \
                 snd->srv_idx,                                                \
                 fst->resp_idx,                                               \
                 snd->resp_idx,                                               \
                 responses,                                                   \
                 evCache,                                                     \
                 this->cmpInfo);                                              \
    switch (cmp) {                                                            \
    case COMPARE_SECOND_IS_BETTER:                                            \
      parent = *snd;        break;                                            \
    case COMPARE_FIRST_IS_BETTER:                                             \
    case COMPARE_EQUAL:             /* choose the first when equal */         \
      parent = *fst;        break;                                            \
    case COMPARE_UNDEFINED:                                                   \
      parent.resp_idx = -1; break;                                            \
    default:                                                                  \
      INTERNAL_WARNING("compare returned wrong value: " << cmp);              \
      parent.resp_idx = -1;                                                   \
    }                                                                         \
  }
#endif
#define COMPARE_NODES(levels,pow,responses,fst,snd,parent)                    \
  if (fst->resp_idx == -1) {                                                  \
    if (snd->resp_idx == -1 ||                                                \
        (*compare)(snd->srv_idx,                                              \
                   snd->srv_idx,                                              \
                   snd->resp_idx,                                             \
                   snd->resp_idx,                                             \
                   responses,                                                 \
                   evCache,                                                   \
                   this->cmpInfo) == COMPARE_UNDEFINED) {                     \
      /* both tree nodes are null                               */            \
      /*          -- OR --                                      */            \
      /* the first is null and the second node cannot be        */            \
      /* treated with this aggregator's compare function        */            \
      /*          -- SO --                                      */            \
      /* promote the first, which is a null node                */            \
      parent = *fst;                                                          \
    }                                                                         \
    else {                                                                    \
      /* first tree node null, second has valid metric          */            \
      /*          -- SO --                                      */            \
      /* promote the second, which is a real node               */            \
      parent = *snd;                                                          \
    }                                                                         \
  }                                                                           \
  else if (snd->resp_idx == -1) {                                             \
    if ((*compare)(fst->srv_idx,                                              \
                   fst->srv_idx,                                              \
                   fst->resp_idx,                                             \
                   fst->resp_idx,                                             \
                   responses,                                                 \
                   evCache,                                                   \
                   this->cmpInfo) == COMPARE_UNDEFINED) {                     \
      /* the second node is null and the first cannot be        */            \
      /* evaluated with this aggregator's compare function      */            \
      /*          -- SO --                                      */            \
      /* promote the second, which is a null node               */            \
      parent = *snd;                                                          \
    }                                                                         \
    else {                                                                    \
      /* second tree node null, first has valid metric          */            \
      /*          -- SO --                                      */            \
      /* promote the first, which is a real node                */            \
      parent = *fst;                                                          \
    }                                                                         \
  }                                                                           \
  else {                                                                      \
    /* both tree nodes are real nodes, so let's do            */              \
    /* full comparison                                        */              \
    int cmp =                                                                 \
      (*compare)(fst->srv_idx,                                                \
                 snd->srv_idx,                                                \
                 fst->resp_idx,                                               \
                 snd->resp_idx,                                               \
                 responses,                                                   \
                 evCache,                                                     \
                 this->cmpInfo);                                              \
    switch (cmp) {                                                            \
    case COMPARE_SECOND_IS_BETTER:                                            \
      parent = *snd;        break;                                            \
    case COMPARE_FIRST_IS_BETTER:                                             \
    case COMPARE_EQUAL:             /* choose the first when equal */         \
      parent = *fst;        break;                                            \
    case COMPARE_UNDEFINED:                                                   \
      INTERNAL_WARNING("compare returned undefined value: " << cmp);          \
      parent.resp_idx = -1; break;                                            \
    default:                                                                  \
      INTERNAL_WARNING("compare returned wrong value: " << cmp);              \
      parent.resp_idx = -1;                                                   \
    }                                                                         \
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
  if (!power_of_two){
    pow++;
  }
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
  /* fill in rest of leaves with -1 */
  for (; (int)idx < (1 << pow); idx++) {
    (levels[pow])[idx].resp_idx = -1;
  }
  /* init the other levels of the tree */
  for (int i = pow - 1; i >= 0; i--) {
    for (int j = 0; j < (1 << i); j++) {
      lft = &((levels[i + 1])[2*j]);
      rht = &((levels[i + 1])[2*j + 1]);
      COMPARE_NODES(levels,pow,responses,lft,rht,(levels[i])[j]);
    }
  }

  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Initial tree:" << leaves;
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

   /** Update the tree, ie propogate changes for selected node up the tree. 
    * We use root, which was the selected node, as guide for which parts of 
    * tree need to be updated.
    */
    size_t changed_srv_idx = root->resp_idx;
    int parent_loc;
    
    for (int i = pow; i > 0; i--) {
      if ((changed_srv_idx & 1) == 0) {  /* even */
        lft = &((levels[i])[changed_srv_idx]);
        rht = &((levels[i])[changed_srv_idx+1]);
      } else {                           /* odd */
        lft = &((levels[i])[changed_srv_idx-1]);
        rht = &((levels[i])[changed_srv_idx]);
      }
      parent_loc = changed_srv_idx >> 1; /* parent location in bin tree */
      COMPARE_NODES(levels,pow,responses,lft,rht,(levels[i-1])[parent_loc]);

      /* Find parent srv_idx in binary tree */
      changed_srv_idx = parent_loc;
    }

    if (TRACE_LEVEL >= TRACE_ALL_STEPS){
      TRACE_TREE(levels,pow);
    }
  }

  /* Clean up memory */
  for (size_t i = 0; i <= pow; i++) {
    delete [] levels[i];
  }
  delete [] levels;

  return 0;

#undef COMPARE_NODES
//#undef UPDATE_TREE
#undef TRACE_TREE

}

/**
 * Return an estVector for the indicated server estimation
 */
estVectorConst_t
Scheduler::getEstVector(int sIdx,
                        int rIdx,
                        const corba_response_t* responses)
{
  const CORBA::Long idx = sIdx;

  estVectorConst_t eVals = &(responses[rIdx].servers[idx].estim);

  return (eVals);
}

/**
 * Return an estVector for the indicated server estimation, using
 * the cached value, if available
 */
estVectorConst_t
Scheduler::getEstVector(int sIdx,
                        int rIdx,
                        const corba_response_t* responses,
                        Vector_t evCache)
{
  assert(evCache != NULL);
  assert(rIdx < Vector_size(evCache));

  Vector_t sv = (Vector_t) Vector_elementAt(evCache, rIdx);
  estVectorConst_t ev;

  if (sIdx < Vector_size(sv) &&
      (ev = (estVector_t) Vector_elementAt(sv, sIdx)) != NULL) {
//     fprintf(stderr, "CACHE(%d,%d) = %d*\n", rIdx, sIdx, (int) ev);
    return (ev);
  }

  ev = Scheduler::getEstVector(sIdx, rIdx, responses);
  Vector_set(sv, ev, sIdx);
//   fprintf(stderr, "CACHE(%d,%d) = %d\n", rIdx, sIdx, (int) ev);
  return (ev);
}


#if !HAVE_CORI
/****************************************************************************/
/* FAST Scheduler                                                           */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "FASTScheduler"

const char*  FASTScheduler::stName     = "FASTScheduler";

static double
__getAggregateCommTime(estVectorConst_t ev)
{
  double aggCommTime = 0.0;
  int numCommTimes = diet_est_array_size_internal(ev, EST_COMMTIME);

  for (int commTimeIter = 0 ; commTimeIter < numCommTimes ; commTimeIter++) {
    double val = diet_est_array_get_internal(ev,
                                             EST_COMMTIME,
                                             commTimeIter,
                                             HUGE_VAL);
    if (val == HUGE_VAL) {
      return (HUGE_VAL);
    }

    aggCommTime += val;
  }

  return (aggCommTime);
}

/** This is designed to fill in FASTScheduler compare member. */
int FASTScheduler_compare(int serverIdx1,
                          int serverIdx2,
                          int responseIdx1,
                          int responseIdx2,
                          const corba_response_t* responses,
                          Vector_t evCache,
                          const void* useless)
{
  const corba_response_t response1 = responses[responseIdx1];
  const corba_response_t response2 = responses[responseIdx2];
  const SeqServerEstimation_t servers1 = response1.servers;
  const SeqServerEstimation_t servers2 = response2.servers;
  const CORBA::Long i1 = serverIdx1;
  const CORBA::Long i2 = serverIdx2;
  const corba_server_estimation_t est1 = servers1[i1];
  const corba_server_estimation_t est2 = servers2[i2];

  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  double s1tt = diet_est_get_internal(s1est, EST_TOTALTIME, HUGE_VAL);
  double s2tt = diet_est_get_internal(s2est, EST_TOTALTIME, HUGE_VAL);
  double s1ct = __getAggregateCommTime(s1est);
  double s2ct = __getAggregateCommTime(s2est);

  if (s1tt == HUGE_VAL) {
    if (s2tt == HUGE_VAL) {
      return (COMPARE_UNDEFINED);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
  else if (s2tt == HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  }
  else {
    if ((s1tt+s1ct) < (s2tt+s2ct)) {
      return (COMPARE_FIRST_IS_BETTER);
    }
    else if ((s1tt+s1ct) == (s2tt+s2ct)) {
      return (COMPARE_EQUAL);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
}


FASTScheduler::FASTScheduler()
{
  this->name    = FASTScheduler::stName;
  this->nameLength = strlen(this->name);
  this->epsilon = 0;
  this->compare = FASTScheduler_compare;
  this->cmpInfo = NULL;
}

FASTScheduler::FASTScheduler(double epsilon)
{
  this->name    = FASTScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = FASTScheduler_compare;
  this->cmpInfo = NULL;
  if (epsilon < 0.0) {
    INTERNAL_WARNING("attempt to initialize FAST Scheduler with a negative "
                     << "epsilon: "
                     << epsilon
                     << ".\nSet epsilon to 0.0");
    this->epsilon = 0.0;
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
FASTScheduler::deserialize(const char* serializedScheduler)
{
  double epsilon(0.0);

  SCHED_TRACE_FUNCTION(serializedScheduler);
  // Add one for the ','
  if (sscanf((char*)(serializedScheduler + 1), "%lg", &epsilon) != 1) {
    INTERNAL_WARNING("invalid parameters for FAST scheduler ("
                     << ((char *)(serializedScheduler + 1))
                     << "), "
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

#define WEIGHT(ev, wi)                                                        \
  (((diet_est_get_internal(ev, EST_FREECPU, 0.0) == 0.0) ||                   \
    (diet_est_get_internal(ev, EST_FREEMEM, 0.0) == 0.0)) ?                   \
   HUGE_VAL                                                                  :\
    ( 1.0                                                                    /\
    (pow(diet_est_get_internal(ev, EST_FREECPU, 0.0),                          \
        (wi)->CPUPower)                                                      *\
    pow(diet_est_get_internal(ev, EST_FREEMEM, 0.0),                          \
        (wi)->memPower))))                                                     


const char*  NWSScheduler::stName     = "NWSScheduler";

/** This is designed to fill in NWSScheduler compare member. */
// int
int NWSScheduler_compare(int serverIdx1,
                         int serverIdx2,
                         int responseIdx1,
                         int responseIdx2,
                         const corba_response_t* responses,
                         Vector_t evCache,
                         const void* cmpInfo)
{
  NWSScheduler::weight_info_t* wi = (NWSScheduler::weight_info_t*)cmpInfo;

  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  double sv1Weight = WEIGHT(s1est, wi);
  double sv2Weight = WEIGHT(s2est, wi);
#if 0
  fprintf(stderr,
          "*****FASTTEST***** NWSScheduler_compare: s1weight = %.4f\n",
          sv1Weight);
  fprintf(stderr,
          "*****FASTTEST***** NWSScheduler_compare: s2weight = %.4f\n",
          sv2Weight);
cerr<<"cpu "<<diet_est_get_internal(s1est, EST_FREECPU, 0.0)<<"^" <<(wi)->CPUPower<<"="<<pow(diet_est_get_internal(s1est, EST_FREECPU, 0.0),(wi)->CPUPower)<<"|| mem " <<diet_est_get_internal(s1est, EST_FREEMEM, 0.0)<<"^"<<(wi)->memPower<<"="<<pow(diet_est_get_internal(s1est, EST_FREEMEM, 0.0),(wi)->memPower)<<endl;
cerr<<"cpu "<<diet_est_get_internal(s2est, EST_FREECPU, 0.0)<<"^" <<(wi)->CPUPower<<"="<<pow(diet_est_get_internal(s2est, EST_FREECPU, 0.0),(wi)->CPUPower)<<"|| mem " <<diet_est_get_internal(s2est, EST_FREEMEM, 0.0)<<"^"<<(wi)->memPower<<"="<<pow(diet_est_get_internal(s2est, EST_FREEMEM, 0.0),(wi)->memPower)<<endl;
#endif

  if (sv1Weight == HUGE_VAL) {
    if (sv2Weight == HUGE_VAL) {
      return (COMPARE_UNDEFINED);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
  else if (sv2Weight == HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  }
  else {
    if (sv1Weight < sv2Weight) {

      return (COMPARE_FIRST_IS_BETTER);
    }
    else if (sv1Weight == sv2Weight) {
      return (COMPARE_EQUAL);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
}

NWSScheduler::NWSScheduler()
{
  this->name          = strdup(NWSScheduler::stName);
  this->nameLength = strlen(this->name);
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
  this->nameLength = strlen(this->name);
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
  this->nameLength = strlen(this->name);
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
NWSScheduler::deserialize(const char* serializedScheduler)
{
  const char* ptr(NULL);
//   char* token(NULL);
  double members[nb_mb];
//   int i(0);
//   NWSScheduler* res;

  SCHED_TRACE_FUNCTION(serializedScheduler);

//   token = strtok_r(serializedScheduler, ",", &ptr);
//   if (*ptr != '\0')
//     ptr[-1] = ',';
//   while (i < nb_mb && ((token = strtok_r(NULL, ",", &ptr)) != NULL)) {
//     if (sscanf(token, "%lg", &(members[i])) != 1)
//       break;
//     if (*ptr != '\0')
//       ptr[-1] = ',';
//     i++;
//   }

  /*
  ** serializedScheduler should start with the comma
  ** after the scheduler name
  */
  assert(*serializedScheduler == ',');

  /* position the pointer after the leading comma */
  ptr = serializedScheduler + 1;

  for (int paramIter = 0 ; paramIter < nb_mb ; paramIter++) {
    if (paramIter == nb_mb-1) {
      /* check that this is the last param */
      if (strchr(ptr, ',') != NULL) {
        INTERNAL_WARNING("too many parameters for NWS scheduler ("
                         << ((char *)(serializedScheduler + 1))
                         << "), "
                         << "reverting to default");
        return (new NWSScheduler());
      }

      { /* extract the value */
        char dummy;
        if (sscanf(ptr, "%lg%c", &(members[paramIter]), &dummy) != 1) {
          INTERNAL_WARNING("invalid parameter " <<
                           paramIter <<
                           " for NWS scheduler, reverting to default");
          return (new NWSScheduler());
        }
      }
    }
    else {
      /* check that this is not the last param */
      if (strchr(ptr, ',') == NULL) {
        INTERNAL_WARNING("too few parameters for NWS scheduler ("
                         << ((char *)(serializedScheduler + 1))
                         << "), "
                         << "reverting to default");
        return (new NWSScheduler());
      }

      { /* extract the value */
        char dummy;
        assert(sscanf(ptr, "%lg%c", &(members[paramIter]), &dummy) == 2);
        if (dummy != ',') {
          INTERNAL_WARNING("invalid parameter " <<
                           paramIter <<
                           " for NWS scheduler, reverting to default");
          return (new NWSScheduler());
        }
      }
    }
  }

  return (new NWSScheduler(members[0], members[1],
                           members[2], members[3]));
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

#endif //HAVE_CORI
/****************************************************************************/
/* Random Scheduler                                                         */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "RandScheduler"

const char*  RandScheduler::stName     = "RandScheduler";

/** This is designed to fill in RandScheduler compare member. */
int RandScheduler_compare(int serverIdx1,
                          int serverIdx2,
                          int responseIdx1,
                          int responseIdx2,
                          const corba_response_t* responses,
                          Vector_t evCache,
                          const void* useless)
{
  return ((rand() % 2) ? COMPARE_FIRST_IS_BETTER : COMPARE_SECOND_IS_BETTER);
}

RandScheduler::RandScheduler()
{
  this->name    = RandScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RandScheduler_compare;
  this->cmpInfo = NULL;
  this->seed    = time(NULL);
  srand(this->seed);
}

RandScheduler::RandScheduler(unsigned int seed)
{
  this->name    = RandScheduler::stName;
  this->nameLength = strlen(this->name);
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
RandScheduler::deserialize(const char* serializedScheduler)
{
  SCHED_TRACE_FUNCTION(serializedScheduler);
  assert(*serializedScheduler == '\0');
  return (new RandScheduler());
}

/****************************************************************************/
/* RR Scheduler                                                             */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "RRScheduler"

const char*  RRScheduler::stName     = "RRScheduler";

/** This is designed to fill in RRScheduler compare member. */
int RRScheduler_compare(int serverIdx1,
                        int serverIdx2,
                        int responseIdx1,
                        int responseIdx2,
                        const corba_response_t* responses,
                        Vector_t evCache,
                        const void* useless)
{
  if (serverIdx1 == serverIdx2 && responseIdx1 == responseIdx2) {
    estVectorConst_t est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
    if (diet_est_get_internal(est, EST_TIMESINCELASTSOLVE, -1.0) == -1.0) {
      return (COMPARE_UNDEFINED);
    }
    return (COMPARE_EQUAL);
  }

  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  double tsls_s1 = diet_est_get_internal(s1est, EST_TIMESINCELASTSOLVE, -1.0);
  double tsls_s2 = diet_est_get_internal(s2est, EST_TIMESINCELASTSOLVE, -1.0);

  if (tsls_s1 < 0.0) {
    if (tsls_s2 < 0.0) {
      return (COMPARE_UNDEFINED);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
  else if (tsls_s2 < 0.0) {
    return (COMPARE_FIRST_IS_BETTER);
  }
  else {
    if (tsls_s1 < tsls_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    }
    else if (tsls_s1 == tsls_s2) {
      return (COMPARE_EQUAL);
    }
    else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
}

RRScheduler::RRScheduler()
{
  this->name    = RRScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RRScheduler_compare;
  this->cmpInfo = NULL;
  this->seed    = time(NULL);
  srand(this->seed);
}

RRScheduler::RRScheduler(unsigned int seed)
{
  this->name    = RRScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RRScheduler_compare;
  this->cmpInfo = NULL;
  this->seed    = seed;
  srand(seed);
}

RRScheduler::~RRScheduler() {}

/**
 * Return the serialized RR scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
RRScheduler::serialize(RRScheduler* S)
{
  char* res = new char[S->nameLength + 1];

  SCHED_TRACE_FUNCTION(S->name);
  strcpy(res, S->stName);
  return res;
}

/**
 * Return the RRScheduler deserialized from the string
 * \c serializedScheduler.
 */
RRScheduler*
RRScheduler::deserialize(const char* serializedScheduler)
{
  SCHED_TRACE_FUNCTION(serializedScheduler);
  assert(*serializedScheduler == '\0');
  return (new RRScheduler());
}

/****************************************************************************/
/* Min Scheduler                                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "MinScheduler"

const char*  MinScheduler::stName     = "MinScheduler";

/** This is designed to fill in MinScheduler compare member. */
int MinScheduler_compare(int serverIdx1,
                         int serverIdx2,
                         int responseIdx1,
                         int responseIdx2,
                         const corba_response_t* responses,
                         Vector_t evCache,
                         const void* tagvalPtr)
{
  int tagval = *((int *) tagvalPtr);
  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  double mval_s1 = diet_est_get_internal(s1est, tagval, HUGE_VAL);
  double mval_s2 = diet_est_get_internal(s2est, tagval, HUGE_VAL);

  if (mval_s1 == HUGE_VAL) {
    if (mval_s2 == HUGE_VAL) {
      return (COMPARE_UNDEFINED);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
  else if (mval_s2 == HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  }
  else {
//     fprintf(stderr, "min metric comparing %.4f %.4f\n", mval_s1, mval_s2);
    if (mval_s1 > mval_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    }
    else if (mval_s1 == mval_s2) {
      return (COMPARE_EQUAL);
    }
    else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
}

MinScheduler::MinScheduler(int tagval)
{
  assert(tagval >= 0);
  this->name    = MinScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = MinScheduler_compare;
  this->tagval  = tagval;
  this->cmpInfo = &(this->tagval);
}

MinScheduler::~MinScheduler() {}

/**
 * Return the serialized Min scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
MinScheduler::serialize(MinScheduler* S)
{
  char* res = new char[S->nameLength + 20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->tagval);
  return res;
}

/**
 * Return the MinScheduler deserialized from the string
 * \c serializedScheduler.
 */
MinScheduler*
MinScheduler::deserialize(const char* serializedScheduler)
{
  int tagval;
  char dummy;

  SCHED_TRACE_FUNCTION(serializedScheduler);
  if (sscanf((char*)(serializedScheduler + 1), "%d%c", &tagval, &dummy) != 1) {
    INTERNAL_ERROR("invalid parameters for Min scheduler", -1);
    return (NULL);
  }

  return (new MinScheduler(tagval));
}

/****************************************************************************/
/* Max Scheduler                                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "MaxScheduler"

const char*  MaxScheduler::stName     = "MaxScheduler";

/** This is designed to fill in MaxScheduler compare member. */
int MaxScheduler_compare(int serverIdx1,
                         int serverIdx2,
                         int responseIdx1,
                         int responseIdx2,
                         const corba_response_t* responses,
                         Vector_t evCache,
                         const void* tagvalPtr)
{
  int tagval = *((int *) tagvalPtr);
  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  double mval_s1 = diet_est_get_internal(s1est, tagval, -HUGE_VAL);
  double mval_s2 = diet_est_get_internal(s2est, tagval, -HUGE_VAL);

  if (mval_s1 == -HUGE_VAL) {
    if (mval_s2 == -HUGE_VAL) {
      return (COMPARE_UNDEFINED);
    }
    else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  }
  else if (mval_s2 == -HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  }
  else {
//     fprintf(stderr, "max metric comparing %.4f %.4f\n", mval_s1, mval_s2);
    if (mval_s1 < mval_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    }
    else if (mval_s1 == mval_s2) {
      return (COMPARE_EQUAL);
    }
    else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
}

MaxScheduler::MaxScheduler(int tagval)
{
  assert(tagval >= 0);
  this->name    = MaxScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = MaxScheduler_compare;
  this->tagval  = tagval;
  this->cmpInfo = &(this->tagval);
}

MaxScheduler::~MaxScheduler() {}

/**
 * Return the serialized Max scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
MaxScheduler::serialize(MaxScheduler* S)
{
  char* res = new char[S->nameLength + 20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->tagval);
  return res;
}

/**
 * Return the MaxScheduler deserialized from the string
 * \c serializedScheduler.
 */
MaxScheduler*
MaxScheduler::deserialize(const char* serializedScheduler)
{
  int tagval;
  char dummy;

  SCHED_TRACE_FUNCTION(serializedScheduler);
  if (sscanf((char*)(serializedScheduler + 1), "%d%c", &tagval, &dummy) != 1) {
    INTERNAL_ERROR("invalid parameters for Max scheduler", -1);
    return (NULL);
  }

  return (new MaxScheduler(tagval));
}

/****************************************************************************/
/* Priority Scheduler                                                       */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "PriorityScheduler"

const char*  PriorityScheduler::stName     = "PriorityScheduler";

/** This is designed to fill in PriorityScheduler compare member. */
int PriorityScheduler_compare(int serverIdx1,
                              int serverIdx2,
                              int responseIdx1,
                              int responseIdx2,
                              const corba_response_t* responses,
                              Vector_t evCache,
                              const void* pListPtr)
{
  estVectorConst_t s1est = Scheduler::getEstVector(serverIdx1,
                                                   responseIdx1,
                                                   responses,
                                                   evCache);
  estVectorConst_t s2est = Scheduler::getEstVector(serverIdx2,
                                                   responseIdx2,
                                                   responses,
                                                   evCache);

  const PriorityScheduler::priorityList *pl =
    (PriorityScheduler::priorityList *) pListPtr;
  for (int pvalIter = 0 ; pvalIter < pl->pl_numValues ; pvalIter++) {
    int minimize = 0;
    int _tag = pl->pl_values[pvalIter];

    if (_tag < 0) {
      _tag = -_tag;
      minimize = 1;
    }

    int tag = (int) _tag;
    const int exists1 = diet_est_defined_internal(s1est, tag);
    const int exists2 = diet_est_defined_internal(s2est, tag);
    const double val1 = diet_est_get_internal(s1est, tag, 0);
    const double val2 = diet_est_get_internal(s2est, tag, 0);

    if (exists1 == 0 && exists2 == 0) {
      return (COMPARE_UNDEFINED);
    }

    /* at least one value exists here */
    if (exists1 == 0) {
      return (COMPARE_SECOND_IS_BETTER);
    }
    if (exists2 == 0) {
      return (COMPARE_FIRST_IS_BETTER);
    }

    /* ok, both values exist! */
    if (minimize) {
      if (val1 < val2) {
        return (COMPARE_FIRST_IS_BETTER);
      }
      else if (val2 < val1) {
        return (COMPARE_SECOND_IS_BETTER);
      }
    }
    else {
      if (val1 > val2) {
        return (COMPARE_FIRST_IS_BETTER);
      }
      else if (val2 > val1) {
        return (COMPARE_SECOND_IS_BETTER);
      }
    }
  }

  /* all comparisons were valid, and equal */
  return (COMPARE_EQUAL);
}

PriorityScheduler::PriorityScheduler(int numValues, int *values)
{
  if (numValues <= 0) {
    INTERNAL_ERROR("Priority scheduler instantiated with <= 0 values", -1);
    return;
  }
  this->name    = PriorityScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = PriorityScheduler_compare;
  this->pl.pl_numValues = numValues;
  this->pl.pl_values = new int[numValues];
  for (int valIter = 0 ; valIter < numValues ; valIter++) {
    this->pl.pl_values[valIter] = values[valIter];
  }
  this->cmpInfo = &(this->pl);
}

PriorityScheduler::~PriorityScheduler() {}

/**
 * Return the serialized Priority scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char*
PriorityScheduler::serialize(PriorityScheduler* S)
{
  char* res = new char[S->nameLength + (8 * S->pl.pl_numValues)];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->pl.pl_numValues);
  for (int valIter = 0 ; valIter < S->pl.pl_numValues ; valIter++) {
    sprintf(res + strlen(res), ",%d", S->pl.pl_values[valIter]);
  }
  return res;
}

/**
 * Return the PriorityScheduler deserialized from the string
 * \c serializedScheduler.
 */
PriorityScheduler*
PriorityScheduler::deserialize(const char* serializedScheduler)
{
	int numValues;
	const char *strPtr = serializedScheduler+1;

	SCHED_TRACE_FUNCTION(serializedScheduler);

  if (sscanf(strPtr, "%d", &numValues) != 1) {
    INTERNAL_ERROR("error reading numValues for Priority scheduler", -1);
    return (NULL);
  }
  if (numValues <= 0) {
    INTERNAL_ERROR("invalid numValues (" <<
                   numValues <<
                   ") for Priority scheduler", -1);
    return (NULL);
  }
  if (strchr(strPtr, ',') == NULL) {
    INTERNAL_ERROR("missing priority values for Priority scheduler", -1);
    return (NULL);
  }
  strPtr = strchr(strPtr, ',') + 1;

  int* values = new int[numValues];
  for (int valIter = 0 ; valIter < numValues ; valIter++) {
    int curVal;
    if (sscanf(strPtr, "%d", &curVal) != 1) {
      INTERNAL_ERROR("error reading value " <<
                     valIter <<
                     ") for Priority scheduler", -1);
      return (NULL);
    }
    values[valIter] = curVal;

    if (valIter < numValues - 1) {
      strPtr = strchr(strPtr, ',') + 1;
    }
  }

  PriorityScheduler *ps = new PriorityScheduler(numValues, values);
  delete [] values;
  return (ps);
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
