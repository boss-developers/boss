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
#include <map>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include "Common/DllDef.h"
#include "Common/Error.h"

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
		conditionalData(string inData, string inConditions);

		string Data() const;
		string Conditions() const;

		void Data(string inData);
		void Conditions(string inConditions);

		bool EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer);
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
		Message(uint32_t inKey, string inData);

		uint32_t Key() const;
		void Key(uint32_t inKey);

		string KeyToString() const;		//Has HTML-safe output.

		bool EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer);
	};

	class BOSS_COMMON Item : public conditionalData {
		friend struct boost::fusion::extension::access;
	private:
		vector<Message> messages;
		//string data is now filename (or group name). Trimmed and case-preserved. ".ghost" extensions are removed.
		uint32_t		type;
	public:

		Item		();
		Item		(string inName);
		Item		(string inName, uint32_t inType);
		Item		(string inName, uint32_t inType, vector<Message> inMessages);

		vector<Message> Messages() const;
		uint32_t Type() const;
		string Name() const;
		void Messages(vector<Message> inMessages);
		void Type(uint32_t inType);
		void Name(string inName);

		bool	operator <	(Item);		//Throws boss_error exception on fail. Timestamp comparison, not content.
		
		bool	IsPlugin	() const;
		bool	IsGroup		() const;
		bool	IsGameMasterFile() const;
		bool	IsMasterFile() const;
		bool	IsFalseFlagged() const;			//True if IsMasterFile does not match file extension.
		bool	IsGhosted	() const;			//Checks if the file exists in ghosted form.
		bool	Exists		() const;			//Checks if the file exists in gl_current_game.DataFolder(), ghosted or not.
		string	GetVersion	() const;			//Outputs the file's header.
		time_t	GetModTime	() const;			//Can throw exception.

		void	UnGhost		() const;			//Can throw exception.
		void	SetModTime	(time_t modificationTime) const;			//Can throw exception.

		void InsertMessage(size_t pos, Message item);
		void ClearMessages();

		bool EvalConditions(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs, ParsingError& errorBuffer);
	};

	class BOSS_COMMON ItemList {
	private:
		vector<Item>			items;
		ParsingError			errorBuffer;
		vector<Message>			globalMessageBuffer;
		size_t					lastRecognisedPos;
		vector<MasterlistVar>	masterlistVariables;
		boost::unordered_map<string,uint32_t> fileCRCs;
	public:

				ItemList();
		void	Load			(fs::path path);	//Load by scanning path. If path is a directory, it scans it for plugins. 
																	//If path is a file, it parses it using the modlist grammar.
																	//May throw exception on fail.
		void	Save			(fs::path file, fs::path oldFile);	//Output to file in MF2. Backs up any existing file to oldFile.
																	//Throws exception on fail.
		void	SavePluginNames(fs::path file, bool activeOnly, bool doEncodingConversion);	//Save only a list of plugin filenames to the given file. For use with Skyrim. Throws exception on fail.
		void	EvalConditions();					//Evaluates the conditionals for each item, discarding those items whose conditionals evaluate to false. Also evaluates global message conditionals.
		void	EvalRegex();
		void	ApplyMasterPartition();				//Puts all master files before other plugins. Can throw exception.
		
		size_t	FindItem		(string name) const;	//Find the position of the item with name 'name'. Case-insensitive.
		size_t	FindLastItem	(string name) const;	//Find the last item with the name 'name'. Case-insensitive.
		size_t	FindGroupEnd	(string name) const;	//Find the end position of the group with the given name. Case-insensitive.
		size_t	GetLastMasterPos() const;				 //Can throw exception.
		size_t	GetNextMasterPos(size_t currPos) const; //Can throw exception.

		Item	ItemAt(size_t pos) const;

		vector<Item>							Items() const;
		ParsingError							ErrorBuffer() const;
		vector<Message>							GlobalMessageBuffer() const;
		size_t									LastRecognisedPos() const;
		vector<MasterlistVar>					Variables() const;
		boost::unordered_map<string,uint32_t>	FileCRCs() const;

		void	Items(vector<Item> items);
		void	ErrorBuffer(ParsingError buffer);
		void	GlobalMessageBuffer(vector<Message> buffer);
		void	LastRecognisedPos(size_t pos);
		void	Variables(vector<MasterlistVar> variables);
		void	FileCRCs(boost::unordered_map<string,uint32_t> crcs);

		void Erase(size_t pos);
		void Erase(size_t startPos, size_t endPos);
		void Insert(size_t pos, vector<Item> source, size_t sourceStart, size_t sourceEnd);
		void Insert(size_t pos, Item item);
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
				RuleLine			(uint32_t inKey, string inObject);

		bool	IsObjectMessage		() const;
		string	KeyToString			() const;		//Has HTML-safe output.
		Message ObjectAsMessage		() const;

		uint32_t Key() const;
		string Object() const;

		void Key(uint32_t inKey);
		void Object(string inObject);
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

		RuleLine LineAt(size_t pos) const;

		void Enabled(bool e);
		void Lines(vector<RuleLine> inLines);
	};

	class BOSS_COMMON RuleList {
	private:
		vector<Rule>			rules;
		vector<ParsingError>	errorBuffer;

		void CheckSyntax();  //Rule checker function, checks for syntax (not parsing) errors.
	public:
		RuleList();
		void 	Load	(fs::path file);		//Throws exception on fail.
		void	Save	(fs::path file);		//Throws exception on fail.
		size_t	FindRule(string ruleObject, bool onlyEnabled) const;

		vector<Rule> Rules() const;
		vector<ParsingError> ErrorBuffer() const;

		Rule RuleAt(size_t pos) const;
		
		void Rules(vector<Rule> inRules);
		void ErrorBuffer(vector<ParsingError>);

		void Erase(size_t pos);
		void Insert(size_t pos, Rule rule);
		void Replace(size_t pos, Rule rule);

		void Clear();
	};


	///////////////////////////////
	//Settings Class
	///////////////////////////////

	class BOSS_COMMON Settings {
	private:
		ParsingError errorBuffer;
		map<string, string> iniSettings;

		string	GetIniGameString	(uint32_t game) const;
		string	GetLogFormatString	() const;
		string	GetLanguageString	() const;
		void	ApplyIniSettings	();
	public:
		void	Load(fs::path file);		//Throws exception on fail.
		void	Save(fs::path file);		//Throws exception on fail.

		ParsingError ErrorBuffer() const;
		void ErrorBuffer(ParsingError buffer);

		string GetValue(string setting) const;
	};
}
#endif