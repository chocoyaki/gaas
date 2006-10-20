/****************************************************************************/
/* The port classes used in workflow node                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id : 
 * $Log : 
 ****************************************************************************/

#include "WfPort.hh"
#include "Node.hh"

/**
 * Input port interface destructor *
 */
AbstractInP::~AbstractInP() {
}


/**
 * Output port interface destructor *
 */
AbstractOutP::~AbstractOutP() {
}

/**
 * Basic Port constructor *
 */
WfPort::WfPort(Node * parent, string _id, string _type, uint _ind, 
	       const string& v) :
  myParent(parent), id(_id),type(_type), index(_ind),value(v) {
  this->nb_r = 0;
  this->nb_c = 0;
} // end WfPort constructor

/**
 * Set the parameter of a matrix argument
 */
void
WfPort::setMatParams(long nbr, long nbc, 
		    diet_matrix_order_t o,
		    diet_base_type_t bt) {
  this->nb_r = nbr;
  this->nb_c = nbc;
  this->order = o;
  this->base_type = bt;
} // end WfPort::setMatParams

/**
 * Return the profile of the node
 */
diet_profile_t * 
WfPort::profile() {
  return myParent->getProfile();
} // end WfPort::profile

/**
 * return an XML  representation of the port *
 */
string 
WfPort::toXML() {
  string xml = "";
  xml += "id=\"" + this->id + "\" type=\"" + this->type +"\" ";
  if (this->type == "DIET_MATRIX") {
    /*
      base_type  = element.attribute("base_type");
      nb_rows  = element.attribute("nb_rows");
      nb_cols  = element.attribute("nb_cols");
      matrix_order  = element.attribute("matrix_order");
    */
    xml += "base_type=\""+getBaseTypeStr(this->base_type)+"\" ";
    xml += "nb_rows=\"" + itoa(this->nb_r)+"\" ";
    xml += "nb_cols=\"" + itoa(this->nb_c)+"\" ";
    xml += "matrix_order=\"" + getMatrixOrderStr(this->order) + "\" ";
  }
  return xml;
} // end WfPort::toXml


/**
 * Return the port id
 */
string
WfPort::getId() {
  return this->id;
} // end WfPort::getId

/**
 * Return the port index (parameter index in diet profile)
 */
unsigned int
WfPort::getIndex() {
  return this->index;
} // end WfPort::getIndex

/**
 * Output port constructor *
 */
WfOutPort::WfOutPort(Node * parent, string _id, string _type, uint _ind,
		     const string& v) :
  WfPort(parent, _id, _type, _ind, v) {
  sink_port = NULL;
} // end  WfOutPort constructor

/**
 * Set the sink of the output port
 */
void 
WfOutPort::set_sink(AbstractInP * _sink_port) {
  this->sink_port = _sink_port;
} // end set_sink

/**
 * Set the sink id of the output port
 */
void 
WfOutPort::set_sink(const string& s) {
  this->sink_port_id = s;
} // end set_sink

/**
 * Return the sink port id
 */
string
WfOutPort::getSinkId() {
  return this->sink_port_id;
} // end WfOutPort::getSink

/**
 * Return the sink port reference
 */
AbstractInP *
WfOutPort::getSink() {
  return this->sink_port;
} // end WfOutPort::getSink

/**
 * return if the output port is a final output port (workflow result) *
 */
bool
WfOutPort::isResult() {
  return (sink_port == NULL);
} // end WfOutPort::isResult

/**
 * return an XML  representation of the output port *
 */
string 
WfOutPort::toXML() {
  string xml = "\t<out ";
  xml += WfPort::toXML();
  if (this->sink_port != NULL)
    xml += "sink=\""+sink_port_id+"\"";
  
  xml += " />\n";
  return xml;
} // end WfOutPort::toXML

/**
 * Input port constructor *
 */
WfInPort::WfInPort(Node * parent, string _id, string _type, uint _ind,
		   const string& v) :
  WfPort(parent, _id, _type, _ind, v) {
  source_port = NULL;
  this->source_port_id = "";
} // end WfInPort constructor

/**
 * Set the source of the input port
 */
void
WfInPort::set_source(AbstractOutP * _src) {
  this->source_port = _src;
} // end WfInPort::set_source
  
/**
 * Set the source of the input port
 */
void
WfInPort::set_source(const string& s) {
  this->source_port_id = s;
} // end WfInPort::set_source

/**
 * Return the source id of the input port
 */
string
WfInPort::getSourceId() {
  return this->source_port_id;
} // end WfInPort::getSourceId
    
/**
 * Return the source port reference of the input port
 */
AbstractOutP *
WfInPort::getSource() {
  return this->source_port;
} // end WfInPort::getSource

/**
 * Return if the input port is an input of the DAG
 */
bool
WfInPort::isInput() {
  return (source_port == NULL);
} // end WfInPort::isInput

/**
 * return an XML  representation of the input port *
 * if b == false (value by default) the source port is not included. used to
 * create the remaining DAG representation
 * if b == true the complete representation is returned
 */
string 
WfInPort::toXML(bool b) {
  string xml = "";
  if (value != "")
    xml = "\t<arg ";
  else
    xml ="\t<in ";
  xml += WfPort::toXML();
  if (value != "")
    xml += "value=\"" + this->value + "\" "; 
  if ((b) && (this->source_port != NULL))
    xml += "source=\""+source_port_id+"\"";
  
  xml += " />\n";
  return xml;
}

/**
 * Input/Output port constructor *
 */
WfInOutPort::WfInOutPort(Node * parent, string _id, string _type, uint _ind,
			 const string& v) :
  WfPort(parent, _id, _type, _ind, v) {
} // end WfInOutPort constructor

void
WfInOutPort::set_source(AbstractOutP * _src) {
  this->source_port = _src;
}
 
void
WfInOutPort::set_source(const string& s) {
  this->source_port_id = s;
}
 
void
WfInOutPort::set_sink(AbstractInP * _sink) {
  this->sink_port = _sink;
}
 
void
WfInOutPort::set_sink(const string& s) {
  this->sink_port_id = s;
}

