/****************************************************************************/
/* Workflows LogService                                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Gaël Le Mahec (gael.le.mahec@ens-lyon.fr)                           */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/07/14 23:45:29  bdepardo
 * Header corrections
 *
 *
 ****************************************************************************/

#ifndef WFLOGSERVICEIMPL_HH
#define WFLOGSERVICEIMPL_HH

#include "WfLogService.hh"

#include "Forwarder.hh"
#include "WfLogServiceFwdr.hh"

class WfLogServiceFwdrImpl : public POA_WfLogServiceFwdr,
	public PortableServer::RefCountServantBase
{
protected:
	Forwarder_ptr forwarder;
	char* objName;
public:
	WfLogServiceFwdrImpl(Forwarder_ptr fwdr, const char* objName);
	
	virtual void setWf(const char* workflow_description);
  virtual void nodeIsDone(const char* node_id);
  virtual void nodeIsRunning(const char* node_id,
			     const char* hostname);
  virtual void nodeIsStarting(const char* node_id);
  virtual void nodeIsWaiting(const char* node_id);
	virtual void ping();
};
#endif
