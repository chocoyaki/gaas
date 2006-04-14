/****************************************************************************/
/* Extended workflow description Reader                                     */
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

#include "marshalling.hh"
#include "WfExtReader.hh"
#include "WfUtils.hh"

using namespace std;

WfExtReader::WfExtReader(const char * content) :
  WfReader(content) {
  myDag = new Dag();
}

WfExtReader::~WfExtReader() {
  if (myDag) {
    delete (myDag);
  }
}

/**
 * Initialize the processing
 */
void 
WfExtReader::setup() {
  initXml();
}

/** 
 * Init the XML processing
 */
bool 
WfExtReader::initXml() {
  bool result = WfReader::initXml();
  cout << "Cheking the precedence ..." << endl;
  myDag->checkPrec();
  cout << myDag->toString() << endl;
  cout << "... done" << endl;  
  return result;
}

/**
 * get a reference to Dag structure
 */
Dag *
WfExtReader::getDag() {
  return myDag;
}

/**
 * parse a node element
 */
bool 
WfExtReader::parseNode (const QDomElement & element, 
			QString nodeId, QString nodePath) {
  diet_profile_t * profile;
  //  corba_pb_desc_t * corba_profile = new corba_pb_desc_t;
  vector <QDomElement> argVect;
  vector <QDomElement> inVect;
  vector <QDomElement> inoutVect;
  vector <QDomElement> outVect;
  vector <QDomElement> precVect;
  QDomElement node;
  QDomElement child = element.firstChildElement();
  while (!child.isNull()) {
    if (child.tagName() == "arg") {
      QString argName = child.attribute("name");
      QString argValue = child.attribute("value");
      if ((argName == "") || (argValue == "")) {
        cerr<<"Argument "<<child.tagName().toStdString()<<" malformed"<<endl;
        return false;
      }
      if (!child.firstChildElement().isNull()) {
	cerr << "Argument element does'nt accept a child element" << endl <<
	  "May be a </arg> is forgotten" << endl;
	return false;
      }

      argVect.push_back(child);
    }
    if (child.tagName() == "in") {
      if (!child.firstChildElement().isNull()) {
	cerr << "input port element does'nt accept a child element" << endl <<
	  "May be a </in> is forgotten" << endl;
	return false;
      }

      inVect.push_back(child);
    }
    if (child.tagName() == "inOut") {
      if (!child.firstChildElement().isNull()) {
	cerr << "inOut port element does'nt accept a child element" << endl <<
	  "May be a </inout> is forgotten" << endl;
	return false;
      }
      inoutVect.push_back(child);
    }
    if (child.tagName() == "out") {
      if (!child.firstChildElement().isNull()) {
	cerr << "output port element does'nt accept a child element" << endl <<
	  "May be a </out> is forgotten" << endl;
	return false;
      }

      outVect.push_back(child);
    } 
    if (child.tagName() == "prec") {
      QString prevNodeId = child.attribute("id");
      if (prevNodeId == "") {
        cerr<<"Precedence element malformed"<<endl;
        return false;
      }
      if (!child.firstChildElement().isNull()) {
	cerr << "Precedence element does'nt accept a child element" << endl <<
	  "May be a </prec> is forgotten" << endl;
	return false;
      }
      QString key = prevNodeId + QString("/") + nodeId;
      precVect.push_back(child);
    }
 
    child = child.nextSiblingElement();
  }

  // last_in last_inout last_out
  int last_in    = inVect.size() + argVect.size() - 1;
  int last_inout = last_in + inoutVect.size();
  int last_out   = last_inout + outVect.size();
  int lastArg    = 0;
  cout << "\t" << "last in " << last_in <<
    ", last_inout " << last_inout <<
    ", last_out " << last_out << endl;
  Node * dagNode = new Node(nodeId.toStdString(), nodePath.toStdString(),
			    last_in, last_inout, last_out);  

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
    checkArg (name, value, type, profile, lastArg, dagNode);
    lastArg++;
  }
  for (uint ix=0; ix < inVect.size(); ix++) {
    node = inVect[ix];
    QString name  = nodeId + "#" + node.attribute("name");
    QString type  = node.attribute("type");
    QString source  = node.attribute("source");
    // check in attribute and add its description to the profile
    checkIn (name, type, source, profile, lastArg, dagNode);
    lastArg++;
  }
  for (uint ix=0; ix < inoutVect.size(); ix++) {
    node = inoutVect[ix];
    QString name  = nodeId + "#" + node.attribute("name");
    QString type  = node.attribute("type");
    QString sink  = node.attribute("sink");
    // check the inout attribute and add its description to the profile
    checkInout(name, type, sink, profile, lastArg, dagNode);
    lastArg++;
  }
  for (uint ix=0; ix < outVect.size(); ix++) {
    node = outVect[ix];
    QString name  = nodeId + "#" + node.attribute("name");
    QString type  = node.attribute("type");
    QString sink  = node.attribute("sink"); 
    // check the out attribute and add its description to the profile
    checkOut (name, type, sink, profile, lastArg, dagNode);
    lastArg++;
  }

  cout << "\t" << "---> Adding the precedence ..." << endl;
  for (uint ix=0; ix< precVect.size(); ix++) {
    node = precVect[ix];
    QString prev_id  = node.attribute("id");
    dagNode ->addPrecId(prev_id.toStdString());
  }
  cout << "\t" << "... done" << endl;

  cout << "\t" << "---> Set the profile to dagNode ..." << endl;
  dagNode->set_pb_desc(profile);
  cout << "\t" << "... done" << endl;


  cout << "\t" << "---> Add a node to the Dag " << endl;
  myDag->addNode(nodeId.toStdString(), dagNode);
  cout << "\t" << "... done" << endl;

  return true;
}

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
WfExtReader::checkArg(const QString& name, 
		   const QString& value, 
		   const QString& type,
		   diet_profile_t * profile, 
		   unsigned int lastArg,
		   Node * dagNode) {
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
WfExtReader::checkIn(const QString& name, 
		  const QString& type, 
		  const QString& source,
		  diet_profile_t * profile,
		  unsigned int lastArg,
		  Node * dagNode) {
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
WfExtReader::checkInout(const QString& name, 
		     const QString& type, 
		     const QString& source,
		     diet_profile_t * profile,
		     unsigned int lastArg,
		     Node * dagNode) {
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
WfExtReader::checkOut(const QString& name, 
		   const QString& type,
		   const QString& sink,
		   diet_profile_t * profile,
		   unsigned int lastArg,
		   Node * dagNode) {
  setParam(OUT_PORT, name, type, profile, lastArg, dagNode);
  if (dagNode)
    dagNode->link_o2i(name.toStdString(), sink.toStdString());
  return true;
}

/**
 * fill a profile with the appropriate parameter type and create the 
 * node ports
 */  
bool
WfExtReader::setParam(const wf_port_t param_type,
		      const QString& name, 
		      const QString& type,
		      diet_profile_t* profile,
		      unsigned int lastArg,
		      Node * dagNode,
		      const QString * value) {
  bool result = true;
  cout << "\t" << "the requested type is " << type.toStdString() << endl;
  if (type == WfCst::DIET_CHAR) {
    if (dagNode) {
      if (value) 
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_CHAR),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_CHAR));
    }
  } // end if DIET_CHAR

  if (type == WfCst::DIET_SHORT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_SHORT),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_SHORT));
    }
  } // end if DIET_SHORT
  if (type == WfCst::DIET_INT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_INT),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_INT));
    }
  } // end if DIET_INT
  if (type == WfCst::DIET_LONGINT) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_LONGINT),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_LONGINT));
    }

  } // end if LONG_INT

  if (type == WfCst::DIET_FLOAT) {
    if (dagNode) {
      if (value) 
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_FLOAT),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_FLOAT));
    }
  } // end if DIET_CHAR


  if (type == WfCst::DIET_STRING) {
    if (dagNode) {
      if (value)
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_STRING),
			 value->toStdString());
      else
	dagNode->newPort(name.toStdString(), lastArg, param_type, 
			 string(WfCst::DIET_STRING));
    }	
  } // end if DIET_STRING
  return result;
}

