/*	libloadorder

	A library for reading and writing the load order of plugin files for
	TES III: Morrowind, TES IV: Oblivion, TES V: Skyrim, Fallout 3 and
	Fallout: New Vegas.

    Copyright (C) 2012    WrinklyNinja

	This file is part of libloadorder.

    libloadorder is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    libloadorder is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libloadorder.  If not, see 
	<http://www.gnu.org/licenses/>.
*/

#ifndef LIBLO_GAME_H
#define LIBLO_GAME_H

#include "plugins.h"
#include <string>
#include <vector>
#include <stdint.h>
#include <boost/filesystem.hpp>

	struct Game {
	public:
		Game(uint32_t id, std::string path);  //Currently missing desync correction.
		~Game();

		void SetMasterFile(std::string file);
	
		uint32_t Id() const;
		std::string MasterFile() const;
		uint32_t LoadOrderMethod() const;

		boost::filesystem::path PluginsFolder() const;
		boost::filesystem::path ActivePluginsFile() const;
		boost::filesystem::path LoadOrderFile() const;

		liblo::LoadOrder loadOrder;
		liblo::ActivePlugins activePlugins;

		uint8_t * extString;
		uint8_t ** extStringArray;

		size_t extStringArraySize;
	private:
		uint32_t id;
		uint32_t loMethod;

		std::string executable;
		std::string masterFile;

		std::string appdataFolderName;
		std::string pluginsFolderName;
		std::string pluginsFileName;

		boost::filesystem::path gamePath;
		boost::filesystem::path pluginsPath;
		boost::filesystem::path loadorderPath;

		boost::filesystem::path GetLocalAppDataPath() const;
	};

#endif