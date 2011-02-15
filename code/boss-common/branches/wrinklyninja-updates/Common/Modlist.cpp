/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Support/Logger.h"
#include "Modlist.h"
#include <boost/algorithm/string.hpp>
#include "../utf8/source/utf8.h"
#include "Globals.h"
#include "Support/Helpers.h"

namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;
	using boost::algorithm::to_lower;
	using boost::algorithm::trim_copy;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(fs::path mod1,fs::path mod2) {
		time_t t1 = 0,t2 = 0;
		try {
			t1 = fs::last_write_time(data_path / mod1);
			t2 = fs::last_write_time(data_path / mod2);
		}catch (fs::filesystem_error e){
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		if (diff > 0)
			return false;
		else
			return true;
	}

	//Adds mods in directory to modlist in date order (AKA load order).
	void Mods::AddMods() {
		LOG_DEBUG("Reading user mods...");
		if (fs::is_directory(data_path)) {
			for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
                const fs::path filename = itr->path().filename();
				const string ext = to_lower_copy(itr->path().extension().string());
				if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
					LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());			
					mods.push_back(filename);
				}
			}
		}
		modmessages.resize((int)mods.size());
		sort(mods.begin(),mods.end(),SortByDate);
		LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", mods.size());
	}

	//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
	void Mods::SaveModList() {
		ofstream modlist;
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(curr_modlist_path)) fs::rename(curr_modlist_path, prev_modlist_path);
		} catch(boost::filesystem::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			LOG_ERROR("Backup of modlist failed with error: %s", e.what());
			throw e.what();
		}
		
		modlist.open(curr_modlist_path.c_str());
		//Provide error message if it can't be written.
		if (modlist.fail()) {
			LOG_ERROR("Backup cannot be saved.");
			const char *err = "Backup cannot be saved.";
			throw err;
		}
		for (int i=0;i<(int)mods.size();i++) {
			modlist << mods[i] << endl;
		}
		modlist.close();
		LOG_INFO("Backup saved successfully.");
		return;
	}

	//Look for a mod in the modlist, even if the mod in question is ghosted.
	int Mods::GetModIndex(fs::path mod) {
		for (int i=0;i<(int)mods.size();i++) {
			if (Tidy(mods[i].string())==Tidy(mod.string()) || Tidy(mods[i].string())==Tidy(mod.string()+".ghost")) return i;
		}
		return -1;
	}

}