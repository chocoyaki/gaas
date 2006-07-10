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


#include "WfUtils.hh"
#include "WfReader.hh"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/util/XMLString.hpp>

#include "marshalling.hh"
#include "debug.hh"

using namespace std;


string
WfReader::getAttributeValue(const char * attr_name, const DOMElement * elt) {
  XMLCh * attr = XMLString::transcode(attr_name);
  const XMLCh * value   = elt->getAttribute(attr);
  char  * value_str = XMLString::transcode(value);

  string result(value_str);

  XMLString::release(&value_str);
  XMLString::release(&attr);

  return result;
}

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
  const XMLCh gLS[] = { chLatin_L, chLatin_S, chNull };

  DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(gLS);


  DOMBuilder  *parser = 
    ((DOMImplementationLS*)impl)->createDOMBuilder(DOMImplementationLS::MODE_SYNCHRONOUS, 0);


  const char * content_id = "workflow_description";

  cout << content << endl;

  MemBufInputSource* memBufIS = new MemBufInputSource
    (
     (const XMLByte*)(this->content.c_str())
     , content.size()
     , content_id
     , false
    );

  Wrapper4InputSource * wrapper = new Wrapper4InputSource(memBufIS);

  cout << "wrapper created" << endl;

  this->document = parser->parse(*wrapper);

  if (document == NULL) {
    cout << "FATL ERROR : wrong description ?" << endl;
    return false;
  }

  DOMNode * root = (DOMNode*)(document->getDocumentElement());

  char * _rootNodeName = XMLString::transcode(root->getNodeName());
  if (strcmp (_rootNodeName, "dag")) {
    cout << "the file is not a valid dag description " << endl;
    return false;
  }
  XMLString::release(&_rootNodeName);

  //  TRACE_TEXT(TRACE_MAIN_STEPS, "root tag name : " << root.tagName().toStdString () << endl);

  DOMNode * child = root->getFirstChild();
  while ((child != NULL)) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;

      char * _nodeName = XMLString::transcode(child_elt->getNodeName());
      string nodeName(_nodeName);
      cout << "\tprocessing '" << nodeName  << "' element"<< endl;
      XMLString::release(&_nodeName);

      if (nodeName != "node") {
	cout << "Founding an element different from node" << endl;
	return false;
      }

      // Node path
      string nodePath(getAttributeValue("path", child_elt));
      // Node ID
      string nodeId(getAttributeValue("id", child_elt));

      //        debug("node name : ", nodeName.toStdString().c_str());
      //        Node * node = new Node(nodeName.toStdString());

      if ((nodePath == "") ||
	  (nodeId   == "")) {
	cout << "FATAL ERROR : Node without Path or Id" << endl;
	return false;
      }
      else {
	cout << "nodeId = " << nodeId << endl;
	cout << "nodePath = " << nodePath << endl;
      }

      if (nodeId[0] == '$') {	
	nodeId.erase(0,1);
	// this is parametrable node
	string nodeInterval(getAttributeValue("interval", child_elt));
	long interval = atoi(nodeInterval.c_str());
	string base_id(nodeId);
	for (long lx=0; lx < interval; lx ++) {
	  nodeId = base_id + itoa(lx).c_str();
	  if (!parseNode(child, nodeId, nodePath, lx)) {
	    return false;
	  }
	} // end for lx
      } // end if == $
      else if (!parseNode(child, nodeId, nodePath)) {
	return false;
      }
    }
    child = child->getNextSibling();
  } // end while
  //  TRACE_TEXT(TRACE_MAIN_STEPS, "parsing xml content successful " <<endl);
  return true;
}

/**
 * parse a node element
 */
bool 
WfReader::parseNode (const DOMNode * element, 
		     string nodeId, string nodePath,
		     long var) {
  diet_profile_t        * profile;
  corba_pb_desc_t       * corba_profile = new corba_pb_desc_t;
  vector <DOMElement*> argVect;
  vector <DOMElement*> inVect;
  vector <DOMElement*> inoutVect;
  vector <DOMElement*> outVect;
  vector <DOMElement*> precVect;
  DOMElement*          node;
  long int mul_arg =0, mul_in = 0, mul_inout = 0, mul_out = 0; 
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
  DOMNode * child = element->getFirstChild();
  while (child != NULL) {
    if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
      DOMElement * child_elt = (DOMElement*)child;

      char *child_name_str = XMLString::transcode(child_elt->getNodeName());
      string child_name(child_name_str);
      XMLString::release(&child_name_str);

      if (child_name == "arg") {
	string argName = getAttributeValue("name", child_elt);
	string argValue = getAttributeValue("value", child_elt);
	if ((argName == "") || (argValue == "")) {
	  ERROR ("Argument "<< child_name <<" malformed", false);
	  return false;
	}
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "Argument element doesn't accept a child element" << endl <<
	    "May be a </arg> is forgotten" << endl;
	  return false;
	}

	argVect.push_back(child_elt);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_arg += atoi(mul_attr.c_str());
	}
      } // end if (arg

      if (child_name == "in") {
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "input port element doesn't accept a child element" << endl <<
	    "May be a </in> is forgotten" << endl;
	  return false;
	}
	inVect.push_back(child_elt);

	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_in += atoi(mul_attr.c_str());
	}
      } // end if (in

      if (child_name == "inOut") {
	if (child_elt->getFirstChild()!=NULL) {
	  cerr << "inOut port element doesn't accept a child element" << endl <<
	    "May be a </inOut> is forgotten" << endl;
	  return false;
	}

	inoutVect.push_back(child_elt);
	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_inout += atoi(mul_attr.c_str());
	}
      } // end if (inout)

      if (child_name == "out") {
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "output port element doesn't accept a child element" << endl <<
	    "May be a </out> is forgotten" << endl;
	  return false;
	}
	
	outVect.push_back(child_elt);
	
	string mul_attr = getAttributeValue("mul", child_elt);
	if (mul_attr != "") {
	  mul_out += atoi(mul_attr.c_str());
	} 
      } // end if (out
      
      if (child_name == "prec") {
	string prevNodeId = getAttributeValue("id", child_elt);
	if (prevNodeId == "") {
	  cout  << "Precedence element malformed" << endl;
	  return false;
	}
	
	if (child_elt->getFirstChild() != NULL) {
	  cerr << "Precedence element doesn't accept a child element" << endl <<
	    "May be a </prec> is forgotten" << endl;
	  return false;
	}
	string key = prevNodeId + string("/") + nodeId;
	
	precVect.push_back(child_elt);
	// Is the previous node useful ?
      } // end if (prec      
    } // end if (ELEMENT_NODE
    child = child->getNextSibling();
  } // end while

  // adjust mul_xxx
  if (mul_arg == 0)
    mul_arg = 1;
  if (mul_in == 0)
    mul_in = 1;
  if (mul_inout == 0)  
    mul_inout = 1;
  if (mul_out == 0)  
    mul_out = 1;
  // last_in last_inout last_out
  int last_in    = inVect.size() + argVect.size() - 1 + 
    (mul_arg - 1) + (mul_in - 1);
  int last_inout = last_in + inoutVect.size() +
    (mul_inout - 1);
  int last_out   = last_inout + outVect.size() +
    (mul_out - 1);

  int lastArg    = 0;
  profile = diet_profile_alloc((char*)nodePath.c_str(), 
				last_in,
				last_inout,
				last_out);
  
  for (uint ix=0; ix < argVect.size(); ix++) {
    node = argVect[ix];
    string name = nodeId + "#";
    /*
    string name  = nodeId + "#" + node.attribute("name");
    if (var != -1) {
      name.append(itoa(var).c_str());
    }
    */

    string value = getAttributeValue("value", node);
    string type  = getAttributeValue("type", node);

    if (type != "DIET_MATRIX")
      // check the arg attribute and add its description to the profile
      checkArg (name, value, type, profile, lastArg, var);
    else
      // check matrix argument
      checkMatrixArg(name, node, profile, lastArg, var);
    lastArg++;
  }

  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];
    string name = nodeId + "#";
    string type  = getAttributeValue("type", node);
    string source  = getAttributeValue("source", node);
    string abs_name = getAttributeValue("name", node);

    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int lx=0; lx < mul; lx++) {
	string real_port_name = abs_name + itoa(lx);
	name.append(real_port_name.c_str());
	if (type != "DIET_MATRIX")
	  // check in attribute and add its description to the profile
	  checkIn (name, type, source, profile, lastArg, var);
	else
	  // check a matrix input port
	  checkMatrixIn(name, node, profile, lastArg, var);
	lastArg++;
      }
    }
    else {
      // single port
      name = name + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check in attribute and add its description to the profile
	checkIn (name, type, source, profile, lastArg, var);
      else
	// check a matrix input port
	checkMatrixIn(name, node, profile, lastArg, var);
      lastArg++;
    }
  }

  // Inout parameters
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];
    string name = nodeId + "#";

    string type  = getAttributeValue("type", node);
    string source  = getAttributeValue("source", node);
    if (type != "DIET_MATRIX")
      // check the inout attribute and add its description to the profile
      checkInout(name, type, source, profile, lastArg, var);
    else
      // check inout port
      checkMatrixInout(name, node, profile, lastArg, var);
    lastArg++;
  }

  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];
    string name = nodeId + "#";

    string type  = getAttributeValue("type", node);
    string sink  = getAttributeValue("sink", node);

    string abs_name = getAttributeValue("name", node);
    if (abs_name[0] == '$') {
      // multiple port
      abs_name = abs_name.erase(0,1);
      string mul_str = getAttributeValue("mul", node);
      long int mul = atoi(mul_str.c_str());
      for (int lx=0; lx < mul; lx++) {
	string real_port_name = abs_name + itoa(lx);
	name = name + real_port_name.c_str();
	if (type != "DIET_MATRIX")
	  // check the inout attribute and add its description to the profile
	  checkInout(name, type, sink, profile, lastArg, var);
	else 
	  // check matrix inout port
	  checkMatrixInout(name, node, profile, lastArg, var);

	lastArg++;
      }
    }
    else {
      // single port
      name = name + getAttributeValue("name", node);
      if (type != "DIET_MATRIX")
	// check the inout attribute and add its description to the profile
	checkInout(name, type, sink, profile, lastArg, var);
      else 
	// check matrix inout port
	checkMatrixInout(name, node, profile, lastArg, var);
      
      lastArg++;
    }
  }

  mrsh_pb_desc(corba_profile, profile);
  nodes_list[nodeId] = (*corba_profile);

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
WfReader::checkArg(const string& name, 
		   const string& value, 
		   const string& type,
		   diet_profile_t * profile, 
		   unsigned int lastArg,
		   long var,
		   BasicNode * dagNode) {
  setParam(ARG_PORT, name, type, profile, lastArg, var, dagNode, &value);
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
WfReader::checkIn(const string& name, 
		  const string& type, 
		  const string& source,
		  diet_profile_t * profile,
		  unsigned int lastArg,
		  long var,
		  BasicNode * dagNode) {
  setParam(IN_PORT, name, type, profile, lastArg, var, dagNode);
  if (dagNode)
    dagNode->link_i2o(name, source);
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
WfReader::checkInout(const string& name, 
		     const string& type, 
		     const string& source,
		     diet_profile_t * profile,
		     unsigned int lastArg,
		     long var,
		     BasicNode * dagNode) {
  setParam(INOUT_PORT, name, type, profile, lastArg, var, dagNode);
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
WfReader::checkOut(const string& name, 
		   const string& type,
		   const string& sink,
		   diet_profile_t * profile,
		   unsigned int lastArg,
		   long var,
		   BasicNode * dagNode) {
  setParam(OUT_PORT, name, type, profile, lastArg, var, dagNode);
  if (dagNode)
    dagNode->link_o2i(name, sink);
  return true;
}
/**
 * fill a profile with the appropriate parameter type
 */  
bool 
WfReader::setParam(const wf_port_t param_type,
		   const string& name,
		   const string& type,
		   diet_profile_t * profile,
		   unsigned int lastArg,
		   long int var,
		   BasicNode * dagNode,
		   const string * value) {
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

/**
 * parse a matrix argument
 */
bool
WfReader::checkMatrixArg(const string& id, const DOMElement * element,
			 diet_profile_t * profile, unsigned int lastArg,
			 long var,
			 BasicNode * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element,
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(ARG_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg);
  return true;
}

/**
 * parse a matrix input port
 */
bool
WfReader::checkMatrixIn(const string& id, const DOMElement * element,
			diet_profile_t * profile, unsigned int lastArg,
			long var,
			BasicNode * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element, 
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(IN_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg);
  return true;
}

/**
 * parse a matrix inout port
 */
bool
WfReader::checkMatrixInout(const string& id, const DOMElement * element,
			   diet_profile_t * profile, unsigned int lastArg,
			   long var,
			   BasicNode * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element, 
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;

  setMatrixParam(INOUT_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg);
  return true;
}

/**
 * parse a matrix output port
  */
bool
WfReader::checkMatrixOut(const string& id, const DOMElement * element,
			 diet_profile_t * profile, unsigned int lastArg,
			 long var,
			 BasicNode * dagNode) {
  string base_type("");
  string nb_rows("");
  string nb_cols("");
  string matrix_order("");

  if (!checkMatrixCommun(id, element,
			 base_type, nb_rows, nb_cols, matrix_order))
    return false;
  setMatrixParam(OUT_PORT, id, base_type, nb_rows, nb_cols, matrix_order,
		 profile, lastArg);
  return true;
}

/**
 * parse a matrix argument. 
 * Check only the commun attributes of In, Inout and Out ports
 */
bool
WfReader::checkMatrixCommun(const string& id, const DOMElement * element,
			    string& base_type, 
			    string& nb_rows, 
			    string& nb_cols, 
			    string& matrix_order) {
  // mandatory attributes
  base_type  = getAttributeValue("base_type", element);
  nb_rows  = getAttributeValue("nb_rows", element);
  nb_cols  = getAttributeValue("nb_cols", element);
  matrix_order  = getAttributeValue("matrix_order", element);
  if ((base_type == "") || (nb_rows == "")  ||
      (nb_cols == "") || (matrix_order == ""))
    return false;
  return true;
}

/**
 * fill a profile with matrix parameter type
 * The data are NULL
 */  
bool 
WfReader::setMatrixParam(const wf_port_t param_type,
			 const string& name,
			 const string& base_type_str,
			 const string& nb_rows_str,
			 const string& nb_cols_str,
			 const string& matrix_order_str,
			 diet_profile_t * profile,
			 unsigned int lastArg,
			 BasicNode * dagNode,
			 const string * value) {
  // get the base type of matrix element
  diet_base_type_t base_type = getBaseType(base_type_str);
  // get the number of rows
  size_t nb_rows = atoi(nb_rows_str.c_str());
  // get the column numbers
  size_t nb_cols = atoi(nb_cols_str.c_str());
  // get the matrix order  
  diet_matrix_order_t matrix_order = 
    getMatrixOrder(matrix_order_str);

  // set the parameter as a matrix
  diet_matrix_set(diet_parameter(profile,lastArg),
		  NULL, DIET_VOLATILE, 
		  base_type, nb_rows, nb_cols, matrix_order);


  return true;
}
