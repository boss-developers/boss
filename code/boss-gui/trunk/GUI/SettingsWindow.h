/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __SETTINGS__HPP__
#define __SETTINGS__HPP__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#       include "wx/wx.h"
#endif

using namespace std;

class SettingsFrame : public wxFrame {
public:
	SettingsFrame(const wxChar *title, wxFrame *parent, int x, int y, int width, int height);
	void OnStartupUpdateChange(wxCommandEvent& event);
	void OnProxyTypeChange(wxCommandEvent& event);
	void OnProxyHostChange(wxCommandEvent& event);
	void OnProxyPortChange(wxCommandEvent& event);
	void OnDebugLoggingChange(wxCommandEvent& event);
	void OnDebugVerbosityChange(wxCommandEvent& event);
	void OnDebugSourceRefsChange(wxCommandEvent& event);
	void OnEditorChange(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
private:
	wxCheckBox *StartupUpdateCheckBox;
	wxCheckBox *UseUserRuleEditorBox;
	wxCheckBox *DebugSourceReferencesBox;
	wxCheckBox *LogDebugOutputBox;
	wxComboBox *ProxyTypeBox;
	wxComboBox *DebugVerbosityBox;
	wxTextCtrl *ProxyHostBox;
	wxTextCtrl *ProxyPortBox;
	wxStaticText *ProxyTypeText;
	wxStaticText *ProxyHostText;
	wxStaticText *ProxyPortText;
};

#endif