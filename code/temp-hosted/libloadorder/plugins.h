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

#ifndef LIBLO_PLUGINS_H
#define LIBLO_PLUGINS_H

#include "exception.h"
#include <string>
#include <vector>
#include <stdint.h>
#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/unordered_set.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

struct Game;

namespace liblo {

	class Plugin {
		friend struct boost::fusion::extension::access;
	public:
		Plugin();
		Plugin(const std::string filename);  //Automatically trims .ghost extension.

		std::string Name() const;

		bool	IsValid		() const;  //.Checks if plugin is a .esp or .esm file.
		bool	IsMasterFile(const Game& parentGame) const;			//This should be implemented using libespm.
		bool	IsFalseFlagged(const Game& parentGame) const;			//True if IsMasterFile does not match file extension.
		bool	IsGhosted	(const Game& parentGame) const;			//Checks if the file exists in ghosted form.
		bool	Exists		(const Game& parentGame) const;			//Checks if the file exists in the data folder, ghosted or not.
		time_t	GetModTime	(const Game& parentGame) const;			//Can throw exception.

		void	UnGhost		(const Game& parentGame) const;			//Can throw exception.
		void	SetModTime	(const Game& parentGame, const time_t modificationTime) const;
		
		bool operator == (Plugin p);
		bool operator != (Plugin p);
	private:
		std::string name;
	};

	bool operator == (Plugin const& a, Plugin const& b);

	std::size_t hash_value(Plugin const& p);

	class LoadOrder : public std::vector<Plugin> {
	public:
		void Load(const Game& parentGame);  //Needs to contain no duplicates.
		void Save(Game& parentGame);  //Also updates mtime and active plugins list.
		
		bool IsValid(const Game& parentGame);  //Game master first, masters before plugins, plugins all exist.

		bool HasChanged(const Game& parentGame);  //Checks timestamp and also if LoadOrder is empty.

		void Move(size_t newPos, const Plugin plugin);

		size_t Find(const Plugin plugin) const;
		size_t LastMasterPos(const Game& parentGame) const;
	private:
		time_t mtime;

		//Assumes that the content of the file is valid.
		void LoadFromFile(const Game& parentGame, const boost::filesystem::path file);
	};

	class ActivePlugins : public boost::unordered_set<Plugin> {
	public:
		void Load(const Game& parentGame);
		void Save(const Game& parentGame);

		bool IsValid(const Game& parentGame);  //not more than 255 plugins active (254 for Skyrim), plugins all exist.

		bool HasChanged(const Game& parentGame);
	private:
		time_t mtime;
	};
}

#endif