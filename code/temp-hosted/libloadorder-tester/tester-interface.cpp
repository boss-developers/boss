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

#include "tester-interface.h"

namespace tester {

	namespace liblo {

		/*------------------------------
		   Version Functions
		------------------------------*/

		bool IsCompatible(const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch) {
			return ::IsCompatibleVersion(versionMajor, versionMinor, versionPatch);
		}

		void GetVersionNums(uint32_t& versionMajor, uint32_t& versionMinor, uint32_t& versionPatch) {
			::GetVersionNums(&versionMajor, &versionMinor, &versionPatch);
		}


		/*------------------------------
		   Error Handling Functions
		------------------------------*/

		exception::exception() : errCode(0) {}

		exception::exception(uint32_t code, std::string message) : errCode(0), errMessage(message) {}

		uint32_t exception::code() {
			return errCode;
		}

		std::string exception::what() {
			return errMessage;
		}


		/*----------------------------------
		   Game Handle Based Functions
		----------------------------------*/

		GameHandle::GameHandle(const uint32_t gameId, const std::string gamePath) {
			uint8_t * p = ToUint8_tString(gamePath);
			Handler(::CreateGameHandle(&gh, gameId, p), p);
		}

		GameHandle::~GameHandle() {
			::DestroyGameHandle(gh);
			gh = NULL;
		}

		void GameHandle::SetGameMaster(const std::string filename) {
			uint8_t * p = ToUint8_tString(filename);
			Handler(::SetNonStandardGameMaster(gh, p), p);
		}

		uint32_t GameHandle::LoadOrderMethod() {
			uint32_t method;
			Handler(::GetLoadOrderMethod(gh, &method));
			return method;
		}

		std::vector<std::string> GameHandle::LoadOrder() {
			uint8_t ** pluginArray;
			size_t arrSize;
			std::vector<std::string> pluginVector;
			Handler(::GetLoadOrder(gh, &pluginArray, &arrSize));
			for (size_t i=0; i<arrSize; i++)
				pluginVector.push_back(ToStdString(pluginArray[i]));
			return pluginVector;
		}

		void GameHandle::LoadOrder(const std::vector<std::string>& newLoadOrder) {
			size_t arrSize = newLoadOrder.size();
			uint8_t ** pluginArray = new uint8_t*[arrSize];
			for (size_t i=0; i < arrSize; i++)
				pluginArray[i] = ToUint8_tString(newLoadOrder[i]);
			Handler(::SetLoadOrder(gh, pluginArray, arrSize), pluginArray, arrSize);
		}

		size_t GameHandle::PluginLoadOrder(const std::string plugin) {
			size_t index;
			uint8_t * p = ToUint8_tString(plugin);
			Handler(::GetPluginLoadOrder(gh, p, &index), p);
			return index;
		}

		void GameHandle::PluginLoadOrder(const std::string plugin, const size_t index) {
			uint8_t * p = ToUint8_tString(plugin);
			Handler(::SetPluginLoadOrder(gh, p, index), p);
		}

		std::string GameHandle::PluginAtIndex(const size_t index) {
			uint8_t * plugin;
			Handler(::GetIndexedPlugin(gh, index, &plugin));
			return ToStdString(plugin);
		}

		boost::unordered_set<std::string> GameHandle::ActivePlugins() {
			uint8_t ** pluginArray;
			size_t arrSize;
			boost::unordered_set<std::string> pluginSet;
			Handler(::GetActivePlugins(gh, &pluginArray, &arrSize));
			for (size_t i=0; i<arrSize; i++)
				pluginSet.emplace(ToStdString(pluginArray[i]));
			return pluginSet;
		}

		void GameHandle::ActivePlugins(const boost::unordered_set<std::string>& newActivePlugins) {
			size_t arrSize = newActivePlugins.size();
			uint8_t ** pluginArray = new uint8_t*[arrSize];
			size_t i = 0;
			for (boost::unordered_set<std::string>::iterator it = newActivePlugins.begin(), endIt = newActivePlugins.end(); it != endIt; ++it) {
				pluginArray[i] = ToUint8_tString(*it);
				i++;
			}
			Handler(::SetActivePlugins(gh, pluginArray, arrSize), pluginArray, arrSize);
		}

		bool GameHandle::IsPluginActive(const std::string plugin) {
			uint8_t * p = ToUint8_tString(plugin);
			bool result;
			Handler(::IsPluginActive(gh, p, &result), p);
			return result;
		}

		void GameHandle::SetPluginActiveStatus(const std::string plugin, const bool active) {
			uint8_t * p = ToUint8_tString(plugin);
			Handler(::SetPluginActiveStatus(gh, p, active), p);
		}

		//Return code handler - throws exception on receiving an error code.
		void GameHandle::Handler(uint32_t retCode) {
			if (retCode != LIBLO_OK) {
				uint8_t * message;
				std::string msgStr;
				if (::GetLastErrorDetails(&message) != LIBLO_OK)
					msgStr = "The error message could not be retrieved as a second error was encountered by the retrieval function.";
				else
					msgStr = ToStdString(message);
				::CleanUpErrorDetails();
				throw exception(retCode, msgStr);
			}
		}

		void GameHandle::Handler(uint32_t retCode, uint8_t * pointer) {
			delete [] pointer;
			Handler(retCode);
		}

		void GameHandle::Handler(uint32_t retCode, uint8_t ** arrPointer, size_t arrSize) {
			for (size_t i=0; i < arrSize; i++)
				delete [] arrPointer[i];
			delete [] arrPointer;
			Handler(retCode);
		}

		//Explicit memory management, need to call delete on the output when finished with it.
		uint8_t * GameHandle::ToUint8_tString(std::string str) {
			size_t length = str.length() + 1;
			uint8_t * p = new uint8_t[length];

			for (size_t j=0; j < str.length(); j++) {  //UTF-8, so this is code-point by code-point rather than char by char, but same result here.
				p[j] = str[j];
			}
			p[length - 1] = '\0';
			return p;
		}

		//No explicit memory management. Returns new string object, so probably (not sure) lasts until calling function terminates.
		std::string	GameHandle::ToStdString(uint8_t * str) {
			return std::string(reinterpret_cast<const char *>(str));
		}
	}
}