/**
 * @file dmat_manips_server.c
 *
 * @brief  DIET server for transpose, MatSUM and MatPROD SqMatSUM, SqMatSUM_opt
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include <stdint.h>
#include <deque>
#include <vector>
#include <utility>
#include <string>
#include "boost/tuple/tuple.hpp"

#include "common.hh"
#include "power_scheduler.hh"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"
#include "progs.h"

#include <string>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>

#include <wait.h>
#include <unistd.h>
#include <libgen.h>
#include <algorithm>
#include <iterator>
#include <vector>

#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>

#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <istream>

using namespace std;

/* This server can offer 5 services (chosen by at launch time):      */
/*   - T = matrix translation                                        */
/*   - MatSUM = matrix addition (and thus SqMatSUM and SqMatSUM_opt) */
/*   - MatPROD = matrix product                                      */

#define NB_SRV 3
static const char *SRV[NB_SRV] =
{"T", "MatPROD", "MatSUM"};

/*
 * SOLVE FUNCTIONS
 */

int
solve_T(diet_profile_t *pb) {
  size_t m;
  size_t n;
  double *A = NULL;
  double *C = NULL;
  diet_matrix_order_t o;
  int res, i;

  printf("Solve T ...");

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &m, &n, &o);
  C = (double *)malloc(n * m * sizeof(double));

  for (i = 0; i < n * m; ++i)
    C[i] = A[i];
  if ((res = T(m, n, A, (o == DIET_ROW_MAJOR)))) {
    free(C);
    return res;
  }

  /* no need to set order */
  diet_matrix_set(diet_parameter(pb, 1), C, DIET_VOLATILE, DIET_DOUBLE, n, m, o);
  print_matrix(A, m, n, o);

  printf(" done\n");
  return 0;
} /* solve_T */

int
solve_MatSUM(diet_profile_t *pb) {
  size_t mA, nA, mB, nB, mC, nC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double *A = NULL;
  double *B = NULL;
  double *C = NULL;
  int res;

  printf("Solve MatSUM ...");

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb, 1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%lu, nA=%lu; mB=%lu, nB=%lu\n",
            mA, nA, mB, nB);
    return 1;
  }

  diet_matrix_get(diet_parameter(pb, 2), &C, NULL, &mC, &nC, &oC);
  C = (double *)calloc(mC * nC, sizeof *C);

  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatSUM(tB, tA, nA, mA, B, A, C);
  } else {
    res = MatSUM(tA, tB, mA, nA, A, B, C);
  }
  diet_matrix_set(diet_parameter(pb,
                                 2), C, DIET_VOLATILE, DIET_DOUBLE, mC, nC, oC);
  if (res) {
    return res;
  }

  printf(" done\n");
  return res;
} /* solve_MatSUM */


int
solve_MatPROD(diet_profile_t *pb) {
  size_t mA, nA, mB, nB, nC, mC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double *A = NULL;
  double *B = NULL;
  double *C = NULL;
  int res;

  printf("Solve MatPROD ...");

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb, 1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld; mB=%ld, nB=%ld\n",
            (long) mA, (long) nA, (long) mB, (long) nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb, 2), &C, NULL, &mC, &nC, &oC);
  C = (double *)calloc(mC * nC, sizeof *C);

  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatPROD(tB, tA, nB, mB, B, mA, A, C);
  } else {
    res = MatPROD(tA, tB, mA, nA, A, nB, B, C);
  }

  diet_matrix_set(diet_parameter(pb,
                                 2), C, DIET_VOLATILE, DIET_DOUBLE, mC, nC, oC);

  printf(" done\n");
  return res;
} /* solve_MatPROD */

int
usage(char *cmd) {
  fprintf(stderr, "Usage: %s <file.cfg> [all | [%s][%s][%s] ]\n",
          cmd, SRV[0], SRV[1], SRV[2]);
  return 1;
}


// curl callback to get the data
size_t get_data_cb(void *buffer, size_t size, size_t nmemb, void *userp) {
  std::stringstream *pbuffer = reinterpret_cast<std::stringstream *>(userp);
  pbuffer->write((char*)buffer, size * nmemb);
  return size * nmemb;
}


// http get an url. returns http body + http response code
std::pair<std::string, int> get_url(std::string url) {
  std::cout << "HTTP GET " << url << std::endl;
  std::stringstream buffer;
  long http_code = 0;
  CURL *hcurl;
  hcurl = curl_easy_init();
  if(hcurl) {
    curl_easy_setopt(hcurl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(hcurl, CURLOPT_WRITEFUNCTION, get_data_cb);
    curl_easy_setopt(hcurl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(hcurl, CURLOPT_SSL_VERIFYPEER, 0);
    CURLcode retcode = curl_easy_perform(hcurl);
    if(retcode != CURLE_OK)
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(retcode) << std::endl;
    retcode = curl_easy_getinfo(hcurl, CURLINFO_RESPONSE_CODE, &http_code);
    if(retcode != CURLE_OK)
      std::cerr << "curl_easy_getinfo() failed: " << curl_easy_strerror(retcode) << std::endl;
    curl_easy_cleanup(hcurl);
  }
  std::pair<std::string, long> result;
  result.first = buffer.str();
  result.second = http_code;
  return result;
}


std::string pexec(std::string cmdline) {
  std::stringstream result;
  if (FILE *f = popen(cmdline.c_str(), "r")) {
    const int bufsize = 1024;
    char buf[bufsize];
    size_t num_read;
    do {
      num_read = fread(buf, 1, bufsize, f);
      result.write(buf, num_read);
    } while (num_read == bufsize);
    pclose(f);
  }
  return result.str();
}

// get the cluster of a node
std::string get_cluster(std::string node) {
  std::string cluster = pexec(std::string("python -c \"import execo_g5k; print execo_g5k.get_host_cluster('") + node + std::string("')\""));
  boost::algorithm::trim(cluster);
  return cluster;
}

// get the site of a node
std::string get_site(std::string node) {
  std::string fullname = pexec(std::string("hostname"));
  boost::algorithm::trim(fullname);
  std::string node_name = pexec(std::string("python -c \"values='") + fullname + std::string("'; print values.split('.')[1]\""));
  boost::algorithm::trim(node_name);
  return node_name;
}


// get the hostname                                                                     
std::string get_node_name() {
  std::string fullname = pexec(std::string("hostname"));
  boost::algorithm::trim(fullname);
  std::string node_name = pexec(std::string("python -c \"values='") + fullname + std::string("'; print values.split('.')[0]\""));
  boost::algorithm::trim(node_name);
  return node_name;
}


double retrieve_instant_consumption(){
  return 0;
}


/*
* Performance fonction
*/
void perfmetric(diet_profile_t *profile, estVector_t estvec) {
  double power = double(retrieve_instant_consumption());
  diet_est_set_internal(estvec, EST_INSTANTPOWER, power);
}


/*
 * MAIN
 */

int
main(int argc, char *argv[]) {
  size_t i, j;
  int res;
  int services[NB_SRV] = {0, 0, 0};
  diet_profile_desc_t *profile = NULL;
  diet_aggregator_desc_t *agg;

  if (argc < 3) {
    return usage(argv[0]);
  }

  for (i = 2; i < argc; i++) {
    char *path = argv[i];
    if (!strcmp("all", path)) {
      for (j = 0; j < NB_SRV; (services[j++] = 1)) ;
      break;
    } else {
      for (j = 0; j < NB_SRV; j++) {
        if (!strcmp(SRV[j], path)) {
          services[j] = 1;
          break;
        }
      }
      if (j == NB_SRV) {
        exit(usage(argv[0]));
      }
    }
  }

  diet_service_table_init(NB_SRV);

  if (services[0]) {
    profile = diet_profile_desc_alloc("T", 0, 0, 1);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
    
    diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR);

    agg = diet_profile_desc_aggregator(profile);
    diet_aggregator_set_type(agg, DIET_AGG_USER);
    diet_service_use_perfmetric(perfmetric);

    diet_service_table_add(profile, NULL, solve_T);
    diet_profile_desc_free(profile);
  }

  if (services[1]) {
    profile = diet_profile_desc_alloc("MatPROD", 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 2), DIET_MATRIX, DIET_DOUBLE);

    diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR);

    agg = diet_profile_desc_aggregator(profile);
    diet_aggregator_set_type(agg, DIET_AGG_USER);
    diet_service_use_perfmetric(perfmetric);


    diet_service_table_add(profile, NULL, solve_MatPROD);
    diet_profile_desc_free(profile);
  }

  if (services[2]) {
    profile = diet_profile_desc_alloc("MatSUM", 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 2), DIET_MATRIX, DIET_DOUBLE);

    diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR);

    agg = diet_profile_desc_aggregator(profile);
    diet_aggregator_set_type(agg, DIET_AGG_USER);
    diet_service_use_perfmetric(perfmetric);


    diet_service_table_add(profile, NULL, solve_MatSUM);
    diet_profile_desc_free(profile);
  }

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
} /* main */
