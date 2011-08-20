/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "Parsing/Grammar.h"

namespace boss {

	///////////////////////////////
	//Common Functions
	///////////////////////////////

	//Checks if a masterlist variable is defined.
	void CheckVar(bool& result, string var) {
		if (setVars.find(var) == setVars.end())
			result = false;
		else
			result = true;
		return;
	}

	//Returns the true path based on what type of file or keyword it is.
	void GetPath(fs::path& file_path, string& file) {
		if (file == "OBSE") {
			file_path = "..";
			file = "obse_1_2_416.dll";  //Don't look for the loader because steam users don't need it.
		} else if (file == "FOSE") {
			file_path = "..";
			file = "fose_loader.exe";
		} else if (file == "NVSE") {
			file_path = "..";
			file = "nvse_loader.exe";
		} else if (file == "SKSE") {
			file_path = "..";
			file = "skse_loader.exe";
		} else if (file == "BOSS") {
			file_path = ".";
			file = "BOSS.exe";
		} else if (file == "TES4") {
			file_path = "..";
			file = "Oblivion.exe";
		} else if (file == "TES5") {
			file_path = "..";
			file = "Skyrim.exe";
		} else if (file == "FO3") {
			file_path = "..";
			file = "Fallout3.exe";
		} else if (file == "FONV") {
			file_path = "..";
			file = "FalloutNV.exe";
		} else {
			fs::path p(file);
			if (Tidy(p.extension().string()) == ".dll" && p.string().find("/") == string::npos && p.string().find("\\") == string::npos) {
				if (fs::exists(data_path / "OBSE"))
					file_path = data_path / fs::path("OBSE/Plugins");  //Oblivion - OBSE plugins.
				else if (fs::exists(data_path / "FOSE"))
					file_path = data_path / fs::path("FOSE/Plugins");  //Fallout 3 - FOSE plugins.
				else if (fs::exists(data_path / "NVSE"))
					file_path = data_path / fs::path("NVSE/Plugins");  //Fallout: New Vegas - NVSE plugins.
				else if (fs::exists(data_path / "SKSE"))
					file_path = data_path / fs::path("SKSE/Plugins");  //Fallout: New Vegas - NVSE plugins.
			} else
				file_path = data_path;
		}

	}

	//Checks if the given mod has a version for which the comparison holds true.
	void CheckVersion(bool& result, string var) {
		char comp = var[0];
		size_t pos = var.find("|") + 1;
		string version = var.substr(1,pos-2);
		string file = var.substr(pos);
		result = false;
		fs::path file_path;

		GetPath(file_path,file);

		if (Exists(file_path / file)) {
			string trueVersion;
			if (file_path == data_path) {
				if (IsGhosted(file_path / file)) 
					trueVersion = GetModHeader(file_path / fs::path(file + ".ghost"));
				else 
					trueVersion = GetModHeader(file_path / file);
			} else
				trueVersion = GetExeDllVersion(file_path / file);

			switch (comp) {
			case '>':
				if (trueVersion.compare(version) > 0)
					result = true;
				break;
			case '<':
				if (trueVersion.compare(version) < 0)
					result = true;
				break;
			case '=':
				if (version == trueVersion)
					result = true;
				break;
			}
		}
		return;
	}

	//Checks if the given mod has the given checksum.
	void CheckSum(bool& result, unsigned int sum, string file) {
		result = false;
		fs::path file_path;
		unsigned int CRC;

		GetPath(file_path,file);
		boost::unordered_map<string,unsigned int>::iterator iter = fileCRCs.find(file);

		if (iter != fileCRCs.end()) {
			CRC = fileCRCs.at(file);
		} else if (Exists(file_path / file)) {
			if (file_path == data_path) {
				if (IsGhosted(file_path / file)) 
					CRC = GetCrc32(file_path / fs::path(file + ".ghost"));
				else
					CRC = GetCrc32(file_path / file);
			} else
				CRC = GetCrc32(file_path / file);
			fileCRCs.emplace(file,CRC);
		}

		if (sum == CRC)
			result = true;
		return;
	}

	void CheckFile(bool& result, string file) {
		result = false;
		fs::path file_path;
		GetPath(file_path,file);
		result = fs::exists(file_path / file);
	}

	//Evaluate a single conditional.
	void EvaluateConditional(bool& result, metaType type, bool condition) {
		result = false;
		if (type == IF && condition == true)
			result = true;
		else if (type == IFNOT && condition == false)
			result = true;
		return;
	}

	//Evaluate the second half of a complex conditional.
	void EvaluateCompoundConditional(bool& result, string andOr, bool condition) {
		if (andOr == "||" && condition == true)
			result = true;
		else if (andOr == "&&" && result == true && condition == false)
			result = false;
	}


	///////////////////////////////
	//Skipper Grammars
	///////////////////////////////
	
	

	///////////////////////////////
	//Modlist/Masterlist Grammar
	///////////////////////////////

	bool storeItem = true;
	bool storeMessage = true;  //Should the current item/message be stored.
	vector<string> openGroups;  //Need to keep track of which groups are open to match up endings properly in MF1.

	//Stores a message, should it be appropriate.
	//The SPECIFIC_REQ and SPECIFIC_INC 'parsers' are not space-safe within items.
	void StoreMessage(vector<message>& messages, message currentMessage) {
		if (storeMessage) {
			if (currentMessage.key == SPECIFIC_REQ) {
				//Modify message output based on what mods are installed.
				stringstream ss(currentMessage.data);
				currentMessage.data = "";
				string mod,file,item;
				while (getline(ss, item, '|')) {
					//Syntax for an item is: "<version comparison>":"<file>"="<mod>"
					//The "<version comparison>": and ="<mod>" are optional.
					string version;
					bool versionCheck = true;
					size_t pos1,pos2,pos3,pos4;
					pos1 = item.find('"');
					if (pos1 == string::npos)
						continue;
					pos2 = item.find('"',pos1+1);
					if (pos2 == string::npos)
						continue;
					pos3 = item.find('"',pos2+1);
					if (pos3 == string::npos) {  //Only a plugin is given.
						file = item.substr(pos1+1, pos2-pos1-1);
						mod = file;
					} else {
						pos4 = item.find(':',pos2+1);
						if (pos4 != string::npos && pos4 < pos3) {  //A version and a file are given. Possibly also a mod.
							version = item.substr(pos1+1, pos2-pos1-1);
							pos4 = item.find('"',pos3+1);
							if (pos4 == string::npos)
								continue;
							file = item.substr(pos3+1, pos4-pos3-1);
							//Check version conditional.
							CheckVersion(versionCheck, version + "|" + file);
							if (versionCheck)
								continue;	//Mod exists and is of the correct version. No message should be printed for this item.
							pos1 = item.find('"',pos4+1);
							if (pos1 != string::npos) {
								pos2 = item.find('"',pos1+1);
								if (pos2 == string::npos)
									continue;
								mod = item.substr(pos1+1, pos2-pos1-1);
							} else
								mod = file;
						} else {
							pos4 = item.find('=',pos2+1);
							if (pos4 != string::npos && pos4 < pos3) {  //A file and a mod are given.
								file = item.substr(pos1+1, pos2-pos1-1);
								pos4 = item.find('"',pos3+1);
								if (pos4 == string::npos)
									continue;
								mod = item.substr(pos3+1, pos4-pos3-1);
							} else
								continue;
						}
					}
					//Get file path.
					fs::path file_path;
					GetPath(file_path,file);
					if (!fs::exists(file_path / file) || !versionCheck) {
						if (currentMessage.data != "")
							currentMessage.data += ", ";
						currentMessage.data += "\""+mod+"\"";
					}
				}
			} else if (currentMessage.key == SPECIFIC_INC) {
				//Modify message output based on what mods are installed.
				stringstream ss(currentMessage.data);
				currentMessage.data = "";
				string mod,file,item;
				while (getline(ss, item, '|')) {
					size_t pos1,pos2;
					pos1 = item.find('"');
					if (pos1 == string::npos)
						continue;
					pos2 = item.find('"',pos1+1);
					if (pos2 == string::npos)
						continue;
					file = item.substr(pos1+1, pos2-pos1-1);
					//Get file path.
					fs::path file_path;
					GetPath(file_path,file);
					if (fs::exists(file_path / file)) {
						mod = file;
						if (item.length() > pos2+1) {
							pos1 = item.find('"', pos2+1);
							if (pos1 != string::npos) {
								pos2 = item.find('"',pos1+1);
								if (pos2 == string::npos)
									continue;
								mod = item.substr(pos1+1, pos2-pos1-1);
							}
						}
						if (currentMessage.data != "")
							currentMessage.data += ", ";
						currentMessage.data += "\""+mod+"\"";
					}
				}
			}
			if (currentMessage.data != "")
				messages.push_back(currentMessage);
		}
		return;
	}

	//Stores the given item, should it be appropriate, and records any changes to open groups.
	void StoreItem(vector<item>& list, item currentItem) {
		if (currentItem.type == BEGINGROUP) {
			openGroups.push_back(currentItem.name.string());
		} else if (currentItem.type == ENDGROUP) {
			openGroups.pop_back();
		}
		if (storeItem)
			list.push_back(currentItem);
		return;
	}

	//Defines the given masterlist variable, if appropriate.
	void StoreVar(string var) {
		if (storeItem)
			setVars.insert(var);
		return;
	}

	//Stores the global message.
	void StoreGlobalMessage(message currentMessage) {
		if (storeMessage)
			globalMessageBuffer.push_back(currentMessage);
		return;
	}

	//MF1 compatibility function. Evaluates the MF1 FCOM conditional. Like it says on the tin.
	void EvalOldFCOMConditional(bool& result, char var) {
		result = false;
		boost::unordered_set<string>::iterator pos = setVars.find("FCOM");
		if (var == '>' && pos != setVars.end())
				result = true;
		else if (var == '<' && pos == setVars.end())
				result = true;
		return;
	}

	//MF1 compatibility function. Evaluates the MF1 OOO/BC conditional message symbols.
	void EvalMessKey(keyType key) {
		if (key == OOOSAY) {
			boost::unordered_set<string>::iterator pos = setVars.find("OOO");
			if (pos == setVars.end())
				storeMessage = false;
		} else if (key == BCSAY) {
			boost::unordered_set<string>::iterator pos = setVars.find("BC");
			if (pos == setVars.end())
				storeMessage = false;
		}
		return;
	}
	
	//Turns a given string into a path. Can't be done directly because of the openGroups checks.
	void path(fs::path& p, string const itemName) {
		if (itemName.length() == 0 && openGroups.size() > 0) 
			p = fs::path(openGroups.back());
		else
			p = fs::path(itemName);
		return;
	}

	

	////////////////////////////
	//Ini Grammar.
	////////////////////////////

	string currentHeading;

	//Set the BOSS variable values while parsing.
	void SetVar(string var, string value) {
			int intVal;
			bool bval;
			if (currentHeading == "GUI.LastOptions" || currentHeading == "GUI.Settings")
				return;
			if (currentHeading == "BOSS.InternetSettings") {
				if (var == "ProxyType")
					proxy_type = value;
				else if (var == "ProxyHostname")
					proxy_host = value;
				else if (var == "ProxyPort")
					proxy_port = value;
				return;
			} else if (currentHeading == "BOSS.RunOptions") {
				if (var == "Game") {
					if (value == "Oblivion")
						game = 1;
					else if (value == "Nehrim")
						game = 3;
					else if (value == "Fallout3")
						game = 2;
					else if (value == "FalloutNV")
						game = 4;
					else if (value == "Skyrim")
						game = 5;
					return;
				} else if (var == "BOSSlogFormat") {
					if (value == "html" || value == "text")
						log_format = value;
					return;
				} else if (var == "ProxyType")
					proxy_type = value;
				else if (var == "ProxyHostname")
					proxy_host = value;
				else if (var == "ProxyPort")
					proxy_port = value;

				intVal = atoi(value.c_str());
				if (intVal == 0)
					bval = false;
				else
					bval = true;

				if (var == "UpdateMasterlist")
					update = bval;
				else if (var == "OnlyUpdateMasterlist")
					update_only = bval;
				else if (var == "DisableMasterlistUpdate") {
					if (bval)
						update = false;
				} else if (var == "SilentRun")
					silent = bval;
				else if (var == "NoVersionParse")
					skip_version_parse = bval;
				else if (var == "Debug")
					debug = bval;
				else if (var == "DisplayCRCs")
					show_CRCs = bval;
				else if (var == "DoTrialRun")
					trial_run = bval;
				else if (var == "RevertLevel") {
					if (intVal >= 0 && intVal < 3)
						revert = intVal;
				} else if (var == "CommandLineVerbosity") {
					if (intVal >= 0 && intVal < 4)
						verbosity = intVal;
				}
				return;
			} else if (currentHeading == "BOSSlog.Filters") {
				intVal = atoi(value.c_str());
				if (intVal == 0)
					bval = false;
				else
					bval = true;
				if (var == "UseDarkColourScheme") {
					UseDarkColourScheme = bval;
				} else if (var == "HideVersionNumbers") {
					HideVersionNumbers = bval;
				} else if (var == "HideGhostedLabel") {
					HideGhostedLabel = bval;
				} else if (var == "HideChecksums") {
					HideChecksums = bval;
				} else if (var == "HideMessagelessMods") {
					HideMessagelessMods = bval;
				} else if (var == "HideGhostedMods") {
					HideGhostedMods = bval;
				} else if (var == "HideCleanMods") {
					HideCleanMods = bval;
				} else if (var == "HideRuleWarnings") {
					HideRuleWarnings = bval;
				} else if (var == "HideAllModMessages") {
					HideAllModMessages = bval;
				} else if (var == "HideNotes") {
					HideNotes = bval;
				} else if (var == "HideBashTagSuggestions") {
					HideBashTagSuggestions = bval;
				} else if (var == "HideRequirements") {
					HideRequirements = bval;
				} else if (var == "HideIncompatibilities") {
					HideIncompatibilities = bval;
				}
				return;
			} else if (currentHeading == "BOSSlog.Styles") {
				if (value == "")
					return;
				else if (var == "body")
					CSSBody = value;
				else if (var == ".filters")
					CSSFilters = value;
				else if (var == ".filters > li")
					CSSFiltersList = value;
				else if (var == "body > div:first-child")
					CSSTitle = value;
				else if (var == "body > div:first-child + div")
					CSSCopyright = value;
				else if (var == "body > div")
					CSSSections = value;
				else if (var == "body > div > span:first-child")
					CSSSectionTitle = value;
				else if (var == "body > div > span:first-child > span")
					CSSSectionPlusMinus = value;
				else if (var == "div > ul")
					CSSTopLevelList = value;
				else if (var == "body > div:last-child")
					CSSLastSection = value;
				else if (var == "body > div:last-child > span:first-child")
					CSSLastSectionTitle = value;
				else if (var == "div > ul > li")
					CSSTopLevelListItem = value;
				else if (var == "ul")
					CSSList = value;
				else if (var == "ul li")
					CSSListItem = value;
				else if (var == "li ul")
					CSSItemList = value;
				else if (var == "input[type='checkbox']")
					CSSCheckbox = value;
				else if (var == "blockquote")
					CSSBlockquote = value;
				else if (var == "#unrecognised > li")
					CSSUnrecognisedList = value;
				else if (var == "#summary > div")
					CSSSummaryRow = value;
				else if (var == "#summary > div > div")
					CSSSummaryCell = value;
				else if (var == ".error")
					CSSError = value;
				else if (var == ".warn")
					CSSWarning = value;
				else if (var == ".success")
					CSSSuccess = value;
				else if (var == ".version")
					CSSVersion = value;
				else if (var == ".ghosted")
					CSSGhost = value;
				else if (var == ".crc")
					CSSCRC = value;
				else if (var == ".tagPrefix")
					CSSTagPrefix = value;
				else if (var == ".dirty")
					CSSDirty = value;
				else if (var == ".message")
					CSSQuotedMessage = value;
				else if (var == ".mod")
					CSSMod = value;
				else if (var == ".tag")
					CSSTag = value;
				else if (var == ".note")
					CSSNote = value;
				else if (var == ".req")
					CSSRequirement = value;
				else if (var == ".inc")
					CSSIncompatibility = value;
			}
		}

	

	////////////////////////////
	//Userlist Grammar.
	////////////////////////////

	bool storeLine = true;  //Should the current message/sort line be stored.

	void AddSyntaxError(keyType const& rule, string const& object, string const& message) {
		string keystring = KeyToString(rule);
		string const msg = (SyntaxErrorFormat % keystring % object % message).str();
		userlistErrorBuffer.push_back(msg);
		return;
	}

	//Rule checker function, checks for syntax (not parsing) errors.
	void RuleSyntaxCheck(vector<rule>& userlist, rule currentRule) {
		bool skip = false;
		try {
			keyType ruleKey = currentRule.ruleKey;
			string ruleObject = currentRule.ruleObject;
			if (IsPlugin(ruleObject)) {
				if (ruleKey != FOR && IsMasterFile(ruleObject))
					throw failure(ruleKey, ruleObject, ESortingMasterEsm.str());
			} else {
				if (Tidy(ruleObject) == "esms")
					throw failure(ruleKey, ruleObject, ESortingGroupEsms.str());
				if (ruleKey == ADD && !IsPlugin(ruleObject))
					throw failure(ruleKey, ruleObject, EAddingModGroup.str());
				else if (ruleKey == FOR)
					throw failure(ruleKey, ruleObject, EAttachingMessageToGroup.str());
			}
			for (size_t i=0; i<currentRule.lines.size(); i++) {
				keyType key = currentRule.lines[i].key;
				string subject = currentRule.lines[i].object;
				if (key == BEFORE || key == AFTER) {
					if (ruleKey == FOR)
						throw failure(ruleKey, ruleObject, ESortLineInForRule.str());
					if ((IsPlugin(ruleObject) && !IsPlugin(subject)) || (!IsPlugin(ruleObject) && IsPlugin(subject)))
						throw failure(ruleKey, ruleObject, EReferencingModAndGroup.str());
					if (key == BEFORE) {
						if (Tidy(subject) == "esms")
							throw failure(ruleKey, ruleObject, ESortingGroupBeforeEsms.str());
						else if (IsMasterFile(subject))
							throw failure(ruleKey, ruleObject, ESortingModBeforeGameMaster.str());
					}
				} else if (key == TOP || key == BOTTOM) {
					if (ruleKey == FOR)
						throw failure(ruleKey, ruleObject, ESortLineInForRule.str());
					if (key == TOP && Tidy(subject) == "esms")
						throw failure(ruleKey, ruleObject, EInsertingToTopOfEsms.str());
					if (!IsPlugin(ruleObject) || IsPlugin(subject))
						throw failure(ruleKey, ruleObject, EInsertingGroupOrIntoMod.str());
				} else if (key == APPEND || key == REPLACE) {
					if (!IsPlugin(ruleObject))
						throw failure(ruleKey, ruleObject, EAttachingMessageToGroup.str());
				}
			}
		} catch (failure & e) {
			skip = true;
			AddSyntaxError(e.ruleKey, e.ruleObject, e.message);
			string const keystring = KeyToString(e.ruleKey);
			LOG_ERROR("Userlist Syntax Error: The rule beginning \"%s: %s\" %s", keystring.c_str(), e.ruleObject.c_str(), e.message.c_str());
		}
		if (!skip)
			userlist.push_back(currentRule);
		return;
	}

	//Stores the global message.
	void StoreCurrentLine(vector<line>& lines, line currentLine) {
		if (storeLine)
			lines.push_back(currentLine);
		return;
	}

	

}