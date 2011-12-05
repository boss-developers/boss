/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2011    BOSS Development Team.

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

#ifndef __BOSS_CLASSES__
#define __BOSS_CLASSES__

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "Common/DllDef.h"
#include "Common/Globals.h"
#include "Common/Error.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	enum keyType : uint32_t {
		NONE,
		//RuleList keywords.
		ADD,
		OVERRIDE,
		FOR,
		BEFORE,
		AFTER,
		TOP,
		BOTTOM,
		APPEND,
		REPLACE,
		//Masterlist keywords.
		SAY,
		TAG,
		REQ,
		INC,
		DIRTY,
		WARN,
		ERR,
		//Legacy masterlist keywords.
		OOOSAY,
		BCSAY
	};

	enum itemType : uint32_t {
		MOD,
		BEGINGROUP,
		ENDGROUP,
		REGEX
	};

	class MasterlistVar {
	public:
		string var;
		string conditionals;
	};

	class Message {
	public:
			Message			();
			Message			(keyType inKey, string inData);
		
		string	KeyToString	();		//Has HTML-safe output.

		keyType key;
		string	data;
		string conditionals;
	};

	class Item {
	public:
				Item		();
				Item		(fs::path inName);
				Item		(fs::path inName, itemType inType);
				Item		(fs::path inName, itemType inType, vector<Message> inMessages);

		bool	operator<	(Item);		//Throws boss_error exception on fail.
		
		bool	IsPlugin	();
		bool	IsGroup		();
		bool	IsMasterFile();
		bool	IsGhosted	();			//Checks if the file exists in ghosted form.
		bool	Exists		();			//Checks if the file exists in data_path, ghosted or not.
		string	GetHeader	();			//Outputs the file's header.
		void	SetModTime	(time_t modificationTime);
		void	EvalConditionals(boost::unordered_set<string> setVars, boost::unordered_map<string,uint32_t> fileCRCs);

		vector<Message> messages;
		fs::path		name;			//Filename (or group name). Trimmed and case-preserved. ".ghost" extensions are removed.
		itemType		type;
		string			conditionals;
	};

	class ItemList {
	public:
		void					Load			(fs::path path);	//Load by scanning path. If path is a directory, it scans it for plugins. 
																	//If path is a file, it parses it using the modlist grammar.
																	//May throw exception on fail.
		void					Save			(fs::path file);	//Output to file in MF2. Backs up any existing file with new ".old" extension.
																	//Throws exception on fail.
		void					EvalConditionals();					//Evaluates the conditionals for each item, discarding those items whose conditionals evaluate to false. Also evaluates global message conditionals.
		vector<Item>::iterator	FindItem		(fs::path name);	//Find the position of the item with name 'name'. Case-insensitive.
		vector<Item>::iterator	FindLastItem	(fs::path name);	//Find the last item with the name 'name'. Case-insensitive.
		vector<Item>::iterator	FindGroupEnd	(fs::path name);	//Find the end position of the group with the given name. Case-insensitive.

		vector<Item>			items;
		ParsingError			errorBuffer;
		vector<Message>			globalMessageBuffer;
		vector<Item>::iterator	lastRecognisedPos;
		vector<MasterlistVar>	masterlistVariables;
		boost::unordered_map<string,uint32_t> fileCRCs;
	};
	
	class RuleLine {
	public:
				RuleLine			();
				RuleLine			(keyType inKey, string inObject);

		bool	IsObjectMessage		();
		keyType ObjectMessageKey	();
		string	ObjectMessageData	();
		string	KeyToString			();		//Has HTML-safe output.

		keyType key;
		string	object;
	};
	
	class Rule {
	public:
		string KeyToString	();  //Has HTML-safe output.

		bool				enabled;
		keyType				ruleKey;
		string				ruleObject;
		vector<RuleLine>	lines;
	};
	
	class RuleList {
	public:
		void 					Load	(fs::path file);		//Throws exception on fail.
		void 					Save	(fs::path file);		//Throws exception on fail.
		vector<Rule>::iterator 	FindRule(string ruleObject, bool onlyEnabled);

		vector<Rule>			rules;
		ParsingError			parsingErrorBuffer;
		vector<ParsingError>	syntaxErrorBuffer;
	};

	class Ini {
	public:
		void	Load(fs::path file);		//Throws exception on fail.
		void	Save(fs::path file);		//Throws exception on fail.
	
		ParsingError errorBuffer;
	private:
		string	GetGameString	();
		string	GetLogFormat	();
	};
}
#endif