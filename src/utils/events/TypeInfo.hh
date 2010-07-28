#ifndef TYPEINFO_H
#define TYPEINFO_H

#include <typeinfo>

class TypeInfo
{
public:
	explicit TypeInfo(const std::type_info& info) : _typeInfo(info) {};

	bool operator < (const TypeInfo& rhs) const
	{
		return _typeInfo.before(rhs._typeInfo) != 0;
	}

private:
	const std::type_info& _typeInfo;
};

#endif