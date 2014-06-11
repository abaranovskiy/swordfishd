/**
 * @file storage_controller.hpp
 * @author Alexander Baranovsky
 */
//-------------------------------------------------------------------------------------------------
#ifndef __WAPSTART_SWORDFISH_STORAGE_CONTROLLER__H__
#define __WAPSTART_SWORDFISH_STORAGE_CONTROLLER__H__

#include "storage.hpp"

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/regex.hpp>

namespace wapstart {
    class Storage;

    class StorageController {
        typedef std::map<std::string, std::string> string_list;
        typedef Command cmd_type;
        typedef std::string result_type;
        typedef string_list regex_map_type;
        typedef boost::ptr_map<std::string, Storage*> storage_list_type;


    public:
        void addStorage(const std::string&, Storage*&, std::vector<std::string>);
        void clean();
        Storage*& getStorage(const std::string&);
        Storage*& getStorageByFunction(const std::string&);
        void processRequest(const cmd_type& command, result_type& result);
    private:
        storage_list_type storages_;
        string_list function_map_;
        regex_map_type regex_map;
    };

}


#endif //__WAPSTART_SWORDFISH_STORAGE_CONTROLLER__H__
