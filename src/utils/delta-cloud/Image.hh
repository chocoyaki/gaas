/**
 * @file Image.hh
 *
 * @brief  DIET Image class header
 *
 * @author  Lamiel TOCH (lamiel.toch@ens-lyon.fr)
 *          Yulin ZHANG (huaxi.zhang@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _IMAGE_HH_
#define _IMAGE_HH_

#include <vector>
#include <string>

namespace IaaS {


/****************************************************************************/
/* Image class                                                                */
/****************************************************************************/
class Image {
  public:

	/*Name of an image*/
    const std::string name;

    /*Identity of an image. It should be unique for each image.*/
    const std::string id;

    /* Contructor with a given image name and id */
    Image(const std::string & _name, const std::string & _id) : name(_name), id(_id) {
    };
};

}

#endif // _IMAGE_HH_
