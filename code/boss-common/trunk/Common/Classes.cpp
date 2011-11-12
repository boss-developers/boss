/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 3135 $, $Date: 2011-08-17 22:01:17 +0100 (Wed, 17 Aug 2011) $
*/

#include "Common/Classes.h"
#include "Common/Globals.h"
#include "Output/Output.h"
#include "Support/Logger.h"
#include "Support/Helpers.h"
#include "Support/ModFormat.h"
#include "Parsing/Grammar.h"

#include <boost/algorithm/string.hpp>

namespace boss {
	using namespace std;
	namespace fs = boost::filesystem;
	
	//////////////////////////////
	// Message Class Functions
	//////////////////////////////

			Message::Message	() {
		key = SAY;
		data.clear();
	}
	
			Message::Message	(keyType inKey, string inData) {
		key = inKey;
		data = inData;
	}

	string	Message::KeyToString() {
		if (key == SAY || key == OOOSAY || key == BCSAY)
			return "SAY";
		else if (key == TAG)
			return "TAG";
		else if (key == REQ)
			return "REQ";
		else if (key == WARN)
			return "WARN";
		else if (key == ERR)
			return "ERROR";
		else if (key == INC)
			return "INC";
		else if (key == DIRTY)
			return "DIRTY";
		else
			return "NONE";
	}

	//////////////////////////////
	// Item Class Functions
	//////////////////////////////
	
			Item::Item			() {
		name = fs::path("");
		type = MOD;
		messages.clear();
	}
	
			Item::Item			(fs::path inName) : name(inName), type(MOD) {
		messages.clear();
	}
	
			Item::Item			(fs::path inName, itemType inType) : name(inName), type(inType) {
		messages.clear();
	}
	
			Item::Item			(fs::path inName, itemType inType, vector<Message> inMessages)
		: name(inName), type(inType), messages(inMessages) {}
	
	bool	Item::IsPlugin		() {
		const string ext = boost::algorithm::to_lower_copy(name.extension().string());
		return (ext == ".esp" || ext == ".esm");
	}

	bool	Item::IsGroup		() { 
		return (!name.has_extension() && !name.empty()); 
	}

	bool	Item::Exists		() { 
		return (fs::exists(data_path / name) || fs::exists(data_path / fs::path(name.string() + ".ghost"))); 
	}
	
	bool	Item::IsMasterFile	() {
		const string lower = boost::algorithm::to_lower_copy(name.string());
		return (lower == "oblivion.esm" || lower == "fallout3.esm" || lower == "nehrim.esm" || lower == "falloutnv.esm" || lower == "skyrim.esm");
	}

	bool	Item::IsGhosted		() {
		return (fs::exists(data_path / fs::path(name.string() + ".ghost")));
	}
	
	string	Item::GetHeader		() {
		if (!IsPlugin())
			return "";
		
		ModHeader header;	

		// Read mod's header now...
		if (IsGhosted())
			header = ReadHeader(data_path / fs::path(name.string() + ".ghost"));
		else
			header = ReadHeader(data_path / name);

		// The current mod's version if found, or empty otherwise.
		return header.Version;
	}

	void	Item::SetModTime	(time_t modificationTime) {
		try {			
			if (IsGhosted())
				fs::last_write_time(data_path / fs::path(name.string() + ".ghost"), modificationTime);
			else
				fs::last_write_time(data_path / name, modificationTime);
		} catch(fs::filesystem_error e) {
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_WRITE_FAIL, name.string(), e.what());
		}
	}

	bool	Item::operator <	(Item item2) {
		time_t t1 = 0,t2 = 0;
		try {
			if (this->IsGhosted())
				t1 = fs::last_write_time(data_path / fs::path(this->name.string() + ".ghost"));
			else
				t1 = fs::last_write_time(data_path / this->name);
			if (item2.IsGhosted())
				t2 = fs::last_write_time(data_path / fs::path(item2.name.string() + ".ghost"));
			else
				t2 = fs::last_write_time(data_path / item2.name);
		}catch (fs::filesystem_error e){
			throw boss_error(BOSS_ERROR_FS_FILE_MOD_TIME_READ_FAIL, this->name.string() + "\" or \"" + item2.name.string(),e.what());
			LOG_WARN("%s; Report the mod in question with a download link to an official BOSS thread.", e.what());
		}
		double diff = difftime(t1,t2);

		return (diff < 0);
	}

	//////////////////////////////
	// ItemList Class Functions
	//////////////////////////////

	void					ItemList::Load			(fs::path path) {
		if (fs::exists(path) && fs::is_directory(path)) {
			LOG_DEBUG("Reading user mods...");
			for (fs::directory_iterator itr(path); itr!=fs::directory_iterator(); ++itr) {
				const fs::path filename = itr->path().filename();
				const string ext = boost::algorithm::to_lower_copy(itr->path().extension().string());
				if (fs::is_regular_file(itr->status()) && (ext==".esp" || ext==".esm" || ext==".ghost")) {
					LOG_TRACE("-- Found mod: '%s'", filename.string().c_str());
					//Add file to modlist. If the filename has a '.ghost' extension, remove it.
					Item tempItem;
					if (ext == ".ghost")
						tempItem = Item(filename.stem());
					else
						tempItem = Item(filename);
					items.push_back(tempItem);
				}
			}
			sort(items.begin(),items.end());
			LOG_DEBUG("Reading user mods done: %" PRIuS " total mods found.", items.size());
		} else {
			Skipper skipper(false);
			modlist_grammar grammar;
			string::const_iterator begin, end;
			string contents;

			grammar.SetErrorBuffer(&errorBuffer);
			grammar.SetGlobalMessageBuffer(&globalMessageBuffer);

			if (!fs::exists(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_FOUND, path.string());
			else if (!ValidateUTF8File(path))
				throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, path.string());

			fileToBuffer(path,contents);

			begin = contents.begin();
			end = contents.end();
			bool r = phrase_parse(begin, end, grammar, skipper, items);

			if (!r || begin != end)  //This might not work correctly.
				throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, path.string());
		}
	}
	
	void					ItemList::Save			(fs::path file) {
		ofstream ofile;
		//Back up file if it already exists.
		try {
			LOG_DEBUG("Saving backup of current modlist...");
			if (fs::exists(file)) 
				fs::rename(file, prev_modlist_path);
		} catch(boost::filesystem::filesystem_error e) {
			//Couldn't rename the file, print an error message.
			LOG_ERROR("Backup of modlist failed with error: %s", e.what());
			throw boss_error(BOSS_ERROR_FS_FILE_RENAME_FAIL, file.string(), e.what());
		}
		//Open output file.
		ofile.open(file.c_str());
		if (ofile.fail()) {  //Provide error message if it can't be written.
			LOG_ERROR("Backup cannot be saved.");
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		}

		//Iterate through items, printing out all group markers, mods and messsages.
		vector<Item>::iterator itemIter = items.begin();
		vector<Message>::iterator messageIter;
		for (itemIter; itemIter != items.end(); ++itemIter) {
			if (itemIter->type == BEGINGROUP)
				ofile << "BEGINGROUP: " << itemIter->name.string() << endl;  //Print the group begin marker
			else if (itemIter->type == ENDGROUP)
				ofile << "ENDGROUP: " << itemIter->name.string() << endl;  //Print the group end marker
			else {
				ofile << itemIter->name.string() << endl;  //Print the mod name.
				//Print the messages with the appropriate syntax.
				messageIter = itemIter->messages.begin();
				for (messageIter; messageIter != itemIter->messages.end(); ++messageIter)
					ofile << " " << messageIter->key << ": " << messageIter->data << endl; 
			}
		}

		ofile.close();
		LOG_INFO("Backup saved successfully.");
		return;
	}
	
	vector<Item>::iterator	ItemList::FindItem		(fs::path name) {
		vector<Item>::iterator itemIter = items.begin();
		while (itemIter != items.end()) {
			if (Tidy(itemIter->name.string()) == Tidy(name.string()))
				break;
			++itemIter;
		}
		return itemIter;
	}

	vector<Item>::iterator	ItemList::FindLastItem	(fs::path name) {
		vector<Item>::iterator itemIter = items.end();
		--itemIter;
		while (itemIter != items.begin()) {
			if (Tidy(itemIter->name.string()) == Tidy(name.string()))
				return itemIter;
			--itemIter;
		}
		return items.end();
	}
	
	//This looks a bit weird, but I need a non-reverse iterator outputted, and searching backwards is probably more efficient for my purposes.
	vector<Item>::iterator	ItemList::FindGroupEnd	(fs::path name) {
		vector<Item>::iterator itemIter = items.end();
		--itemIter;
		while (itemIter != items.begin()) {
			if (itemIter->type == ENDGROUP && Tidy(itemIter->name.string()) == Tidy(name.string()))
				return itemIter;
			--itemIter;
		}
		return items.end();
	}

	//////////////////////////////
	// RuleLine Class Functions
	//////////////////////////////

			RuleLine::RuleLine			() 
		: key(NONE), object("") {}

			RuleLine::RuleLine			(keyType inKey, string inObject) 
		: key(inKey), object(inObject) {}

	bool	RuleLine::IsObjectMessage	() {
		if (key != APPEND && key != REPLACE)
			return false;

		//First character of message, must be a message symbol, or part of an MF2 keyword.
		if (object[0] == '?' || object[0] == '$' || object[0] == '^' || object[0] == '%' || object[0] == ':' || object[0] == '"' || object[0] == '*')
			return true;
		else {
			size_t pos = object.find(':');
			if (pos == string::npos)
				return false;
			string keyString = object.substr(0,pos);
			if (keyString == "SAY" || keyString == "TAG" || keyString == "REQ" || keyString == "INC" || keyString == "DIRTY" || keyString == "WARN" || keyString == "ERROR")
				return true;
			else
				return false;
		}
	}
	
	keyType	RuleLine::ObjectMessageKey	() {
		if (key != APPEND && key != REPLACE)
			return NONE;

		switch(object[0]) {
		case '?':
			return SAY;
		case '$':
			return SAY;
		case '^':
			return SAY;
		case '%':
			return TAG;
		case ':':
			return REQ;
		case '"':
			return INC;
		case '*':
			return ERR;
		default:
			size_t pos = object.find(':');
			if (pos == string::npos)
				return NONE;
			string keyString = object.substr(0,pos);
			if (keyString == "SAY")
				return SAY;
			else if (keyString == "TAG")
				return TAG;
			else if (keyString == "REQ")
				return REQ;
			else if (keyString == "INC")
				return INC;
			else if (keyString == "DIRTY")
				return DIRTY;
			else if (keyString == "WARN")
				return WARN;
			else if (keyString == "ERROR")
				return ERR;
			else 
				return NONE;
		}
	}

	string	RuleLine::ObjectMessageData	() {
		if (key != APPEND && key != REPLACE)
			return "";

		//First character of message, must be a message symbol, or part of an MF2 keyword.
		if (object[0] == '?' || object[0] == '$' || object[0] == '^' || object[0] == '%' || object[0] == ':' || object[0] == '"' || object[0] == '*')
			return object.substr(1);
		else {
			size_t pos = object.find(':');
			if (pos == string::npos)
				return "";
			return object.substr(pos+1);
		}
	}

	string	RuleLine::KeyToString		() {
		if (key == BEFORE)
			return "BEFORE";
		else if (key == AFTER)
			return "AFTER";
		else if (key == TOP)
			return "TOP";
		else if (key == BOTTOM)
			return "BOTTOM";
		else if (key == APPEND)
			return "APPEND";
		else if (key == REPLACE)
			return "REPLACE";
		else
			return "NONE";
	}

	//////////////////////////////
	// Rule Class Functions
	//////////////////////////////

	string Rule::KeyToString() {
		if (ruleKey == ADD)
			return "ADD";
		else if (ruleKey == OVERRIDE)
			return "OVERRIDE";
		else if (ruleKey == FOR)
			return "FOR";
		else
			return "NONE";
	}

	//////////////////////////////
	// RuleList Class Functions
	//////////////////////////////

	void RuleList::Load(fs::path file) {
		Skipper skipper(false);
		userlist_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		grammar.SetParsingErrorBuffer(&parsingErrorBuffer);

		if (!fs::exists(file)) {
			ofstream userlist_file(file.c_str(),ios_base::binary);
			if (!userlist_file.fail())
				userlist_file << '\xEF' << '\xBB' << '\xBF';  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			else
				throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
			userlist_file.close();
			return;
		} else if (!ValidateUTF8File(file))
			throw boss_error(BOSS_ERROR_FILE_NOT_UTF8, file.string());

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();
		bool r = phrase_parse(begin, end, grammar, skipper, rules);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());
	}

	void RuleList::Save(fs::path file) {
		ofstream outFile(file.c_str(),ios_base::trunc);

		if (outFile.fail()) {  //Provide error message if it can't be written.
			LOG_ERROR("Backup cannot be saved.");
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		}

		for (vector<Rule>::iterator ruleIter = rules.begin(); ruleIter != rules.end(); ++ruleIter) {
			if (!ruleIter->enabled)
				outFile << "DISABLE ";
			outFile << boost::algorithm::to_upper_copy(ruleIter->KeyToString()) << ": " << ruleIter->ruleObject << endl;

			for (vector<RuleLine>::iterator lineIter = ruleIter->lines.begin(); lineIter != ruleIter->lines.end(); ++lineIter)
				outFile << boost::algorithm::to_upper_copy(lineIter->KeyToString()) << ": " << lineIter->object << endl;
			outFile << endl;
		}
		outFile.close();
	}

	//////////////////////////////
	// Ini Class Functions
	//////////////////////////////

	void	Ini::Load			(fs::path file) {
		Skipper skipper(true);
		ini_grammar grammar;
		string::const_iterator begin, end;
		string contents;

		grammar.SetErrorBuffer(&errorBuffer);

		fileToBuffer(file,contents);

		begin = contents.begin();
		end = contents.end();

		bool r = phrase_parse(begin, end, grammar, skipper);

		if (!r || begin != end)  //This might not work correctly.
			throw boss_error(BOSS_ERROR_FILE_PARSE_FAIL, file.string());
	}

	string Ini::GetLogFormat() {
		if (log_format == HTML)
			return "html";
		else
			return "text";
	}

	void	Ini::Save			(fs::path file) {
		ofstream ini(file.c_str(), ios_base::trunc);
		if (ini.fail())
			throw boss_error(BOSS_ERROR_FILE_WRITE_FAIL, file.string());
		ini <<  '\xEF' << '\xBB' << '\xBF'  //Write UTF-8 BOM to ensure the file is recognised as having the UTF-8 encoding.
			<<	"#---------------" << endl
			<<	"# BOSS Ini File" << endl
			<<	"#---------------" << endl
			<<	"# Settings with names starting with 'b' are boolean and accept values of 'true' or 'false'." << endl
			<<	"# Settings with names starting with 'i' are unsigned integers and accept varying ranges of whole numbers." << endl
			<<	"# Settings with names starting with 's' are strings and their accepted values vary." << endl
			<<	"# See the BOSS ReadMe for details on what each setting does and the accepted values for integer and string settings." << endl << endl

			<<	"[BOSS.GeneralSettings]" << endl
			<<	"bDoStartupUpdateCheck    = " << BoolToString(do_startup_update_check) << endl
			<<	"bUseUserRulesEditor      = " << BoolToString(use_user_rules_editor) << endl << endl

			<<	"[BOSS.InternetSettings]" << endl
			<<	"sProxyHostname           = " << proxy_host << endl
			<<	"iProxyPort               = " << IntToString(proxy_port) << endl
			<<	"sProxyUsername           = " << proxy_user << endl
			<<	"sProxyPassword           = " << proxy_passwd << endl << endl

			<<	"[BOSS.RunOptions]" << endl
			<<	"sGame                    = " << GetGameString() << endl
			<<	"sBOSSLogFormat           = " << GetLogFormat() << endl
			<<	"iRunType                 = " << IntToString(run_type) << endl
			<<	"iDebugVerbosity          = " << IntToString(debug_verbosity) << endl
			<<	"iRevertLevel             = " << IntToString(revert) << endl
			<<	"bUpdateMasterlist        = " << BoolToString(update) << endl
			<<	"bOnlyUpdateMasterlist    = " << BoolToString(update_only) << endl
			<<	"bSilentRun               = " << BoolToString(silent) << endl
			<<	"bNoVersionParse          = " << BoolToString(skip_version_parse) << endl
			<<	"bDebugWithSourceRefs     = " << BoolToString(debug_with_source) << endl
			<<	"bDisplayCRCs             = " << BoolToString(show_CRCs) << endl
			<<	"bDoTrialRun              = " << BoolToString(trial_run) << endl
			<<	"bLogDebugOutput          = " << BoolToString(log_debug_output) << endl << endl
			
			<<	"[BOSSLog.Filters]" << endl
			<<	"bUseDarkColourScheme     = " << BoolToString(UseDarkColourScheme) << endl
			<<	"bHideVersionNumbers      = " << BoolToString(HideVersionNumbers) << endl
			<<	"bHideGhostedLabel        = " << BoolToString(HideGhostedLabel) << endl
			<<	"bHideChecksums           = " << BoolToString(HideChecksums) << endl
			<<	"bHideMessagelessMods     = " << BoolToString(HideMessagelessMods) << endl
			<<	"bHideGhostedMods         = " << BoolToString(HideGhostedMods) << endl
			<<	"bHideCleanMods           = " << BoolToString(HideCleanMods) << endl
			<<	"bHideRuleWarnings        = " << BoolToString(HideRuleWarnings) << endl
			<<	"bHideAllModMessages      = " << BoolToString(HideAllModMessages) << endl
			<<	"bHideNotes               = " << BoolToString(HideNotes) << endl
			<<	"bHideBashTagSuggestions  = " << BoolToString(HideBashTagSuggestions) << endl
			<<	"bHideRequirements        = " << BoolToString(HideRequirements) << endl
			<<	"bHideIncompatibilities   = " << BoolToString(HideIncompatibilities) << endl
			<<	"bHideDoNotCleanMessages  = " << BoolToString(HideDoNotCleanMessages) << endl << endl

			<<	"[BOSSLog.Styles]" << endl
			<<	"# A style with nothing specified uses the coded defaults." << endl
			<<	"# These defaults are given in the BOSS ReadMe as with the rest of the ini settings." << endl
			<<	"\"body\"                                     = " << CSSBody << endl
			<<	"\"#darkBody\"                                = " << CSSDarkBody << endl
			<<	"\".darkLink:link\"                           = " << CSSDarkLink << endl
			<<	"\".darkLink:visited\"                        = " << CSSDarkLinkVisited << endl
			<<	"\"#filters\"                                 = " << CSSFilters << endl
			<<	"\"#filters > li\"                            = " << CSSFiltersList << endl
			<<	"\"#darkFilters\"                             = " << CSSDarkFilters << endl
			<<	"\"body > div:first-child\"                   = " << CSSTitle << endl
			<<	"\"body > div:first-child + div\"             = " << CSSCopyright << endl
			<<	"\"h3 + *\"                                   = " << CSSSections << endl
			<<	"\"h3\"                                       = " << CSSSectionTitle << endl
			<<	"\"h3 > span\"                                = " << CSSSectionPlusMinus << endl
			<<	"\"#end\"                                     = " << CSSLastSection << endl
			<<	"\"td\"                                       = " << CSSTable << endl
			<<	"\"ul\"                                       = " << CSSList << endl
			<<	"\"ul li\"                                    = " << CSSListItem << endl
			<<	"\"li ul\"                                    = " << CSSSubList << endl
			<<	"\"input[type='checkbox']\"                   = " << CSSCheckbox << endl
			<<	"\"blockquote\"                               = " << CSSBlockquote << endl
			<<	"\".error\"                                   = " << CSSError << endl
			<<	"\".warn\"                                    = " << CSSWarning << endl
			<<	"\".success\"                                 = " << CSSSuccess << endl
			<<	"\".version\"                                 = " << CSSVersion << endl
			<<	"\".ghosted\"                                 = " << CSSGhost << endl
			<<	"\".crc\"                                     = " << CSSCRC << endl
			<<	"\".tagPrefix\"                               = " << CSSTagPrefix << endl
			<<	"\".dirty\"                                   = " << CSSDirty << endl
			<<	"\".message\"                                 = " << CSSQuotedMessage << endl
			<<	"\".mod\"                                     = " << CSSMod << endl
			<<	"\".tag\"                                     = " << CSSTag << endl
			<<	"\".note\"                                    = " << CSSNote << endl
			<<	"\".req\"                                     = " << CSSRequirement << endl
			<<	"\".inc\"                                     = " << CSSIncompatibility;
		ini.close();
	}

	string	Ini::GetGameString	() {
		if (game == AUTODETECT)
			return "auto";
		else if (game == OBLIVION)
			return "Oblivion";
		else if (game == FALLOUT3)
			return "Fallout3";
		else if (game == NEHRIM)
			return "Nehrim";
		else if (game == FALLOUTNV)
			return "FalloutNV";
		else if (game == SKYRIM)
			return "Skyrim";
		else
			return "";
	}
}