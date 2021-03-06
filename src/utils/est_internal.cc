/**
 * @file  est_internal.cc
 *
 * @brief  Plug-in scheduler: DIET-internal estimation vector access functions
 *
 * @author  Alan Su (Alan.Su@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef OLD_EV
#include "DIET_data.h"

#include "debug.hh"
#include "est_internal.hh"

#define EV_ELT(e, i) (((e)->estValues)[(i)])
#define EV_LEN(e) ((e)->estValues.length())
#define EV_SETLEN(e, l) ((e)->estValues.length(l))

int
diet_est_set_internal(estVector_t ev, int tag, double value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx < 0) {
      (EV_ELT(ev, valIter)).v_value.d(value);
      return (1);
    }
  }

  EV_SETLEN(ev, EV_LEN(ev) + 1);
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_tag = tag;
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_idx = EST_IDX_SCALAR;
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_value.d(value);

  return (1);
} // diet_est_set_internal

double
diet_est_get_internal(estVectorConst_t ev, int tag, double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": tag must be non-negative (" <<
          tag <<
          ")\n",
          errVal);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx < 0) {
      return ((EV_ELT(ev, valIter)).v_value.d());
    }
  }

  return (errVal);
} // diet_est_get_internal

int
diet_est_defined_internal(estVectorConst_t ev, int tag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }

  // cout << "AS: [" << __FUNCTION__ << "] num values = " << EV_LEN(ev) << "\n";

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx < 0) {
      return (1);
    }
  }

  return (0);
} // diet_est_defined_internal

int
diet_est_array_size_internal(estVectorConst_t ev, int tag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }

  int maxIdx = -1;
  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        maxIdx < (EV_ELT(ev, valIter)).v_idx) {
      maxIdx = (EV_ELT(ev, valIter)).v_idx;
    }
  }

  return (maxIdx + 1);
} // diet_est_array_size_internal

int
diet_est_array_set_internal(estVector_t ev, int tag, int idx, double value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": idx must be non-negative (" << idx << ")\n", -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx == idx) {
      (EV_ELT(ev, valIter)).v_value.d(value);
      return (1);
    }
  }

  EV_SETLEN(ev, EV_LEN(ev) + 1);
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_tag = tag;
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_idx = idx;
  (EV_ELT(ev, EV_LEN(ev) - 1)).v_value.d(value);

  return (1);
} // diet_est_array_set_internal

double
diet_est_array_get_internal(estVectorConst_t ev,
                            int tag,
                            int idx,
                            double errVal) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", errVal);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": tag must be non-negative (" <<
          tag <<
          ")\n",
          errVal);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": idx must be non-negative (" <<
          idx <<
          ")\n",
          errVal);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx == idx) {
      return ((EV_ELT(ev, valIter)).v_value.d());
    }
  }

  return (errVal);
} // diet_est_array_get_internal

int
diet_est_array_defined_internal(estVectorConst_t ev, int tag, int idx) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }
  if (idx < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": idx must be non-negative (" << idx << ")\n", -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == (EV_ELT(ev, valIter)).v_tag &&
        (EV_ELT(ev, valIter)).v_idx == idx) {
      return (1);
    }
  }

  return (0);
} // diet_est_array_defined_internal

int
diet_est_set_str_internal(estVector_t ev, int tag, const char *value) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == EV_ELT(ev, valIter).v_tag &&
        EV_ELT(ev, valIter).v_idx < 0) {
      EV_ELT(ev, valIter).v_value.s(value);
      return (1);
    }
  }

  EV_SETLEN(ev, EV_LEN(ev) + 1);
  EV_ELT(ev, EV_LEN(ev) - 1).v_tag = tag;
  EV_ELT(ev, EV_LEN(ev) - 1).v_idx = EST_IDX_SCALAR;
  EV_ELT(ev, EV_LEN(ev) - 1).v_value.s(value);

  return (1);
}

const char*
diet_est_get_str_internal(estVectorConst_t ev, int tag) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", NULL);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": tag must be non-negative (" <<
          tag <<
          ")\n",
          NULL);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == EV_ELT(ev, valIter).v_tag &&
        EV_ELT(ev, valIter).v_idx < 0) {
      return (EV_ELT(ev, valIter).v_value.s());
    }
  }

  return (NULL);
}

int
diet_est_set_bin_internal(estVector_t ev, int tag, const unsigned char *buf, size_t size) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ << ": tag must be non-negative (" << tag << ")\n", -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == EV_ELT(ev, valIter).v_tag &&
        EV_ELT(ev, valIter).v_idx < 0) {
      SeqOctet b;
      b.length(size);
      memcpy(b.get_buffer(), buf, size);
      EV_ELT(ev, valIter).v_value.b(b);
      return (1);
    }
  }

  EV_SETLEN(ev, EV_LEN(ev) + 1);
  EV_ELT(ev, EV_LEN(ev) - 1).v_tag = tag;
  EV_ELT(ev, EV_LEN(ev) - 1).v_idx = EST_IDX_SCALAR;
  SeqOctet b;
  b.length(size);
  memcpy(b.get_buffer(), buf, size);
  EV_ELT(ev, EV_LEN(ev) - 1).v_value.b(b);

  return (1);
}

int
diet_est_get_bin_internal(estVectorConst_t ev, int tag, const unsigned char **buf, size_t *size) {
  if (ev == NULL) {
    ERROR_DEBUG(__FUNCTION__ << ": NULL estimation vector", -1);
  }
  if (tag < 0) {
    ERROR_DEBUG(__FUNCTION__ <<
          ": tag must be non-negative (" <<
          tag <<
          ")\n",
          -1);
  }

  for (unsigned int valIter = 0; valIter < EV_LEN(ev); valIter++) {
    if (tag == EV_ELT(ev, valIter).v_tag &&
        EV_ELT(ev, valIter).v_idx < 0) {
      *size = EV_ELT(ev, valIter).v_value.b().length();
      *buf = EV_ELT(ev, valIter).v_value.b().get_buffer();
      return (1);
    }
  }

  return (-1);
}

#endif /* OLD_EV */
