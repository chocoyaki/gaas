/****************************************************************************/
/* MultiCall.                                                               */
/* It is used to devide a profile and make several calls with just one SeD. */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/*   - Ghislain Charrier (Ghislain.Charrier@ens-lyon.fr)                    */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2011/01/13 23:42:40  ecaron
 * Add missing header
 *
 ****************************************************************************/


#include "MultiCall.hh"
#include "est_internal.hh"
#include <iostream>
#include <cstdlib>

using namespace std;

corba_response_t * MultiCall::corba_response = NULL;

void MultiCall::set_response(corba_response_t *response) {
  corba_response = response;
}

corba_response_t* MultiCall::get_response() {
  return corba_response;
}

//computes the number of scenario per cluster
vector<int> MultiCall::cerfacsSchedule() {
  vector<int> nbDags;
  int nbClusters = corba_response->servers.length();
  int NS = (&(corba_response->servers[0]).estim)->estValues.length() - 1;
  int dag;
  int MSmin;
  int temp;
  int clusterMin;
  int i;

  for (i = 0; i < nbClusters; i++) {
    nbDags.push_back(0);
  }

  for (dag = 0; dag < NS; dag++) {
    MSmin = -1;
    clusterMin = 0;
    for (i = 0; i < nbClusters; i++) {
      temp = diet_est_get_internal((&(corba_response->servers[i]).estim), EST_USERDEFINED + (nbDags[i]), 0);
      if (MSmin == -1 || temp < MSmin) {
	MSmin = temp;
	clusterMin = i;
      }
    }
    nbDags[clusterMin]++;
  }
  return nbDags;
}

bool MultiCall::updateCall(diet_profile_t* profile, SeD_var& chosenServer) {
  static vector<int> nb_scenarios;
  static int counter = 0;
  static char* save;
  char* s;
  char c[2];
  char * stemp;
  c[0] = '#';
  c[1] = '\0';
  

  //some datas need to be initialized just once
  if (counter == 0) {
    nb_scenarios = MultiCall::cerfacsSchedule();
    diet_paramstring_get(diet_parameter(profile, 2), &save, NULL);
  }

  s = (char*)malloc((strlen(save) + 1) * sizeof(char));
  strcpy(s, save);

  stemp = (char*)malloc((strlen(s) + 1) * sizeof(char));  
  stemp[0] = '\0';

  //if there is at least a scenario on this SeD
  if (nb_scenarios[counter] != 0) {
    chosenServer = corba_response->servers[counter].loc.ior;
    diet_scalar_set(diet_parameter(profile, 0), 
		    &(nb_scenarios[counter]), 
		    DIET_VOLATILE, DIET_INT);
    //splits the mnemonics
    char* tmp;
    tmp = strtok(save, c); 
    for (int counter2 = 0; counter2 < nb_scenarios[counter]; counter2++) {
      if (counter2 != 0) { 
	stemp = strcat(stemp, c);
      }
      stemp = strcat(stemp, tmp);
      tmp = strtok(NULL, c);
    }
    //reconstruct the end of the string
    int counter2 = 0;
    s[0] = '\0';
    while (tmp != NULL) {
      if (counter2 > 0) {
	s = strcat(s, c);
      }
      s = strcat(s, tmp);
      tmp = strtok(NULL, c);
      counter2++;
    }

    //updating the profile
    diet_paramstring_set(diet_parameter(profile, 2), stemp, DIET_VOLATILE);
  }
  
  strcpy(save, s);
  free(s);
  
  //there are scenarios to send, so call
  if (nb_scenarios[counter] != 0) {
    counter++;
    return true;
  }

  counter++;

  return false;
}
