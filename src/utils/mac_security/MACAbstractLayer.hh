/**
 * @file  MACAbstractLayer.hh
 *
 * @brief  Class used to manage security contexts regardless the implementation of Mandatory Access Control
 *
 * @author  Lamiel Toch (lamiel.toch@ens-lyon.fr
 *
 * @section Licence
 *   |LICENSE|
 */

#include <string>

class MACAbstractLayer {
public:
	
	/**
	* Get the default context of a diet process
	* @return the default context
	*
	*/
	virtual std::string getDietDefaultContext() = 0;
	
    /**
    * Set the context of the current diet process to the default one
    * @return 0 (success) or non-zero (error)
    */
	virtual int changeToDietDefaultContext() = 0;
	
	/**
	* Change the context of the current diet process
	* @param ctx the targeted context
	* @return 0 (success) or non-zero (error)
	*/
	virtual int changeDietContext(const std::string& ctx) = 0;
	
	/***
	* Get the context of the current diet process
	* @return the context
	*/
	virtual std::string getDietCurrentContext() = 0;
	
	/***
	* Get the the directory where data are stored,
	* @param ctx the context associated to the data directory
	* @return the path of the data directory
	*
	*/
    virtual std::string getDietDataDir(const std::string& ctx) = 0;
};
