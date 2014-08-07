#include <boost/algorithm/string/split.hpp>

#include "storage_controller.hpp"
#include "storage.hpp"
#include "timer.hpp"


namespace wapstart {


    void StorageController::clean()
    {
        function_map_.clear();
        regex_map.clear();
        storages_.clear();
    }

    void StorageController::addStorage(const std::string& name, Storage* &storage, std::vector<std::string> function_list)
    {
        storages_[name] = storage;
        std::vector<std::string>::iterator i = function_list.begin();
        while (i != function_list.end()) {
            std::string func = *i;
            function_map_[func] = name;

            std::string regex("^" + func + ":");
            regex_map[regex] = name;
            i++;
        }
    }

    Storage*& StorageController::getStorage(const std::string &name)
    {
        return storages_.at(name);
    }

    Storage*& StorageController::getStorageByFunction(const std::string &func)
    {
        __LOG_DEBUG << "GET STORAGE BY FUNCTION: " << func;
        std::string name = function_map_.at(func);
        __LOG_DEBUG << "FOUND STORAGE NAME: " << name;

        if (name.empty()) {
            name = "default";
        }
        return getStorage(name);
    }

    void StorageController::processRequest(const cmd_type& cmd, Storage::result_type& result)
    {

        if (cmd.name() == "stats") {
            storage_list_type::iterator storageIter = storages_.begin();
            while(storageIter != storages_.end()) {
                (*storageIter->second)->get_stats(storageIter->first, result);
                storageIter ++;
            }

            result.append("END\n\r");
            return;
        }

        Timer timer("[StorageController::processRequest]: preprocess commands");
        string_list key_map;
        for(Command::arg_iterator x = cmd.arg_begin(); x != cmd.arg_end(); ++x) {
            std::string command;
            std::string::const_iterator i;

            for (i = (*x).begin(); i != (*x).end() && (*i) != ':'; i++) {
                command += (*i);
            }

            string_list::iterator funcMapIterator = function_map_.find(command);

            if (funcMapIterator == function_map_.end()  ) {
                key_map[*x] = "default";
            } else {
                key_map[*x] = funcMapIterator->second;
            }

           __LOG_DEBUG << "storage for: " << *x << " - " << key_map[*x];
        }
        timer.show();

        Timer searchStorageTimer("Search storage for command");
        string_list::iterator i = key_map.begin();
        std::map<std::string, Command> command_list;
        while (i != key_map.end()) {
            if ((command_list.find(i->second)) == command_list.end()) {
                cmd_type command ("");
                command.name(cmd.name());
                command_list[i->second] = command;
            }
            command_list.at(i->second).insert(i->first);
            i++;
        }
        searchStorageTimer.show();

        std::map<std::string, Command>::iterator j = command_list.begin();
        while (j != command_list.end()) {
            Timer storageTimer("Storage " + j->first + " processed request");
            getStorage(j->first)->_do(j->second, result);
            storageTimer.show();
            j++;
        }

    	result.append("END\r\n");
    }
}


//boost::split(splitted_command, *x, boost::is_any_of(":"));



/*pos = (*x).find(":");
std::string command = (*x).substr(0, pos - 1);

if (pos == std::string::npos) {
    key_map[*x] = "default";
} else {
    std::string command = std::string( pivot + key.size(), original.end() );

    string_list::iterator funcMapIterator = function_map_.find(command);

    if (funcMapIterator == function_map_.end()  ) {
        key_map[*x] = "default";
    } else {
        key_map[*x] = funcMapIterator->second;
    }
    __LOG_CRIT  << command;
}
*/
/*
regex_map_type::iterator regexIter = regex_map.begin();
while (regexIter != regex_map.end()) {
    if (boost::regex_search(*x, boost::regex(regexIter->first)))
    {
        key_map[*x] = regexIter->second;
        break;
    }
    regexIter ++;
}

if (regexIter == regex_map.end())
{

    key_map[*x] = "default";
}*/
//std::vector<std::string> splitted_command;
