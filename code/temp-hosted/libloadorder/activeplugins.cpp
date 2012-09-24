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
#include "game.h"
#include "helpers.h"
#include "exception.h"

using namespace std;
using namespace liblo;

/*----------------------------------
   Plugin Active Status Functions
----------------------------------*/

/* Returns the list of active plugins. */
LIBLO uint32_t GetActivePlugins(game_handle gh, uint8_t *** plugins, size_t * numPlugins) {
	if (gh == NULL || plugins == NULL || numPlugins == NULL)
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	//Free memory if in use.
	if (gh->extStringArray != NULL) {
		for (size_t i=0; i < gh->extStringArraySize; i++)
			delete[] gh->extStringArray[i];  //Clear all the uint8_t strings created.
		delete[] gh->extStringArray;  //Clear the string array.
		gh->extStringArray = NULL;
		gh->extStringArraySize = 0;
	}
	
	//Update cache if necessary.
	try {
		if (gh->activePlugins.HasChanged(*gh))
			gh->activePlugins.Load(*gh);
	} catch (error& e) {
		return e.code();
	}
	
	//Check array size. Exit if zero.
	if (gh->activePlugins.empty())
		return LIBLO_OK;

	//Allocate memory.
	gh->extStringArraySize = gh->activePlugins.size();
	try {
		gh->extStringArray = new uint8_t*[gh->extStringArraySize];
		size_t i = 0;
		for (boost::unordered_set<Plugin>::iterator it = gh->activePlugins.begin(), endIt = gh->activePlugins.end(); it != endIt; ++it) {
			gh->extStringArray[i] = ToUint8_tString(it->Name());
			i++;
		}
	} catch(bad_alloc /*&e*/) {
		return error(LIBLO_ERROR_NO_MEM).code();
	}

	//Set outputs.
	*plugins = gh->extStringArray;
	*numPlugins = gh->extStringArraySize;

	return LIBLO_OK;
}

/* Replaces the current list of active plugins with the given list. */
LIBLO uint32_t SetActivePlugins(game_handle gh, uint8_t ** plugins, const size_t numPlugins) {
	if (gh == NULL || plugins == NULL)
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();
	
	//Put input into activePlugins object.
	gh->activePlugins.clear();
	for (size_t i=0; i < numPlugins; i++) {
		Plugin plugin(string(reinterpret_cast<const char *>(plugins[i])));
		if (gh->activePlugins.find(plugin) != gh->activePlugins.end()) {  //Not necessary for unordered set, but present so that invalid active plugin lists are refused.
			gh->activePlugins.clear();
			return error(LIBLO_ERROR_INVALID_ARGS, "The supplied active plugins list is invalid.").code();
		} else if (plugin.Exists(*gh))
			gh->activePlugins.emplace(plugin);
		else {
			gh->activePlugins.clear();
			return error(LIBLO_ERROR_FILE_NOT_FOUND, plugin.Name()).code();
		}
	}

	//Check to see if basic rules are being obeyed.
	if (!gh->activePlugins.IsValid(*gh)) {
		gh->activePlugins.clear();
		return error(LIBLO_ERROR_INVALID_ARGS, "Invalid active plugins list supplied.").code();
	}

	//Now save changes.
	try {
		gh->activePlugins.Save(*gh);
	} catch (error& e) {
		gh->activePlugins.clear();
		return e.code();
	}

	return LIBLO_OK;
}

/* Activates or deactivates the given plugin depending on the value of the active argument. */
LIBLO uint32_t SetPluginActiveStatus(game_handle gh, const uint8_t * plugin, const bool active) {
	if (gh == NULL || plugin == NULL)
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	Plugin pluginObj(string(reinterpret_cast<const char *>(plugin)));

	//Check that plugin exists if activating it.
	if (active && !pluginObj.Exists(*gh))
		return error(LIBLO_ERROR_FILE_NOT_FOUND, pluginObj.Name()).code();

	//Unghost plugin if ghosted.
	try {
		pluginObj.UnGhost(*gh);
	} catch (error& e) {
		return e.code();
	}
	
	//Update cache if necessary.
	try {
		if (gh->activePlugins.HasChanged(*gh))
			gh->activePlugins.Load(*gh);
	} catch (error& e) {
		return e.code();
	}

	//Look for plugin in active plugins list.
	boost::unordered_set<Plugin>::iterator it = gh->activePlugins.find(pluginObj);
	if (active)  //No need to check for duplication, unordered set will silently handle avoidance.
		gh->activePlugins.emplace(pluginObj);
	else if (!active && it != gh->activePlugins.end())
		gh->activePlugins.erase(it);

	//Check that active plugins list is valid.
	if (!gh->activePlugins.IsValid(*gh)) {
		gh->activePlugins.clear();
		return error(LIBLO_ERROR_INVALID_ARGS, "The operation results in an invalid active plugins list.").code();
	}

	//Now save changes.
	try {
		gh->activePlugins.Save(*gh);
	} catch (error& e) {
		gh->activePlugins.clear();
		return e.code();
	}

	return LIBLO_OK;
}

/* Checks to see if the given plugin is active. */
LIBLO uint32_t IsPluginActive(game_handle gh, const uint8_t * plugin, bool * result) {
	if (gh == NULL || plugin == NULL || result == NULL)
		return error(LIBLO_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	Plugin pluginObj(string(reinterpret_cast<const char *>(plugin)));

	//Update cache if necessary.
	try {
		if (gh->activePlugins.HasChanged(*gh))
			gh->activePlugins.Load(*gh);
	} catch (error& e) {
		return e.code();
	}

	if (gh->activePlugins.find(pluginObj) == gh->activePlugins.end())
		*result = false;
	else
		*result = true;

	return LIBLO_OK;
}