/****************************************************************************/
/* Basic workflow description Reader                                        */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id: $
 * $@Log: $
 ****************************************************************************/


#include <iostream>
#include <fstream>
#include <Qt/QString>

#include "WfUtils.hh"
#include "WfReader.hh"

#include "marshalling.hh"
#include "debug.hh"
using namespace std;

WfReader::WfReader(const char * wf_desc):
  content(wf_desc) {
  current_profile = nodes_list.begin();
  current_pb = -1;
}

WfReader::~WfReader() {
}

/**
 * Initialize the processing
 */
void
WfReader::setup() {
  initXml();
}

/**
 * read the workflow description file
 */
bool
WfReader::getContent(const char * fileName) {
  ifstream file (fileName);
  char * line = new char[1025];
  if (!file) {
    ERROR ("Workflow file description Not found", false);
    return false;
  }
  while (!file.eof()) {
    if (file.getline(line, 1024, '\n')) {
      content += line;
      content += "\n";
    }
  } 
  return true;
}

/**
 * test if there is node after the current one
 */
bool
WfReader::hasNext() {
  return current_profile != nodes_list.end();
}

/**
 * return the next problem description
 */
map<string, corba_pb_desc_t>::iterator
WfReader::next() {
  if (hasNext()) {
    return current_profile++;
  }
  else
    return NULL;
}

/**
 * set the iterator to the first element of nodes_list *
 */
void
WfReader::reset() {
  current_profile = nodes_list.begin();
}
#if 0

/**
 * return the first node of the problems list
 */
corba_pb_desc_t * 
WfReader::begin() {
  if (nodes_list.size() > 0) {
    current_profile = 0;
    return &((corba_pb_desc_t)(nodes_list.begin()->second));
  }
  else {
    current_profile = -1;
    return NULL;
  }
}

/**
 * return the last node of the problems list
 */
corba_pb_desc_t * 
WfReader::end() {
  if (nodes_list.size() > 0) {
    current_profile = nodes_list.size() - 1;
    return &((corba_pb_desc_t)(nodes_list.begin()->second));
  }
  else {
    current_profile = -1;
    return NULL;
  }
}

#endif


/** 
 * Init the XML processing
 */
bool 
WfReader::initXml() {
  document.setContent(QString(content.c_str()), false);
  QDomElement root = document.documentElement();
  //  TRACE_TEXT(TRACE_MAIN_STEPS, "root tag name : " << root.tagName().toStdString () << endl);
  if (root.tagName() != "dag") {
    ERROR("the file is not a valid dag description ", false);
    return false;
  }
  else {
    QDomElement child = root.firstChildElement();
    while (!child.isNull()) {
        TRACE_TEXT(TRACE_MAIN_STEPS, "\tprocessing '" << 
		   child.tagName().toStdString().c_str() <<
		   "' element"<< endl);
      if (child.tagName() == "node") {
	//        debug(" ==> processing node element");
        QString nodePath = child.attribute("path");
	QString nodeId   = child.attribute("id");
	//        debug("node name : ", nodeName.toStdString().c_str());
	//        Node * node = new Node(nodeName.toStdString());
        if (!parseNode(child, nodeId, nodePath)) {
	  //          delete node;
          return false;
        }
	//        myDag->addNode(nodeName.toStdString().c_str(), node);
      }
      else {
	return false;
      }
      child = child.nextSiblingElement();
    } // end while
  }
  //  TRACE_TEXT(TRACE_MAIN_STEPS, "parsing xml content successful " <<endl);
  return true;
}

/**
 * parse a node element
 */
bool 
WfReader::parseNode (const QDomElement & element, 
		     QString nodeId, QString nodePath ) {
  diet_profile_t        * profile;
  corba_pb_desc_t       * corba_profile = new corba_pb_desc_t;
  vector <QDomElement> argVect;
  vector <QDomElement> inVect;
  vector <QDomElement> inoutVect;
  vector <QDomElement> outVect;
  vector <QDomElement> precVect;
  QDomElement          node;
  /*
    typedef struct {
      char*       pb_name;
      int         last_in, last_inout, last_out;
      diet_arg_t* parameters;
      ....
  }
  */
  /*
  profile.pb_name = strdup(nodeName.toStdString().c_str());  
  profile.last_in    = 0;
  profile.last_inout = 0;
  profile.last_out   = 0;
  */
  QDomElement child = element.firstChildElement();
  while (!child.isNull()) {
    if (child.tagName() == "arg") {
      QString argName = child.attribute("name");
      QString argValue = child.attribute("value");
      if ((argName == "") || (argValue == "")) {
        ERROR ("Argument "<<child.tagName().toStdString()<<" malformed", false);
        return false;
      }
      if (!child.firstChildElement().isNull()) {
	cerr << "Argument element doesn't accept a child element" << endl <<
	  "May be a </arg> is forgotten" << endl;
	return false;
      }
      argVect.push_back(child);
    }
    if (child.tagName() == "in") {
      if (!child.firstChildElement().isNull()) {
	cerr << "input port element doesn't accept a child element" << endl <<
	  "May be a </in> is forgotten" << endl;
	return false;
      }
      inVect.push_back(child);
    }
    if (child.tagName() == "inOut") {
      if (!child.firstChildElement().isNull()) {
	cerr << "inOut port element doesn't accept a child element" << endl <<
	  "May be a </inOut> is forgotten" << endl;
	return false;
      }
      inoutVect.push_back(child);
    }
    if (child.tagName() == "out") {
      if (!child.firstChildElement().isNull()) {
	cerr << "output port element doesn't accept a child element" << endl <<
	  "May be a </out> is forgotten" << endl;
	return false;
      }
      outVect.push_back(child);
    } 
    if (child.tagName() == "prec") {
      QString prevNodeId = child.attribute("id");
      if (prevNodeId == "") {
        ERROR("Precedence element malformed", false);
        return false;
      }
      if (!child.firstChildElement().isNull()) {
	cerr << "Precedence element doesn't accept a child element" << endl <<
	  "May be a </prec> is forgotten" << endl;
	return false;
      }
      QString key = prevNodeId + QString("/") + nodeId;
      precVect.push_back(child);
      // Is the previous node useful ?
    }
 
    child = child.nextSiblingElement();
  }

  // last_in last_inout last_out
  int last_in    = inVect.size() + argVect.size() - 1;
  int last_inout = last_in + inoutVect.size();
  int last_out   = last_inout + outVect.size();
  int lastArg    = 0;
  profile = diet_profile_alloc((char*)nodePath.toStdString().c_str(), 
				last_in,
				last_inout,
				last_out);
  
  for (uint ix=0; ix < argVect.size(); ix++) {
    node = argVect[ix];
    QString name  = nodeId + "#"+ node.attribute("name");
    QString value = node.attribute("value");
    QString type  = node.attribute("type");
    // check the arg attribute and add its description to the profile
    checkArg (name, value, type, profile, lastArg);
    lastArg++;
  }
  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];
    QString name  = node.attribute("name");
    QString type  = node.attribute("type");
    QString source  = node.attribute("source");
    // check in attribute and add its description to the profile
    checkIn (name, type, source, profile, lastArg);
    lastArg++;
  }
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];
    QString name  = node.attribute("name");
    QString type  = node.attribute("type");
    QString source  = node.attribute("source");
    // check the inout attribute and add its description to the profile
    checkInout(name, type, source, profile, lastArg);
    lastArg++;
  }
  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];
    QString name  = node.attribute("name");
    QString type  = node.attribute("type");
    QString sink  = node.attribute("sink");
    // check the out attribute and add its description to the profile
    checkOut (name, type, sink, profile, lastArg);
    lastArg++;
  }

  mrsh_pb_desc(corba_profile, profile);
  nodes_list[nodeId.toStdString()] = (*corba_profile);

  // add the corba profile to problem list if it is a new one
  if (! pbAlreadyRegistred(*corba_profile) ) {
    pbs_list.push_back(*corba_profile);
  }


  return true;
} // end parseNode

/**
 * parse an argument element
 * @param name     argument id
 * @param value    argument value
 * @param type     argument data type 
 * @param profile  current profile reference
 * @param lastArg  the argument index in the profile
 * @param dagNode  node reference in the Dag structure
 */
bool 
WfReader::checkArg(const QString& name, 
		   const QString& value, 
		   const QString& type,
		   diet_profile_t * profile, 
		   unsigned int lastArg,
		   BasicNode * dagNode) {
  setParam(ARG_PORT, name, type, profile, lastArg, dagNode, &value);
  return true;
}

/**
 * parse an input port element
 * @param name     input port id
 * @param type     input port data type 
 * @param source   linked output port id
 * @param profile  current profile reference
 * @param lastArg  the input port index in the profile
 * @param dagNode  node reference in the Dag structure
 */
bool 
WfReader::checkIn(const QString& name, 
		  const QString& type, 
		  const QString& source,
		  diet_profile_t * profile,
		  unsigned int lastArg,
		  BasicNode * dagNode) {
  setParam(IN_PORT, name, type, profile, lastArg, dagNode);
  if (dagNode)
    dagNode->link_i2o(name.toStdString(), source.toStdString());
  return true;
}

/**
 * parse an input/output port element
 * @param name     inoutput port id
 * @param type     inoutput port data type 
 * @param source   linked output port id
 * @param profile  current profile reference
 * @param lastArg  the inoutput port index in the profile
 * @param dagNode  node reference in the Dag structure
 */  
bool
WfReader::checkInout(const QString& name, 
		     const QString& type, 
		     const QString& source,
		     diet_profile_t * profile,
		     unsigned int lastArg,
		     BasicNode * dagNode) {
  setParam(INOUT_PORT, name, type, profile, lastArg, dagNode);
  return true;
}

/**
 * parse an output port element
 * @param name     output port id
 * @param type     output port data type 
 * @param sink     linked input port id
 * @param profile  current profile reference
 * @param lastArg  the output port index in the profile
 * @param dagNode  node reference in the Dag structure
 */  
bool
WfReader::checkOut(const QString& name, 
		   const QString& type,
		   const QString& sink,
		   diet_profile_t * profile,
		   unsigned int lastArg,
		   BasicNode * dagNode) {
  setParam(OUT_PORT, name, type, profile, lastArg, dagNode);
  if (dagNode)
    dagNode->link_o2i(name.toStdString(), sink.toStdString());
  return true;
}
/**
 * fill a profile with the appropriate parameter type
 */  
bool 
WfReader::setParam(const wf_port_t param_type,
		   const QString& name,
		   const QString& type,
		   diet_profile_t * profile,
		   unsigned int lastArg,
		   BasicNode * dagNode,
		   const QString * value) {
  if (type == WfCst::DIET_CHAR) {
    diet_scalar_set(diet_parameter(profile, lastArg), 
		    NULL, 
		    DIET_VOLATILE,
		    DIET_CHAR);
    /*
    if (dagNode)
      dagNode->newPort(name.toStdString(), lastArg, param_type, 
		       string(WfCst::DIET_CHAR));
    */
  }
  if (type == WfCst::DIET_SHORT) {
    diet_scalar_set(diet_parameter(profile, lastArg), 
		    NULL, 
		    DIET_VOLATILE,
		    DIET_SHORT);
    /*
    if (dagNode)
      dagNode->newPort(name.toStdString(), lastArg, param_type, 
		       string(WfCst::DIET_SHORT));
    */
  }
  if (type == WfCst::DIET_INT) {
    diet_scalar_set(diet_parameter(profile, lastArg), 
		    NULL, 
		    DIET_VOLATILE,
		    DIET_INT);
    /*
    if (dagNode)
      dagNode->newPort(name.toStdString(), lastArg, param_type, 
		       string(WfCst::DIET_INT));
    */
  }
  if (type == WfCst::DIET_LONGINT) {
    diet_scalar_set(diet_parameter(profile, lastArg), 
		    NULL, 
		    DIET_VOLATILE,
		    DIET_LONGINT);
    /*
    if (dagNode)
      dagNode->newPort(name.toStdString(), lastArg, param_type, 
		       string(WfCst::DIET_LONGINT));
    */
  }
  if (type == WfCst::DIET_STRING) {
    /*
      diet_string_set(diet_arg_t* arg, 
                      char* value, diet_persistence_mode_t mode);
    */
    diet_string_set(diet_parameter(profile, lastArg),
		    new char[1024],
		    DIET_VOLATILE);
    /*
    if (dagNode)
      dagNode->newPort(name.toStdString(), lastArg, param_type, 
		       string(WfCst::DIET_STRING));
    */
  }
  return true;
}

bool operator != (corba_data_id_t& id1,
		  corba_data_id_t& id2) {
  if (strcmp(id1.idNumber, id2.idNumber) ||
      (id1.dataCopy != id2.dataCopy) ||
      (id1.state != id2.state)
      )
    return true;
  return false;
}

bool operator != (corba_data_desc_t& d1,
		  corba_data_desc_t& d2) {
  if ((d1.mode      != d2.mode) ||
      (d1.base_type != d2.base_type) ||
      //      (d1.specific  != d2.specific) ||
      (d1.id        != d2.id)
      )
    return true;
  return false;
}

bool operator != (SeqCorbaDataDesc_t& s1, 
		  SeqCorbaDataDesc_t& s2) {
  if (s1.length() != s2.length())
    return true;
  for (unsigned int ix=0; ix<s1.length(); ix++) {
    if (s1[ix] != s2[ix])
      return true;
  }
  return false;
}

bool operator == (corba_pb_desc_t& a,   corba_pb_desc_t& b) {
  if (strcmp(a.path, b.path) ||
      (a.last_in    != b.last_in) ||
      (a.last_inout != b.last_inout) ||
      (a.last_out   != b.last_out) ||
      (a.param_desc != b.param_desc)
      )
    return false;
  return true;
}
/**
 * check if the profile is already in the problems list
 */
bool
WfReader::pbAlreadyRegistred(corba_pb_desc_t& pb_desc) {
  for (unsigned int ix=0; ix<pbs_list.size(); ix++) {
    if (pbs_list[ix] == pb_desc)
      return true;
  }
  return false;
}

/**
 * test if there is node after the current one
 */
bool
WfReader::hasPbNext() {
  if (pbs_list.size() < 1)
    return false;
  if ((current_pb + 1) >= (int)pbs_list.size())
    return false;
  else
    return true;
}

/**
 * return the first node of the problems list
 */
corba_pb_desc_t * 
WfReader::pbReset() {
  current_pb = -1;
  return NULL;
}

/**
 * return the last node of the problems list
 */
corba_pb_desc_t * 
WfReader::pbEnd() {
  if (pbs_list.size() > 0) {
    current_pb = pbs_list.size() - 1;
    return &pbs_list[current_pb];
  }
  else {
    current_pb = -1;
    return NULL;
  }
}

/**
 * return the next problem description
 */
corba_pb_desc_t * 
WfReader::pbNext() {
  if (hasPbNext()) {
    current_pb++;
    return &pbs_list[current_pb];
  }
  else
    return NULL;
}

/**
 * return the index of the provided problem in pbs_list vector
 */
unsigned int 
WfReader::indexOfPb(corba_pb_desc_t& pb) {
  unsigned int index = pbs_list.size() + 1;
  unsigned int len = pbs_list.size();
  for (unsigned int ix=0; ix < len; ix++) {
    if (pbs_list[ix] == pb) {
      index = ix;
      return index;
    }
      
  }
  return index;
}
