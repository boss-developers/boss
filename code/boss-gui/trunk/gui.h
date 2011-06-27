/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __MAIN__HPP__
#define __MAIN__HPP__

#include "helpers.h"
#include <string>

//Program class.
class BossGUI : public wxApp {
public:
	virtual bool OnInit();
};

//Main frame class.
class MainFrame : public wxFrame {
public:
	MainFrame(const wxChar *title, int x, int y, int width, int height);
	void OnQuit(wxCommandEvent& event);
	void OnRunBOSS(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnRunTypeChange(wxCommandEvent& event);
	void OnFormatChange(wxCommandEvent& event);
	void OnVerbosityChange(wxCommandEvent& event);
	void OnGameChange(wxCommandEvent& event);
	void OnRevertChange(wxCommandEvent& event);
	void OnLogDisplayChange(wxCommandEvent& event);
	void OnDebugChange(wxCommandEvent& event);
	void OnUpdateChange(wxCommandEvent& event);
	void OnVersionDisplayChange(wxCommandEvent& event);
	void OnCRCDisplayChange(wxCommandEvent& event);
	void OnLoggingChange(wxCommandEvent& event);
	void OnTrialRunChange(wxCommandEvent& event);
	void OnUpdateCheck(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);
	DECLARE_EVENT_TABLE()
	void CheckForUpdate();
	void Update();
private:
	wxMenuBar *MenuBar;
	wxMenu *FileMenu;
	wxMenu *HelpMenu;
	wxButton *RunBOSSButton;
	wxButton *OpenBOSSlogButton;
	wxButton *OpenUserlistButton;
	wxButton *CheckForUpdatesButton;
	wxCheckBox *ShowLogBox;
	wxCheckBox *DebugBox;
	wxCheckBox *LoggingBox;
	wxComboBox *FormatBox;
	wxComboBox *VerbosityBox;
	wxRadioButton *SortOption;
	wxCheckBox *UpdateBox;
	wxCheckBox *SortVersionBox;
	wxCheckBox *SortCRCBox;
	wxCheckBox *TrialRunBox;
	wxRadioButton *UpdateOption;
	wxComboBox *GameBox;
	wxRadioButton *UndoOption;
	wxComboBox *RevertBox;
	wxCheckBox *UndoVersionBox;
	wxCheckBox *UndoCRCBox;
	wxStaticText *GameText;
	wxStaticText *RevertText;
};

enum {
    OPTION_OpenUserlist = wxID_HIGHEST + 1, // declares an id which will be used to call our button
	OPTION_OpenBOSSlog,
	OPTION_Run,
	OPTION_CheckForUpdates,
    MENU_Quit,
	MENU_OpenMReadMe,
	MENU_OpenURReadMe,
	MENU_ShowAbout,
	DROPDOWN_LogFormat,
	DROPDOWN_Verbosity,
	DROPDOWN_Game,
	DROPDOWN_Revert,
	CHECKBOX_ShowBOSSlog,
	CHECKBOX_EnableDebug,
	CHECKBOX_Update,
	CHECKBOX_SortEnableVersions,
	CHECKBOX_SortEnableCRCs,
	CHECKBOX_RevertEnableVersions,
	CHECKBOX_RevertEnableCRCs,
	CHECKBOX_EnableLogging,
	CHECKBOX_TrialRun,
	RADIOBUTTON_SortOption,
	RADIOBUTTON_UpdateOption,
	RADIOBUTTON_UndoOption,
};
#endif
