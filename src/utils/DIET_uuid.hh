#ifndef _DIET_UUID_HH_
#define _DIET_UUID_HH_

#include <boost/thread/mutex.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>


/**
 * @brief since boost::uuid generators are not thread-safe,
 * we wrote this wrapper
 * @return boost::uuids::uuid;
 */
boost::uuids::uuid
diet_generate_uuid();

#endif /* _DIET_UUID_HH_ */
