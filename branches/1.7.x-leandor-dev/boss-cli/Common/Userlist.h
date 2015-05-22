/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2003 $, $Date: 2010-12-06 00:44:09 +0000 (Mon, 06 Dec 2010) $
*/

//This is the start of implementing userlist and modlist classes into trunk. 
//Starting with only the modlist stuff.

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__


#include "Globals.h"
#include "Support/Helpers.h"

#include "Parsing.h"

#include <string>
#include <fstream>
#include <vector>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>


namespace boss {
	using namespace std;

	//Date comparison, used for sorting mods in modlist class.
	bool SortByDate(wstring mod1,wstring mod2);

	//Checks if a given object is an esp, an esm or a ghosted mod.
	bool IsPlugin(wstring object);
	
	//Class to store userlist rules.
	class Rules : public parsing::IRulesManager  {
	protected:
		virtual void AddRule(parsing::Rule const& rule);

		virtual void SyntaxError(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string const& what);

		virtual void ParsingFailed(
				string::const_iterator const& begin, 
				string::const_iterator const& end, 
				string::const_iterator const& error_pos, 
				string::difference_type lineNo);

	private:
		static const wstring FormatMesssage(wstring const& class_, wstring const& rule, wstring const& object, boost::wformat const& message);
		static const wstring FormatMesssage(wstring const& class_, boost::wformat const& message);

		void AddMessage(bool skipped, wstring const& rule, wstring const& object, boost::wformat const& message, wstring const& class_);
		void AddError(bool skipped, wstring const& rule, wstring const& object, boost::wformat const& message);
		void AddError(boost::wformat const& message);
	public:
		vector<wstring> keys,objects;			//Holds keys and objects for each rule line.
		vector<int> rules;						//Tells BOSS where each rule starts.
		wstring messages;						//Stores output messages.
		void AddRules();						//Populates object vectors with rules from userlist.txt.
		void PrintMessages(wofstream& output);	//Prints the output messages.
	};

	//Class to store and use modlist.
	class Mods {
	public:
		vector<wstring> mods;					//Stores the mods in your data folder.
		vector<vector<wstring>> modmessages;	//Stores the messages attached to each mod. First dimension matches up with the mods vector, then second lists the messages attached to that mod.
		void AddMods();							//Adds mods in directory to modlist in date order (AKA load order).
		int SaveModList();						//Save mod list to modlist.txt. Backs up old modlist.txt as modlist.old first.
		int GetModIndex(wstring mod);			//Look for a mod in the modlist, even if the mod in question is ghosted.
	};
}

#endif
