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

#include "game.h"
#include "libloadorder.h"
#include "helpers.h"

#if _WIN32 || _WIN64
#	include "Windows.h"
#	include "Shlobj.h"
#endif

using namespace std;
using namespace liblo;

namespace fs = boost::filesystem;

	Game::Game(uint32_t gameId, string path) 
		: id(gameId), 
		gamePath(fs::path(path)),
		extString(NULL),
		extStringArray(NULL),
		extStringArraySize(0)
	{
		//Set game-specific data.
		if (id == LIBLO_GAME_TES3) {
			executable = "Morrwind.exe";
			masterFile = "Morrowind.esm";

			appdataFolderName = "";
			pluginsFolderName = "Data Files";
			pluginsFileName = "Morrowind.ini";
		} else if (id == LIBLO_GAME_TES4) {
			executable = "Oblivion.exe";
			masterFile = "Oblivion.esm";
			
			appdataFolderName = "Oblivion";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (id == LIBLO_GAME_TES5) {
			executable = "TESV.exe";
			masterFile = "Skyrim.esm";

			appdataFolderName = "Skyrim";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (id == LIBLO_GAME_FO3) {
			executable = "Fallout3.exe";
			masterFile = "Fallout3.esm";

			appdataFolderName = "Fallout3";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else if (id == LIBLO_GAME_FNV) {
			executable = "FalloutNV.exe";
			masterFile = "FalloutNV.esm";

			appdataFolderName = "FalloutNV";
			pluginsFolderName = "Data";
			pluginsFileName = "plugins.txt";
		} else
			throw error(LIBLO_ERROR_INVALID_ARGS, "Invalid game ID passed.");

		//Set active plugins and load order files.
		if (id == LIBLO_GAME_TES4 && fs::exists(gamePath / "Oblivion.ini")) {
			//Looking up bUseMyGamesDirectory, which only has effect if =0 and exists in Oblivion folder. Messy code, but one lookup hardly qualifies for a full ini parser to be included.
			string iniContent;
			string iniSetting = "bUseMyGamesDirectory=";
			fileToBuffer(gamePath / "Oblivion.ini", iniContent);

			size_t pos = iniContent.find(iniSetting);
			if (pos != string::npos && pos + iniSetting.length() < iniContent.length() && iniContent[pos + iniSetting.length()] == '0') {
				pluginsPath = gamePath / pluginsFileName;
				loadorderPath = gamePath / "loadorder.txt";
			} else {
				pluginsPath = GetLocalAppDataPath() / appdataFolderName / pluginsFileName;
				loadorderPath = GetLocalAppDataPath() / appdataFolderName / "loadorder.txt";
			}
		} else if (Id() == LIBLO_GAME_TES3) {
			pluginsPath = gamePath / pluginsFileName;
			loadorderPath = gamePath / "loadorder.txt";
		} else {
			pluginsPath = GetLocalAppDataPath() / appdataFolderName / pluginsFileName;
			loadorderPath = GetLocalAppDataPath() / appdataFolderName / "loadorder.txt";
		}

		//Set load order method.
		if (id == LIBLO_GAME_TES5 && Version(gamePath / executable) >= Version("1.4.26.0"))
			loMethod = LIBLO_METHOD_TEXTFILE;
		else
			loMethod = LIBLO_METHOD_TIMESTAMP;
	}

	Game::~Game() {
		delete[] extString;

		if (extStringArray != NULL) {
			for (size_t i=0; i < extStringArraySize; i++)
				delete[] extStringArray[i];  //Clear all the uint8_t strings created.
			delete[] extStringArray;  //Clear the string array.
		}
	}

	void Game::SetMasterFile(string file) {
		masterFile = file;
	}

	uint32_t Game::Id() const {
		return id;
	}

	string Game::MasterFile() const {
		return masterFile;
	}

	uint32_t Game::LoadOrderMethod() const {
		return loMethod;
	}

	boost::filesystem::path Game::PluginsFolder() const {
		return gamePath / pluginsFolderName;
	}

	boost::filesystem::path Game::ActivePluginsFile() const {
		return pluginsPath;
	}

	boost::filesystem::path Game::LoadOrderFile() const {
		return loadorderPath;
	}

	boost::filesystem::path Game::GetLocalAppDataPath() const {
#if _WIN32 || _WIN64
		HWND owner;
		TCHAR path[MAX_PATH];

		HRESULT res = SHGetFolderPath(owner, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

		if (res == S_OK)
			return fs::path(path);
		else
			return fs::path("");
#else
		return fs::path("");
#endif
	}