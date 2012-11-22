#ifndef _IMAGE_HH_
#define _IMAGE_HH_

namespace IaaS {

#include <vector>
#include <string>

class Image {
  public:
    const std::string name;
    const std::string id;

    /* ctor */
    Image(const std::string & _name, const std::string & _id) : name(_name), id(_id) {
    };
};

}

#endif // _IMAGE_HH_
