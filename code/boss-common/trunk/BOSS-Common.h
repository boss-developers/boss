/*	Better Oblivion Sorting Software
	
	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja & the BOSS 
	development team. Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __BOSS_COMMON_H__
#define __BOSS_COMMON_H__

#include "Common/Globals.h"
#include "Common/Execute.h"
#include "Common/Classes.h"
#include "Common/Error.h"
#include "Output/Output.h"
#include "Support/Helpers.h"
#include "Support/Logger.h"
#include "Updating/Updater.h"

/* For easy reference, the following functions may throw exceptions (using boss_error) on failure:

bool			Item::operator<				(Item);

void			ItemList::Load				(fs::path path);

void			ItemList::Save				(fs::path file);

void			RuleList::Load				(fs::path file);

void			RuleList::Save				(fs::path file);

void			Ini::Load					(fs::path file);

void			Ini::Save					(fs::path file);

void			GetGame						();

time_t			GetMasterTime				();

void			Outputter::Save				(fs::path file, bool overwrite);

void			CleanUp						();

void			UpdateMasterlist			(uiStruct ui, unsigned int& localRevision, string& localDate, unsigned int& remoteRevision, string& remoteDate);

string			IsBOSSUpdateAvailable		();

string			FetchReleaseNotes			(const string updateVersion);

vector<string>	DownloadInstallBOSSUpdate	(uiStruct ui, const int updateType, const string updateVersion);
*/

#endif