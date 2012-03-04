/*	BOSS
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

    BOSS is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    BOSS is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BOSS.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#include "Common/Classes.h"
#include "Common/Globals.h"
#include "Output/Output.h"
#include "Support/Logger.h"
#include "Support/Helpers.h"
#include "Support/ModFormat.h"
#include "Parsing/Grammar.h"

#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	using boost::algorithm::to_lower_copy;

	/////////////////////////////////////
	// conditionalData Class Methods
	/////////////////////////////////////

	conditionalData::conditionalData() {
		data = "";
		conditions = "";
	}

	conditionalData::conditionalData(string inData, string inConditions) {
		data = inData;
		conditions = inConditions;
	}

	string conditionalData::Data() const {
		return data;
	}

	string conditionalData::Conditions() const {
		return conditions;
	}

	void conditionalData::Data(string inData) {
		data = inData;
	}

	void conditionalData::Conditions(string inConditions) {
		conditions = inConditions;
	}

	bool conditionalData::EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer) {
		if (!conditions.empty()) {
			Skipper skipper;
			conditional_grammar grammar;
			string::const_iterator begin, end;
			bool eval;
		
			skipper.SkipIniComments(false);
			grammar.SetVarStore(&setVars);
			grammar.SetCRCStore(&fileCRCs);
			grammar.SetErrorBuffer(&errorBuffer);

			begin = conditions.begin();
			end = conditions.end();

		//	iterator_type u32b(begin);
		//	iterator_type u32e(end);

		//	bool r = phrase_parse(u32b, u32e, grammar, skipper, eval);
			bool r = phrase_parse(begin, end, grammar, skipper, eval);

			if (!r || begin != end)
				throw boss_error(BOSS_ERROR_CONDITION_EVAL_FAIL, conditions);

			return eval;
		} else
			return true;
	}

	/////////////////////////////////////
	// MasterlistVar Class Methods
	/////////////////////////////////////

	MasterlistVar::MasterlistVar() : conditionalData() {};

	MasterlistVar::MasterlistVar(string inData, string inConditions) : conditionalData(inData, inConditions) {};
	
	//////////////////////////////
	// Message Class Functions
	//////////////////////////////

	Message::Message	() : conditionalData() {
		key = SAY;
	}
	
	Message::Message	(keyType inKey, string inData) : conditionalData(inData, "") {
		key = inKey;
	}

	keyType Message::Key() const {
		return key;
	}

	void Message::Key(keyType inKey) {
		key = inKey;
	}

	string	Message::KeyToString() const {
		switch(key) {
		case SAY:
			return "SAY";
		case TAG:
			return "TAG";
		case REQ:
			return "REQ";
		case WARN:
			return "WARN";
		case ERR:
			return "ERROR";
		case INC:
			return "INC";
		case DIRTY:
			return "DIRTY";
		default:
			return "NONE";
		}
	}

	bool	Message::EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer) {
		LOG_TRACE("Evaluating conditional for message \"%s\"", Data().c_str());
		bool eval = conditionalData::EvalConditions(setVars, fileCRCs, errorBuffer);

		if (!eval)
			return false;
		else if (!Data().empty()) {
			Skipper skipper;
			shorthand_grammar grammar;
			string::const_iterator begin, end;
			string newMessage;
		
			skipper.SkipIniComments(false);
			grammar.SetVarStore(&setVars);
			grammar.SetCRCStore(&fileCRCs);
			grammar.SetErrorBuffer(&errorBuffer);

			//Now we must check if the message is using a conditional shorthand and evaluate that if so.
			grammar.SetMessageType(key);

			string contents = Data();
			begin = contents.begin();
			end = contents.end();
			
		//	iterator_type u32b(begin);
		//	iterator_type u32e(end);

		//	bool r = phrase_parse(u32b, u32e, grammar, skipper, newMessage);
			bool r = phrase_parse(begin, end, grammar, skipper, newMessage);

			if (!r || begin != end)
				throw boss_error(BOSS_ERROR_CONDITION_EVAL_FAIL, Data());

			if (newMessage.empty())
				return false;
			Data(newMessage);
		}
		return true;
	}

	//////////////////////////////
	// Item Class Functions
	//////////////////////////////
	
	Item::Item			() : conditionalData() {
		type = MOD;
		messages.clear();
	}
	
	Item::Item			(string inName) : conditionalData(inName, "") {
		type = MOD;
		messages.clear();
	}
	
	Item::Item			(string inName, itemType inType) : conditionalData(inName, "") {
		type = inType;
		messages.clear();
	}
	
	Item::Item			(string inName, itemType inType, vector<Message> inMessages) : conditionalData(inName, "") {
		type = inType;
		messages = inMessages;
	}

	vector<Message> Item::Messages() const {
		return messages;
	}

	itemType Item::Type() const {
		return type;
	}

	string Item::Name() const {
		return Data();
	}

	void Item::Messages(vector<Message> inMessages) {
		messages = inMessages;
	}

	void Item::Type(itemType inType) {
		type = inType;
	}

	void Item::Name(string inName) {
		Data(inName);
	}
	
	bool	Item::IsPlugin		() const {
		const string ext = boost::algorithm::to_lower_copy(fs::path(Data()).extension().string());
		return (ext == ".esp" || ext == ".esm");
	}

	bool	Item::IsGroup		() const { 
		return (!fs::path(Data()).has_extension() && !Data().empty()); 
	}

	bool	Item::Exists		() const { 
		return (fs::exists(data_path / Data()) || fs::exists(data_path / fs::path(Data() + ".ghost"))); 
	}
	
	bool	Item::IsGameMasterFile	() const {
		return boost::iequals(Data(), GetGameMasterFile(gl_current_game));
	}

	bool	Item::IsMasterFile() const {
		return ReadHeader(data_path / Data()).IsMaster;
	}

	bool	Item::IsGhosted		() const {
		return (fs::exists(data_path / fs::path(Data() + ".ghost")));
	}
	
	string	Item::GetVersion		() const {
		if (!IsPlugin())
			return "";
		
		ModHeader header;	

		// Read mod's header now...
		if (IsGhosted())
			header = ReadHeader(data_path / fs::path(Data() + ".ghost"));
		else
			header = ReadHeader(data_path / Data());

		// The current mod's version if found, or empty otherwise.
		return header.Version;
	}

	void	Item::SetModTime	(time_t modificationTime) const {
		try {			
			if (IsGhosted())
				fs::last_write_time(data_path / fs::path(Data() + ".ghost"), modificationTime);
			else
				fs::last_write_time(data_path / Data(), modificationTime);
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL, Data(), e.what());
		}
	}

	void	Item::UnGhost		() const {			//Can throw exception.
		if (IsGhosted()) {
			try {
				fs::rename(data_path / fs::path(Data() + ".ghost"), data_path / Data());
			} catch (fs::filesystem_error e) {
				throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, Data() + ".ghost", e.what());
			}
		}
	}

	time_t	Item::GetModTime	() const {			//Can throw exception.
		try {			
			if (IsGhosted())
				return fs::last_write_time(data_path / fs::path(Data() + ".ghost"));
			else
				return fs::last_write_time(data_path / Data());
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL, Data(), e.what());
		}
	}

	bool	Item::operator <	(Item item2) {
		//Two things matter when ordering plugins in timestamp-based load order systems: timestamp and whether a plugin is a master or not.
		time_t t1 = 0,t2 = 0;
		try {
			if (this->IsGhosted())
				t1 = fs::last_write_time(data_path / fs::path(Data() + ".ghost"));
			else
				t1 = fs::last_write_time(data_path / Data());
			if (item2.IsGhosted())
				t2 = fs::last_write_time(data_path / fs::path(item2.Data() + ".ghost"));
			else
				t2 = fs::last_write_time(data_path / item2.Data());
		}catch (fs::filesystem_error e){
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, Data() + "\" or \"" + item2.Data(), e.what());
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		//Masters always load before non-masters.
		if (IsMasterFile() && !item2.IsMasterFile()) 
			return true;
		else if (!IsMasterFile() && item2.IsMasterFile())
			return false;
		else
			return (diff < 0);
	}

	bool	Item::EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer) {
		LOG_TRACE("Evaluating conditions for item \"%s\"", Data().c_str());

		bool eval = conditionalData::EvalConditions(setVars, fileCRCs, errorBuffer);

		vector<Message>::iterator messageIter = messages.begin();
		while (messageIter != messages.end()) {
			if (messageIter->EvalConditions(setVars, fileCRCs, errorBuffer))
				++messageIter;
			else
				messageIter = messages.erase(messageIter);
		}

		return eval;
	}

	//////////////////////////////
	// ItemList Class Functions
	//////////////////////////////

	bool	CompareItems(Item item1, Item item2) {
		//Return true if item1 goes before item2, false otherwise.
		//Master files should go before other files.
		//Groups should not change position (but master files should be able to cross groups).
		if (item1.IsMasterFile() && !item2.IsMasterFile())
			return true;
		else
			return false;
	}

							ItemList::ItemList					() {
		items.clear();
		errorBuffer = ParsingError();
		globalMessageBuffer.clear();
		lastRecognisedPos = 0;
		masterlistVariables.clear();
		fileCRCs.clear();
	}

	/*I need to worry about the encoding of the file only if it's plugins_path() being loaded.
	 Otherwise it's going to be UTF-8 and I can check validation.
	 If it is plugins_path():
	 1. Detect encoding.
	 2. Convert string buffer from detected encoding to UTF-8.*/
	void	ItemList::Load				(fs::path path) {
		if (fs::exists(path) && fs::is_directory(path)) {
			LOG_DEBUG("Reading user mods...");
			if (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0")) {
				/*Game is Skyrim v1.4.26, so uses the new load order system.

				Check if loadorder.txt exists, and read that if it does.
				If it doesn't exist, then read plugins.txt and scan the given directory for mods,
				adding those that weren't in the plugins.txt to the end of the load order, in the order they are read.

				There is no sure-fire way of managing such a situation. If no loadorder.txt, then
				no utilties compatible with that load order method have been installed, so it won't
				break anything apart from the load order not matching the load order in the Bashed
				Patch's Masters list if it exists. That isn't something that can be easily accounted
				for though.
				*/
				LOG_INFO("Game is Skyrim v1.4.26+. Using non-timestamp-based load order mechanism.");
				if (fs::exists(loadorder_path())) {  //If the loadorder.txt exists, get the active plugin load order from that.
					Load(loadorder_path());
					//We may need to update the loadorder as some plugins may have been installed/uninstalled.
					//First scan through loadorder, removing any plugins that aren't in the data folder.
					vector<Item>::iterator itemIter = items.begin();
					while (itemIter != items.end()) {
						if (!itemIter->Exists())
							itemIter = items.erase(itemIter);
						else
							++itemIter;
					}
					//Now scan through Data folder. Add any plugins that aren't already in loadorder to loadorder, at the end.
					size_t max = items.size();
					for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
						const fs::path filename = itr->path().filename();
						const string ext = boost::algorithm::to_lower_copy(itr->path().extension().string());
						if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
							LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
							//Add file to modlist. If the filename has a '.ghost' extension, remove it.
							Item tempItem;
							if (ext == ".ghost")
								tempItem = Item(filename.stem().string());
							else
								tempItem = Item(filename.string());
							if (FindItem(tempItem.Name()) == max) {  //If the plugin is not in loadorder, add it.
								Insert(max, tempItem);
								max++;
							}
						}
					}
				} else { 
					//First add Skyrim.esm and Update.esm.
					items.push_back(Item("Skyrim.esm"));
					items.push_back(Item("Update.esm"));
					//Now check if plugins.txt exists. If so, add any plugins in it that aren't in loadorder.
					ItemList plugins;
					size_t max;
					if (fs::exists(plugins_path())) {
						plugins.Load(plugins_path());
						vector<Item> pluginsVec = plugins.Items();
						max = pluginsVec.size();
						for (size_t i=0; i < max; i++) {
							if (pluginsVec[i].Name() != "Skyrim.esm" && pluginsVec[i].Name() != "Update.esm")
								items.push_back(pluginsVec[i]);
						}
					}
					max = items.size();
					//Now iterate through the Data directory, adding any plugins to loadorder that aren't already in it.
					for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
						const fs::path filename = itr->path().filename();
						const string ext = boost::algorithm::to_lower_copy(itr->path().extension().string());
						if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
							LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
							//Add file to modlist. If the filename has a '.ghost' extension, remove it.
							Item tempItem;
							if (ext == ".ghost")
								tempItem = Item(filename.stem().string());
							else
								tempItem = Item(filename.string());
							if (FindItem(tempItem.Name()) == max) {  //If the plugin is not present, add it.
								items.push_back(tempItem);
								max++;
							}
						}
					}
				}
				sort(items.begin(),items.end(), CompareItems);  //Does this work?
			} else {  //Non-Skyrim.
				for (fs::directory_iterator itr(path); itr!=fs::directory_iterator(); ++itr) {
					const fs::path filename = itr->path().filename();
					const string ext = boost::algorithm::to_lower_copy(itr->path().extension().string());
					if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
						LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
						//Add file to modlist. If the filename has a '.ghost' extension, remove it.
						Item tempItem;
						if (ext == ".ghost")
							tempItem = Item(filename.stem().string());
						else
							tempItem = Item(filename.string());
						items.push_back(tempItem);
					}
				}
				sort(items.begin(),items.end());
				LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", items.size());
			}
		} else if (path == loadorder_path() && fs::exists(path)) {  //Only try loading loadorder.txt if it exists. Just in case somewhere calls it without checking first.
			if (!ValidateUTF8File(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			//loadorder.txt is simple enough that we can avoid needing the full modlist parser which has the crashing issue.
			//It's just a text file with a plugin filename on each line. Skip lines which are blank or start with '#'.
			 std::ifstream in(path.c_str());
			 if (in.fail())
				 throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());

			 string line;
			 
			 while (in.good()) {
				 getline(in, line);

				 if (line.empty() || line[0] == '#')  //Character comparison is OK because it's ASCII.
					 continue;

				 items.push_back(Item(line));
			 }
			 in.close();
		} else {
			Skipper skipper;
			if (path == plugins_path()) //We should skip ini comments. (#)
				skipper.SkipIniComments(true);
			modlist_grammar grammar;
			string::const_iterator begin, end;
			string contents;

			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetGlobalMessageBuffer(&globalMessageBuffer);
			grammar.SetVarStore(&masterlistVariables);
			grammar.SetCRCStore(&fileCRCs);

			if (!fs::exists(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());
			else if (path != plugins_path() && !ValidateUTF8File(path))  //plugins.txt is not going to be UTF-8.
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			fileToBuffer(path,contents);

			//Now check if plugins_path() then detect encoding if it is and translate to UTF-8.
			if (path == plugins_path()) {
				Transcoder trans;
				trans.SetEncoding(1252);
				contents = trans.EncToUtf8(contents);
			}

			begin = contents.begin();
			end = contents.end();
			
		//	iterator_type u32b(begin);
		//	iterator_type u32e(end);

		//	bool r = phrase_parse(u32b, u32e, grammar, skipper, items);
			bool r = phrase_parse(begin, end, grammar, skipper, items);

			if (!r || begin != end)
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());

			if (path == plugins_path())
				sort(items.begin(),items.end(), CompareItems);  //Does this work?
		}
	}
	
	void	ItemList::Save				(fs::path file, fs::path oldFile) {
		ofstream ofile;
		//Back up file if it already exists.
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(file)) 
				fs::rename(file, oldFile);
		} catch(fs::filesystem_error e) {
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
		vector<Item>::iterator itemIter = items.begin();
		vector<Message>::iterator messageIter;
		for (itemIter; itemIter != items.end(); ++itemIter) {
			if (itemIter->Type() == BEGINGROUP)
				ofile << "BEGINGROUP: " << itemIter->Name() << endl;  //Print the group begin marker
			else if (itemIter->Type() == ENDGROUP)
				ofile << "ENDGROUP: " << itemIter->Name() << endl;  //Print the group end marker
			else {
				if (!itemIter->Conditions().empty()) {
					ofile << itemIter->Conditions() << ' ';
					if (itemIter->Type() == MOD)
						ofile << "MOD: ";
				}
				if (itemIter->Type() == REGEX)
					ofile << "REGEX: ";
				ofile << itemIter->Name() << endl;  //Print the mod name.
				//Print the messages with the appropriate syntax.
				vector<Message> messages = itemIter->Messages();
				for (messageIter = messages.begin(); messageIter != messages.end(); ++messageIter) {
					if (!messageIter->Conditions().empty())
						ofile << ' ' << messageIter->Conditions();
					ofile << ' ' << messageIter->KeyToString() << ": " << messageIter->Data() << endl; 
				}
			}
		}

		ofile.close();
		LOG_INFO("Backup saved successfully.");
		return;
	}
	
	/*I need to worry about the encoding of the file only if it's plugins_path() being saved.
	 Otherwise it's going to be UTF-8 and BOSS uses that internally already.
	 If it is plugins_path():
	 1. Detect the encoding of plugins_path().
	 2. Convert output from UTF-8 to the detected encoding before writing it. 
	 3. If the output cannot be converted, throw an exception (BOSS_ERROR_ENCODING_CONVERSION_FAIL).
	    The errSubject should be the plugin causing problems and errString should be the encoding detected. 
		Don't throw an exception immediately though! Set a "there's been a problem" flag and record the plugin
		name, then throw the exception when the file has been written. It's not a breaking bug, but using an
		exception is the neatest way to go about showing it occurred.
	activeOnly is the measure of whether a conversion is necessary.*/
	void	ItemList::SavePluginNames(fs::path file, bool activeOnly, bool doEncodingConversion) {
		string badFilename = "";
		ItemList activePlugins;
		size_t numActivePlugins;
		Transcoder trans;
		if (activeOnly) {
			//To save needing a new parser, load plugins.txt into an ItemList then fill a hashset from that.
			//Also check if plugins_path() then detect encoding if it is and translate outputted text from UTF-8 to the detected encoding.
			LOG_INFO("Loading plugins.txt into ItemList.");
			activePlugins.Load(plugins_path());
			numActivePlugins = activePlugins.Items().size();
		}
		if (doEncodingConversion) {
			string contents;
			fileToBuffer(file, contents);
			trans.SetEncoding(1252);
		}

		bool isSkyrim1426plus = (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0"));

		LOG_INFO("Writing new \"%s\"", file.string().c_str());
		ofstream outfile;
		outfile.open(file.c_str(), ios_base::trunc);
		if (outfile.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == MOD) {
				if (activeOnly && (activePlugins.FindItem(items[i].Name()) == numActivePlugins || (isSkyrim1426plus && items[i].Name() == "Skyrim.esm")))
					continue;
				LOG_DEBUG("Writing \"%s\" to \"%s\"", items[i].Name().c_str(), file.string().c_str());
				if (doEncodingConversion) {  //Not UTF-8.
					try {
						outfile << trans.Utf8ToEnc(items[i].Name()) << endl;
					} catch (boss_error &e) {
						badFilename = items[i].Name();
					}
				} else
					outfile << items[i].Name() << endl;
			}
		}
		outfile.close();

		if (!badFilename.empty())
			throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, badFilename, IntToString(1252));
	}

	void		ItemList::EvalConditions	() {
		boost::unordered_set<string> setVars;

		//First eval variables.
		//Need to convert these from a vector to an unordered set.
		LOG_INFO("Starting to evaluate variable conditionals.");
		vector<MasterlistVar>::iterator varIter = masterlistVariables.begin();
		while (varIter != masterlistVariables.end()) {
			if (varIter->EvalConditions(setVars, fileCRCs, errorBuffer)) {
				setVars.insert(varIter->Data());
				++varIter;
			} else
				varIter = masterlistVariables.erase(varIter);
		}

		//Now eval items.
		LOG_INFO("Starting to evaluate item conditionals.");
		vector<Item>::iterator itemIter = items.begin();
		while (itemIter != items.end()) {
			if (itemIter->EvalConditions(setVars, fileCRCs, errorBuffer))
				++itemIter;
			else
				itemIter = items.erase(itemIter);
		}

		//Now eval global messages.
		LOG_INFO("Starting to evaluate global message conditionals.");
		vector<Message>::iterator messageIter = globalMessageBuffer.begin();
		while (messageIter != globalMessageBuffer.end()) {
			if (messageIter->EvalConditions(setVars, fileCRCs, errorBuffer))
				++messageIter;
			else
				messageIter = globalMessageBuffer.erase(messageIter);
		}
	}

	

	void		ItemList::ApplyMasterPartition() {
		//Need to iterate through items vector, sorting it according to the rule that master items come before other items.
		/* However, have to think about masterlist groups. They shouldn't change.
		   Easiest to just use std::sort().
		*/
		sort(items.begin(), items.end(), CompareItems);  //Does this work?
	}
	
	size_t		ItemList::FindItem			(string name) const {
		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (boost::iequals(items[i].Name(), name))
				return i;
		}
		return max;
	}

	size_t		ItemList::FindLastItem		(string name) const {
		size_t max = items.size();
		for (size_t i=max-1; i >= 0; i--) {
			if (boost::iequals(items[i].Name(), name))
				return i;
		}
		return max;
	}
	
	//This looks a bit weird, but I need a non-reverse iterator outputted, and searching backwards is probably more efficient for my purposes.
	size_t		ItemList::FindGroupEnd		(string name) const {
		size_t max = items.size();
		for (size_t i=max-1; i >= 0; i--) {
			if (items[i].Type() == ENDGROUP && boost::iequals(items[i].Name(), name))
				return i;
		}
		return max;
	}

	size_t		ItemList::GetLastMasterPos() const {
		size_t i=0;
		while (i < items.size() && items[i].IsMasterFile()) {  //SLLOOOOOWWWWW probably.
			i++;
		}
		if (i > 0)
			return i-1;  //i is position of first plugin.
		else 
			return items.size();
	}

	size_t	ItemList::GetNextMasterPos(size_t currPos) const {
		size_t i=currPos;
		while (i < items.size() && !items[i].IsMasterFile()) {  //SLLOOOOOWWWWW probably.
			i++;
		}
		return i;  //i is position of first master after currPos.
	}

	vector<Item> ItemList::Items() const {
		return items;
	}

	ParsingError ItemList::ErrorBuffer() const {
		return errorBuffer;
	}

	vector<Message> ItemList::GlobalMessageBuffer() const {
		return globalMessageBuffer;
	}

	size_t ItemList::LastRecognisedPos() const {
		return lastRecognisedPos;
	}

	vector<MasterlistVar> ItemList::Variables() const {
		return masterlistVariables;
	}

	boost::unordered_map<string,uint32_t> ItemList::FileCRCs() const {
		return fileCRCs;
	}

	void ItemList::Items(vector<Item> inItems) {
		items = inItems;
	}

	void ItemList::ErrorBuffer(ParsingError buffer) {
		errorBuffer = buffer;
	}

	void ItemList::GlobalMessageBuffer(vector<Message> buffer) {
		globalMessageBuffer = buffer;
	}

	void ItemList::LastRecognisedPos(size_t pos) {
		lastRecognisedPos = pos;
	}

	void ItemList::Variables(vector<MasterlistVar> variables) {
		masterlistVariables = variables;
	}

	void ItemList::FileCRCs(boost::unordered_map<string,uint32_t> crcs) {
		fileCRCs = crcs;
	}

	void ItemList::Erase(size_t pos) {
		items.erase(items.begin() + pos);
	}
	
	void ItemList::Erase(size_t startPos, size_t endPos) {
		items.erase(items.begin() + startPos, items.begin() + endPos);
	}
	
	void ItemList::Insert(size_t pos, vector<Item> source, size_t sourceStart, size_t sourceEnd) {
		items.insert(items.begin()+pos, source.begin()+sourceStart, source.begin()+sourceEnd);
	}

	void ItemList::Insert(size_t pos, Item item) {
		items.insert(items.begin()+pos, item);
	}

	//////////////////////////////
	// RuleLine Class Functions
	//////////////////////////////

			RuleLine::RuleLine			() {
				key = NONE;
				object = "";
			}

			RuleLine::RuleLine			(keyType inKey, string inObject) {
				key = inKey;
				object = inObject;
			}

	bool	RuleLine::IsObjectMessage	() const {
		if (key != APPEND && key != REPLACE)
			return false;

		//First character of message, must be a message symbol, or part of an MF2 keyword.
		if (object[0] == '?' || object[0] == '$' || object[0] == '^' || object[0] == '%' || object[0] == ':' || object[0] == '"' || object[0] == '*')
			return true;
		else {
			size_t pos = object.find(':');
			if (pos == string::npos)
				return false;
			string keyString = object.substr(0,pos);
			if (keyString == "SAY" || keyString == "TAG" || keyString == "REQ" || keyString == "INC" || keyString == "DIRTY" || keyString == "WARN" || keyString == "ERROR")
				return true;
			else
				return false;
		}
	}
	
	keyType	RuleLine::ObjectMessageKey	() const {
		if (key != APPEND && key != REPLACE)
			return NONE;

		switch(object[0]) {
		case '?':
			return SAY;
		case '$':
			return SAY;
		case '^':
			return SAY;
		case '%':
			return TAG;
		case ':':
			return REQ;
		case '"':
			return INC;
		case '*':
			return ERR;
		default:
			size_t pos = object.find(':');
			if (pos == string::npos)
				return NONE;
			string keyString = object.substr(0,pos);
			if (keyString == "SAY")
				return SAY;
			else if (keyString == "TAG")
				return TAG;
			else if (keyString == "REQ")
				return REQ;
			else if (keyString == "INC")
				return INC;
			else if (keyString == "DIRTY")
				return DIRTY;
			else if (keyString == "WARN")
				return WARN;
			else if (keyString == "ERROR")
				return ERR;
			else 
				return NONE;
		}
	}

	string	RuleLine::ObjectMessageData	() const {
		if (key != APPEND && key != REPLACE)
			return "";

		//First character of message, must be a message symbol, or part of an MF2 keyword.
		if (object[0] == '?' || object[0] == '$' || object[0] == '^' || object[0] == '%' || object[0] == ':' || object[0] == '"' || object[0] == '*')
			return object.substr(1);
		else {
			size_t pos = object.find(':');
			if (pos == string::npos)
				return "";
			return object.substr(pos+1);
		}
	}

	string	RuleLine::KeyToString		() const {
		switch(key) {
		case ADD:
			return "ADD";
		case OVERRIDE:
			return "OVERRIDE";
		case FOR:
			return "FOR";
		case BEFORE:
			return "BEFORE";
		case AFTER:
			return "AFTER";
		case TOP:
			return "TOP";
		case BOTTOM:
			return "BOTTOM";
		case APPEND:
			return "APPEND";
		case REPLACE:
			return "REPLACE";
		default:
			return "NONE";
		}
	}

	keyType RuleLine::Key() const {
		return key;
	}

	string RuleLine::Object() const {
		return object;
	}

	void RuleLine::Key(keyType inKey) {
		key = inKey;
	}

	void RuleLine::Object(string inObject) {
		object = inObject;
	}

	//////////////////////////////
	// Rule Class Functions
	//////////////////////////////

	Rule::Rule() : RuleLine() {
		enabled = true;
		lines.clear();
	}

	bool Rule::Enabled() const {
		return enabled;
	}

	vector<RuleLine> Rule::Lines() const {
		return lines;
	}

	void Rule::Enabled(bool e) {
		enabled = e;
	}

	void Rule::Lines(vector<RuleLine> inLines) {
		lines = inLines;
	}


	//////////////////////////////
	// RuleList Class Functions
	//////////////////////////////

	RuleList::RuleList() {
		rules.clear();
		parsingErrorBuffer = ParsingError();
		syntaxErrorBuffer.clear();
	}

	void RuleList::Load(fs::path file) {
		Skipper skipper;
		userlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		skipper.SkipIniComments(false);
		grammar.SetParsingErrorBuffer(&parsingErrorBuffer);

		if (!fs::exists(file)) {
			ofstream userlist_file(file.c_str(),ios_base::binary);
			if (!userlist_file.fail())
				userlist_file << '\xEF' << '\xBB' << '\xBF';  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			else
				throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
			userlist_file.close();
			return;
		} else if (!ValidateUTF8File(file))
			throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, file.string());

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		
	//	iterator_type u32b(begin);
	//	iterator_type u32e(end);

	//	bool r = phrase_parse(u32b, u32e, grammar, skipper, rules);
		bool r = phrase_parse(begin, end, grammar, skipper, rules);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());
	}

	void RuleList::Save(fs::path file) {
		ofstream outFile(file.c_str(),ios_base::trunc);

		if (outFile.fail()) {  //Provide error message if it can't be written.
			LOG_ERROR("Backup cannot be saved.");
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		}

		for (vector<Rule>::iterator ruleIter = rules.begin(); ruleIter != rules.end(); ++ruleIter) {
			if (!ruleIter->Enabled())
				outFile << "DISABLE ";
			outFile << boost::algorithm::to_upper_copy(ruleIter->KeyToString()) << ": " << ruleIter->Object() << endl;

			vector<RuleLine> lines = ruleIter->Lines();
			for (vector<RuleLine>::iterator lineIter = lines.begin(); lineIter != lines.end(); ++lineIter)
				outFile << boost::algorithm::to_upper_copy(lineIter->KeyToString()) << ": " << lineIter->Object() << endl;
			outFile << endl;
		}
		outFile.close();
	}

	size_t RuleList::FindRule(string ruleObject, bool onlyEnabled) const {
		size_t max = rules.size();
		for (size_t i=0; i<max; i++) {
			if ((onlyEnabled && rules[i].Enabled()) || !onlyEnabled) {
				if (boost::iequals(rules[i].Object(), ruleObject))
					return i;
			}
		}
		return max;
	}

	vector<Rule> RuleList::Rules() const {
		return rules;
	}

	ParsingError RuleList::ParsingErrorBuffer() const {
		return parsingErrorBuffer;
	}

	vector<ParsingError> RuleList::SyntaxErrorBuffer() const {
		return syntaxErrorBuffer;
	}
		
	void RuleList::Rules(vector<Rule> inRules) {
		rules = inRules;
	}

	void RuleList::ParsingErrorBuffer(ParsingError buffer) {
		parsingErrorBuffer = buffer;
	}

	void RuleList::SyntaxErrorBuffer(vector<ParsingError> buffer) {
		syntaxErrorBuffer = buffer;
	}

	//////////////////////////////
	// Ini Class Functions
	//////////////////////////////

	
}