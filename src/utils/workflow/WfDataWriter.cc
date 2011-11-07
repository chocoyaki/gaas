/**
* @file  WfDataWriter.cc
* 
* @brief  Abstract interface and implementations of data writers 
* 
* @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#include "WfDataWriter.hh"
#include <iostream>
#include "WfPortAdapter.hh"  // for voidRef

/*****************************************************************************/
/*                          WfDataWriter class                               */
/*****************************************************************************/

WfDataWriter::WfDataWriter(std::ostream& output)
  : myOutput(output) {
}

WfDataWriter::~WfDataWriter() {
}

void
WfDataWriter::rawValue(const std::string& valueStr) {
  myOutput << valueStr;
}

void
WfDataWriter::rawValue(void * valuePtr,
                       WfCst::WfDataType valueType) {
  switch (valueType) {
  case WfCst::TYPE_DOUBLE:
    myOutput << *((double *) valuePtr);
    break;
  case WfCst::TYPE_INT:
    myOutput << *((int *) valuePtr);
    break;
  case WfCst::TYPE_LONGINT:
    myOutput << *((long *) valuePtr);
    break;
  case WfCst::TYPE_FLOAT:
    myOutput << *((float *) valuePtr);
    break;
  case WfCst::TYPE_SHORT:
    myOutput << *((short *) valuePtr);
    break;
  case WfCst::TYPE_CHAR:
  case WfCst::TYPE_PARAMSTRING:
  case WfCst::TYPE_STRING:
  case WfCst::TYPE_FILE:
    myOutput << (char *) valuePtr;
    break;
  case WfCst::TYPE_MATRIX:
  case WfCst::TYPE_CONTAINER:
  case WfCst::TYPE_UNKNOWN:
    myOutput << "#ERROR DATA RAW VALUE WRITER#";
  }
}

/*****************************************************************************/
/*                        WfXMLDataWriter class                              */
/*****************************************************************************/

WfXMLDataWriter::WfXMLDataWriter(std::ostream& output)
  : WfDataWriter(output) {}

void
WfXMLDataWriter::startContainer() {
  myOutput << "<array>\n";
}

void
WfXMLDataWriter::startContainer(const std::string& dataID) {
  myOutput << "<array dataId=\"" << dataID << "\">\n";
}

void
WfXMLDataWriter::endContainer() {
  myOutput << "</array>\n";
}

void
WfXMLDataWriter::itemValue(const std::string& valueStr) {
  // FIXME tag should not be added if no array was open previously
  myOutput << "<item>";
  rawValue(valueStr);
  myOutput << "</item>\n";
}

void
WfXMLDataWriter::itemValue(const std::string& valueStr,
                           const std::string& dataID) {
  myOutput << "<item dataId=\"" << dataID << "\">";
  rawValue(valueStr);
  myOutput << "</item>\n";
}

void
WfXMLDataWriter::itemValue(void * valuePtr, WfCst::WfDataType valueType) {
  myOutput << "<item>";
  rawValue(valuePtr, valueType);
  myOutput << "</item>\n";
}

void
WfXMLDataWriter::itemValue(void * valuePtr, WfCst::WfDataType valueType,
                           const std::string& dataID) {
  myOutput << "<item dataId=\"" << dataID << "\">";
  rawValue(valuePtr, valueType);
  myOutput << "</item>\n";
}

void
WfXMLDataWriter::matrix(void * valuePtr, WfCst::WfDataType valueType,
                        unsigned int nbRows, unsigned int nbColumns,
                        diet_matrix_order_t order) {
  // TODO
}

void
WfXMLDataWriter::voidElement() {
}

void
WfXMLDataWriter::error() {
}

/*****************************************************************************/
/*                        WfListDataWriter class                             */
/*****************************************************************************/


WfListDataWriter::WfListDataWriter(std::ostream& output)
  : WfDataWriter(output), firstElt(true) {
}

void
WfListDataWriter::startContainer() {
  if (!firstElt) {
    myOutput << ", ";
  }
  myOutput << "(";
  firstElt = true;
}

void
WfListDataWriter::endContainer() {
  myOutput << ")";
  firstElt = false;
}

void
WfListDataWriter::itemValue(const std::string& valueStr) {
  if (!firstElt) {
    myOutput << ", ";
  }
  rawValue(valueStr);
  firstElt = false;
}

void
WfListDataWriter::itemValue(void * valuePtr, WfCst::WfDataType valueType) {
  rawValue(valuePtr, valueType);
}

void
WfListDataWriter::matrix(void * valuePtr, WfCst::WfDataType valueType,
                         unsigned int nbRows, unsigned int nbColumns,
                         diet_matrix_order_t order) {
  // TODO
}

void
WfListDataWriter::voidElement() {
  myOutput << WfVoidAdapter::voidRef;
}

void
WfListDataWriter::error() {
}
