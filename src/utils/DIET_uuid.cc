#include "DIET_uuid.hh"


boost::uuids::uuid
diet_generate_uuid() {
  static boost::uuids::random_generator uuid_rg;
  static boost::mutex mutex;

  boost::uuids::uuid uuid;
  {
    boost::mutex::scoped_lock lock(mutex);
    uuid = uuid_rg();
  }

  return uuid;
}
