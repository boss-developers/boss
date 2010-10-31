/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include "Support/Logger.h"
#include "Userlist.h"
#include <boost/algorithm/string.hpp>


namespace fs = boost::filesystem;


namespace boss {
	using namespace std;
	using boost::algorithm::to_lower_copy;
	using boost::algorithm::to_lower;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(string mod1,string mod2) {
			time_t t1 = fs::last_write_time(mod1) ;
			time_t t2 = fs::last_write_time(mod2) ;
			double diff = difftime(t1,t2);

			if (diff>0) 
				return false;
			else if (diff < 0)
				return true;
			else
				return mod1 < mod2;
	}

	//Checks if a given object is an esp, an esm or a ghosted mod.
	bool IsPlugin(string object) {
		to_lower(object);
		return (object.find(".esp")!=string::npos || object.find(".esm")!=string::npos);
	}

	//Prints messages generated by userlist parsing and rule processing to the output file stream given.
	void Rules::PrintMessages(ofstream& output) {
		output << messages;
	}

	//Add rules from userlist.txt into the rules object.
	//Then checks rule syntax and discards rules with incorrect structures.
	//Also checks if the mods referenced by rules are in your Data folder, and discards rule that reference missing mods.
	//Generates error messages for rules that are discarded.
	void Rules::AddRules() {
		ifstream userlist;
		string line,key,object;
		size_t pos;
		bool skip = false;
		userlist.open(userlist_path.external_file_string().c_str());
		messages += "<p>";
		while (!userlist.eof()) {
			char cbuffer[MAXLENGTH];
			userlist.getline(cbuffer,MAXLENGTH);
			line=cbuffer;
			if (line.length()>0) {
				if (line.substr(0,2)!="//") {
					pos = line.find(':');
					if (pos!=string::npos) {
						key = line.substr(0,pos);
						object = line.substr(pos+2);
						if (key=="ADD" || key=="OVERRIDE" || key=="FOR") {
							if (skip) {
								keys.erase(keys.begin()+rules.back(), keys.end());
								objects.erase(objects.begin()+rules.back(), objects.end());
								rules.pop_back();
							}
								keys.push_back(key);
								objects.push_back(object);
								rules.push_back((int)keys.size()-1);
								skip = false;
							if (IsPlugin(object) && !(fs::exists(object) || fs::exists(object+".ghost"))) {
								messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "\""+object+"\" is not installed.<br />";
								skip = true;
							}
							if (key=="ADD" && !IsPlugin(object)) {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to add a group.</span><br />";
								skip = true;
							}
							if (!IsPlugin(object) && Tidy(object)=="esms" && key=="OVERRIDE") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to sort the group \"ESMs\".</span><br />";
								skip = true;
							}
							if ((Tidy(object)=="oblivion.esm" || Tidy(object)=="fallout3.esm" || Tidy(object)=="nehrim.esm" || Tidy(object)=="falloutnv.esm") && key=="OVERRIDE") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to sort the master .ESM file.</span><br />";
								skip = true;
							}
						} else if ((key=="BEFORE" || key=="AFTER")) {
							keys.push_back(key);
							objects.push_back(object);
							if ((IsPlugin(object) && !IsPlugin(objects[rules.back()])) || (!IsPlugin(object) && IsPlugin(objects[rules.back()]))) {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It references a mod and a group.</span><br />";
								skip = true;
							}
							if (keys[rules.back()]=="FOR") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It includes a sort line in a rule with a FOR rule keyword.</span><br />";
								skip = true;
							}
							if (Tidy(object)=="esms" && key=="BEFORE") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to sort a group before the group \"ESMs\".</span><br />";
								skip = true;
							}
							if ((Tidy(object)=="oblivion.esm" || Tidy(object)=="fallout3.esm" || Tidy(object)=="nehrim.esm" || Tidy(object)=="falloutnv.esm") && key=="BEFORE") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to sort a mod before the master .ESM file.</span><br />";
								skip = true;
							}
						} else if ((key=="TOP" || key=="BOTTOM")) {
							keys.push_back(key);
							objects.push_back(object);
							if (keys[rules.back()]=="FOR") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It includes a sort line in a rule with a FOR rule keyword.</span><br />";
								skip = true;
							}
							if (Tidy(object)=="esms" && key=="TOP") {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to insert a mod into the top of the group \"ESMs\", before the master .ESM file.</span><br />";
								skip = true;
							}
							if ((!IsPlugin(objects[rules.back()])) || IsPlugin(object)) {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to insert a group or insert a mod into another mod.</span><br />";
								skip = true;
							}

						} else if ((key=="APPEND" || key=="REPLACE")) {
							keys.push_back(key);
							objects.push_back(object);
							if (!IsPlugin(objects[rules.back()])) {
								if (skip==false) messages += "</p><p style='margin-left:40px; text-indent:-40px;'>The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" has been skipped as it has the following problem(s):<br />";
								messages += "<span class='error'>It tries to attach a message to a group.</span><br />";
								skip = true;
							}
						} else {			
							//Key not written correctly. Cannot tell if this was the start of a rule, or in the middle of a rule.
							//Therefore to prevent problems, skip the rule. If it is the start of a new rule, that rule and the previous rule will be skipped.
							messages += "</p><p><span class='error'>The line \""+key+": "+object+"\" has a keyword that was not recognised.<br />Make sure that you have spelt the keyword correctly and that it is written in block capitals.<br />";
							messages += "The rule beginning \""+keys[rules.back()]+": "+objects[rules.back()]+"\" will be skipped to prevent problems. If the line with the unrecognised keyword is the start of a new rule, that rule will also be skipped.</span><br />";
							skip = true;
						}
					}
				}
			}
		}
		messages += "</p>";
		if (skip) {
			keys.erase(keys.begin()+rules.back(), keys.end());
			objects.erase(objects.begin()+rules.back(), objects.end());
			rules.pop_back();
		}
		userlist.close();
	}

	//Adds mods in directory to modlist in date order (AKA load order).
	void Mods::AddMods() {
		LOG_DEBUG("Reading user mods...");
		fs::path p(".");
		if (fs::is_directory(p)) {
			for (fs::directory_iterator itr(p); itr!=fs::directory_iterator(); ++itr) {
				const string filename = itr->filename();
				const string ext = to_lower_copy(fs::extension(filename));
				if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
					LOG_TRACE("-- Found mod: '%s'", filename.c_str());
					mods.push_back(filename);
				}
			}
		}
		modmessages.resize((int)mods.size());
		sort(mods.begin(),mods.end(),SortByDate);
		LOG_DEBUG("Reading user mods done: %zd total mods found.", mods.size());
	}

	//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
	int Mods::SaveModList() {
		ofstream modlist;
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			//There's a bug in the boost rename function - it should replace existing files, but it throws an exception instead, so remove the file first.
			//This bug will be fixed in BOOST 1.45, but that will break the AddMods() function and possibly other things, so be sure to change that when we upgrade.
			if (fs::exists(prev_modlist_path)) fs::remove(prev_modlist_path);
			if (fs::exists(curr_modlist_path)) fs::rename(curr_modlist_path, prev_modlist_path);
		} catch(boost::filesystem::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			LOG_ERROR("Backup of modlist failed with error: %s", e.what());
			return 1;
		}
		
		modlist.open(curr_modlist_path.external_file_string().c_str());
		//Provide error message if it can't be written.
		if (modlist.fail()) {
			LOG_ERROR("Backup cannot be saved.");
			return 2;
		}
		for (int i=0;i<(int)mods.size();i++) {
			modlist << mods[i] << endl;
		}
		modlist.close();
		LOG_INFO("Backup saved successfully.");
		return 0;
	}

	//Look for a mod in the modlist, even if the mod in question is ghosted.
	int Mods::GetModIndex(string mod) {
		for (int i=0;i<(int)mods.size();i++) {
			if (Tidy(mods[i])==Tidy(mod) || Tidy(mods[i])==Tidy(mod+".ghost")) return i;
		}
		return -1;
	}
}