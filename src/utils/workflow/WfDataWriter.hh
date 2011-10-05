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
 * Revision 1.2  2009/08/26 10:32:11  bisnard
 * corrected  warnings
 *
 * Revision 1.1  2009/06/15 12:01:20  bisnard
 * new class WfDataWriter to abstract the different ways of
 * displaying data within a workflow (XML or list)
 *
 */

#ifndef _WFDATAWRITER_HH_
#define _WFDATAWRITER_HH_

#include "WfUtils.hh"


/*****************************************************************************/
/*                    WfDataWriter class (ABSTRACT)                          */
/*****************************************************************************/

class WfDataWriter {

public:

  WfDataWriter(std::ostream& output);

  virtual ~WfDataWriter();

  virtual void  startContainer() = 0;
  virtual void  endContainer() = 0;
  virtual void  itemValue(const std::string& valueStr) = 0;
  virtual void  itemValue(void * valuePtr,
                          WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN) = 0;
  virtual void  matrix(void * valuePtr,
                       WfCst::WfDataType valueType,
                       unsigned int nbRows,
                       unsigned int nbColumns,
                       diet_matrix_order_t order) = 0;
  virtual void  voidElement() = 0;
  virtual void  error() = 0;

protected:
  void  rawValue(const std::string& valueStr);
  void  rawValue(void * valuePtr,
                 WfCst::WfDataType valueType);

  std::ostream& myOutput;

};

class WfXMLDataWriter : public WfDataWriter  {

public:

  WfXMLDataWriter(std::ostream& output);

  virtual void  startContainer();
  virtual void  endContainer();
  virtual void  itemValue(const std::string& valueStr);
  virtual void  itemValue(void * valuePtr,
                          WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN);
  virtual void  matrix(void * valuePtr,
                       WfCst::WfDataType valueType,
                       unsigned int nbRows,
                       unsigned int nbColumns,
                       diet_matrix_order_t order);
  virtual void  voidElement();
  virtual void  error();

  // additional methods to include dataId attribute in tags
  void startContainer( const std::string& dataID );
  void itemValue( const std::string& valueStr,
                  const std::string& dataID);
  void itemValue( void * valuePtr,
                  WfCst::WfDataType valueType,
                  const std::string& dataID );
};

class WfListDataWriter : public WfDataWriter  {

public:
  WfListDataWriter(std::ostream& output);

  virtual void  startContainer();
  virtual void  endContainer();
  virtual void  itemValue(const std::string& valueStr);
  virtual void  itemValue(void * valuePtr,
                          WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN);
  virtual void  matrix(void * valuePtr,
                       WfCst::WfDataType valueType,
                       unsigned int nbRows,
                       unsigned int nbColumns,
                       diet_matrix_order_t order);
  virtual void  voidElement();
  virtual void  error();

protected:
  bool  firstElt;
};


#endif // _WFDATAWRITER_HH_
