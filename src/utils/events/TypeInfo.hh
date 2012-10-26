/**
 * @file TypeInfo.hh
 *
 * @brief   Class TypeInf
 *
 * @author Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef TYPEINFO_H
#define TYPEINFO_H

#include <typeinfo>

class TypeInfo {
public:
  explicit
  TypeInfo(const std::type_info &info): _typeInfo(info) {
  }

  bool
      operator < (const TypeInfo &rhs) const {
    return (_typeInfo.before(rhs._typeInfo) != 0);
  }

private:
  const std::type_info &_typeInfo;
};

#endif /* ifndef TYPEINFO_H */
