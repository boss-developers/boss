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

#include <iostream>
#include <stdint.h>
#include <fstream>
#include <clocale>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>

#include "libloadorder.h"
#include "tester-interface.h"

using std::endl;

int main() {
	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	uint32_t vMajor, vMinor, vPatch;

	game_handle db;
	uint8_t * gamePath = reinterpret_cast<uint8_t *>("C:/Program Files (x86)/Steam/steamapps/common/oblivion");
	uint32_t game = LIBLO_GAME_TES4;
	uint32_t ret;

	uint32_t loMethod;
	const uint8_t * master = reinterpret_cast<uint8_t *>("Oblivion.esm");
	const uint8_t * plugin = reinterpret_cast<uint8_t *>("Unofficial Oblivion Patch.esp");
	uint8_t ** loadOrder;
	size_t len;
	size_t index;
	uint8_t * outPlugin;
	
	uint8_t ** activePlugins;
	bool active;

	std::ofstream out("libloadorder-tester.txt");

	//First test the library interface directly.


	out << "TESTING C LIBRARY INTERFACE" << endl;

	out << "TESTING IsCompatibleVersion(...)" << endl;
	bool b = IsCompatibleVersion(1,0,0);
	if (b)
		out << '\t' << "library is compatible." << endl;
	else {
		out << '\t' << "library is incompatible." << endl;
		exit(0);
	}

	out << "TESTING GetVersionNums(...)" << endl;
	GetVersionNums(&vMajor, &vMinor, &vPatch);
	out << '\t' << "Version: " << vMajor << '.' << vMinor << '.' << vPatch << endl;
	
	out << "TESTING CreateGameHandle(...)" << endl;
	ret = CreateGameHandle(&db, game, gamePath);
	if (ret != LIBLO_OK) 
		out << '\t' << "CreateGameHandle(...) failed. Error: " << ret << endl;
	else {

		out << "TESTING SetNonStandardGameMaster(...)" << endl;
		ret = SetNonStandardGameMaster(db, master);
		if (ret != LIBLO_OK)
			out << '\t' << "SetNonStandardGameMaster(...) failed. Error: " << ret << endl;
		else
			out << '\t' << "SetNonStandardGameMaster(...) successful." << endl;

		out << "TESTING GetLoadOrderMethod(...)" << endl;
		ret = GetLoadOrderMethod(db, &loMethod);
		if (ret != LIBLO_OK)
			out << '\t' << "GetLoadOrderMethod(...) failed. Error: " << ret << endl;
		else
			out << '\t' << "Load Order Method: " << loMethod << endl;

		out << "TESTING GetLoadOrder(...)" << endl;
		ret = GetLoadOrder(db, &loadOrder, &len);
		if (ret != LIBLO_OK)
			out << '\t' << "GetLoadOrder(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "List size: " << len << endl;
			for (size_t i=0; i<len; i++) {
				out << '\t' << '\t' << i << " : " << loadOrder[i] << endl;
			}
		}
				
		out << "TESTING SetLoadOrder(...)" << endl;
		ret = SetLoadOrder(db, loadOrder, len);
		if (ret != LIBLO_OK)
			out << '\t' << "SetLoadOrder(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "List size: " << len << endl;
			for (size_t i=0; i<len; i++) {
				out << '\t' << '\t' << i << " : " << loadOrder[i] << endl;
			}
		}
				
		out << "TESTING GetPluginLoadOrder(...)" << endl;
		ret = GetPluginLoadOrder(db, plugin, &index);
		if (ret != LIBLO_OK)
			out << '\t' << "GetPluginLoadOrder(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "\"" << plugin << "\" position: " << index << endl;
		}
				
		out << "TESTING SetPluginLoadOrder(...)" << endl;
		len = 1;
		ret = SetPluginLoadOrder(db, plugin, index);
		if (ret != LIBLO_OK)
			out << '\t' << "SetPluginLoadOrder(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "\"" << plugin << "\" set position: " << index << endl;
		}

		index++;

		out << "TESTING GetIndexedPlugin(...)" << endl;
		len = 10;
		ret = GetIndexedPlugin(db, index, &outPlugin);
		if (ret != LIBLO_OK)
			out << '\t' << "GetIndexedPlugin(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "Plugin at position " << index << " : " << outPlugin << endl;
		}

		out << "TESTING GetActivePlugins(...)" << endl;
		ret = GetActivePlugins(db, &activePlugins, &len);
		if (ret != LIBLO_OK)
			out << '\t' << "GetActivePlugins(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "List size: " << len << endl;
			for (size_t i=0; i<len; i++) {
				out << '\t' << '\t' << i << " : " << activePlugins[i] << endl;
			}
		}
				
		out << "TESTING SetActivePlugins(...)" << endl;
		ret = SetActivePlugins(db, activePlugins, len);
		if (ret != LIBLO_OK)
			out << '\t' << "SetActivePlugins(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "List size: " << len << endl;
			for (size_t i=0; i<len; i++) {
				out << '\t' << '\t' << i << " : " << activePlugins[i] << endl;
			}
		}
				
		out << "TESTING IsPluginActive(...)" << endl;
		ret = IsPluginActive(db, plugin, &active);
		if (ret != LIBLO_OK)
			out << '\t' << "IsPluginActive(...) failed. Error: " << ret << endl;
		else {
			out << '\t' << "\"" << plugin << "\" active status: " << active << endl;
		}

		out << "TESTING SetPluginActiveStatus(...)" << endl;
		ret = SetPluginActiveStatus(db, plugin, !active);
		if (ret != LIBLO_OK) {
			ret = GetLastErrorDetails(&outPlugin);
			if (ret != LIBLO_OK)
				out << '\t' << "GetLastErrorDetails(...) failed. Error: " << ret << endl;
			out << '\t' << "SetPluginActiveStatus(...) failed. Error: " << outPlugin << endl;
		} else {
			out << '\t' << "\"" << plugin << "\" active status: " << active << endl;
		}

		out << "TESTING GetLastErrorDetails(...)" << endl;
		ret = SetPluginActiveStatus(db, NULL, !active);
		if (ret != LIBLO_OK) {
			ret = GetLastErrorDetails(&outPlugin);
			if (ret != LIBLO_OK)
				out << '\t' << "GetLastErrorDetails(...) failed. Error: " << ret << endl;
			else
				out << '\t' << "SetPluginActiveStatus(...) failed. Error: " << outPlugin << endl;
		} else {
			out << '\t' << "\"" << plugin << "\" active status: " << active << endl;
		}

		out << "TESTING DestroyGameHandle(...)" << endl;
		DestroyGameHandle(db);
		out << "DestroyGameHandle(...) successful." << endl;
	}

	//Now let's test the C++ wrapper.
	std::string gamePathStr = "C:/Program Files (x86)/Steam/steamapps/common/oblivion";
	std::string masterStr = "Oblivion.esm";
	std::string pluginStr = "Unofficial Oblivion Patch.esp";
	std::vector<std::string> vec;
	boost::unordered_set<std::string> unord_set;


	out << "TESTING C++ WRAPPER INTERFACE" << endl;

	out << "TESTING IsCompatible(...)" << endl;
	if (tester::liblo::IsCompatible(1, 0, 0))
		out << '\t' << "library is compatible." << endl;
	else
		out << '\t' << "library is incompatible." << endl;

	out << "TESTING GetVersionNums(...)" << endl;
	tester::liblo::GetVersionNums(vMajor, vMinor, vPatch);
	out << '\t' << "Version: " << vMajor << '.' << vMinor << '.' << vPatch << endl;
	
	try {
		out << "TESTING GameHandle(...)" << endl;
		tester::liblo::GameHandle gh(game, gamePathStr);
		out << '\t' << "~GameHandle(...) successful." << endl;

		out << "TESTING SetGameMaster(...)" << endl;
		gh.SetGameMaster(masterStr);
		out << '\t' << "~SetGameMaster(...) successful." << endl;
		
		out << "TESTING LoadOrderMethod(...)" << endl;
		loMethod = gh.LoadOrderMethod();
		out << '\t' << "Load Order Method: " << loMethod << endl;

		out << "TESTING LoadOrder(...) (getter)" << endl;
		vec = gh.LoadOrder();
		out << '\t' << "List size: " << vec.size() << endl;
		for (size_t i=0, max=vec.size(); i < max; i++)
			out << '\t' << '\t' << i << " : " << vec[i] << endl;

		out << "TESTING LoadOrder(...) (setter)" << endl;
		gh.LoadOrder(vec);
		out << '\t' << "List size: " << vec.size() << endl;
		for (size_t i=0, max=vec.size(); i < max; i++)
			out << '\t' << '\t' << i << " : " << vec[i] << endl;

		out << "TESTING PluginLoadOrder(...) (getter)" << endl;
		index = gh.PluginLoadOrder(pluginStr);
		out << '\t' << "Position of plugin \"" << pluginStr << "\": " << index << endl;

		out << "TESTING PluginLoadOrder(...) (setter)" << endl;
		gh.PluginLoadOrder(pluginStr, index);
		out << '\t' << "Position of plugin \"" << pluginStr << "\": " << index << endl;

		index++;
		out << "TESTING PluginAtIndex(...)" << endl;
		pluginStr = gh.PluginAtIndex(index);
		out << '\t' << "Plugin at position " << index << ": \"" << pluginStr << "\"" << endl;

		out << "TESTING ActivePlugins(...) (getter)" << endl;
		unord_set = gh.ActivePlugins();
		out << '\t' << "List size: " << unord_set.size() << endl;
		size_t i = 0;
		for (boost::unordered_set<std::string>::iterator it=unord_set.begin(), endIt=unord_set.end(); it != endIt; ++it) {
			out << '\t' << '\t' << i << " : " << *it << endl;
			i++;
		}

		out << "TESTING ActivePlugins(...) (setter)" << endl;
		gh.ActivePlugins(unord_set);
		out << '\t' << "List size: " << unord_set.size() << endl;
		i = 0;
		for (boost::unordered_set<std::string>::iterator it=unord_set.begin(), endIt=unord_set.end(); it != endIt; ++it) {
			out << '\t' << '\t' << i << " : " << *it << endl;
			i++;
		}

		out << "TESTING IsPluginActive(...)" << endl;
		active = gh.IsPluginActive(pluginStr);
		out << '\t' << "Plugin \"" << pluginStr << "\" is active: " << active << endl;

		active = !active;
		out << "TESTING SetPluginActiveStatus(...)" << endl;
		gh.SetPluginActiveStatus(pluginStr, active);
		out << '\t' << "Plugin \"" << pluginStr << "\" set to active status: " << active << endl;

		out << "TESTING ~GameHandle(...)" << endl;
		gh.~GameHandle();
		out << '\t' << "~GameHandle(...) successful." << endl;

	} catch (tester::liblo::exception& e) {
		out << '\t'<< "Exception thrown." << endl
			<< '\t'<< '\t' << "Error code: " << e.code() << endl
			<< '\t'<< '\t' << "Error message: " << e.what() << endl;
	}


	out.close();
	return 0;
}