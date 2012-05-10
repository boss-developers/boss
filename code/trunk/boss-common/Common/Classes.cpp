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
	
	Message::Message	(uint32_t inKey, string inData) : conditionalData(inData, "") {
		key = inKey;
	}

	uint32_t Message::Key() const {
		return key;
	}

	void Message::Key(uint32_t inKey) {
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
		return IsPluginMaster(data_path / Data());
	}

	bool	Item::IsFalseFlagged() const {
		return ((IsMasterFile() && boost::algorithm::to_lower_copy(fs::path(Data()).extension().string()) != ".esm") || (!IsMasterFile() && boost::algorithm::to_lower_copy(fs::path(Data()).extension().string()) == ".esm"));
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
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, Data(), e.what());
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

	void Item::InsertMessage(size_t pos, Message message) {
		messages.insert(messages.begin()+pos, message);
	}

	void Item::ClearMessages() {
		messages.clear();
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

			ItemList::ItemList			() {
		items.clear();
		errorBuffer = ParsingError();
		globalMessageBuffer.clear();
		lastRecognisedPos = 0;
		masterlistVariables.clear();
		fileCRCs.clear();
	}

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
				LOG_INFO("Game is Skyrim v1.4.26+. Using textfile-based load order mechanism.");
				size_t max;
				if (fs::exists(loadorder_path()))  //If the loadorder.txt exists, get the active plugin load order from that.
					Load(loadorder_path());
				else { 
					//First add Skyrim.esm.
					items.push_back(Item("Skyrim.esm"));
					//Now check if plugins.txt exists. If so, add any plugins in it that aren't in loadorder.
					ItemList plugins;
					if (fs::exists(plugins_path())) {
						plugins.Load(plugins_path());
						vector<Item> pluginsVec = plugins.Items();
						max = pluginsVec.size();
						for (size_t i=0; i < max; i++) {
							if (pluginsVec[i].Name() != "Skyrim.esm")
								items.push_back(pluginsVec[i]);
						}
					}
					//Add Update.esm if not already present.
					if (Item("Update.esm").Exists() && FindItem("Update.esm") == items.size())
						Insert(GetLastMasterPos() + 1, Item("Update.esm"));  //Previous master check ensures that GetLastMasterPos() will be not be loadorder.size().
				}
				//Then scan through loadorder, removing any plugins that aren't in the data folder.
				vector<Item>::iterator itemIter = items.begin();
				while (itemIter != items.end()) {
					if (!itemIter->Exists())
						itemIter = items.erase(itemIter);
					else
						++itemIter;
				}
				max = items.size();
				//Now scan through Data folder. Add any plugins that aren't already in loadorder to loadorder, at the end.
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
			sort(items.begin(),items.end(), CompareItems);  //Does this work?
		} else if (path == loadorder_path() || path == plugins_path()) {
			
			Transcoder trans;
			trans.SetEncoding(1252);  //Only used if path == plugins_path().

			if (path == loadorder_path() && !ValidateUTF8File(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			//loadorder.txt is simple enough that we can avoid needing the full modlist parser which has the crashing issue.
			//It's just a text file with a plugin filename on each line. Skip lines which are blank or start with '#'.
			std::ifstream in(path.c_str());
			if (in.fail())
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());

			string line;
			 
			if (gl_current_game == MORROWIND) {  //Morrowind's active file list is stored in Morrowind.ini, and that has a different format from plugins.txt.
				boost::regex reg = boost::regex("GameFile[0-9]{1,3}=.+\\.es(m|p)", boost::regex::extended|boost::regex::icase);
				while (in.good()) {
					getline(in, line);

					if (line.empty() || !boost::regex_match(line, reg))
						continue;

					//Now cut off everything up to and including the = sign.
					line = line.substr(line.find('=')+1);
					if (path == plugins_path())
						line = trans.EncToUtf8(line);
					items.push_back(Item(line));
				}
			} else {
				while (in.good()) {
					getline(in, line);

					if (line.empty() || line[0] == '#')  //Character comparison is OK because it's ASCII.
						continue;

					if (path == plugins_path())
						line = trans.EncToUtf8(line);
					items.push_back(Item(line));
				}
			}
			in.close();

			sort(items.begin(),items.end(), CompareItems);  //Does this work?
		} else {
			Skipper skipper;
			modlist_grammar grammar;
			string::const_iterator begin, end;
			string contents;

			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetGlobalMessageBuffer(&globalMessageBuffer);
			grammar.SetVarStore(&masterlistVariables);
			grammar.SetCRCStore(&fileCRCs);

			if (!fs::exists(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());
			else if (!ValidateUTF8File(path))  //plugins.txt is not going to be UTF-8.
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			fileToBuffer(path,contents);

			begin = contents.begin();
			end = contents.end();
			
		//	iterator_type u32b(begin);
		//	iterator_type u32e(end);

		//	bool r = phrase_parse(u32b, u32e, grammar, skipper, items);
			bool r = phrase_parse(begin, end, grammar, skipper, items);

			if (!r || begin != end || !errorBuffer.Empty())
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());
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

	void	ItemList::SavePluginNames(fs::path file, bool activeOnly, bool doEncodingConversion) {
		string badFilename = "",  contents, settings;
		ItemList activePlugins;
		size_t numActivePlugins;
		Transcoder trans;
		if (activeOnly) {
			//To save needing a new parser, load plugins.txt into an ItemList then fill a hashset from that.
			//Also check if plugins_path() then detect encoding if it is and translate outputted text from UTF-8 to the detected encoding.
			LOG_INFO("Loading plugins.txt into ItemList.");
			if (fs::exists(plugins_path())) {
				activePlugins.Load(plugins_path());
				numActivePlugins = activePlugins.Items().size();
			}
		}
		if (doEncodingConversion)
			trans.SetEncoding(1252);
		if (gl_current_game == MORROWIND) {  //Must be the plugins file, since loadorder.txt isn't used for MW.
			//If Morrowind, BOSS writes active plugin list to Morrowind.ini, which also holds a lot of other game settings.
			//BOSS needs to read everything up to the active plugin list in the current ini and stick that on before the first saved plugin name.
			fileToBuffer(file, contents);
			size_t pos = contents.find("[Game Files]");
			if (pos != string::npos)
				settings = contents.substr(0, pos + 12); //+12 is for the characters in "[Game Files]".
		}

		bool isSkyrim1426plus = (gl_current_game == SKYRIM && Version(GetExeDllVersion(data_path.parent_path() / "TESV.exe")) >= Version("1.4.26.0"));

		LOG_INFO("Writing new \"%s\"", file.string().c_str());
		ofstream outfile;
		outfile.open(file.c_str(), ios_base::trunc);
		if (outfile.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

		if (!settings.empty())
			outfile << settings << endl;  //Get those Morrowind settings back in.

		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == MOD) {
				if (activeOnly && (activePlugins.FindItem(items[i].Name()) == numActivePlugins || (isSkyrim1426plus && items[i].Name() == "Skyrim.esm")))
					continue;
				else if (!items[i].Exists())  //Only installed plugins should be written to the plugins.txt/loadorder.txt. The vector may contain others for user rule sorting purposes.
					continue;
				LOG_DEBUG("Writing \"%s\" to \"%s\"", items[i].Name().c_str(), file.string().c_str());
				if (gl_current_game == MORROWIND) //Need to write "GameFileN=" before plugin name, where N is an integer from 0 up.
					outfile << "GameFile" << i << "=";
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

	void		ItemList::EvalRegex() {
		//Store installed mods in a hashset. Case insensitivity not required as regex itself is case-insensitive.
		boost::unordered_set<string> hashset;
		boost::unordered_set<string>::iterator setPos;
		for (fs::directory_iterator itr(data_path); itr!=fs::directory_iterator(); ++itr) {
			const string ext = to_lower_copy(itr->path().extension().string());
			if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {	//Add file to hashset.
				if (ext == ".ghost")
					hashset.insert(itr->path().filename().stem().string());
				else
					hashset.insert(itr->path().filename().string());

			}
		}

		//Now iterate through items vector, working on regex entries.
		//First remove a regex entry, then look for matches in the hashset.
		//Add all matches with the messages attached to the regex entry to the items vector in the position the regex entry occupied.
		vector<Item>::iterator itemIter = items.begin();
		while (itemIter != items.end()) {
			if (itemIter->Type() == REGEX) {
				boost::regex reg;  //Form a regex.
				try {
					reg = boost::regex(itemIter->Name()+"(\\.ghost)?", boost::regex::extended|boost::regex::icase);  //Ghost extension is added so ghosted mods will also be found.
				} catch (boost::regex_error &e) {
					boss_error be = boss_error(BOSS_ERROR_REGEX_EVAL_FAIL, itemIter->Name());
					LOG_ERROR("\"%s\" is not a valid regular expression. Item skipped.", be.getString().c_str());
					errorBuffer = ParsingError(be.getString());
					++itemIter;
					continue;
				}
				vector<Message> messages = itemIter->Messages();
				itemIter = items.erase(itemIter);
				//Now start looking.
				setPos = FindRegexMatch(hashset, reg, hashset.begin());
				while (setPos != hashset.end()) {  //Now insert the current found mod in the position of the regex mod.
					itemIter = items.insert(itemIter, Item(*setPos, MOD, messages));
					setPos = FindRegexMatch(hashset, reg, ++setPos);
					++itemIter;
				}
			} else
				++itemIter;
		}
	}

	void		ItemList::ApplyMasterPartition() {
		//Need to iterate through items vector, sorting it according to the rule that master items come before other items.
		size_t lastMasterPos = GetLastMasterPos();
		size_t pos = GetNextMasterPos(lastMasterPos+1);
		while (pos < items.size()) {
			Item master = items[pos];
			items.erase(items.begin() + pos);
			items.insert(items.begin() + lastMasterPos + 1, master);
			++lastMasterPos;
			LOG_INFO("Master file \"%s\" moved before non-master plugins.", master.Name().c_str());
			pos = GetNextMasterPos(pos+1);
		}
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
		while (i < items.size() && (items[i].IsGroup() || items[i].IsMasterFile())) {  //SLLOOOOOWWWWW probably.
			i++;
		}
		if (i > 0)
			return i-1;  //i is position of first plugin.
		else 
			return 0;
	}

	size_t	ItemList::GetNextMasterPos(size_t currPos) const {
		if (currPos >= items.size())
			return items.size();
		while (currPos < items.size() && (items[currPos].IsGroup() || !items[currPos].IsMasterFile())) {  //SLLOOOOOWWWWW probably.
			currPos++;
		}
		return currPos;  //position of first master after currPos.
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

			RuleLine::RuleLine			(uint32_t inKey, string inObject) {
				key = inKey;
				object = inObject;
			}

	bool	RuleLine::IsObjectMessage	() const {
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

	Message RuleLine::ObjectAsMessage		() const {
		switch(object[0]) {
		case '?':
			return Message(SAY, object.substr(1));
		case '$':
			return Message(SAY, object.substr(1));
		case '^':
			return Message(SAY, object.substr(1));
		case '%':
			return Message(TAG, object.substr(1));
		case ':':
			return Message(REQ, object.substr(1));
		case '"':
			return Message(INC, object.substr(1));
		case '*':
			return Message(ERR, object.substr(1));
		default:
			size_t pos = object.find(':');
			if (pos == string::npos)
				return Message(NONE, "");

			string keyString = object.substr(0,pos);
			if (keyString == "SAY")
				return Message(SAY, object.substr(pos+1));
			else if (keyString == "TAG")
				return Message(TAG, object.substr(pos+1));
			else if (keyString == "REQ")
				return Message(REQ, object.substr(pos+1));
			else if (keyString == "INC")
				return Message(INC, object.substr(pos+1));
			else if (keyString == "DIRTY")
				return Message(DIRTY, object.substr(pos+1));
			else if (keyString == "WARN")
				return Message(WARN, object.substr(pos+1));
			else if (keyString == "ERROR")
				return Message(ERR, object.substr(pos+1));
			else 
				return Message(NONE, object.substr(pos+1));
		}
	}

	uint32_t RuleLine::Key() const {
		return key;
	}

	string RuleLine::Object() const {
		return object;
	}

	void RuleLine::Key(uint32_t inKey) {
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


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	void	Settings::Load			(fs::path file) {
		Skipper skipper;
		ini_grammar grammar;
		string::const_iterator begin, end;
		string contents;
		
		skipper.SkipIniComments(true);
		grammar.SetErrorBuffer(&errorBuffer);

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		
	//	iterator_type u32b(begin);
	//	iterator_type u32e(end);

	//	bool r = phrase_parse(u32b, u32e, grammar, skipper, iniSettings);
		bool r = phrase_parse(begin, end, grammar, skipper, iniSettings);

		if (!r || begin != end || !errorBuffer.Empty())  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());

		ApplyIniSettings();
	}

	void	Settings::Save			(fs::path file) {
		ofstream ini(file.c_str(), ios_base::trunc);
		if (ini.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		ini <<  '\xEF' << '\xBB' << '\xBF'  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			<<	"#---------------" << endl
			<<	"# BOSS Ini File" << endl
			<<	"#---------------" << endl
			<<	"# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'." << endl
			<<	"# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers." << endl
			<<	"# Settings with names starting with 's' are strings and their accepted values vary." << endl
			<<	"# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings." << endl << endl

			<<	"[General Settings]" << endl
			<<	"bDoStartupUpdateCheck    = " << BoolToString(gl_do_startup_update_check) << endl
			<<	"bUseUserRulesManager     = " << BoolToString(gl_use_user_rules_manager) << endl 
			<<	"sLanguage                = " << GetLanguageString() << endl << endl

			<<	"[Internet Settings]" << endl
			<<	"sProxyHostname           = " << gl_proxy_host << endl
			<<	"iProxyPort               = " << IntToString(gl_proxy_port) << endl
			<<	"sProxyUsername           = " << gl_proxy_user << endl
			<<	"sProxyPassword           = " << gl_proxy_passwd << endl << endl

			<<	"[Run Options]" << endl
			<<	"sGame                    = " << GetIniGameString(gl_game) << endl
			<<	"sLastGame                = " << GetIniGameString(gl_current_game) << endl  //Writing current game because that's what we want recorded when BOSS writes the ini.
			<<	"sBOSSLogFormat           = " << GetLogFormatString() << endl
			<<	"iDebugVerbosity          = " << IntToString(gl_debug_verbosity) << endl
			<<	"iRevertLevel             = " << IntToString(gl_revert) << endl
			<<	"bUpdateMasterlist        = " << BoolToString(gl_update) << endl
			<<	"bOnlyUpdateMasterlist    = " << BoolToString(gl_update_only) << endl
			<<	"bSilentRun               = " << BoolToString(gl_silent) << endl
			<<	"bDebugWithSourceRefs     = " << BoolToString(gl_debug_with_source) << endl
			<<	"bDisplayCRCs             = " << BoolToString(gl_show_CRCs) << endl
			<<	"bDoTrialRun              = " << BoolToString(gl_trial_run) << endl
			<<	"bLogDebugOutput          = " << BoolToString(gl_log_debug_output) << endl << endl;
		ini.close();
	}

	string	Settings::GetIniGameString	(uint32_t game) const {
		if (game == AUTODETECT)
			return "auto";
		else if (game == OBLIVION)
			return "Oblivion";
		else if (game == FALLOUT3)
			return "Fallout3";
		else if (game == NEHRIM)
			return "Nehrim";
		else if (game == FALLOUTNV)
			return "FalloutNV";
		else if (game == SKYRIM)
			return "Skyrim";
		else if (game == MORROWIND)
			return "Morrowind";
		else
			return "";
	}

	string	Settings::GetLanguageString	() const {
		if (gl_language == ENGLISH)
			return "english";
		else if (gl_language == SPANISH)
			return "spanish";
		else if (gl_language == GERMAN)
			return "german";
		else if (gl_language == RUSSIAN)
			return "russian";
		else
			return "";
	}

	string Settings::GetLogFormatString() const {
		if (gl_log_format == HTML)
			return "html";
		else
			return "text";
	}

	ParsingError Settings::ErrorBuffer() const {
		return errorBuffer;
	}

	void Settings::ErrorBuffer(ParsingError buffer) {
		errorBuffer = buffer;
	}

	void Settings::ApplyIniSettings() {
		for (map<string, string>::iterator iter = iniSettings.begin(); iter != iniSettings.end(); ++iter) {
			if (iter->second.empty())
				continue;

			//String settings.
			if (iter->first == "sProxyHostname")
				gl_proxy_host = iter->second;
			else if (iter->first == "sProxyUsername")
				gl_proxy_user = iter->second;
			else if (iter->first == "sProxyPassword")
				gl_proxy_passwd = iter->second;
			else if (iter->first == "sBOSSLogFormat") {
				if (iter->second == "html")
					gl_log_format = HTML;
				else
					gl_log_format = PLAINTEXT;
			} else if (iter->first == "sGame") {
				if (iter->second == "auto")
					gl_game = AUTODETECT;
				else if (iter->second == "Oblivion")
					gl_game = OBLIVION;
				else if (iter->second == "Nehrim")
					gl_game = NEHRIM;
				else if (iter->second == "Fallout3")
					gl_game = FALLOUT3;
				else if (iter->second == "FalloutNV")
					gl_game = FALLOUTNV;
				else if (iter->second == "Skyrim")
					gl_game = SKYRIM;
				else if (iter->second == "Morrowind")
					gl_game = MORROWIND;
			} else if (iter->first == "sLastGame") {
				if (iter->second == "auto")
					gl_last_game = AUTODETECT;
				else if (iter->second == "Oblivion")
					gl_last_game = OBLIVION;
				else if (iter->second == "Nehrim")
					gl_last_game = NEHRIM;
				else if (iter->second == "Fallout3")
					gl_last_game = FALLOUT3;
				else if (iter->second == "FalloutNV")
					gl_last_game = FALLOUTNV;
				else if (iter->second == "Skyrim")
					gl_last_game = SKYRIM;
				else if (iter->second == "Morrowind")
					gl_last_game = MORROWIND;
			} else if (iter->first == "sLanguage") {
				if (iter->second == "english")
					gl_language = ENGLISH;
				else if (iter->second == "spanish")
					gl_language = SPANISH;
				else if (iter->second == "german")
					gl_language = GERMAN;
				else if (iter->second == "russian")
					gl_language = RUSSIAN;
			}
			//Now integers.
			else if (iter->first == "iProxyPort")
				gl_proxy_port = atoi(iter->second.c_str());
			else if (iter->first == "iRevertLevel") {
				uint32_t value = atoi(iter->second.c_str());
				if (value >= 0 && value < 3)
					gl_revert = value;
			} else if (iter->first == "iDebugVerbosity") {
				uint32_t value = atoi(iter->second.c_str());
				if (value >= 0 && value < 4)
					gl_debug_verbosity = value;
			//Now on to boolean settings.
			} else if (iter->first == "bDoStartupUpdateCheck")
				gl_do_startup_update_check = StringToBool(iter->second);
			else if (iter->first == "bUseUserRulesEditor")
				gl_use_user_rules_manager = StringToBool(iter->second);
			if (iter->first == "bUpdateMasterlist")
				gl_update = StringToBool(iter->second);
			else if (iter->first == "bOnlyUpdateMasterlist")
				gl_update_only = StringToBool(iter->second);
			else if (iter->first == "bSilentRun")
				gl_silent = StringToBool(iter->second);
			else if (iter->first == "bDebugWithSourceRefs")
				gl_debug_with_source = StringToBool(iter->second);
			else if (iter->first == "bDisplayCRCs")
				gl_show_CRCs = StringToBool(iter->second);
			else if (iter->first == "bDoTrialRun")
				gl_trial_run = StringToBool(iter->second);
			else if (iter->first == "bLogDebugOutput")
				gl_log_debug_output = StringToBool(iter->second);


			//Don't evaluate whether these should have any effect here, as the game will not have been set yet.
			if (iter->first == "bUseMyGamesDirectory")
				gl_using_local_app_data_folder = StringToBool(iter->second);
			/*else if (iter->first == "sLocalMasterPath")
				//This setting does not seem to function correctly on Oblivion, and isn't used in Fallout 3, Fallout New Vegas and Skyrim.
				gl_local_data_path = fs::path(iter->second);  //Incorrect if absolute paths can be specified.*/
		}
	}
}