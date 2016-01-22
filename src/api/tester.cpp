/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2011    BOSS Development Team.

	This file is part of BOSS.

	BOSS is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	BOSS is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with BOSS.  If not, see
	<http://www.gnu.org/licenses/>.

	$Revision: 1783 $, $Date: 2010-10-31 23:05:28 +0000 (Sun, 31 Oct 2010) $
*/

#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <locale>

#include <boost/filesystem.hpp>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>

#include "api/boss.h"
#include "base/fstream.h"

int main() {
	// Set the locale to get encoding conversions working correctly.
	std::setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	boss_db db;
	std::uint8_t *mPath = reinterpret_cast<std::uint8_t *>("Oblivion/masterlist.txt");
	std::uint8_t *uPath = reinterpret_cast<std::uint8_t *>("");
	std::uint8_t *gPath = reinterpret_cast<std::uint8_t *>("..");
	std::uint32_t game = BOSS_API_GAME_OBLIVION;

	const std::uint8_t *file = reinterpret_cast<std::uint8_t *>("minimal.txt");
	const std::uint8_t *cleanMod = reinterpret_cast<std::uint8_t *>("All Natural Base.esm");
	const std::uint8_t *doNotCleanMod = reinterpret_cast<std::uint8_t *>("bgBalancingEVLAMEAddition.esp");
	const std::uint8_t *inactiveMod = reinterpret_cast<std::uint8_t *>("français.esp");
	const std::uint8_t *messageMod = reinterpret_cast<std::uint8_t *>("PerkUP-5555.esp");
	const std::uint8_t *isActiveMod = reinterpret_cast<std::uint8_t *>("汉语漢語.esp");
	const std::uint8_t *info = reinterpret_cast<std::uint8_t *>("Testing the API's SubmitUnrecognisedPlugin function.");
	std::uint8_t **sortedPlugins;
	std::uint8_t **unrecPlugins;
	std::size_t len;
	std::uint8_t *message;
	std::size_t numTagsAdded, numTagsRemoved;
	bool modified;
	std::uint32_t *tagIDsAdded, *tagIDsRemoved;
	bool active;
	BashTag *BTmap;
	std::size_t size;
	std::uint32_t toClean;
	BossMessage *messages;

	std::uint32_t ret;
	boss::fstream::ofstream out("API test.txt");

	out << "TESTING IsCompatibleVersion(...)" << std::endl;
	bool b = IsCompatibleVersion(2, 1, 0);
	if (b) {
		out << '\t' << "API is compatible." << std::endl;
	} else {
		out << '\t' << "API is incompatible." << std::endl;
		std::exit(0);
	}

	out << "TESTING GetVersionString(...)" << std::endl;
	ret = GetVersionString(&message);
	if (ret != BOSS_API_OK)
		out << '\t' << "Failed to get version string. Error: " << ret << std::endl;
	else
		out << '\t' << "Version: " << *message << std::endl;

	out << "TESTING CreateBossDb(...)" << std::endl;
	ret = CreateBossDb(&db, game, NULL);
	if (ret != BOSS_API_OK) {
		out << '\t' << "Creation failed. Error: " << ret << std::endl;
	} else {
		out << "TESTING UpdateMasterlist(...)" << std::endl;
		ret = UpdateMasterlist(db, mPath);
		if (ret != BOSS_API_OK)
			out << '\t' << "Masterlist update failed. Error: " << ret << std::endl;

		out << "TESTING SubmitUnrecognisedPlugin(...)" << std::endl;
		ret = SubmitUnrecognisedPlugin(db, cleanMod, NULL, info);
		if (ret != BOSS_API_OK)
			out << '\t' << "SubmitUnrecognisedPlugin(...) failed. Error: " << ret << std::endl;

		out << "TESTING Load(...)" << std::endl;
		ret = Load(db, mPath, NULL);
		if (ret != BOSS_API_OK) {
			std::uint32_t ret2 = GetLastErrorDetails(&message);
			out << '\t' << "Loading failed. Error: " << ret << ", " << ret2 << ", " << message << std::endl;
		} else {
			out << "TESTING EvalConditionals(...)" << std::endl;
			ret = EvalConditionals(db);
			if (BOSS_API_OK != ret) {
				out << '\t' << "Conditional evaluation failed. Error: " << ret << std::endl;
			} else {
				out << "TESTING SortMods(...)" << std::endl;
				ret = SortMods(db, false, &sortedPlugins, &len, &unrecPlugins, &size);
				if (BOSS_API_OK != ret) {
					out << '\t' << "Sorting failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "List size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << std::endl;
					}
					out << '\t' << "Unrecognised plugin list size: " << size << std::endl;
					for (size_t i = 0; i < size; i++) {
						out << '\t' << '\t' << i << " : " << unrecPlugins[i] << std::endl;
					}
				}

				out << "TESTING GetDirtyMessage(...)" << std::endl;
				// This should give no dirty message.
				ret = GetDirtyMessage(db, cleanMod, &message, &toClean);
				if (ret != BOSS_API_OK) {
					out << '\t' << "Failed to get dirty info on \"" << cleanMod << "\". Error no " << ret << std::endl;
				} else {
					out << '\t' << "\"" << cleanMod << "\" clean status: " << toClean << std::endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << std::endl;
					}
				}

				out << "TESTING GetDirtyMessage(...)" << std::endl;
				// Now try getting dirty message from one that will have one.
				ret = GetDirtyMessage(db, doNotCleanMod, &message, &toClean);
				if (ret != BOSS_API_OK) {
					out << '\t' << "Failed to get dirty info on \"" << doNotCleanMod << "\". Error no " << ret << std::endl;
				} else {
					out << '\t' << "\"" << doNotCleanMod << "\" clean status: " << toClean << std::endl;
					if (message != NULL) {
						out << '\t' << "Message: " << message << std::endl;
					}
				}

				out << "TESTING DumpMinimal(...)" << std::endl;
				ret = DumpMinimal(db, file, true);
				if (ret != BOSS_API_OK)
					out << '\t' << "Dump failed. Error no " << ret << std::endl;

				out << "TESTING GetBashTagMap(...)" << std::endl;
				ret = GetBashTagMap(db, &BTmap, &size);
				if (ret != BOSS_API_OK) {
					out << '\t' << "Failed to get Bash Tag map. Error no " << ret << std::endl;
				} else {
					out << '\t' << "Tag map size: " << size << std::endl;
					out << '\t' << "Bash Tags:" << std::endl;
					for (std::size_t i = 0; i < size; i++)
						out << '\t' << '\t' << BTmap[i].id << " : " << BTmap[i].name << std::endl;
				}

				out << "TESTING GetModBashTags(...)" << std::endl;
				ret = GetModBashTags(db, doNotCleanMod, &tagIDsAdded, &numTagsAdded, &tagIDsRemoved, &numTagsRemoved, &modified);
				if (ret != BOSS_API_OK) {
					out << '\t' << "Failed to get Bash Tags for \"" << doNotCleanMod << "\". Error no " << ret << std::endl;
				} else {
					out << '\t' << "Tags for \"" << doNotCleanMod << "\":" << std::endl
						<< '\t' << '\t' << "Modified by userlist: " << modified << std::endl
						<< '\t' << '\t' << "Number of tags added: " << numTagsAdded << std::endl
						<< '\t' << '\t' << "Number of tags removed: " << numTagsRemoved << std::endl
						<< '\t' << '\t' << "Tags added:" << std::endl;
					for (std::size_t i = 0; i < numTagsAdded; i++)
						out << '\t' << '\t' << tagIDsAdded[i] << std::endl;
					out << '\t' << '\t' << "Tags removed:" << std::endl;
					for (std::size_t i = 0; i < numTagsRemoved; i++)
						out << '\t' << '\t' << tagIDsRemoved[i] << std::endl;
				}

				out << "TESTING GetLoadOrder(...)" << std::endl;
				ret = GetLoadOrder(db, &sortedPlugins, &len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "GetLoadOrder(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "List size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << std::endl;
					}
				}

				out << "TESTING SetLoadOrder(...)" << std::endl;
				ret = SetLoadOrder(db, sortedPlugins, len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "SetLoadOrder(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "List size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << std::endl;
					}
				}

				out << "TESTING GetActivePlugins(...)" << std::endl;
				ret = GetActivePlugins(db, &sortedPlugins, &len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "GetActivePlugins(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "List size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << std::endl;
					}
				}

				out << "TESTING SetActivePlugins(...)" << std::endl;
				ret = SetActivePlugins(db, sortedPlugins, len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "SetActivePlugins(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "List size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++) {
						out << '\t' << '\t' << i << " : " << sortedPlugins[i] << std::endl;
					}
				}

				out << "TESTING GetPluginLoadOrder(...)" << std::endl;
				ret = GetPluginLoadOrder(db, doNotCleanMod, &len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "GetPluginLoadOrder(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "\"" << doNotCleanMod << "\" position: " << len << std::endl;
				}

				out << "TESTING SetPluginLoadOrder(...)" << std::endl;
				len = 1;
				ret = SetPluginLoadOrder(db, doNotCleanMod, len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "SetPluginLoadOrder(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "\"" << doNotCleanMod << "\" set position: " << len << std::endl;
				}

				out << "TESTING GetIndexedPlugin(...)" << std::endl;
				len = 10;
				ret = GetIndexedPlugin(db, len, &message);
				if (BOSS_API_OK != ret) {
					out << '\t' << "GetIndexedPlugin(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "Plugin at position " << len << " : " << message << std::endl;
				}

				out << "TESTING GetPluginMessages(...)" << std::endl;
				ret = GetPluginMessages(db, messageMod, &messages, &len);
				if (BOSS_API_OK != ret) {
					out << '\t' << "GetIndexedPlugin(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "Message array size: " << len << std::endl;
					for (std::size_t i = 0; i < len; i++)
						out << '\t' << '\t' << messages[i].type << " : " << messages[i].message << std::endl;
				}

				// Assume there are at least 3 plugins.
				out << "TESTING SetPluginActive(...)" << std::endl;
				ret = SetPluginActive(db, sortedPlugins[2], true);
				if (BOSS_API_OK != ret) {
					out << '\t' << "SetPluginActive(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "\"" << sortedPlugins[2] << "\" activated." << std::endl;
				}

				// Assume there are at least 3 plugins.
				out << "TESTING IsPluginActive(...)" << std::endl;
				ret = IsPluginActive(db, sortedPlugins[2], &active);
				if (BOSS_API_OK != ret) {
					out << '\t' << "IsPluginActive(...) failed. Error: " << ret << std::endl;
				} else {
					out << '\t' << "\"" << sortedPlugins[2] << "\" active status: " << active << std::endl;
				}

				out << "TESTING IsPluginMaster(...)" << std::endl;
				ret = IsPluginMaster(db, cleanMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginMaster(...) failed. Error: " << ret << std::endl;
				else if (active)
					out << '\t' << "\"" << cleanMod << "\" is a master." << std::endl;
				else
					out << '\t' << "\"" << cleanMod << "\" is not a master." << std::endl;

				out << "TESTING IsPluginMaster(...)" << std::endl;
				ret = IsPluginMaster(db, doNotCleanMod, &active);
				if (BOSS_API_OK != ret)
					out << '\t' << "IsPluginMaster(...) failed. Error: " << ret << std::endl;
				else if (active)
					out << '\t' << "\"" << doNotCleanMod << "\" is a master." << std::endl;
				else
					out << '\t' << "\"" << doNotCleanMod << "\" is not a master." << std::endl;
			}
		}

		DestroyBossDb(db);
	}

	out.close();
	return 0;
}
