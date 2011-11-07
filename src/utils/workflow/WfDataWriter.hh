/**
* @file  WfDataWriter.hh
* 
* @brief  Abstract interface and implementations of data writers 
* 
* @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _WFDATAWRITER_HH_
#define _WFDATAWRITER_HH_

#include <string>
#include "WfUtils.hh"

/*****************************************************************************/
/*                    WfDataWriter class (ABSTRACT)                          */
/*****************************************************************************/

class WfDataWriter {
public:
  explicit WfDataWriter(std::ostream& output);

  virtual ~WfDataWriter();

  virtual void
  startContainer() = 0;

  virtual void
  endContainer() = 0;

  virtual void
  itemValue(const std::string& valueStr) = 0;

  virtual void
  itemValue(void * valuePtr,
            WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN) = 0;

  virtual void
  matrix(void * valuePtr, WfCst::WfDataType valueType,
         unsigned int nbRows, unsigned int nbColumns,
         diet_matrix_order_t order) = 0;

  virtual void
  voidElement() = 0;

  virtual void
  error() = 0;

protected:
  void
  rawValue(const std::string& valueStr);

  void
  rawValue(void * valuePtr, WfCst::WfDataType valueType);

  std::ostream& myOutput;
};

class WfXMLDataWriter : public WfDataWriter  {
public:
  explicit WfXMLDataWriter(std::ostream& output);

  virtual void
  startContainer();

  virtual void
  endContainer();

  virtual void
  itemValue(const std::string& valueStr);

  virtual void
  itemValue(void * valuePtr, WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN);

  virtual void
  matrix(void * valuePtr, WfCst::WfDataType valueType,
         unsigned int nbRows, unsigned int nbColumns,
         diet_matrix_order_t order);

  virtual void
  voidElement();

  virtual void
  error();

  // additional methods to include dataId attribute in tags
  void
  startContainer(const std::string& dataID);

  void
  itemValue(const std::string& valueStr, const std::string& dataID);

  void
  itemValue(void * valuePtr, WfCst::WfDataType valueType,
            const std::string& dataID);
};

class WfListDataWriter : public WfDataWriter  {
public:
  explicit WfListDataWriter(std::ostream& output);

  virtual void
  startContainer();

  virtual void
  endContainer();

  virtual void
  itemValue(const std::string& valueStr);

  virtual void
  itemValue(void * valuePtr, WfCst::WfDataType valueType = WfCst::TYPE_UNKNOWN);

  virtual void
  matrix(void * valuePtr, WfCst::WfDataType valueType,
         unsigned int nbRows, unsigned int nbColumns,
         diet_matrix_order_t order);

  virtual void
  voidElement();

  virtual void
  error();

protected:
  bool  firstElt;
};

#endif  // _WFDATAWRITER_HH_
