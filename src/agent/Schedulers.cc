/**
 * @file  Schedulers.cc
 *
 * @brief  DIET agent schedulers implementation : add yours !!!
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#include "Schedulers.hh"

#include <iostream>
using namespace std;

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "est_internal.hh"
#include "debug.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

// Use SCHED_CLASS for the name of the classSCHED_TRACE_FUNCTION
// (this->name cannot be used in static member functions)
#define SCHED_TRACE_FUNCTION(formatted_text)            \
  TRACE_TEXT(TRACE_ALL_STEPS, SCHED_CLASS << "::");     \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)


/*
 * Random permutation of the sub-sequence [\c first_idx, ..., \c last_idx] of
 * \c seq.
 */
void
random_permute(SeqLong *seq, int first_idx, int last_idx);


/****************************************************************************/
/* Scheduler                                                                */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "Scheduler"

Scheduler::Scheduler() {
  this->name = "Scheduler";
}
Scheduler::~Scheduler() {
}

/**
 * Return the serialized scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
Scheduler::serialize(Scheduler *S) {
  SCHED_TRACE_FUNCTION((void *) S->name);
  if (!strncmp(S->name, RandScheduler::stName, S->nameLength)) {
    return (RandScheduler::serialize((RandScheduler *) S));
  } else if (!strncmp(S->name, MinScheduler::stName, S->nameLength)) {
    return (MinScheduler::serialize((MinScheduler *) S));
  } else if (!strncmp(S->name, MaxScheduler::stName, S->nameLength)) {
    return (MaxScheduler::serialize((MaxScheduler *) S));
  } else if (!strncmp(S->name, PriorityScheduler::stName, S->nameLength)) {
    return (PriorityScheduler::serialize((PriorityScheduler *) S));
  } else if (!strncmp(S->name, RRScheduler::stName, S->nameLength)) {
    return (RRScheduler::serialize((RRScheduler *) S));
  } else {
    INTERNAL_ERROR("unable to serialize scheduler named " << S->name, 1);
    /* this line never executes, but exists to avoid  warning */
    return NULL;
  }
} // serialize

/**
 * Return the Scheduler deserialized from the string \c serializedScheduler.
 */
Scheduler *
Scheduler::deserialize(const char *serializedScheduler) {
  // SCHED_TRACE_FUNCTION(serializedScheduler);
  int nameLength;

  {
    const char *comma;
    if ((comma = strchr(serializedScheduler, ',')) != NULL) {
      nameLength = comma - serializedScheduler;
    } else {
      nameLength = strlen(serializedScheduler);
    }
  }

  if (!strncmp(serializedScheduler, RandScheduler::stName, nameLength)) {
    return (RandScheduler::deserialize(serializedScheduler + nameLength));
  } else if (!strncmp(serializedScheduler, MinScheduler::stName, nameLength)) {
    return (MinScheduler::deserialize(serializedScheduler + nameLength));
  } else if (!strncmp(serializedScheduler, MaxScheduler::stName, nameLength)) {
    return (MaxScheduler::deserialize(serializedScheduler + nameLength));
  } else if (!strncmp(serializedScheduler,
                      PriorityScheduler::stName,
                      nameLength)) {
    return (PriorityScheduler::deserialize(serializedScheduler + nameLength));
  } else if (!strncmp(serializedScheduler, RRScheduler::stName, nameLength)) {
    return (RRScheduler::deserialize(serializedScheduler + nameLength));
  } else {
    INTERNAL_WARNING("unable to deserialize scheduler \""
                     << serializedScheduler
                     << "\"; "
                     << "reverting to default (random)");
    return (new RandScheduler());
  }
} // deserialize

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
Scheduler::aggregate(corba_response_t &aggrResp,
                     int *lastAggregated,
                     const size_t nb_responses,
                     const corba_response_t *responses,
                     int *lastAggr,
                     Vector_t evCache) {
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
  typedef node_t *level_t;

  /* Number of actual leaves of the tree (== nb_responses). */
  size_t nb_leaves = nb_responses;
  /* The lowest power of 2 greater or equal than nb_leaves. */
  size_t pow;
  /* Array of (pow + 1) arrays: each array is a level of the binary tree. */
  level_t *levels;
  /* Point at the root. */
  node_t *root;
  /* Point at the leaves array. */
  level_t leaves;
  /* Left and right storage for nodes for comparisons */
  node_t *lft;
  node_t *rht;

  /** Print the tree on standard output */
#define TRACE_TREE(levels, pow)                                          \
  for (size_t i = 0; i <= pow; i++) {                                   \
    cout << ' ';                                                        \
    for (int j = 0; j < (1 << i); j++) {                                \
      cout << ' ' << (levels[i])[j].resp_idx << ',' << (levels[i])[j].srv_idx; \
    }                                                                   \
    cout << endl;                                                       \
  }

  /** Compare 2 nodes */
#if 0
#define COMPARE_NODES(levels, pow, responses, fst, snd, parent)              \
  if (fst->resp_idx == -1) {                                              \
    parent = *snd; }                                                      \
  else if (snd->resp_idx == -1) {                                         \
    parent = *fst; }                                                      \
  else {                                                                \
    int cmp =                                                           \
      (*compare)(fst->srv_idx,                                          \
                 snd->srv_idx,                                          \
                 fst->resp_idx,                                         \
                 snd->resp_idx,                                         \
                 responses,                                             \
                 evCache,                                               \
                 this->cmpInfo);                                        \
    switch (cmp) {                                                      \
    case COMPARE_SECOND_IS_BETTER:                                      \
      parent = *snd;        break;                                      \
    case COMPARE_FIRST_IS_BETTER:                                       \
    case COMPARE_EQUAL:             /* choose the first when equal */   \
      parent = *fst;        break;                                      \
    case COMPARE_UNDEFINED:                                             \
      parent.resp_idx = -1; break;                                      \
    default:                                                            \
      INTERNAL_WARNING("compare returned wrong value: " << cmp);        \
      parent.resp_idx = -1;                                             \
    }                                                                   \
  }
#endif // if 0
#define COMPARE_NODES(levels, pow, responses, fst, snd, parent)              \
  if (fst->resp_idx == -1) {                                            \
    if (snd->resp_idx == -1 ||                                          \
        (*compare)(snd->srv_idx,                                        \
                   snd->srv_idx,                                        \
                   snd->resp_idx,                                       \
                   snd->resp_idx,                                       \
                   responses,                                           \
                   evCache,                                             \
                   this->cmpInfo) == COMPARE_UNDEFINED) {               \
      /* both tree nodes are null                               */      \
      /*          -- OR --                                      */      \
      /* the first is null and the second node cannot be        */      \
      /* treated with this aggregator's compare function        */      \
      /*          -- SO --                                      */      \
      /* promote the first, which is a null node                */      \
      parent = *fst;                                                    \
    }                                                                   \
    else {                                                              \
      /* first tree node null, second has valid metric          */      \
      /*          -- SO --                                      */      \
      /* promote the second, which is a real node               */      \
      parent = *snd;                                                    \
    }                                                                   \
  }                                                                     \
  else if (snd->resp_idx == -1) {                                       \
    if ((*compare)(fst->srv_idx,                                        \
                   fst->srv_idx,                                        \
                   fst->resp_idx,                                       \
                   fst->resp_idx,                                       \
                   responses,                                           \
                   evCache,                                             \
                   this->cmpInfo) == COMPARE_UNDEFINED) {               \
      /* the second node is null and the first cannot be        */      \
      /* evaluated with this aggregator's compare function      */      \
      /*          -- SO --                                      */      \
      /* promote the second, which is a null node               */      \
      parent = *snd;                                                    \
    }                                                                   \
    else {                                                              \
      /* second tree node null, first has valid metric          */      \
      /*          -- SO --                                      */      \
      /* promote the first, which is a real node                */      \
      parent = *fst;                                                    \
    }                                                                   \
  }                                                                     \
  else {                                                                \
    /* both tree nodes are real nodes, so let's do            */        \
    /* full comparison                                        */        \
    int cmp =                                                           \
      (*compare)(fst->srv_idx,                                          \
                 snd->srv_idx,                                          \
                 fst->resp_idx,                                         \
                 snd->resp_idx,                                         \
                 responses,                                             \
                 evCache,                                               \
                 this->cmpInfo);                                        \
    switch (cmp) {                                                      \
    case COMPARE_SECOND_IS_BETTER:                                      \
      parent = *snd;        break;                                      \
    case COMPARE_FIRST_IS_BETTER:                                       \
    case COMPARE_EQUAL:             /* choose the first when equal */   \
      parent = *fst;        break;                                      \
    case COMPARE_UNDEFINED:                                             \
      INTERNAL_WARNING("compare returned undefined value: " << cmp);    \
      parent.resp_idx = -1; break;                                      \
    default:                                                            \
      INTERNAL_WARNING("compare returned wrong value: " << cmp);        \
      parent.resp_idx = -1;                                             \
    }                                                                   \
  }

  SCHED_TRACE_FUNCTION("nb_responses=" << nb_responses);

  /* Initialize the tree */
  size_t idx;

  if (*lastAggregated >= ((int) aggrResp.servers.length() - 1)) { // Nothing to do
    return 0;
  }

  /* compute the lower power of 2 greater than nb_leaves */
  pow = 0;
  bool power_of_two = true;
  while (nb_leaves > 1) {
    if ((nb_leaves & 1) == 1) {
      power_of_two = false;
    }
    nb_leaves = (nb_leaves >> 1);
    pow++;
  }
  if (!power_of_two) {
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
    (levels[pow])[idx].srv_idx = lastAggr[idx] + 1;
  }
  /* fill in rest of leaves with -1 */
  for (; (int) idx < (1 << pow); idx++) {
    (levels[pow])[idx].resp_idx = -1;
  }
  /* init the other levels of the tree */
  for (int i = pow - 1; i >= 0; i--) {
    for (int j = 0; j < (1 << i); j++) {
      lft = &((levels[i + 1])[2 * j]);
      rht = &((levels[i + 1])[2 * j + 1]);
      COMPARE_NODES(levels, pow, responses, lft, rht, (levels[i])[j]);
    }
  }

  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    cout << "Initial tree:" << leaves;
    TRACE_TREE(levels, pow);
  }

  /* Perform the aggregation itself. */
  while ((root->resp_idx != -1)
         && (*lastAggregated < ((int) aggrResp.servers.length() - 1))) {
    size_t new_srv_idx;
    (*lastAggregated)++;
    aggrResp.servers[*lastAggregated]
      = responses[root->resp_idx].servers[root->srv_idx];

    new_srv_idx = ++leaves[root->resp_idx].srv_idx;
    if (new_srv_idx >= responses[root->resp_idx].servers.length()) {
      leaves[root->resp_idx].resp_idx = -1;  // this response is aggregated
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
        rht = &((levels[i])[changed_srv_idx + 1]);
      } else {                           /* odd */
        lft = &((levels[i])[changed_srv_idx - 1]);
        rht = &((levels[i])[changed_srv_idx]);
      }
      parent_loc = changed_srv_idx >> 1; /* parent location in bin tree */
      COMPARE_NODES(levels, pow, responses, lft, rht,
                    (levels[i - 1])[parent_loc]);

      /* Find parent srv_idx in binary tree */
      changed_srv_idx = parent_loc;
    }

    if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
      TRACE_TREE(levels, pow);
    }
  }

  /* Clean up memory */
  for (size_t i = 0; i <= pow; i++) {
    delete [] levels[i];
  }
  delete [] levels;

  return 0;

#undef COMPARE_NODES
  // #undef UPDATE_TREE
#undef TRACE_TREE
} // aggregate

/**
 * Return an estVector for the indicated server estimation
 */
estVectorConst_t
Scheduler::getEstVector(int sIdx,
                        int rIdx,
                        const corba_response_t *responses) {
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
                        const corba_response_t *responses,
                        Vector_t evCache) {
  assert(evCache != NULL);
  assert(rIdx < Vector_size(evCache));

  Vector_t sv = (Vector_t) Vector_elementAt(evCache, rIdx);
  estVectorConst_t ev;

  if (sIdx < Vector_size(sv) &&
      (ev = (estVector_t) Vector_elementAt(sv, sIdx)) != NULL) {
    // fprintf(stderr, "CACHE(%d,%d) = %d*\n", rIdx, sIdx, (int) ev);
    return (ev);
  }

  ev = Scheduler::getEstVector(sIdx, rIdx, responses);
  Vector_set(sv, ev, sIdx);
  // fprintf(stderr, "CACHE(%d,%d) = %d\n", rIdx, sIdx, (int) ev);
  return (ev);
} // getEstVector


/****************************************************************************/
/* Random Scheduler                                                         */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "RandScheduler"

const char *RandScheduler::stName = "RandScheduler";

/** This is designed to fill in RandScheduler compare member. */
int
RandScheduler_compare(int serverIdx1,
                      int serverIdx2,
                      int responseIdx1,
                      int responseIdx2,
                      const corba_response_t *responses,
                      Vector_t evCache,
                      const void *useless) {
  return ((rand() % 2) ? COMPARE_FIRST_IS_BETTER : COMPARE_SECOND_IS_BETTER);
}

RandScheduler::RandScheduler() {
  this->name = RandScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RandScheduler_compare;
  this->cmpInfo = NULL;
  this->seed = time(NULL);
  srand(this->seed);
}

RandScheduler::RandScheduler(unsigned int seed) {
  this->name = RandScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RandScheduler_compare;
  this->cmpInfo = NULL;
  this->seed = seed;
  srand(seed);
}

RandScheduler::~RandScheduler() {
}

/**
 * Return the serialized Rand scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
RandScheduler::serialize(RandScheduler *S) {
  char *res = new char[S->nameLength + 1];

  SCHED_TRACE_FUNCTION(S->name);
  strcpy(res, S->stName);
  return res;
}

/**
 * Return the RandScheduler deserialized from the string
 * \c serializedScheduler.
 */
RandScheduler *
RandScheduler::deserialize(const char *serializedScheduler) {
  SCHED_TRACE_FUNCTION(serializedScheduler);
  assert(*serializedScheduler == '\0');
  return (new RandScheduler());
}

/****************************************************************************/
/* RR Scheduler                                                             */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "RRScheduler"

const char *RRScheduler::stName = "RRScheduler";

/** This is designed to fill in RRScheduler compare member. */
int
RRScheduler_compare(int serverIdx1,
                    int serverIdx2,
                    int responseIdx1,
                    int responseIdx2,
                    const corba_response_t *responses,
                    Vector_t evCache,
                    const void *useless) {
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
    } else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  } else if (tsls_s2 < 0.0) {
    return (COMPARE_FIRST_IS_BETTER);
  } else {
    if (tsls_s1 < tsls_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    } else if (tsls_s1 == tsls_s2) {
      return (COMPARE_EQUAL);
    } else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
} // RRScheduler_compare

RRScheduler::RRScheduler() {
  this->name = RRScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RRScheduler_compare;
  this->cmpInfo = NULL;
  this->seed = time(NULL);
  srand(this->seed);
}

RRScheduler::RRScheduler(unsigned int seed) {
  this->name = RRScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = RRScheduler_compare;
  this->cmpInfo = NULL;
  this->seed = seed;
  srand(seed);
}

RRScheduler::~RRScheduler() {
}

/**
 * Return the serialized RR scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
RRScheduler::serialize(RRScheduler *S) {
  char *res = new char[S->nameLength + 1];

  SCHED_TRACE_FUNCTION(S->name);
  strcpy(res, S->stName);
  return res;
}

/**
 * Return the RRScheduler deserialized from the string
 * \c serializedScheduler.
 */
RRScheduler *
RRScheduler::deserialize(const char *serializedScheduler) {
  SCHED_TRACE_FUNCTION(serializedScheduler);
  assert(*serializedScheduler == '\0');
  return (new RRScheduler());
}

/****************************************************************************/
/* Min Scheduler                                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "MinScheduler"

const char *MinScheduler::stName = "MinScheduler";

/** This is designed to fill in MinScheduler compare member. */
int
MinScheduler_compare(int serverIdx1,
                     int serverIdx2,
                     int responseIdx1,
                     int responseIdx2,
                     const corba_response_t *responses,
                     Vector_t evCache,
                     const void *tagvalPtr) {
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
    } else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  } else if (mval_s2 == HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  } else {
    // fprintf(stderr, "min metric comparing %.4f %.4f\n", mval_s1, mval_s2);
    if (mval_s1 > mval_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    } else if (mval_s1 == mval_s2) {
      return (COMPARE_EQUAL);
    } else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
} // MinScheduler_compare

MinScheduler::MinScheduler(int tagval) {
  assert(tagval >= 0);
  this->name = MinScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = MinScheduler_compare;
  this->tagval = tagval;
  this->cmpInfo = &(this->tagval);
}

MinScheduler::~MinScheduler() {
}

/**
 * Return the serialized Min scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
MinScheduler::serialize(MinScheduler *S) {
  char *res = new char[S->nameLength + 20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->tagval);
  return res;
}

/**
 * Return the MinScheduler deserialized from the string
 * \c serializedScheduler.
 */
MinScheduler *
MinScheduler::deserialize(const char *serializedScheduler) {
  int tagval;
  char dummy;

  SCHED_TRACE_FUNCTION(serializedScheduler);
  if (sscanf((char *) (serializedScheduler + 1), "%d%c", &tagval,
             &dummy) != 1) {
    INTERNAL_ERROR("invalid parameters for Min scheduler", -1);
    return (NULL);
  }

  return (new MinScheduler(tagval));
} // deserialize

/****************************************************************************/
/* Max Scheduler                                                            */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "MaxScheduler"

const char *MaxScheduler::stName = "MaxScheduler";

/** This is designed to fill in MaxScheduler compare member. */
int
MaxScheduler_compare(int serverIdx1,
                     int serverIdx2,
                     int responseIdx1,
                     int responseIdx2,
                     const corba_response_t *responses,
                     Vector_t evCache,
                     const void *tagvalPtr) {
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
    } else {
      return (COMPARE_SECOND_IS_BETTER);
    }
  } else if (mval_s2 == -HUGE_VAL) {
    return (COMPARE_FIRST_IS_BETTER);
  } else {
    // fprintf(stderr, "max metric comparing %.4f %.4f\n", mval_s1, mval_s2);
    if (mval_s1 < mval_s2) {
      return (COMPARE_SECOND_IS_BETTER);
    } else if (mval_s1 == mval_s2) {
      return (COMPARE_EQUAL);
    } else {
      return (COMPARE_FIRST_IS_BETTER);
    }
  }
} // MaxScheduler_compare

MaxScheduler::MaxScheduler(int tagval) {
  assert(tagval >= 0);
  this->name = MaxScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = MaxScheduler_compare;
  this->tagval = tagval;
  this->cmpInfo = &(this->tagval);
}

MaxScheduler::~MaxScheduler() {
}

/**
 * Return the serialized Max scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
MaxScheduler::serialize(MaxScheduler *S) {
  char *res = new char[S->nameLength + 20];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->tagval);
  return res;
}

/**
 * Return the MaxScheduler deserialized from the string
 * \c serializedScheduler.
 */
MaxScheduler *
MaxScheduler::deserialize(const char *serializedScheduler) {
  int tagval;
  char dummy;

  SCHED_TRACE_FUNCTION(serializedScheduler);
  if (sscanf((char *) (serializedScheduler + 1), "%d%c", &tagval,
             &dummy) != 1) {
    INTERNAL_ERROR("invalid parameters for Max scheduler", -1);
    return (NULL);
  }

  return (new MaxScheduler(tagval));
} // deserialize

/****************************************************************************/
/* Priority Scheduler                                                       */
/****************************************************************************/
#undef SCHED_CLASS
#define SCHED_CLASS "PriorityScheduler"

const char *PriorityScheduler::stName = "PriorityScheduler";

/** This is designed to fill in PriorityScheduler compare member. */
int
PriorityScheduler_compare(int serverIdx1,
                          int serverIdx2,
                          int responseIdx1,
                          int responseIdx2,
                          const corba_response_t *responses,
                          Vector_t evCache,
                          const void *pListPtr) {
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
  for (int pvalIter = 0; pvalIter < pl->pl_numValues; pvalIter++) {
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
      } else if (val2 < val1) {
        return (COMPARE_SECOND_IS_BETTER);
      }
    } else {
      if (val1 > val2) {
        return (COMPARE_FIRST_IS_BETTER);
      } else if (val2 > val1) {
        return (COMPARE_SECOND_IS_BETTER);
      }
    }
  }

  /* all comparisons were valid, and equal */
  return (COMPARE_EQUAL);
} // PriorityScheduler_compare

PriorityScheduler::PriorityScheduler(int numValues, int *values) {
  if (numValues <= 0) {
    INTERNAL_ERROR("Priority scheduler instantiated with <= 0 values", -1);
    return;
  }
  this->name = PriorityScheduler::stName;
  this->nameLength = strlen(this->name);
  this->compare = PriorityScheduler_compare;
  this->pl.pl_numValues = numValues;
  this->pl.pl_values = new int[numValues];
  for (int valIter = 0; valIter < numValues; valIter++) {
    this->pl.pl_values[valIter] = values[valIter];
  }
  this->cmpInfo = &(this->pl);
}

PriorityScheduler::~PriorityScheduler() {
}

/**
 * Return the serialized Priority scheduler (a string)
 * NB: doubles are serialized with a precision of 10 significant decimals.
 */
char *
PriorityScheduler::serialize(PriorityScheduler *S) {
  char *res = new char[S->nameLength + (8 * S->pl.pl_numValues)];

  SCHED_TRACE_FUNCTION(S->name);
  sprintf(res, "%s,%d", S->stName, S->pl.pl_numValues);
  for (int valIter = 0; valIter < S->pl.pl_numValues; valIter++) {
    sprintf(res + strlen(res), ",%d", S->pl.pl_values[valIter]);
  }
  return res;
}

/**
 * Return the PriorityScheduler deserialized from the string
 * \c serializedScheduler.
 */
PriorityScheduler *
PriorityScheduler::deserialize(const char *serializedScheduler) {
  int numValues;
  const char *strPtr = serializedScheduler + 1;

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

  int *values = new int[numValues];
  for (int valIter = 0; valIter < numValues; valIter++) {
    int curVal;
    if (sscanf(strPtr, "%d", &curVal) != 1) {
      delete[] values;
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
} // deserialize

/****************************************************************************/
/* Utils                                                                    */
/****************************************************************************/


/**
 * Random permutation of a sub-sequence of \c seq, starting at index
 * \c first_idx, ending at index \c last_idx.
 */
void
random_permute(SeqLong *seq, int first_idx, int last_idx) {
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
} // random_permute
