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

#include "libloadorder.h"
#include "helpers.h"
#include "game.h"
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <locale>

using namespace std;
using namespace liblo;

/*------------------------------
   Global variables
------------------------------*/

const uint32_t LIBLO_VERSION_MAJOR = 1;
const uint32_t LIBLO_VERSION_MINOR = 0;
const uint32_t LIBLO_VERSION_PATCH = 0;

uint8_t * extErrorString = NULL;


/*------------------------------
   Constants
------------------------------*/

/* The following are the possible codes that the library can return. */
LIBLO const uint32_t LIBLO_OK							= 0;
LIBLO const uint32_t LIBLO_WARN_BAD_FILENAME			= 1;
LIBLO const uint32_t LIBLO_WARN_LO_MISMATCH				= 2;
LIBLO const uint32_t LIBLO_ERROR_FILE_READ_FAIL			= 3;
LIBLO const uint32_t LIBLO_ERROR_FILE_WRITE_FAIL		= 4;
LIBLO const uint32_t LIBLO_ERROR_FILE_NOT_UTF8			= 5;
LIBLO const uint32_t LIBLO_ERROR_FILE_NOT_FOUND			= 6;
LIBLO const uint32_t LIBLO_ERROR_FILE_RENAME_FAIL		= 7;
LIBLO const uint32_t LIBLO_ERROR_TIMESTAMP_READ_FAIL	= 8;
LIBLO const uint32_t LIBLO_ERROR_TIMESTAMP_WRITE_FAIL	= 9;
LIBLO const uint32_t LIBLO_ERROR_FILE_PARSE_FAIL		= 10;
LIBLO const uint32_t LIBLO_ERROR_NO_MEM					= 11;
LIBLO const uint32_t LIBLO_ERROR_INVALID_ARGS			= 12;
LIBLO const uint32_t LIBLO_RETURN_MAX					= LIBLO_ERROR_INVALID_ARGS;

/* The following are for signifying what load order method is being used. */
LIBLO const uint32_t LIBLO_METHOD_TIMESTAMP				= 0;
LIBLO const uint32_t LIBLO_METHOD_TEXTFILE				= 1;

/* The following are the games identifiers used by the library. */
LIBLO const uint32_t LIBLO_GAME_TES3					= 1;
LIBLO const uint32_t LIBLO_GAME_TES4					= 2;
LIBLO const uint32_t LIBLO_GAME_TES5					= 3;
LIBLO const uint32_t LIBLO_GAME_FO3						= 4;
LIBLO const uint32_t LIBLO_GAME_FNV						= 5;


/*------------------------------
   Version Functions
------------------------------*/

/* Returns whether this version of libloadorder is compatible with the given
   version of libloadorder. */
LIBLO bool IsCompatibleVersion(const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch) {
	if (versionMajor == 1 && versionMinor == 0 && versionPatch == 0)
		return true;
	else
		return false;
}

LIBLO void GetVersionNums(uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch) {
	*versionMajor = LIBLO_VERSION_MAJOR;
	*versionMinor = LIBLO_VERSION_MINOR;
	*versionPatch = LIBLO_VERSION_PATCH;
}


/*----------------------------------
   Lifecycle Management Functions
----------------------------------*/

/* Creates a handle for the game given by gameId, which is found at gamePath. This handle allows
   clients to free memory when they want to. gamePath is case-sensitive if the underlying filesystem
   is case-sensitive. */
LIBLO uint32_t CreateGameHandle(game_handle * gh, const uint32_t gameId, const uint8_t * gamePath) {
	if (gh == NULL) //Check for valid args.
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();
	else if (gameId != LIBLO_GAME_TES3 && gameId != LIBLO_GAME_TES4 && gameId != LIBLO_GAME_TES5 && gameId != LIBLO_GAME_FO3 && gameId != LIBLO_GAME_FNV)
		return error(LIBLO_ERROR_INVALID_ARGS, "Invalid game specified.").code();

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Create handle.
	try {
		*gh = new Game(gameId, string(reinterpret_cast<const char *>(gamePath)));
	} catch (error& e) {
		return e.code();
	}

	if ((*gh)->LoadOrderMethod() == LIBLO_METHOD_TEXTFILE && boost::filesystem::exists((*gh)->ActivePluginsFile()) && boost::filesystem::exists((*gh)->LoadOrderFile())) {
		//Check for desync.
		LoadOrder PluginsFileLO;
		LoadOrder LoadOrderFileLO;

		try {
			//First get load order according to loadorder.txt.
			LoadOrderFileLO.Load(**gh);
			//Now temporarily rename loadorder.txt to loadorder.txt.bak, so that load order can be gotten from plugins.txt.
			boost::filesystem::rename((*gh)->LoadOrderFile(), (*gh)->LoadOrderFile().string() + ".bak");
		} catch (error& e) {
			delete *gh;
			return e.code();
		} catch (boost::filesystem::filesystem_error& e) {
			delete *gh;
			return error(LIBLO_ERROR_FILE_RENAME_FAIL, (*gh)->LoadOrderFile().string(), e.what()).code();
		}

		try {
			//Get load order from plugins.txt.
			PluginsFileLO.Load(**gh);
			//Now undo file rename.
			boost::filesystem::rename((*gh)->LoadOrderFile().string() + ".bak", (*gh)->LoadOrderFile());
		} catch (error& e) {
			delete *gh;
			return e.code();
		} catch (boost::filesystem::filesystem_error& e) {
			delete *gh;
			return error(LIBLO_ERROR_FILE_RENAME_FAIL, (*gh)->LoadOrderFile().string(), e.what()).code();
		}

		//Remove any plugins from LoadOrderFileLO that are not in PluginsFileLO.
		vector<Plugin>::iterator it=LoadOrderFileLO.begin(), endIt=LoadOrderFileLO.end(), pEndIt=PluginsFileLO.end();
		while (it != endIt) {
			if (PluginsFileLO.begin() + PluginsFileLO.Find(*it) == pEndIt)
				it = LoadOrderFileLO.erase(it);
			else
				++it;
		}

		//Compare the two LoadOrder objects: they should be identical (since mtimes for each have not been touched).
		if (PluginsFileLO == LoadOrderFileLO)
			return error(LIBLO_WARN_LO_MISMATCH).code();
	}

	return LIBLO_OK;
}

/* Destroys the given game handle, freeing up memory allocated during its use. */
LIBLO void DestroyGameHandle(game_handle gh) {
	delete gh;
}

/* Sets the game's master file to a given filename, eg. for use with total conversions where
   the original main master file is replaced. */
LIBLO uint32_t SetNonStandardGameMaster(game_handle gh, const uint8_t * masterFile) {
	if (gh == NULL || masterFile == NULL) //Check for valid args.
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	try {
		gh->SetMasterFile(string(reinterpret_cast<const char *>(masterFile)));
	} catch (error& e) {
		return e.code();
	}

	return LIBLO_OK;
}


/*------------------------------
   Error Handling Functions
------------------------------*/

/* Outputs a string giving the a message containing the details of the
   last error or warning encountered by a function called for the given
   game handle. */
LIBLO uint32_t GetLastErrorDetails(uint8_t ** details) {
	if (details == NULL)
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	//Free memory if in use.
	delete [] extErrorString;
	extErrorString = NULL;

	try {
		extErrorString = ToUint8_tString(lastException.what());
	} catch (bad_alloc /*&e*/) {
		return error(LIBLO_ERROR_NO_MEM).code();
	}

	*details = extErrorString;
	return LIBLO_OK;
}

LIBLO void CleanUpErrorDetails() {
	delete [] extErrorString;
	extErrorString = NULL;
}