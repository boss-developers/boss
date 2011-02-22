/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains some helpful functions for dealing with the modlist (inc. masterlist) and userlist data structures.
//DOES NOT CONTAIN ANYTHING TO DO WITH THE FILE PARSERS.

#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Lists.h"
#include "Error.h"
#include "Globals.h"
#include <boost/algorithm/string.hpp>
#include "../utf8/source/utf8.h"

namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;

	//Find a mod by name. Will also find the starting position of a group.
	int GetModPos(modlist list, string filename) {
		for (int i=0; i<(int)list.items.size(); i++) {
			if (Tidy(list.items[i].name.string()) == Tidy(filename)) {
				return i;
			}
		}
		return -1;
	}

	//Find the end of a group by name.
	int GetGroupEndPos(modlist list, string groupname) {
		for (int i=0; i<(int)list.items.size(); i++) {
			if (list.items[i].type == GROUPEND && Tidy(list.items[i].name.string()) == Tidy(groupname)) {
				return i;
			}
		}
		return -1;
	}

	//Date comparison, used for sorting mods in modlist.
	bool SortModsByDate(item mod1,item mod2) {
		time_t t1 = 0,t2 = 0;
		try {
			t1 = fs::last_write_time(data_path / mod1.name);
			t2 = fs::last_write_time(data_path / mod2.name);
		}catch (fs::filesystem_error e){
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		if (diff > 0)
			return false;
		else
			return true;
	}

	//Build modlist (the one that gets saved to file, not the masterlist).
	//Adds mods in directory to modlist in date order (AKA load order).
	void BuildModlist(modlist &list) {
		LOG_DEBUG("Reading user mods...");
		if (fs::is_directory(data_path)) {
			for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
                const fs::path filename = itr->path().filename();
				const string ext = to_lower_copy(itr->path().extension().string());
				if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
					LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());			
					//Add file to modlist.
					item mod;
					mod.name = filename;
					mod.type = MOD;
					list.items.push_back(mod);
				}
			}
		}
		sort(list.items.begin(),list.items.end(),SortModsByDate);
		LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", list.items.size());
	}

	//Save the modlist (not masterlist) to a file, printing out all the information in the data structure.
	//This will likely just be a list of filenames, if it's the modlist.
	//However, if used on the masterlist, could prove useful for debugging the parser.
	void SaveModlist(modlist list, fs::path file) {
		ofstream ofile;
		//Back up file if it already exists.
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(file)) fs::rename(file, prev_modlist_path);
		} catch(boost::filesystem::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			LOG_ERROR("Backup of modlist failed with error: %s", e.what());
			throw boss_error() << err_detail(e.what());
		}
		//Open output file.
		ofile.open(file.c_str());
		if (ofile.fail()) {  //Provide error message if it can't be written.
			LOG_ERROR("Backup cannot be saved.");
			throw boss_error() << err_detail("Backup cannot be saved.");
		}

		//Iterate through items, printing out all group markers, mods and messsages.
		for (int i=0; i<(int)list.items.size(); i++) {
			if (list.items[i].type == GROUPBEGIN)
				ofile << "BEGINGROUP: " << list.items[i].name.string() << endl;  //Print the group begin marker
			else if (list.items[i].type == GROUPEND)
				ofile << "ENDGROUP: " << list.items[i].name.string() << endl;  //Print the group end marker
			else {
				ofile << list.items[i].name.string() << endl;  //Print the mod name.
				//Print the messages with the appropriate syntax.
				for (int j=0; j<(int)list.items[i].messages.size(); j++) {
					ofile << " " << list.items[i].messages[j].key << ": " << list.items[i].messages[j].data << endl;  
				}
			}
		}

		ofile.close();
		LOG_INFO("Backup saved successfully.");
		return;
	}
}