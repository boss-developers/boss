/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
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
		Skipper skipper(false);
		conditional_grammar cond_grammar;
		string::const_iterator begin, end;
		bool eval;

		cond_grammar.SetVarStore(&setVars);
		cond_grammar.SetCRCStore(&fileCRCs);
		cond_grammar.SetErrorBuffer(&errorBuffer);
		
		if (!conditions.empty()) {
			begin = conditions.begin();
			end = conditions.end();

			bool r = phrase_parse(begin, end, cond_grammar, skipper, eval);
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
		Skipper skipper(false);
		shorthand_grammar short_grammar;
		string::const_iterator begin, end;
		string newMessage;
		bool eval;

		short_grammar.SetVarStore(&setVars);
		short_grammar.SetCRCStore(&fileCRCs);
		short_grammar.SetErrorBuffer(&errorBuffer);

		LOG_DEBUG("Evaluating conditional for message \"%s\"", Data().c_str());

		eval = conditionalData::EvalConditions(setVars, fileCRCs, errorBuffer);

		if (!eval)
			return false;
		else if (!Data().empty()) {
			//Now we must check if the message is using a conditional shorthand and evaluate that if so.
			short_grammar.SetMessageType(key);

			string da = Data();
			begin = da.begin();
			end = da.end();

			bool r = phrase_parse(begin, end, short_grammar, skipper, newMessage);
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
	
	bool	Item::IsMasterFile	() const {
		const string lower = boost::algorithm::to_lower_copy(Data());
		return (lower == "oblivion.esm" || lower == "fallout3.esm" || lower == "nehrim.esm" || lower == "falloutnv.esm" || lower == "skyrim.esm");
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

	bool	Item::operator <	(Item item2) {
		time_t t1 = 0,t2 = 0;
		try {
			if (this->IsGhosted())
				t1 = fs::last_write_time(data_path / fs::path(this->Data() + ".ghost"));
			else
				t1 = fs::last_write_time(data_path / this->Data());
			if (item2.IsGhosted())
				t2 = fs::last_write_time(data_path / fs::path(item2.Data() + ".ghost"));
			else
				t2 = fs::last_write_time(data_path / item2.Data());
		}catch (fs::filesystem_error e){
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, this->Data() + "\" or \"" + item2.Data(),e.what());
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		return (diff < 0);
	}

	bool	Item::EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer) {
		Skipper skipper(false);
		conditional_grammar cond_grammar;
		string::const_iterator begin, end;

		cond_grammar.SetErrorBuffer(&errorBuffer);
		cond_grammar.SetVarStore(&setVars);
		cond_grammar.SetCRCStore(&fileCRCs);
		
		LOG_INFO("Evaluating conditions for item \"%s\"", Data().c_str());

		bool eval= conditionalData::EvalConditions(setVars, fileCRCs, errorBuffer);

		vector<Message>::iterator messageIter = messages.begin();
		while (messageIter != messages.end()) {
			if (messageIter->EvalConditions(setVars, fileCRCs, errorBuffer))
				++messageIter;
			else
				messageIter = messages.erase(messageIter);
		}

		return true;
	}

	//////////////////////////////
	// ItemList Class Functions
	//////////////////////////////

							ItemList::ItemList					() {
		items.clear();
		errorBuffer = ParsingError();
		globalMessageBuffer.clear();
		lastRecognisedPos = 0;
		masterlistVariables.clear();
		fileCRCs.clear();
	}

	void					ItemList::Load				(fs::path path) {
		if (fs::exists(path) && fs::is_directory(path)) {
			LOG_DEBUG("Reading user mods...");
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
		} else {
			Skipper skipper(false);
			modlist_grammar grammar;
			string::const_iterator begin, end;
			string contents;

			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetGlobalMessageBuffer(&globalMessageBuffer);
			grammar.SetVarStore(&masterlistVariables);
			grammar.SetCRCStore(&fileCRCs);

			if (!fs::exists(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());
			else if (!ValidateUTF8File(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			fileToBuffer(path,contents);

			begin = contents.begin();
			end = contents.end();
			bool r = phrase_parse(begin, end, grammar, skipper, items);

			if (!r || begin != end)
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());
		}
	}
	
	void					ItemList::Save				(fs::path file, fs::path oldFile) {
		ofstream ofile;
		//Back up file if it already exists.
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(file)) 
				fs::rename(file, oldFile);
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

	void					ItemList::EvalConditions	() {
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
	
	size_t					ItemList::FindItem			(string name) const {
		size_t max = items.size();
		for (size_t i=0; i < max; i++) {
			if (Tidy(items[i].Name()) == Tidy(name))
				return i;
		}
		return max;
	}

	size_t					ItemList::FindLastItem		(string name) const {
		size_t max = items.size();
		for (size_t i=max-1; i >= 0; i--) {
			if (Tidy(items[i].Name()) == Tidy(name))
				return i;
		}
		return max;
	}
	
	//This looks a bit weird, but I need a non-reverse iterator outputted, and searching backwards is probably more efficient for my purposes.
	size_t					ItemList::FindGroupEnd		(string name) const {
		size_t max = items.size();
		for (size_t i=max-1; i >= 0; i--) {
			if (items[i].Type() == ENDGROUP && Tidy(items[i].Name()) == Tidy(name))
				return i;
		}
		return max;
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
		Skipper skipper(false);
		userlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

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
				if (Tidy(rules[i].Object()) == Tidy(ruleObject))
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