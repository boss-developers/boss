/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2009-2012    BOSS Development Team.

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

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef GUI_MAIN_WINDOW_H_
#define GUI_MAIN_WINDOW_H_

#include <cstdint>

#include <string>
#include <vector>

#include <wx/hyperlink.h>
#include <wx/progdlg.h>
#include <wx/thread.h>

#include "common/game.h"

// TODO(MCP): Replace these includes with the ones we need as opposed to including all of them

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

namespace boss {

// Program class.
class BossGUI : public wxApp {
 public:
	bool OnInit();
	wxLocale *wxLoc;
};

// Main frame class.
class MainFrame : public wxFrame {
 public:
	explicit MainFrame(const wxChar *title);
	// MCP Note: Update doesn't seem to exist in main_window.cpp; need to find out what it was
	void Update(std::string updateVersion);
	void OnOpenSettings(wxCommandEvent &event);
	void OnQuit(wxCommandEvent &event);
	void OnRunBOSS(wxCommandEvent &event);
	void OnOpenFile(wxCommandEvent &event);
	void OnAbout(wxCommandEvent &event);
	void OnRunTypeChange(wxCommandEvent &event);
	void OnFormatChange(wxCommandEvent &event);
	void OnGameChange(wxCommandEvent &event);
	void OnRevertChange(wxCommandEvent &event);
	void OnLogDisplayChange(wxCommandEvent &event);
	void OnUpdateChange(wxCommandEvent &event);
	void OnCRCDisplayChange(wxCommandEvent &event);
	void OnTrialRunChange(wxCommandEvent &event);
	void OnEditUserRules(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);

	void SetGames(const Game &inGame,
	              const std::vector<std::uint32_t> inGames);
	void DisableUndetectedGames();

 protected:
	std::uint32_t updateCheckCode;  // 0 = update, 1 = no update, 2 = error.
	std::string updateCheckString;  // Holds wxMessageBox text.
	wxCriticalSection updateData;   // Protects fields above
	wxDECLARE_EVENT_TABLE();

 private:
	wxMenuBar *MenuBar;
	wxMenu *FileMenu;
	wxMenu *EditMenu;
	wxMenu *GameMenu;
	wxMenu *HelpMenu;
	wxButton *RunBOSSButton;
	wxButton *OpenBOSSlogButton;
	wxButton *EditUserRulesButton;
	wxCheckBox *ShowLogBox;
	wxCheckBox *CRCBox;
	wxCheckBox *UpdateBox;
	wxCheckBox *TrialRunBox;
	wxChoice *FormatChoice;
	wxChoice *RevertChoice;
	wxRadioButton *SortOption;
	wxRadioButton *UpdateOption;
	wxRadioButton *UndoOption;
	wxStaticText *RevertText;

	bool isStartup;
	std::vector<std::uint32_t> detectedGames;
	Game game;
};

}  // namespace boss
#endif  // GUI_MAIN_WINDOW_H_
