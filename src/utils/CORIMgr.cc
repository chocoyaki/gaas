#include "FASTMgr.hh"
#include "CORIMgr.hh"
#include <math.h>
#include "est_internal.hh"

#include <iostream>

using namespace std;


vector <Cori_Metric>* CORIMgr::collector_v=new vector <Cori_Metric>();
int CORIMgr::is_initiated=0;

int
CORIMgr::init(){
  if (!is_initiated)
  is_initiated=1;
  return 0;
}

int
CORIMgr::add(diet_est_collect_tag_t collectorName,
	     const void * datav)
{

  Cori_Metric tmp ( collectorName, datav);
  collector_v->push_back(tmp);
  //#else is no possible to add something

  return 0;
}


int
CORIMgr::call_cori_mgr(estVector_t *ev,
			    diet_est_tag_t info_type,
			    diet_est_collect_tag_t name,
			    const void * datav)
{
  vector <Cori_Metric>::iterator iter1;
  iter1 = collector_v->begin();
  while( iter1 != collector_v->end()) {
    
    if (name==iter1->get_Collector_type()){    
      return iter1->call_cori_metric(info_type,ev,datav);
    }
    else 
      iter1++;
  }
  return 0;
}
