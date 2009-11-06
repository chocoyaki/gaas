/****************************************************************************/
/* Abstract interface and implementations of data writers                   */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2009/11/06 13:06:37  bisnard
 * replaced 'list' tag by 'array' tag in data files
 *
 * Revision 1.3  2009/08/26 10:32:11  bisnard
 * corrected  warnings
 *
 * Revision 1.2  2009/07/01 15:05:25  bisnard
 * bug correction (display pointer instead of value)
 *
 * Revision 1.1  2009/06/15 12:01:20  bisnard
 * new class WfDataWriter to abstract the different ways of
 * displaying data within a workflow (XML or list)
 *
 */


#include "WfDataWriter.hh"
#include "WfPortAdapter.hh" // for voidRef
#include <iostream>

using namespace std;

/*****************************************************************************/
/*                          WfDataWriter class                               */
/*****************************************************************************/

WfDataWriter::WfDataWriter(std::ostream& output)
  : myOutput(output) {}

WfDataWriter::~WfDataWriter() {}

void
WfDataWriter::rawValue(const std::string& valueStr) {
  myOutput << valueStr;
}

void
WfDataWriter::rawValue(void * valuePtr,
                       WfCst::WfDataType valueType) {
  switch(valueType) {

    case WfCst::TYPE_DOUBLE:
      myOutput << *((double *) valuePtr); break;
    case WfCst::TYPE_INT:
      myOutput << *((int *) valuePtr); break;
    case WfCst::TYPE_LONGINT:
      myOutput << *((long *) valuePtr); break;
    case WfCst::TYPE_FLOAT:
      myOutput << *((float *) valuePtr); break;
    case WfCst::TYPE_SHORT:
      myOutput << *((short *) valuePtr); break;

    case WfCst::TYPE_CHAR:
    case WfCst::TYPE_PARAMSTRING:
    case WfCst::TYPE_STRING:
    case WfCst::TYPE_FILE:
      myOutput << (char *) valuePtr; break;

    case WfCst::TYPE_MATRIX:
    case WfCst::TYPE_CONTAINER:
    case WfCst::TYPE_UNKNOWN:
      myOutput << "#ERROR DATA RAW VALUE WRITER#";
  }
}

/*****************************************************************************/
/*                        WfXMLDataWriter class                              */
/*****************************************************************************/

WfXMLDataWriter::WfXMLDataWriter(ostream& output)
  : WfDataWriter(output) {}

void
WfXMLDataWriter::startContainer() {
  myOutput << "<array>" << endl;
}

void
WfXMLDataWriter::startContainer(const string& dataID) {
  myOutput << "<array dataId=\"" << dataID << "\">" << endl;
}

void
WfXMLDataWriter::endContainer() {
  myOutput << "</array>" << endl;
}

void
WfXMLDataWriter::itemValue(const string& valueStr) {
  // FIXME tag should not be added if no array was open previously
  myOutput << "<item>";
  rawValue(valueStr);
  myOutput << "</item>" << endl;
}

void
WfXMLDataWriter::itemValue(const string& valueStr,
			   const string& dataID) {
  myOutput << "<item dataId=\"" << dataID << "\">";
  rawValue(valueStr);
  myOutput << "</item>" << endl;
}

void
WfXMLDataWriter::itemValue(void * valuePtr,
                           WfCst::WfDataType valueType) {
  myOutput << "<item>";
  rawValue(valuePtr, valueType);
  myOutput << "</item>" << endl;
}

void
WfXMLDataWriter::itemValue(void * valuePtr,
                           WfCst::WfDataType valueType,
			   const string& dataID) {
  myOutput << "<item dataId=\"" << dataID << "\">";
  rawValue(valuePtr, valueType);
  myOutput << "</item>" << endl;
}

void
WfXMLDataWriter::matrix(void * valuePtr,
                        WfCst::WfDataType valueType,
                        unsigned int nbRows,
                        unsigned int nbColumns,
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


WfListDataWriter::WfListDataWriter(ostream& output)
  : WfDataWriter(output), firstElt(true) {}

void
WfListDataWriter::startContainer() {
  if (!firstElt)
    myOutput << ",";
  myOutput << "(";
  firstElt = true;
}

void
WfListDataWriter::endContainer() {
  myOutput << ")";
  firstElt = false;
}

void
WfListDataWriter::itemValue(const string& valueStr) {
  if (!firstElt)
    myOutput << ",";
  rawValue(valueStr);
  firstElt = false;
}

void
WfListDataWriter::itemValue(void * valuePtr,
                           WfCst::WfDataType valueType) {
  rawValue(valuePtr, valueType);
}

void
WfListDataWriter::matrix(void * valuePtr,
                        WfCst::WfDataType valueType,
                        unsigned int nbRows,
                        unsigned int nbColumns,
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
