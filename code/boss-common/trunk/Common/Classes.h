/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011 BOSS Development Team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#ifndef __BOSS_CLASSES__
#define __BOSS_CLASSES__

#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include "Common/DllDef.h"
#include "Common/Globals.h"
#include "Common/Error.h"

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;

	enum keyType {
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

	enum itemType {
		MOD,
		BEGINGROUP,
		ENDGROUP,
		REGEX
	};

	class Message {
	public:
			Message			();
			Message			(keyType inKey, string inData);
		
		string	KeyToString	();		//Has HTML-safe output.

		keyType key;
		string	data;
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

		vector<Message> messages;
		fs::path		name;			//Filename (or group name). Trimmed and case-preserved. ".ghost" extensions are removed.
		itemType		type;
	};

	class ItemList {
	public:
		void					Load			(fs::path path);	//Load by scanning path. If path is a directory, it scans it for plugins. 
																	//If path is a file, it parses it using the modlist grammar.
																	//May throw exception on fail.
		void					Save			(fs::path file);	//Output to file in MF2. Backs up any existing file with new ".old" extension.
																	//Throws exception on fail.
		vector<Item>::iterator	FindItem		(fs::path name);	//Find the position of the item with name 'name'. Case-insensitive.
		vector<Item>::iterator	FindLastItem	(fs::path name);	//Find the last item with the name 'name'. Case-insensitive.
		vector<Item>::iterator	FindGroupEnd	(fs::path name);	//Find the end position of the group with the given name. Case-insensitive.

		vector<Item>			items;
		ParsingError			errorBuffer;
		vector<Message>			globalMessageBuffer;
		vector<Item>::iterator	lastRecognisedPos;
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
		void Load	(fs::path file);		//Throws exception on fail.
		void Save	(fs::path file);		//Throws exception on fail.

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