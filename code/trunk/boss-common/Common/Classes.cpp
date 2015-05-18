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
#include "Common/Game.h"
#include "Common/Globals.h"
#include "Output/Output.h"
#include "Support/Logger.h"
#include "Support/ModFormat.h"
#include "Parsing/Grammar.h"

#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	namespace loc = boost::locale;

	using boost::algorithm::to_lower_copy;

	//DO NOT CHANGE THESE VALUES. THEY MUST BE CONSTANT FOR API USERS.
	BOSS_COMMON const uint32_t NONE		= 0;
	//RuleList keywords.
	BOSS_COMMON const uint32_t ADD		= 1;
	BOSS_COMMON const uint32_t OVERRIDE	= 2;
	BOSS_COMMON const uint32_t FOR		= 3;
	BOSS_COMMON const uint32_t BEFORE	= 4;
	BOSS_COMMON const uint32_t AFTER	= 5;
	BOSS_COMMON const uint32_t TOP		= 6;
	BOSS_COMMON const uint32_t BOTTOM	= 7;
	BOSS_COMMON const uint32_t APPEND	= 8;
	BOSS_COMMON const uint32_t REPLACE	= 9;
	//Masterlist keywords.
	BOSS_COMMON const uint32_t SAY		= 10;
	BOSS_COMMON const uint32_t TAG		= 11;
	BOSS_COMMON const uint32_t REQ		= 12;
	BOSS_COMMON const uint32_t INC		= 13;
	BOSS_COMMON const uint32_t DIRTY	= 14;
	BOSS_COMMON const uint32_t WARN		= 15;
	BOSS_COMMON const uint32_t ERR		= 16;

	//Item types.
	BOSS_COMMON const uint32_t MOD			= 0;
	BOSS_COMMON const uint32_t BEGINGROUP	= 1;
	BOSS_COMMON const uint32_t ENDGROUP		= 2;
	BOSS_COMMON const uint32_t REGEX		= 3;

	/////////////////////////////////////
	// conditionalData Class Methods
	/////////////////////////////////////

	conditionalData::conditionalData() {
		data = "";
		conditions = "";
	}

	conditionalData::conditionalData(const string inData, const string inConditions) {
		data = inData;
		conditions = inConditions;
	}

	string conditionalData::Data() const {
		return data;
	}

	string conditionalData::Conditions() const {
		return conditions;
	}

	void conditionalData::Data(const string inData) {
		data = inData;
	}

	void conditionalData::Conditions(const string inConditions) {
		conditions = inConditions;
	}

	bool conditionalData::EvalConditions(boost::unordered_set<string>& setVars, boost::unordered_map<string,uint32_t>& fileCRCs, boost::unordered_set<string>& activePlugins, bool * condResult, ParsingError& errorBuffer, const Game& parentGame) {
		if (!conditions.empty()) {
			Skipper skipper;
			conditional_grammar grammar;
			string::const_iterator begin, end;
			bool eval;

			skipper.SkipIniComments(false);
			grammar.SetVarStore(&setVars);
			grammar.SetCRCStore(&fileCRCs);
			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetParentGame(&parentGame);
			grammar.SetActivePlugins(&activePlugins);
			grammar.SetLastConditionalResult(condResult);

			begin = conditions.begin();
			end = conditions.end();

		//	iterator_type u32b(begin);
		//	iterator_type u32e(end);

		//	bool r = phrase_parse(u32b, u32e, grammar, skipper, eval);
			bool r = phrase_parse(begin, end, grammar, skipper, eval);

			if (!r || begin != end)
				throw boss_error(BOSS_ERROR_CONDITION_EVAL_FAIL, conditions, data);

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

	Message::Message	()
		: conditionalData(), key(SAY) {}

	Message::Message	(const uint32_t inKey, const string inData)
		: conditionalData(inData, ""), key(inKey) {}

	uint32_t Message::Key() const {
		return key;
	}

	void Message::Key(const uint32_t inKey) {
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

	//////////////////////////////
	// Item Class Functions
	//////////////////////////////

	Item::Item			()
		: conditionalData(), type(MOD) {}

	Item::Item			(const string inName)
		: conditionalData(inName, ""), type(MOD) {}

	Item::Item			(const string inName, const uint32_t inType)
		: conditionalData(inName, ""), type(inType) {}

	Item::Item			(const string inName, const uint32_t inType, const vector<Message> inMessages)
		: conditionalData(inName, ""), type(inType), messages(inMessages) {}

	vector<Message> Item::Messages() const {
		return messages;
	}

	uint32_t Item::Type() const {
		return type;
	}

	string Item::Name() const {
		return Data();
	}

	void Item::Messages(const vector<Message> inMessages) {
		messages = inMessages;
	}

	void Item::Type(const uint32_t inType) {
		type = inType;
	}

	void Item::Name(const string inName) {
		Data(inName);
	}

	bool	Item::IsPlugin		() const {
		const string ext = boost::algorithm::to_lower_copy(fs::path(Data()).extension().string());
		return (ext == ".esp" || ext == ".esm");
	}

	bool	Item::IsGroup		() const {
		return (!fs::path(Data()).has_extension() && !Data().empty());
	}

	bool	Item::Exists		(const Game& parentGame) const {
		return (fs::exists(parentGame.DataFolder() / Data()) || fs::exists(parentGame.DataFolder() / fs::path(Data() + ".ghost")));
	}

	bool	Item::IsGameMasterFile	(const Game& parentGame) const {
		return boost::iequals(Data(), parentGame.MasterFile().Name());
	}

	bool	Item::IsMasterFile(const Game& parentGame) const {
		if (IsGhosted(parentGame))
			return IsPluginMaster(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
		else
			return IsPluginMaster(parentGame.DataFolder() / Data());
	}

	bool	Item::IsFalseFlagged(const Game& parentGame) const {
		string ext;
		if (IsGhosted(parentGame))
			ext = fs::path(Data()).stem().extension().string();
		else
			ext = fs::path(Data()).extension().string();
		return ((IsMasterFile(parentGame) && !boost::iequals(ext, ".esm")) || (!IsMasterFile(parentGame) && boost::iequals(ext, ".esm")));
	}

	bool	Item::IsGhosted		(const Game& parentGame) const {
		return (fs::exists(parentGame.DataFolder() / fs::path(Data() + ".ghost")));
	}

	Version	Item::GetVersion		(const Game& parentGame) const {
		if (!IsPlugin())
			return Version();

		ModHeader header;

		// Read mod's header now...
		if (IsGhosted(parentGame))
			header = ReadHeader(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
		else
			header = ReadHeader(parentGame.DataFolder() / Data());

		// The current mod's version if found, or empty otherwise.
		return Version(header.Version);
	}

	void	Item::SetModTime	(const Game& parentGame, const time_t modificationTime) const {
		try {
			if (IsGhosted(parentGame))
				fs::last_write_time(parentGame.DataFolder() / fs::path(Data() + ".ghost"), modificationTime);
			else
				fs::last_write_time(parentGame.DataFolder() / Data(), modificationTime);
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL, Data(), e.what());
		}
	}

	void	Item::UnGhost		(const Game& parentGame) const {			//Can throw exception.
		if (IsGhosted(parentGame)) {
			try {
				fs::rename(parentGame.DataFolder() / fs::path(Data() + ".ghost"), parentGame.DataFolder() / Data());
			} catch (fs::filesystem_error e) {
				throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, Data() + ".ghost", e.what());
			}
		}
	}

	time_t	Item::GetModTime	(const Game& parentGame) const {			//Can throw exception.
		try {
			if (IsGhosted(parentGame))
				return fs::last_write_time(parentGame.DataFolder() / fs::path(Data() + ".ghost"));
			else
				return fs::last_write_time(parentGame.DataFolder() / Data());
		} catch(fs::filesystem_error e) {
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, Data(), e.what());
		}
	}

	void Item::InsertMessage(const size_t pos, const Message message) {
		messages.insert(messages.begin()+pos, message);
	}

	void Item::ClearMessages() {
		messages.clear();
	}

	bool	Item::EvalConditions(boost::unordered_set<string>& setVars, boost::unordered_map<string,uint32_t>& fileCRCs, boost::unordered_set<string>& activePlugins, bool * condResult, ParsingError& errorBuffer, const Game& parentGame) {
		if (Type() == ENDGROUP)
			return true;

		LOG_TRACE("Evaluating conditions for item \"%s\"", Data().c_str());

		if (!conditionalData::EvalConditions(setVars, fileCRCs, activePlugins, condResult, errorBuffer, parentGame))  //Plugin needs to know what previous plugin's condition eval result was.
			return false;

		if (Type() == BEGINGROUP)
			return true;

		//Eval attached messages.
		if (!messages.empty()) {
			vector<Message>::iterator messageIter = messages.begin();
			bool res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL, errorBuffer, parentGame);  //No previous message for this plugin.
			if (res)
				++messageIter;
			else
				messageIter = messages.erase(messageIter);
			//Eval the rest of the messages now that res has been initialised.
			while (messageIter != messages.end()) {
				res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins, &res, errorBuffer, parentGame);
				if (res)
					++messageIter;
				else
					messageIter = messages.erase(messageIter);
			}
		}

		return true;
	}

	//////////////////////////////
	// ItemList Class Functions
	//////////////////////////////

	struct itemComparator {
		const Game& parentGame;
		itemComparator(const Game& game) : parentGame(game) {}

		bool	operator () (const Item item1, const Item item2) {
			//Return true if item1 goes before item2, false otherwise.
			//Master files should go before other files.
			//Groups should not change position (but master files should be able to cross groups).

			bool isItem1MasterFile = item1.IsMasterFile(parentGame);
			bool isItem2MasterFile = item2.IsMasterFile(parentGame);

			if (isItem1MasterFile && !isItem2MasterFile)
				return true;
			else if (parentGame.GetLoadOrderMethod() == LOMETHOD_TIMESTAMP) {
				if (!isItem1MasterFile && isItem2MasterFile)
					return false;
				else
					return (difftime(item1.GetModTime(parentGame), item2.GetModTime(parentGame)) < 0);
			} else
				return false;
		}
	};

			ItemList::ItemList			() : lastRecognisedPos(0) {}

	void	ItemList::Load				(const Game& parentGame, const fs::path path) {
		Clear();
		if (fs::exists(path) && fs::is_directory(path)) {
			LOG_DEBUG("Reading user mods...");
			size_t max;
			if (parentGame.GetLoadOrderMethod() == LOMETHOD_TEXTFILE) {
				/*Game uses the new load order system.

				Check if loadorder.txt exists, and read that if it does.
				If it doesn't exist, then read plugins.txt and scan the given directory for mods,
				adding those that weren't in the plugins.txt to the end of the load order, in the order they are read.

				There is no sure-fire way of managing such a situation. If no loadorder.txt, then
				no utilties compatible with that load order method have been installed, so it won't
				break anything apart from the load order not matching the load order in the Bashed
				Patch's Masters list if it exists. That isn't something that can be easily accounted
				for though.
				*/
				LOG_INFO("Using textfile-based load order mechanism.");
				if (fs::exists(parentGame.LoadOrderFile()))  //If the loadorder.txt exists, get the load order from that.
					Load(parentGame, parentGame.LoadOrderFile());
				else {
					if (fs::exists(parentGame.ActivePluginsFile()))  //If the plugins.txt exists, get the active load order from that.
						Load(parentGame, parentGame.ActivePluginsFile());
					if (parentGame.Id() == SKYRIM) {
						//Make sure that Skyrim.esm is first.
						Move(0, Item("Skyrim.esm"));
						//Add Update.esm if not already present.
						if (Item("Update.esm").Exists(parentGame) && FindItem("Update.esm", MOD) == items.size())
							Move(GetLastMasterPos(parentGame) + 1, Item("Update.esm"));
					}
				}
				//Then scan through loadorder, removing any plugins that aren't in the data folder.
				vector<Item>::iterator itemIter = items.begin();
				while (itemIter != items.end()) {
					if (!itemIter->Exists(parentGame))
						itemIter = items.erase(itemIter);
					else
						++itemIter;
				}
			}
			max = items.size();
			//Now scan through Data folder. Add any plugins that aren't already in loadorder to loadorder, at the end.
			for (fs::directory_iterator itr(path); itr!=fs::directory_iterator(); ++itr) {
				if (fs::is_regular_file(itr->status())) {
					fs::path filename = itr->path().filename();
					string ext = filename.extension().string();
					if (boost::iequals(ext, ".ghost")) {
						filename = filename.stem();
						ext = filename.extension().string();
					}
					if (boost::iequals(ext, ".esp") || boost::iequals(ext, ".esm")) {
						LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
						//Add file to modlist. If the filename has a '.ghost' extension, remove it.
						const Item tempItem = Item(filename.string());
						if (parentGame.GetLoadOrderMethod() == LOMETHOD_TIMESTAMP || (parentGame.GetLoadOrderMethod() == LOMETHOD_TEXTFILE && FindItem(tempItem.Name(), MOD) == max)) {  //If the plugin is not in loadorder, add it.
							items.push_back(tempItem);
							max++;
						}
					}
				}
			}
			LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", items.size());
			itemComparator ic(parentGame);
			sort(items.begin(),items.end(), ic);
		} else if (path == parentGame.LoadOrderFile() || path == parentGame.ActivePluginsFile()) {

			//loadorder.txt is simple enough that we can avoid needing the full modlist parser which has the crashing issue.
			//It's just a text file with a plugin filename on each line. Skip lines which are blank or start with '#'.

			//MCP Note: changed from path.c_str() to path.string(); needs testing as error was about not being able to convert wchar_t to char
			std::ifstream in(path.string());
			if (in.fail())
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());

			string line;
			while (in.good()) {
				getline(in, line);

				if (line.empty() || line[0] == '#')  //Character comparison is OK because it's ASCII.
					continue;

				if (path == parentGame.ActivePluginsFile())
                    line = From1252ToUTF8(line);
				items.push_back(Item(line));
			}
			in.close();


			//Then scan through items, removing any plugins that aren't in the data folder.
			vector<Item>::iterator itemIter = items.begin();
			while (itemIter != items.end()) {
				if (!itemIter->Exists(parentGame))
					itemIter = items.erase(itemIter);
				else
					++itemIter;
			}

			itemComparator ic(parentGame);
			sort(items.begin(),items.end(), ic);  //Does this work?
		} else {
			Skipper skipper;
			modlist_grammar grammar;
			string::const_iterator begin, end;
			string contents;

			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetGlobalMessageBuffer(&globalMessageBuffer);
			grammar.SetVarStore(&masterlistVariables);
			grammar.SetCRCStore(&fileCRCs);
			grammar.SetParentGame(&parentGame);

			if (!fs::exists(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());

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

	void	ItemList::Save				(const fs::path file, const fs::path oldFile) {
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

		//MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		ofile.open(file.string());
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

	void	ItemList::SavePluginNames(const Game& parentGame, const fs::path file, const bool activeOnly, const bool doEncodingConversion) {
		string badFilename = "",  contents;
		ItemList activePlugins;
		size_t numActivePlugins;
		if (activeOnly) {
			//To save needing a new parser, load plugins.txt into an ItemList then fill a hashset from that.
			//Also check if gl_current_game.ActivePluginsFile() then detect encoding if it is and translate outputted text from UTF-8 to the detected encoding.
			LOG_INFO("Loading plugins.txt into ItemList.");
			if (fs::exists(parentGame.ActivePluginsFile())) {
				activePlugins.Load(parentGame, parentGame.ActivePluginsFile());
				numActivePlugins = activePlugins.Items().size();
			}
		}

		LOG_INFO("Writing new \"%s\"", file.string().c_str());
		ofstream outfile;

		//MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		outfile.open(file.string(), ios_base::trunc);
		if (outfile.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());

		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == MOD) {
				if (activeOnly && (activePlugins.FindItem(items[i].Name(), MOD) == numActivePlugins || (parentGame.Id() == SKYRIM && items[i].Name() == "Skyrim.esm")))
					continue;
				LOG_DEBUG("Writing \"%s\" to \"%s\"", items[i].Name().c_str(), file.string().c_str());
				if (doEncodingConversion) {  //Not UTF-8.
					try {
                        outfile << FromUTF8To1252(items[i].Name()) << endl;
					} catch (boss_error /*&e*/) {
						badFilename = items[i].Name();
					}
				} else
					outfile << items[i].Name() << endl;
			}
		}
		outfile.close();

		if (!badFilename.empty())
			throw boss_error(BOSS_ERROR_ENCODING_CONVERSION_FAIL, badFilename, "1252");
	}

	void		ItemList::EvalConditions	(const Game& parentGame) {
		boost::unordered_set<string> setVars;
		boost::unordered_set<string> activePlugins;
		bool res;

		if (fs::exists(parentGame.ActivePluginsFile())) {
			ItemList active;
			active.Load(parentGame, parentGame.ActivePluginsFile());
			vector<Item> items = active.Items();
			for (size_t i=0, max=items.size(); i < max; i++) {
				activePlugins.insert(to_lower_copy(items[i].Name()));
			}
		}

		//First eval variables.
		//Need to convert these from a vector to an unordered set.
		if (!masterlistVariables.empty()) {
			LOG_INFO("Starting to evaluate variable conditionals.");
			vector<MasterlistVar>::iterator varIter = masterlistVariables.begin();
			res = varIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL, errorBuffer, parentGame);
			if (res) {
				setVars.insert(varIter->Data());
				++varIter;
			} else
				varIter = masterlistVariables.erase(varIter);
			//Eval the rest of the vars now that res has been initialised.
			while (varIter != masterlistVariables.end()) {
				res = varIter->EvalConditions(setVars, fileCRCs, activePlugins, &res, errorBuffer, parentGame);
				if (res) {
					setVars.insert(varIter->Data());
					++varIter;
				} else
					varIter = masterlistVariables.erase(varIter);
			}
		}

		//Now eval global messages.
		if (!globalMessageBuffer.empty()) {
			LOG_INFO("Starting to evaluate global message conditionals.");
			vector<Message>::iterator messageIter = globalMessageBuffer.begin();
			res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL, errorBuffer, parentGame);
			if (res)
				++messageIter;
			else
				messageIter = globalMessageBuffer.erase(messageIter);
			//Eval the rest of the global messages now that res has been initialised.
			while (messageIter != globalMessageBuffer.end()) {
				res = messageIter->EvalConditions(setVars, fileCRCs, activePlugins, &res, errorBuffer, parentGame);
				if (res)
					++messageIter;
				else
					messageIter = globalMessageBuffer.erase(messageIter);
			}
		}

		//Now eval items. Need to keep track of the previous item.
		LOG_INFO("Starting to evaluate item conditionals.");
		bool wasPlugin = false;
		vector<Item>::iterator itemIter = items.begin();
		while (itemIter != items.end()) {
			if (itemIter->Type() == MOD || itemIter->Type() == REGEX) {
				if (!wasPlugin)
					res = itemIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL, errorBuffer, parentGame);
				else
					res = itemIter->EvalConditions(setVars, fileCRCs, activePlugins, &res, errorBuffer, parentGame);  //Look at previous plugin's conditional eval result.
				if (res)
					++itemIter;
				else
					itemIter = items.erase(itemIter);
				wasPlugin = true;
			} else if (itemIter->Type() == BEGINGROUP) {
				if (itemIter->EvalConditions(setVars, fileCRCs, activePlugins, NULL, errorBuffer, parentGame))  //Don't need to record result as nothing will look at a previous group's conditional.
					++itemIter;
				else {
					//Need to remove all the plugins in the group.
					size_t endPos = FindLastItem(itemIter->Name(), ENDGROUP);
					itemIter = items.erase(itemIter, items.begin() + endPos + 1);
				}
				wasPlugin = false;
			} else {
				++itemIter;  //ENDGROUP items should not be conditional, so treat them like they're not.
				wasPlugin = false;
			}
		}
	}

	void		ItemList::EvalRegex(const Game& parentGame) {
		//Store installed mods in a hashset. Case insensitivity not required as regex itself is case-insensitive.
		boost::unordered_set<string> hashset;
		boost::unordered_set<string>::iterator setPos;
		for (fs::directory_iterator itr(parentGame.DataFolder()); itr!=fs::directory_iterator(); ++itr) {
			if (fs::is_regular_file(itr->status())) {
				fs::path filename = itr->path().filename();
				string ext = filename.extension().string();
				if (boost::iequals(ext, ".ghost")) {
					filename = filename.stem();
					ext = filename.extension().string();
				}
				if (boost::iequals(ext, ".esp") || boost::iequals(ext, ".esm")) {
					hashset.insert(filename.string());
				}
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
				} catch (boost::regex_error /*&e*/) {
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

	void		ItemList::ApplyMasterPartition(const Game& parentGame) {
		//Need to iterate through items vector, sorting it according to the rule that master items come before other items.
		size_t lastMasterPos = GetLastMasterPos(parentGame);
		size_t pos = GetNextMasterPos(parentGame, lastMasterPos+1);
		while (pos < items.size()) {
			Item master = items[pos];
			items.erase(items.begin() + pos);
			items.insert(items.begin() + lastMasterPos + 1, master);
			++lastMasterPos;
			LOG_INFO("Master file \"%s\" moved before non-master plugins.", master.Name().c_str());
			pos = GetNextMasterPos(parentGame, pos+1);
		}
	}

	size_t		ItemList::FindItem			(const string name, const uint32_t type) const {
		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (items[i].Type() == type && boost::iequals(items[i].Name(), name))
				return i;
		}
		return max;
	}

	size_t		ItemList::FindLastItem		(const string name, const uint32_t type) const {
		size_t max = items.size();
		for (vector<Item>::const_iterator it=items.end(), begin=items.begin(); it != begin; --it) {
			if (it->Type() == type && boost::iequals(it->Name(), name))
				return size_t(it - begin);
		}
		return max;
	}

	size_t		ItemList::GetLastMasterPos(const Game& parentGame) const {
		size_t i=0;
		while (i < items.size() && (items[i].IsGroup() || items[i].IsMasterFile(parentGame))) {  //SLLOOOOOWWWWW probably.
			i++;
		}
		if (i > 0)
			return i-1;  //i is position of first plugin.
		else
			return 0;
	}

	size_t	ItemList::GetNextMasterPos(const Game& parentGame, size_t currPos) const {
		if (currPos >= items.size())
			return items.size();
		while (currPos < items.size() && (items[currPos].IsGroup() || !items[currPos].IsMasterFile(parentGame))) {  //SLLOOOOOWWWWW probably.
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

	Item ItemList::ItemAt(const size_t pos) const {
		if (pos < items.size())
			return items[pos];
		else
			return Item();
	}

	void ItemList::Items(const vector<Item> inItems) {
		items = inItems;
	}

	void ItemList::ErrorBuffer(const ParsingError buffer) {
		errorBuffer = buffer;
	}

	void ItemList::GlobalMessageBuffer(const vector<Message> buffer) {
		globalMessageBuffer = buffer;
	}

	void ItemList::LastRecognisedPos(const size_t pos) {
		lastRecognisedPos = pos;
	}

	void ItemList::Variables(const vector<MasterlistVar> variables) {
		masterlistVariables = variables;
	}

	void ItemList::FileCRCs(const boost::unordered_map<string,uint32_t> crcs) {
		fileCRCs = crcs;
	}

	void ItemList::Clear() {
		items.clear();
		errorBuffer.Clear();
		globalMessageBuffer.clear();
		lastRecognisedPos = 0;
		masterlistVariables.clear();
		fileCRCs.clear();
	}

	void ItemList::Erase(const size_t pos) {
		items.erase(items.begin() + pos);
	}

	void ItemList::Erase(const size_t startPos, const size_t endPos) {
		items.erase(items.begin() + startPos, items.begin() + endPos);
	}

	void ItemList::Insert(const size_t pos, const vector<Item> source, const size_t sourceStart, const size_t sourceEnd) {
		items.insert(items.begin()+pos, source.begin()+sourceStart, source.begin()+sourceEnd);
	}

	void ItemList::Insert(const size_t pos, const Item item) {
		items.insert(items.begin()+pos, item);
	}

	void ItemList::Move(size_t newPos, const Item item) {
		size_t itemPos = FindItem(item.Name(), item.Type());
		if (itemPos == items.size())
			Insert(newPos, item);
		else {
			if (itemPos < newPos)
				newPos--;
			Erase(itemPos);
			Insert(newPos, item);
		}
	}

	//Searches a hashset for the first matching string of a regex and returns its iterator position. Usage internal to BOSS-Common.
	boost::unordered_set<string>::iterator ItemList::FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos) {
		while(startPos != set.end()) {
			if (boost::regex_match(*startPos, reg))
				break;
			++startPos;
		}
		return startPos;
	}

	//////////////////////////////
	// RuleLine Class Functions
	//////////////////////////////

			RuleLine::RuleLine			() {
				key = NONE;
				object = "";
			}

			RuleLine::RuleLine			(const uint32_t inKey, const string inObject) {
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

	void RuleLine::Key(const uint32_t inKey) {
		key = inKey;
	}

	void RuleLine::Object(const string inObject) {
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

	RuleLine Rule::LineAt(const size_t pos) const {
		if (pos == 0)
			return RuleLine(Key(), Object());  //Return sort line.
		else if (pos - 1 < lines.size())
			return lines[pos-1];
		else
			return RuleLine();
	}

	void Rule::Enabled(const bool e) {
		enabled = e;
	}

	void Rule::Lines(const vector<RuleLine> inLines) {
		lines = inLines;
	}


	//////////////////////////////
	// RuleList Class Functions
	//////////////////////////////

	RuleList::RuleList() {
		rules.clear();
		errorBuffer.clear();
	}

	void RuleList::Load(const Game& parentGame, const fs::path file) {
		Skipper skipper;
		userlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		Clear();

		skipper.SkipIniComments(false);
		grammar.SetErrorBuffer(&errorBuffer);

        if (!fs::exists(file)) {

			//MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
            ofstream userlist_file(file.string(), ios_base::binary);
            if (!userlist_file.fail())
                userlist_file << '\xEF' << '\xBB' << '\xBF';  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
            else
                throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
            userlist_file.close();
            return;
        }

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = phrase_parse(begin, end, grammar, skipper, rules);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());

		CheckSyntax(parentGame);
	}

	void RuleList::Save(const fs::path file) {

		//MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		ofstream outFile(file.string(),ios_base::trunc);

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

	void RuleList::CheckSyntax(const Game& parentGame) {
		// Loop through rules, check syntax of each. If a rule has invalid syntax, remove it.
		vector<Rule>::iterator it=rules.begin();
		while(it != rules.end()) {
			string ruleKeyString = it->KeyToString();
			Item ruleObject = Item(it->Object());
			try {
				if (ruleObject.IsPlugin()) {
					if (ruleKeyString != "FOR" && ruleObject.IsGameMasterFile(parentGame))
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingMasterEsm).str());
				} else {
					if (boost::iequals(ruleObject.Name(), "esms"))
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingGroupEsms).str());
					if (ruleKeyString == "ADD")
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAddingModGroup).str());
					else if (ruleKeyString == "FOR")
						throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingMessageToGroup).str());
				}
				vector<RuleLine> lines = it->Lines();
				bool hasSortLine = false, hasReplaceLine = false;
				for (size_t i=0, max=lines.size(); i<max; i++) {
					Item subject = Item(lines[i].Object());
					if (lines[i].Key() == BEFORE || lines[i].Key() == AFTER) {
						if (hasSortLine)
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleSortLines).str());
						if (i != 0)
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortNotSecond).str());
						if (ruleKeyString == "FOR")
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortLineInForRule).str());
						if (boost::iequals(ruleObject.Name(), subject.Name()))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingToItself).str());
						if ((ruleObject.IsPlugin() && !subject.IsPlugin()) || (!ruleObject.IsPlugin() && subject.IsPlugin()))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EReferencingModAndGroup).str());
						if (lines[i].Key() == BEFORE) {
							if (boost::iequals(subject.Name(), "esms"))
								throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingGroupBeforeEsms).str());
							else if (subject.IsGameMasterFile(parentGame))
								throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingModBeforeGameMaster).str());
							else if (!ruleObject.IsMasterFile(parentGame) && subject.IsMasterFile(parentGame))
								throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingPluginBeforeMaster).str());
						} else if (ruleObject.IsMasterFile(parentGame) && !subject.IsMasterFile(parentGame))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortingMasterAfterPlugin).str());
						hasSortLine = true;
					} else if (lines[i].Key() == TOP || lines[i].Key() == BOTTOM) {
						if (hasSortLine)
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleSortLines).str());
						if (i != 0)
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortNotSecond).str());
						if (ruleKeyString == "FOR")
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % ESortLineInForRule).str());
						if (lines[i].Key() == TOP && boost::iequals(subject.Name(), "esms"))
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EInsertingToTopOfEsms).str());
						if (!ruleObject.IsPlugin() || subject.IsPlugin())
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EInsertingGroupOrIntoMod).str());
						hasSortLine = true;
					} else if (lines[i].Key() == APPEND || lines[i].Key() == REPLACE) {
						if (!ruleObject.IsPlugin())
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingMessageToGroup).str());
						if (lines[i].Key() == REPLACE) {
							if (hasReplaceLine)
								throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EMultipleReplaceLines).str());
							if ((ruleKeyString == "FOR" && i != 0) || (ruleKeyString != "FOR" && i != 1))
								throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EReplaceNotFirst).str());
							hasReplaceLine = true;
						}
						if (!lines[i].IsObjectMessage())
							throw ParsingError((RuleListSyntaxErrorMessage % ruleKeyString % ruleObject.Name() % EAttachingNonMessage).str());
					}
				}
				++it;
			} catch (ParsingError & e) {
				it = rules.erase(it);
				errorBuffer.push_back(e);
				LOG_ERROR(Outputter(PLAINTEXT, e).AsString().c_str());
			}
		}
		return;
	}

	size_t RuleList::FindRule(const string ruleObject, const bool onlyEnabled) const {
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

	vector<ParsingError> RuleList::ErrorBuffer() const {
		return errorBuffer;
	}

	Rule RuleList::RuleAt(const size_t pos) const {
		if (pos < rules.size())
			return rules[pos];
		else
			return Rule();
	}

	void RuleList::Rules(const vector<Rule> inRules) {
		rules = inRules;
	}

	void RuleList::ErrorBuffer(const vector<ParsingError> buffer) {
		errorBuffer = buffer;
	}

	void RuleList::Clear() {
		rules.clear();
		errorBuffer.clear();
	}

	void RuleList::Erase(const size_t pos) {
		rules.erase(rules.begin() + pos);
	}

	void RuleList::Insert(const size_t pos, const Rule rule) {
		rules.insert(rules.begin()+pos, rule);
	}

	void RuleList::Replace(const size_t pos, const Rule rule) {
		if (pos < rules.size())
			rules[pos] = rule;
	}


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	void	Settings::Load			(const fs::path file) {
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

	void	Settings::Save			(const fs::path file, const uint32_t currentGameId) {

		//MCP Note: changed from file.c_str() to file.string(); needs testing as error was about not being able to convert wchar_t to char
		ofstream ini(file.string(), ios_base::trunc);
		if (ini.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
        ini << '\xEF' << '\xBB' << '\xBF'  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
            << "#---------------" << endl
            << "# BOSS Ini File" << endl
            << "#---------------" << endl
            << loc::translate("# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'.") << endl
            << loc::translate("# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers.") << endl
            << loc::translate("# Settings with names starting with 's' are strings and their accepted values vary.") << endl
            << loc::translate("# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings.") << endl << endl

            << "[General Settings]" << endl
            << "bUseUserRulesManager    = " << BoolToString(gl_use_user_rules_manager) << endl
            << "bCloseGUIAfterSorting   = " << BoolToString(gl_close_gui_after_sorting) << endl
            << "sLanguage               = " << GetLanguageString() << endl << endl

            << "[Run Options]" << endl
            << "sGame                   = " << GetIniGameString(gl_game) << endl
            << "sLastGame               = " << GetIniGameString(currentGameId) << endl  //Writing current game because that's what we want recorded when BOSS writes the ini.
            << "sBOSSLogFormat          = " << GetLogFormatString() << endl
            << "iDebugVerbosity         = " << IntToString(gl_debug_verbosity) << endl
            << "iRevertLevel            = " << IntToString(gl_revert) << endl
            << "bUpdateMasterlist       = " << BoolToString(gl_update) << endl
            << "bOnlyUpdateMasterlist   = " << BoolToString(gl_update_only) << endl
            << "bSilentRun              = " << BoolToString(gl_silent) << endl
            << "bDisplayCRCs            = " << BoolToString(gl_show_CRCs) << endl
            << "bDoTrialRun             = " << BoolToString(gl_trial_run) << endl << endl

            << "[Repository URLs]" << endl
            << "sOblivionRepoURL        = " << gl_oblivion_repo_url << endl
            << "sNehrimRepoURL          = " << gl_nehrim_repo_url << endl
            << "sSkyrimRepoURL          = " << gl_skyrim_repo_url << endl
            << "sFallout3RepoURL        = " << gl_fallout3_repo_url << endl
            << "sFalloutNVRepoURL       = " << gl_falloutnv_repo_url << endl;
		ini.close();
	}

	string	Settings::GetIniGameString	(const uint32_t game) const {
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
		else if (gl_language == SIMPCHINESE)
			return "chinese";
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

	void Settings::ErrorBuffer(const ParsingError buffer) {
		errorBuffer = buffer;
	}

	void Settings::ApplyIniSettings() {
		for (boost::unordered_map<string, string>::iterator iter = iniSettings.begin(); iter != iniSettings.end(); ++iter) {
			if (iter->second.empty())
				continue;

			//String settings.
			if (iter->first == "sBOSSLogFormat") {
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
            }
            else if (iter->first == "sLanguage") {
                if (iter->second == "english")
                    gl_language = ENGLISH;
                else if (iter->second == "spanish")
                    gl_language = SPANISH;
                else if (iter->second == "german")
                    gl_language = GERMAN;
                else if (iter->second == "russian")
                    gl_language = RUSSIAN;
                else if (iter->second == "chinese")
                    gl_language = SIMPCHINESE;
            }
            else if (iter->first == "sOblivionRepoURL")
                gl_oblivion_repo_url = iter->second;
            else if (iter->first == "sNehrimRepoURL")
                gl_nehrim_repo_url = iter->second;
            else if (iter->first == "sSkyrimRepoURL")
                gl_skyrim_repo_url = iter->second;
            else if (iter->first == "sFallout3RepoURL")
                gl_fallout3_repo_url = iter->second;
            else if (iter->first == "sFalloutNVRepoURL")
                gl_falloutnv_repo_url = iter->second;
			//Now integers.
			else if (iter->first == "iRevertLevel") {
				uint32_t value = atoi(iter->second.c_str());
				if (value >= 0 && value < 3)
					gl_revert = value;
            }
            else if (iter->first == "iDebugVerbosity") {
                uint32_t value = atoi(iter->second.c_str());
                if (value >= 0 && value < 4)
                    gl_debug_verbosity = value;
                //Now on to boolean settings.
            }
            else if (iter->first == "bUseUserRulesEditor")
				gl_use_user_rules_manager = StringToBool(iter->second);
			else if (iter->first == "bCloseGUIAfterSorting")
				gl_close_gui_after_sorting = StringToBool(iter->second);
			else if (iter->first == "bUpdateMasterlist")
				gl_update = StringToBool(iter->second);
			else if (iter->first == "bOnlyUpdateMasterlist")
				gl_update_only = StringToBool(iter->second);
			else if (iter->first == "bSilentRun")
				gl_silent = StringToBool(iter->second);
			else if (iter->first == "bDisplayCRCs")
				gl_show_CRCs = StringToBool(iter->second);
			else if (iter->first == "bDoTrialRun")
				gl_trial_run = StringToBool(iter->second);
		}
	}

	string Settings::GetValue(const string setting) const {
		boost::unordered_map<string, string>::const_iterator it = iniSettings.find(setting);
		if (it != iniSettings.end())
			return it->second;
		else
			return "";
	}
}