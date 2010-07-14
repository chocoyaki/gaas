#include "wfLogServiceImpl.hh"

WfLogServiceFwdrImpl::WfLogServiceFwdrImpl(Forwarder_ptr fwdr,
																					 const char* objName)
{
	this->forwarder = Forwarder::_duplicate(fwdr);
	this->objName = CORBA::string_dup(objName);
}

void WfLogServiceFwdrImpl::setWf(const char* workflow_description) {
	forwarder->setWf(workflow_description, objName);
}

void WfLogServiceFwdrImpl::nodeIsDone(const char* node_id) {
	forwarder->nodeIsDone(node_id, objName);
}

void WfLogServiceFwdrImpl::nodeIsRunning(const char* node_id,
																				 const char* hostname)
{
	forwarder->nodeIsRunning(node_id, hostname, objName);
}

void WfLogServiceFwdrImpl::nodeIsStarting(const char* node_id) {
	forwarder->nodeIsStarting(node_id, objName);
}

void WfLogServiceFwdrImpl::nodeIsWaiting(const char* node_id) {
	forwarder->nodeIsWaiting(node_id, objName);
}

void WfLogServiceFwdrImpl::ping() {
	forwarder->ping(objName);
}
