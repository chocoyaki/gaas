/****************************************************************************/
/* CoRI: FAST module                                                        */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.9  2008/04/07 15:33:44  ycaniou
 * This should remove all HAVE_BATCH occurences (still appears in the doc, which
 *   must be updated.. soon :)
 * Add the definition of DIET_BATCH_JOBID wariable in batch scripts
 *
 * Revision 1.8  2007/04/16 22:43:44  ycaniou
 * Make all necessary changes to have the new option HAVE_ALT_BATCH operational.
 * This is indented to replace HAVE_BATCH.
 *
 * First draw to manage batch systems with a new Cori plug-in.
 *
 * Revision 1.7  2006/10/31 23:02:51  ecaron
 * CoRI: FAST module
 *
 ****************************************************************************/
#include "Cori_Fast.hh"
#include "FASTMgr.hh"

#include "est_internal.hh"
#include "debug.hh"
#include "marshalling.hh"  //mrsh_profile_desc

#define HOSTNAME_BUFLEN 256

Cori_Fast::Cori_Fast(){
}

int
Cori_Fast::start(){
  return FASTMgr::init();
}
int 
Cori_Fast::get_Information(int type_Info,       
			   estVector_t* info,
			   const void* data)
{
  if (type_Info==EST_COMMTIME){
    commTime_t* params=(commTime_t *)data;
    FASTMgr::commTime(params->host1, 
		      params->host2, 
		      params->size, 
		      params->to);
  }
  else
    {
    fast_param_t* fast_param=(fast_param_t*)data;

				     
    char hostnameBuf[HOSTNAME_BUFLEN];
    int stRef;
    
    if (gethostname(hostnameBuf, HOSTNAME_BUFLEN-1)) {
      ERROR("error getting hostname", 0);
    }
    stRef = diet_service_table_lookup_by_profile( fast_param->initprofilePtr,fast_param->SRVT);
    FASTMgr::estimate(hostnameBuf,
                    fast_param->initprofilePtr,
                    fast_param->SRVT,
                    (ServiceTable::ServiceReference_t) stRef,
		     type_Info, 
                    *info);                
  }  
  
  return (0);
}

int
Cori_Fast::diet_service_table_lookup_by_profile(
			const diet_profile_t* const profile,
			ServiceTable* SRVT)
{
  /* Called from diet_estimate_fast */
  int refNum;
  corba_profile_desc_t corbaProfile;
  diet_profile_desc_t profileDesc;

  if (profile == NULL) {
    ERROR(__FUNCTION__ << ": NULL profile", -1);
  }

  if (SRVT == NULL) {
    ERROR(__FUNCTION__ << ": service table not yet initialized", -1);
  }

  { /* create the corresponding profile description */
    profileDesc.path = strdup(profile->pb_name);
    profileDesc.last_in = profile->last_in;
    profileDesc.last_inout = profile->last_inout;
    profileDesc.last_out = profile->last_out;
#if defined HAVE_ALT_BATCH
    /* In case of client explicitly ask for a batch resolution */
    profileDesc.parallel_flag = profile->parallel_flag ;
#endif
    int numArgs = profile->last_out + 1;
    profileDesc.param_desc =
      (diet_arg_desc_t*) calloc (numArgs, sizeof (diet_arg_desc_t));
    for (int argIter = 0 ; argIter < numArgs ; argIter++) {
      profileDesc.param_desc[argIter] =
        (profile->parameters[argIter]).desc.generic;
    }

    profileDesc.aggregator.agg_method = DIET_AGG_DEFAULT;
  }

  mrsh_profile_desc(&corbaProfile, &profileDesc);
  refNum = SRVT->lookupService(&corbaProfile);

  { /* deallocate the dynamic parts of the created profile description */
    free(profileDesc.path);
    free(profileDesc.param_desc);
  }

  return (refNum);
}