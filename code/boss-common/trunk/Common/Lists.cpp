/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

//Contains some helpful functions for dealing with the modlist (inc. masterlist) and userlist data structures.

#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Common/Lists.h"
#include "Common/Error.h"
#include "Common/Globals.h"
#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;

	vector<string> userlistErrorBuffer;  //Holds any error messages generated during parsing for printing later.
	vector<string> masterlistErrorBuffer;  //Holds any error messages generated during parsing for printing later.
	vector<string> iniErrorBuffer;  //Holds any error messages generated during parsing for printing later.

	vector<message> globalMessageBuffer;  //Holds any global messages from the masterlist to be printed in BOSS.

	//Find a mod by name. Will also find the starting position of a group.
	size_t GetModPos(vector<item> list, string filename) {
		size_t size = list.size();
		for (size_t i=0; i<size; i++) {
			if (Tidy(list[i].name.string()) == Tidy(filename))  //Look for exact match.
				return i;
			else if (Tidy(list[i].name.string()) == Tidy(filename + ".ghost"))  //Look for match with ghosted mod.
				return i;
		}
		return (size_t)-1;
	}

	//Find the end of a group by name.
	size_t GetGroupEndPos(vector<item> list, string groupname) {
		size_t size = list.size();
		for (size_t i=0; i<size; i++) {
			if (list[i].type == ENDGROUP && Tidy(list[i].name.string()) == Tidy(groupname)) {
				return i;
			}
		}
		return (size_t)-1;
	}

	//Date comparison, used for sorting mods in modlist.
	bool SortModsByDate(item mod1,item mod2) {
		time_t t1 = 0,t2 = 0;
		try {
			t1 = fs::last_write_time(data_path / mod1.name);
			t2 = fs::last_write_time(data_path / mod2.name);
		}catch (fs::filesystem_error e){
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, mod1.name.string() + "\" or \"" + mod2.name.string(),e.what());
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		return (diff < 0);
	}

	//Build modlist (the one that gets saved to file, not the masterlist).
	//Adds mods in directory to modlist in date order (AKA load order).
	void BuildModlist(vector<item> &list) {
		LOG_DEBUG("Reading user mods...");
		for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
            const fs::path filename = itr->path().filename();
			const string ext = to_lower_copy(itr->path().extension().string());
			if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
				LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());			
				//Add file to modlist.
				item mod;
				mod.name = filename;
				mod.type = MOD;
				list.push_back(mod);
			}
		}
		sort(list.begin(),list.end(),SortModsByDate);
		LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", list.size());
	}

	//Save the modlist (not masterlist) to a file, printing out all the information in the data structure.
	//This will likely just be a list of filenames, if it's the modlist.
	//However, if used on the masterlist, could prove useful for debugging the parser.
	void SaveModlist(vector<item> list, fs::path file) {
		ofstream ofile;
		//Back up file if it already exists.
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(file)) fs::rename(file, prev_modlist_path);
		} catch(boost::filesystem::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			LOG_ERROR("Backup of modlist failed with error: %s", e.what());
			throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, file.string(), e.what());
		}
		//Open output file.
		ofile.open(file.c_str());
		if (ofile.fail()) {  //Provide error message if it can't be written.
			LOG_ERROR("Backup cannot be saved.");
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		}

		//Iterate through items, printing out all group markers, mods and messsages.
		size_t size = list.size();
		for (size_t i=0; i<size; i++) {
			if (list[i].type == BEGINGROUP)
				ofile << "BEGINGROUP: " << list[i].name.string() << endl;  //Print the group begin marker
			else if (list[i].type == ENDGROUP)
				ofile << "ENDGROUP: " << list[i].name.string() << endl;  //Print the group end marker
			else {
				ofile << list[i].name.string() << endl;  //Print the mod name.
				//Print the messages with the appropriate syntax.
				size_t messagesSize = list[i].messages.size();
				for (size_t j=0; j<messagesSize; j++) {
					ofile << " " << list[i].messages[j].key << ": " << list[i].messages[j].data << endl;  
				}
			}
		}

		ofile.close();
		LOG_INFO("Backup saved successfully.");
		return;
	}

	//Returns a string representation of the given key, for use in output messages.
	//Only really required for userlist keywords.
	string KeyToString(keyType key) {
		if (key == ADD)
			return "add";
		else if (key == OVERRIDE)
			return "override";
		else if (key == FOR)
			return "for";
		else if (key == BEFORE)
			return "before";
		else if (key == AFTER)
			return "after";
		else if (key == TOP)
			return "top";
		else if (key == BOTTOM)
			return "bottom";
		else if (key == APPEND)
			return "append";
		else if (key == REPLACE)
			return "replace";
		else if (key == SAY || key == OOOSAY || key == BCSAY)
			return "SAY";
		else if (key == TAG)
			return "TAG";
		else if (key == REQ)
			return "REQ";
		else if (key == WARN)
			return "WARN";
		else if (key == ERR)
			return "ERROR";
		else if (key == INC)
			return "INC";
		else if (key == DIRTY)
			return "DIRTY";
		else
			return "NONE";
	}

	//Returns a keyType representation of the given key string.
	//Only really required for masterlist message keys.
	keyType StringToKey(string key) {
		boost::algorithm::to_lower(key);
		if (key == "add")
			return ADD;
		else if (key == "override")
			return OVERRIDE;
		else if (key == "for")
			return FOR;
		else if (key == "before")
			return BEFORE;
		else if (key == "after")
			return AFTER;
		else if (key == "top")
			return TOP;
		else if (key == "bottom")
			return BOTTOM;
		else if (key == "append")
			return APPEND;
		else if (key == "replace")
			return REPLACE;
		else if (key == "say" || key == "?" || key == "^" || key == "$")
			return SAY;
		else if (key == "tag" || key == "%")
			return TAG;
		else if (key == "req" || key == ":")
			return REQ;
		else if (key == "warn")
			return WARN;
		else if (key == "error" || key == "*")
			return ERR;
		else if (key == "inc" || key == "\"")
			return INC;
		else if (key == "dirty")
			return DIRTY;
		else
			return NONE;
	}
}