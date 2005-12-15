#include "Cori_Fast.hh"
#include "FASTMgr.hh"

#include "est_internal.hh"
#include "debug.hh"

Cori_Fast::Cori_Fast(){
  FASTMgr::init();
}

int 
Cori_Fast::get_Information(diet_est_tag_t type_Info,       
			   estVector_t* info,
			   const diet_profile_t* const initprofilePtr,
			   ServiceTable* SRVT)
{
  char hostnameBuf[HOSTNAME_BUFLEN];
  int stRef;

  if (gethostname(hostnameBuf, HOSTNAME_BUFLEN-1)) {
    ERROR("error getting hostname", 0);
  }

  stRef = diet_service_table_lookup_by_profile(initprofilePtr);
  FASTMgr::estimate(hostnameBuf,
                    initprofilePtr,
                    SRVT,
                    (ServiceTable::ServiceReference_t) stRef,
                    *info);
                    
  return (0);
}
