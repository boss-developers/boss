/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2011  WrinklyNinja & the BOSS development team. 
	Copyright license:
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef __SETTINGS__HPP__
#define __SETTINGS__HPP__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#       include "wx/wx.h"
#endif

#include <wx/notebook.h>

using namespace std;

class SettingsFrame : public wxFrame {
public:
	SettingsFrame(const wxChar *title, wxFrame *parent);
	void OnOKQuit(wxCommandEvent& event);
	void OnCancelQuit(wxCommandEvent& event);
	void SetDefaultValues(wxString * DebugVerbosity);
	DECLARE_EVENT_TABLE()
private:
	wxCheckBox *StartupUpdateCheckBox;
	wxCheckBox *UseUserRuleEditorBox;
	wxCheckBox *DebugSourceReferencesBox;
	wxCheckBox *LogDebugOutputBox;
	wxCheckBox *UseDarkColourSchemeBox;
	wxCheckBox *HideVersionNumbersBox;
	wxCheckBox *HideGhostedLabelBox;
	wxCheckBox *HideChecksumsBox;
	wxCheckBox *HideMessagelessModsBox;
	wxCheckBox *HideGhostedModsBox;
	wxCheckBox *HideCleanModsBox;
	wxCheckBox *HideRuleWarningsBox;
	wxCheckBox *HideAllModMessagesBox;
	wxCheckBox *HideNotesBox;
	wxCheckBox *HideBashTagSuggestionsBox;
	wxCheckBox *HideRequirementsBox;
	wxCheckBox *HideIncompatibilitiesBox;
	wxCheckBox *HideDoNotCleanMessagesBox;
	wxChoice *DebugVerbosityChoice;
	wxTextCtrl *ProxyHostBox;
	wxTextCtrl *ProxyPortBox;
	wxTextCtrl *ProxyUserBox;
	wxTextCtrl *ProxyPasswdBox;
	wxTextCtrl *CSSBodyBox;
	wxTextCtrl *CSSFiltersBox;
	wxTextCtrl *CSSFiltersListBox;
	wxTextCtrl *CSSTitleBox;
	wxTextCtrl *CSSCopyrightBox;
	wxTextCtrl *CSSSectionsBox;
	wxTextCtrl *CSSSectionTitleBox;
	wxTextCtrl *CSSSectionPlusMinusBox;
	wxTextCtrl *CSSLastSectionBox;
	wxTextCtrl *CSSTableBox;
	wxTextCtrl *CSSListBox;
	wxTextCtrl *CSSListItemBox;
	wxTextCtrl *CSSSubListBox;
	wxTextCtrl *CSSCheckboxBox;
	wxTextCtrl *CSSBlockquoteBox;
	wxTextCtrl *CSSErrorBox;
	wxTextCtrl *CSSWarningBox;
	wxTextCtrl *CSSSuccessBox;
	wxTextCtrl *CSSVersionBox;
	wxTextCtrl *CSSGhostBox;
	wxTextCtrl *CSSCRCBox;
	wxTextCtrl *CSSTagPrefixBox;
	wxTextCtrl *CSSDirtyBox;
	wxTextCtrl *CSSQuotedMessageBox;
	wxTextCtrl *CSSModBox;
	wxTextCtrl *CSSTagBox;
	wxTextCtrl *CSSNoteBox;
	wxTextCtrl *CSSRequirementBox;
	wxTextCtrl *CSSIncompatibilityBox;
	wxNotebook *TabHolder;
	wxPanel *GeneralTab;
	wxPanel *InternetTab;
	wxPanel *DebugTab;
	wxScrolledWindow *FiltersTab;
	wxScrolledWindow *CSSTab;
};

#endif