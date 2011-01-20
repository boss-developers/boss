/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

//This is the start of implementing userlist and modlist classes into trunk. 
//Starting with only the modlist stuff.

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__


#include "Globals.h"
#include "Support/Helpers.h"

#include <string>
#include <fstream>
#include <vector>
#include "boost/filesystem.hpp"


namespace boss {
	using namespace std;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(fs::path mod1, fs::path mod2);

	//Checks if a given object is an esp, an esm or a ghosted mod.
	bool IsPlugin(string object);
	
	//Checks if the plugin is in the Data directory, even if ghosted.
	bool PluginExists(fs::path plugin);

	//Class to store userlist rules.
	class Rules {
	public:
		vector<string> keys,objects;			//Holds keys and objects for each rule line.
		vector<int> rules;						//Tells BOSS where each rule starts.
		string messages;						//Stores output messages.
		void AddRules();						//Populates object vectors with rules from userlist.txt.
		void PrintMessages(ofstream& output);	//Prints the output messages.
	};

	//Class to store and use modlist.
	class Mods {
	public:
		vector<fs::path> mods;					//Stores the mods in your data folder.
		vector< vector<string> > modmessages;	//Stores the messages attached to each mod. First dimension matches up with the mods vector, then second lists the messages attached to that mod.
		void AddMods();							//Adds mods in directory to modlist in date order (AKA load order).
		int SaveModList();						//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int GetModIndex(fs::path mod);			//Look for a mod in the modlist, even if the mod in question is ghosted.
	};


	//Now a dumping ground for un-sorted testing code.
	//Try implementing a BOSSLogger function or something.
	enum attr {
		NO_ATTR = 0,
		BR = 1,
		START_DIV = 2,
		START_PARA = 3,
		END_DIV = 4,
		END_PARA = 5,
		END_LOG = 6
	};

	enum styleType {
		NO_STYLE = 0,
		TITLE = 1,
		LINK = 2,
		ERR = 3,
		SUCCESS = 4,
		WARN = 5,
		GHOST = 6,
		VER = 7,
		TAG = 8,
		SUBTITLE = 9,
		LI = 10
	};

	enum logFormat {
		HTML = 0,
		PLAINTEXT = 1
	};

	void printHTMLHead(ofstream& log);

	void printLogText(ofstream& log, string text, logFormat format, attr attribute, styleType style);
	void printLogText(ofstream& log, string text, string link, logFormat format, attr attribute, styleType style);
}

#endif
