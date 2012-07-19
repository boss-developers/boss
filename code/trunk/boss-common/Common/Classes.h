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

#ifndef __BOSS_CLASSES__
#define __BOSS_CLASSES__

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/regex.hpp>
#include "Common/DllDef.h"
#include "Common/Error.h"
#include "Support/Helpers.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	BOSS_COMMON extern const uint32_t NONE;
	//RuleList keywords.
	BOSS_COMMON extern const uint32_t ADD;
	BOSS_COMMON extern const uint32_t OVERRIDE;
	BOSS_COMMON extern const uint32_t FOR;
	BOSS_COMMON extern const uint32_t BEFORE;
	BOSS_COMMON extern const uint32_t AFTER;
	BOSS_COMMON extern const uint32_t TOP;
	BOSS_COMMON extern const uint32_t BOTTOM;
	BOSS_COMMON extern const uint32_t APPEND;
	BOSS_COMMON extern const uint32_t REPLACE;
	//Masterlist keywords.
	BOSS_COMMON extern const uint32_t SAY;
	BOSS_COMMON extern const uint32_t TAG;
	BOSS_COMMON extern const uint32_t REQ;
	BOSS_COMMON extern const uint32_t INC;
	BOSS_COMMON extern const uint32_t DIRTY;
	BOSS_COMMON extern const uint32_t WARN;
	BOSS_COMMON extern const uint32_t ERR;

	//Item types.
	BOSS_COMMON extern const uint32_t MOD;
	BOSS_COMMON extern const uint32_t BEGINGROUP;
	BOSS_COMMON extern const uint32_t ENDGROUP;
	BOSS_COMMON extern const uint32_t REGEX;

	class BOSS_COMMON Game;


	//////////////////////////////
	// Masterlist Classes
	//////////////////////////////

	//Base class for all conditional data types.
	class BOSS_COMMON conditionalData {	
		friend struct boost::fusion::extension::access;
	private:
		string data;
		string conditions;
	public:
		conditionalData();
		conditionalData(const string inData, const string inConditions);

		string Data() const;
		string Conditions() const;

		void Data(const string inData);
		void Conditions(const string inConditions);

		bool EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, boost::unordered_set<string> activePlugins, bool &condResult, ParsingError& errorBuffer, const Game& parentGame);
	};

	class BOSS_COMMON MasterlistVar : public conditionalData {
		friend struct boost::fusion::extension::access;
	public:
		MasterlistVar();
		MasterlistVar(string inData, string inConditions);
	};

	class BOSS_COMMON Message : public conditionalData {
		friend struct boost::fusion::extension::access;
	private:
		uint32_t key;
	public:
		Message();
		Message(const uint32_t inKey, const string inData);

		uint32_t Key() const;
		void Key(const uint32_t inKey);

		string KeyToString() const;		//Has HTML-safe output.
	};

	class BOSS_COMMON Item : public conditionalData {
		friend struct boost::fusion::extension::access;
	private:
		vector<Message> messages;
		//string data is now filename (or group name). Trimmed and case-preserved. ".ghost" extensions are removed.
		uint32_t		type;
	public:

		Item		();
		Item		(const string inName);
		Item		(const string inName, const uint32_t inType);
		Item		(const string inName, const uint32_t inType, const vector<Message> inMessages);

		vector<Message> Messages() const;
		uint32_t Type() const;
		string Name() const;
		void Messages(const vector<Message> inMessages);
		void Type(const uint32_t inType);
		void Name(const string inName);

		bool	IsPlugin	() const;
		bool	IsGroup		() const;
		bool	IsGameMasterFile(const Game& parentGame) const;
		bool	IsMasterFile(const Game& parentGame) const;
		bool	IsFalseFlagged(const Game& parentGame) const;			//True if IsMasterFile does not match file extension.
		bool	IsGhosted	(const Game& parentGame) const;			//Checks if the file exists in ghosted form.
		bool	Exists		(const Game& parentGame) const;			//Checks if the file exists in the data folder, ghosted or not.
		Version	GetVersion	(const Game& parentGame) const;			//Outputs the file's header.
		time_t	GetModTime	(const Game& parentGame) const;			//Can throw exception.

		void	UnGhost		(const Game& parentGame) const;			//Can throw exception.
		void	SetModTime	(const Game& parentGame, const time_t modificationTime) const;			//Can throw exception.

		void InsertMessage(size_t pos, Message item);
		void ClearMessages();

		bool EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, boost::unordered_set<string> activePlugins, bool &condResult, ParsingError& errorBuffer, const Game& parentGame);
	};

	class BOSS_COMMON ItemList {
	private:
		vector<Item>			items;
		ParsingError			errorBuffer;
		vector<Message>			globalMessageBuffer;
		size_t					lastRecognisedPos;
		vector<MasterlistVar>	masterlistVariables;
		boost::unordered_map<string,uint32_t> fileCRCs;

		//Searches a hashset for the first matching string of a regex and returns its iterator position.
		boost::unordered_set<string>::iterator FindRegexMatch(const boost::unordered_set<string> set, const boost::regex reg, boost::unordered_set<string>::iterator startPos);
	public:
				ItemList();
		void	Load			(const Game& parentGame, const fs::path path);	//Load by scanning path. If path is a directory, it scans it for plugins. 
																	//If path is a file, it parses it using the modlist grammar.
																	//May throw exception on fail.
		void	Save			(const fs::path file, const fs::path oldFile);	//Output to file in MF2. Backs up any existing file to oldFile.
																	//Throws exception on fail.
		void	SavePluginNames(const Game& parentGame, const fs::path file, const bool activeOnly, const bool doEncodingConversion);	//Save only a list of plugin filenames to the given file. For use with Skyrim. Throws exception on fail.
		void	EvalConditions(const Game& parentGame);					//Evaluates the conditionals for each item, discarding those items whose conditionals evaluate to false. Also evaluates global message conditionals.
		void	EvalRegex(const Game& parentGame);
		void	ApplyMasterPartition(const Game& parentGame);				//Puts all master files before other plugins. Can throw exception.
		
		size_t	FindItem		(const string name, const uint32_t type) const;	//Find the position of the item with name 'name'. Case-insensitive.
		size_t	FindLastItem	(const string name, const uint32_t type) const;	//Find the last item with the name 'name'. Case-insensitive.
		size_t	GetLastMasterPos(const Game& parentGame) const;				 //Can throw exception.
		size_t	GetNextMasterPos(const Game& parentGame, size_t currPos) const; //Can throw exception.

		Item	ItemAt(size_t pos) const;

		vector<Item>							Items() const;
		ParsingError							ErrorBuffer() const;
		vector<Message>							GlobalMessageBuffer() const;
		size_t									LastRecognisedPos() const;
		vector<MasterlistVar>					Variables() const;
		boost::unordered_map<string,uint32_t>	FileCRCs() const;

		void	Items(const vector<Item> items);
		void	ErrorBuffer(const ParsingError buffer);
		void	GlobalMessageBuffer(const vector<Message> buffer);
		void	LastRecognisedPos(const size_t pos);
		void	Variables(const vector<MasterlistVar> variables);
		void	FileCRCs(const boost::unordered_map<string,uint32_t> crcs);
		
		void Clear();
		void Erase(const size_t pos);
		void Erase(const size_t startPos, const size_t endPos);
		void Insert(const size_t pos, const vector<Item> source, size_t sourceStart, size_t sourceEnd);
		void Insert(const size_t pos, const Item item);
		void Move(size_t newPos, const Item item);  //Adds the item if it isn't already present.
	};

	//////////////////////////////
	// Userlist Classes
	//////////////////////////////
	
	class BOSS_COMMON RuleLine {
		friend struct boost::fusion::extension::access;
	private:
		uint32_t key;
		string	object;
	public:
				RuleLine			();
				RuleLine			(const uint32_t inKey, const string inObject);

		bool	IsObjectMessage		() const;
		string	KeyToString			() const;		//Has HTML-safe output.
		Message ObjectAsMessage		() const;

		uint32_t Key() const;
		string Object() const;

		void Key(const uint32_t inKey);
		void Object(const string inObject);
	};

	class BOSS_COMMON Rule : public RuleLine {
		friend struct boost::fusion::extension::access;
	private:
		bool				enabled;
		vector<RuleLine>	lines;
	public:
		Rule();

		bool Enabled() const;
		vector<RuleLine> Lines() const;

		RuleLine LineAt(const size_t pos) const;

		void Enabled(const bool e);
		void Lines(const vector<RuleLine> inLines);
	};

	class BOSS_COMMON RuleList {
	private:
		vector<Rule>			rules;
		vector<ParsingError>	errorBuffer;

		void CheckSyntax(const Game& parentGame);  //Rule checker function, checks for syntax (not parsing) errors.
	public:
		RuleList();
		void 	Load	(const Game& parentGame, const fs::path file);		//Throws exception on fail.
		void	Save	(const fs::path file);		//Throws exception on fail.
		size_t	FindRule(const string ruleObject, const bool onlyEnabled) const;

		vector<Rule> Rules() const;
		vector<ParsingError> ErrorBuffer() const;

		Rule RuleAt(const size_t pos) const;
		
		void Rules(const vector<Rule> inRules);
		void ErrorBuffer(const vector<ParsingError>);

		void Erase(const size_t pos);
		void Insert(const size_t pos, const Rule rule);
		void Replace(const size_t pos, const Rule rule);

		void Clear();
	};


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	class BOSS_COMMON Settings {
	private:
		ParsingError errorBuffer;
		boost::unordered_map<string, string> iniSettings;

		string	GetIniGameString	(const uint32_t game) const;
		string	GetLogFormatString	() const;
		string	GetLanguageString	() const;
		void	ApplyIniSettings	();
	public:
		void	Load(const fs::path file);		//Throws exception on fail.
		void	Save(const fs::path file, const uint32_t currentGameId);		//Throws exception on fail.

		ParsingError ErrorBuffer() const;
		void ErrorBuffer(const ParsingError buffer);

		string GetValue(const string setting) const;
	};
}
#endif