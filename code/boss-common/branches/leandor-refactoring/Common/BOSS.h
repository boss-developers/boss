/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_H__
#define __BOSS_H__

#include "Globals.h"
#include "Sorting.h"
#include "Updater.h"
#include "Masterlist.h"
#include "Userlist.h"
#include "Mods.h"

#include <vector>
#include <boost/filesystem.hpp>

namespace boss {

	using namespace std;
	namespace fs = boost::filesystem;

	// Names each one of the supported games.
	enum Game 
	{
		Unknown,
		Oblivion,
		Fallout,
		Morrowind
	};

	// The BOSS application
	class BOSS 
	{
	public:
		typedef vector<string> ArgumentList;

	public:

		class Options
		{
		public:
			//To update masterlist or not?
			bool update;
			
			//Enable parsing of mod's headers to look for version strings
			bool version_parse;
			
			//What game's mods are we sorting? 1 = Oblivion, 2 = Fallout 3, 3 = Morrowind.
			Game game;

			//What level to revert to?
			int revert;

		public:
			Options() 
			{
				update = false;
				version_parse = true;
				game = Game::Unknown;
				revert = 0;
			}
		};

	public:
		static const fs::path ROOT = ".\\BOSS";
		static const string LEGAL;

	public:
		Options options;

	public:
		// Initializes the global state of the BOSS program
		BOSS(int argc, char *argv[]) : arguments(argv[0], argv[argc])
		{
			ParseArguments(arguments);
		}

	private:
		void ParseArguments(const ArgumentList& args);
		Game DetectGame();

	private:
		ArgumentList arguments;
	};
}

#endif
