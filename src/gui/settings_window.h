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

#ifndef GUI_SETTINGS_WINDOW_H_
#define GUI_SETTINGS_WINDOW_H_

#include <wx/notebook.h>

// TODO(MCP): Replace these includes with the ones we need as opposed to including all of them

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

namespace boss {

class SettingsFrame : public wxFrame {
 public:
	SettingsFrame(const wxString title, wxFrame *parent);
	void OnOKQuit(wxCommandEvent &event);
	void OnCancelQuit(wxCommandEvent &event);
	void SetDefaultValues();
	DECLARE_EVENT_TABLE()
 private:
	wxCheckBox *UseUserRuleManagerBox;
	wxCheckBox *CloseGUIAfterRunningBox;
	wxChoice *DebugVerbosityChoice;
	wxChoice *GameChoice;
	wxChoice *LanguageChoice;
	wxTextCtrl *OblivionRepoURLTxt;
	wxTextCtrl *NehrimRepoURLTxt;
	wxTextCtrl *SkyrimRepoURLTxt;
	wxTextCtrl *Fallout3RepoURLTxt;
	wxTextCtrl *FalloutNVRepoURLTxt;
};

}  // namespace boss
#endif  // GUI_SETTINGS_WINDOW_H_
