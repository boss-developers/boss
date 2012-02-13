/*	Better Oblivion Sorting Software

	A "one-click" program for users that quickly optimises and avoids 
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge, 
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

    Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of Better Oblivion Sorting Software.

    Better Oblivion Sorting Software is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    Better Oblivion Sorting Software is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Better Oblivion Sorting Software.  If not, see 
	<http://www.gnu.org/licenses/>.

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/SettingsWindow.h"

BEGIN_EVENT_TABLE( SettingsFrame, wxFrame )
	EVT_BUTTON ( OPTION_OKExitSettings, SettingsFrame::OnOKQuit)
	EVT_BUTTON ( OPTION_CancelExitSettings, SettingsFrame::OnCancelQuit)
END_EVENT_TABLE()

using namespace boss;
using namespace std;

SettingsFrame::SettingsFrame(const wxChar *title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(600,800)) {

	wxString DebugVerbosity[] = {
        wxT("Standard (0)"),
        wxT("Level 1"),
		wxT("Level 2"),
		wxT("Level 3")
    };

	//Set up stuff in the frame.
	SetBackgroundColour(wxColour(255,255,255));

	TabHolder = new wxNotebook(this,wxID_ANY);

	wxSizerFlags ContentSizerFlags(1);
	ContentSizerFlags.Expand().Border(wxTOP|wxBOTTOM, 5);

	wxSizerFlags ItemSizerFlags(1);
	ItemSizerFlags.Border(wxLEFT|wxRIGHT, 10);

	wxSizerFlags BorderSizerFlags(0);
	BorderSizerFlags.Border(wxALL, 10);

	//Create General Settings tab.
	GeneralTab = new wxPanel(TabHolder);
	wxBoxSizer *GeneralTabSizer = new wxBoxSizer(wxVERTICAL);

	GeneralTabSizer->Add(StartupUpdateCheckBox = new wxCheckBox(GeneralTab,wxID_ANY,wxT("Check for BOSS updates on startup")), BorderSizerFlags);
	GeneralTabSizer->Add(UseUserRuleEditorBox = new wxCheckBox(GeneralTab,wxID_ANY,wxT("Use User Rules Editor")), BorderSizerFlags);

	GeneralTab->SetSizer(GeneralTabSizer);

	//Create Internet Settings tab.
	InternetTab = new wxPanel(TabHolder);
	wxBoxSizer *InternetTabSizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer *proxyHostSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyHostSizer->Add(new wxStaticText(InternetTab, wxID_ANY, wxT("Proxy Hostname:")), 3, wxEXPAND);
	proxyHostSizer->Add(ProxyHostBox = new wxTextCtrl(InternetTab,wxID_ANY), 1);
	InternetTabSizer->Add(proxyHostSizer, BorderSizerFlags);

	wxBoxSizer *proxyPortSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyPortSizer->Add(new wxStaticText(InternetTab, wxID_ANY, wxT("Proxy Port Number:")), 3, wxEXPAND);
	proxyPortSizer->Add(ProxyPortBox = new wxTextCtrl(InternetTab,wxID_ANY), 1);
	InternetTabSizer->Add(proxyPortSizer, BorderSizerFlags);

	wxBoxSizer *proxyUserSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyUserSizer->Add(new wxStaticText(InternetTab, wxID_ANY, wxT("Proxy Username:")), 3, wxEXPAND);
	proxyUserSizer->Add(ProxyUserBox = new wxTextCtrl(InternetTab,wxID_ANY), 1);
	InternetTabSizer->Add(proxyUserSizer, BorderSizerFlags);

	wxBoxSizer *proxyPasswdSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyPasswdSizer->Add(new wxStaticText(InternetTab, wxID_ANY, wxT("Proxy Password:")), 3, wxEXPAND);
	proxyPasswdSizer->Add(ProxyPasswdBox = new wxTextCtrl(InternetTab,wxID_ANY), 1);
	InternetTabSizer->Add(proxyPasswdSizer, BorderSizerFlags);

	InternetTab->SetSizer(InternetTabSizer);

	//Create Debugging Settings tab.
	DebugTab = new wxPanel(TabHolder);
	wxBoxSizer *DebugTabSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *verbosityBox = new wxBoxSizer(wxHORIZONTAL);
	verbosityBox->Add(new wxStaticText(DebugTab, wxID_ANY, wxT("Debug Output Verbosity:")), 3, wxEXPAND);
	verbosityBox->Add(DebugVerbosityChoice = new wxChoice(DebugTab, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, DebugVerbosity), 1);
	DebugTabSizer->Add(verbosityBox, BorderSizerFlags);
	DebugTabSizer->Add(DebugSourceReferencesBox = new wxCheckBox(DebugTab,wxID_ANY, wxT("Include Source Code References")), BorderSizerFlags);
	DebugTabSizer->Add(LogDebugOutputBox = new wxCheckBox(DebugTab,wxID_ANY, wxT("Log Debug Output")), BorderSizerFlags);

	DebugTab->SetSizer(DebugTabSizer);

	//Create BOSS Log Filters tab.
	FiltersTab = new wxScrolledWindow(TabHolder);
	wxBoxSizer *FiltersTabSizer = new wxBoxSizer(wxVERTICAL);

	FiltersTabSizer->Add(UseDarkColourSchemeBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Use Dark Colour Scheme")), BorderSizerFlags);
	FiltersTabSizer->Add(HideRuleWarningsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Rule Warnings")), BorderSizerFlags);
	FiltersTabSizer->Add(HideVersionNumbersBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Version Numbers")), BorderSizerFlags);
	FiltersTabSizer->Add(HideGhostedLabelBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide 'Ghosted' Label")), BorderSizerFlags);
	FiltersTabSizer->Add(HideChecksumsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Checksums")), BorderSizerFlags);
	FiltersTabSizer->Add(HideMessagelessModsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Messageless Mods")), BorderSizerFlags);
	FiltersTabSizer->Add(HideGhostedModsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Ghosted Mods")), BorderSizerFlags);
	FiltersTabSizer->Add(HideCleanModsBox = new wxCheckBox(FiltersTab ,wxID_ANY, wxT("Hide Clean Mods")), BorderSizerFlags);
	FiltersTabSizer->Add(HideAllModMessagesBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide All Mod Nessages")), BorderSizerFlags);
	FiltersTabSizer->Add(HideNotesBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Notes")), BorderSizerFlags);
	FiltersTabSizer->Add(HideBashTagSuggestionsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Bash Tag Suggestions")), BorderSizerFlags);
	FiltersTabSizer->Add(HideRequirementsBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Requirements")), BorderSizerFlags);
	FiltersTabSizer->Add(HideIncompatibilitiesBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide Incompatibilities")), BorderSizerFlags);
	FiltersTabSizer->Add(HideDoNotCleanMessagesBox = new wxCheckBox(FiltersTab, wxID_ANY, wxT("Hide 'Do Not Clean' Messages")), BorderSizerFlags);

	FiltersTab->SetSizer(FiltersTabSizer);

	//Make tab scolling.
	FiltersTab->FitInside();
	FiltersTab->SetScrollRate(10, 10);

	//Create BOSS Log CSS tab.
	CSSTab = new wxScrolledWindow(TabHolder);
	wxBoxSizer *CSSTabSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *horBox;

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("body")), ItemSizerFlags);
	horBox->Add(CSSBodyBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("#darkBody")), ItemSizerFlags);
	horBox->Add(CSSDarkBodyBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".darkLink:link")), ItemSizerFlags);
	horBox->Add(CSSDarkLinkBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".darkLink:visited")), ItemSizerFlags);
	horBox->Add(CSSDarkLinkVisitedBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("#filters")), ItemSizerFlags);
	horBox->Add(CSSFiltersBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("#filters > li")), ItemSizerFlags);
	horBox->Add(CSSFiltersListBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("#darkFilters")), ItemSizerFlags);
	horBox->Add(CSSDarkFiltersBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("body > div:first-child")), ItemSizerFlags);
	horBox->Add(CSSTitleBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("body > div:first-child + div")), ItemSizerFlags);
	horBox->Add(CSSCopyrightBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("body > div:first-child + div")), ItemSizerFlags);
	horBox->Add(CSSSectionsBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("h3")), ItemSizerFlags);
	horBox->Add(CSSSectionTitleBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("h3 > span")), ItemSizerFlags);
	horBox->Add(CSSSectionPlusMinusBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("#end")), ItemSizerFlags);
	horBox->Add(CSSLastSectionBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("td")), ItemSizerFlags);
	horBox->Add(CSSTableBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("ul")), ItemSizerFlags);
	horBox->Add(CSSListBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("ul li")), ItemSizerFlags);
	horBox->Add(CSSListItemBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("li ul")), ItemSizerFlags);
	horBox->Add(CSSSubListBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("input[type='checkbox']")), ItemSizerFlags);
	horBox->Add(CSSCheckboxBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT("blockquote")), ItemSizerFlags);
	horBox->Add(CSSBlockquoteBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".error")), ItemSizerFlags);
	horBox->Add(CSSErrorBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".warn")), ItemSizerFlags);
	horBox->Add(CSSWarningBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".success")), ItemSizerFlags);
	horBox->Add(CSSSuccessBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".version")), ItemSizerFlags);
	horBox->Add(CSSVersionBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".ghosted")), ItemSizerFlags);
	horBox->Add(CSSGhostBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".crc")), ItemSizerFlags);
	horBox->Add(CSSCRCBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".tagPrefix")), ItemSizerFlags);
	horBox->Add(CSSTagPrefixBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".dirty")), ItemSizerFlags);
	horBox->Add(CSSDirtyBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".message")), ItemSizerFlags);
	horBox->Add(CSSQuotedMessageBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".mod")), ItemSizerFlags);
	horBox->Add(CSSModBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".tag")), ItemSizerFlags);
	horBox->Add(CSSTagBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".note")), ItemSizerFlags);
	horBox->Add(CSSNoteBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".req")), ItemSizerFlags);
	horBox->Add(CSSRequirementBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	horBox = new wxBoxSizer(wxHORIZONTAL);
	horBox->Add(new wxStaticText(CSSTab, wxID_ANY, wxT(".inc")), ItemSizerFlags);
	horBox->Add(CSSIncompatibilityBox = new wxTextCtrl(CSSTab, wxID_ANY), ItemSizerFlags);
	CSSTabSizer->Add(horBox, ContentSizerFlags);

	CSSTab->SetSizer(CSSTabSizer);

	//Make tab scolling.
	CSSTab->FitInside();
	CSSTab->SetScrollRate(10, 10);


	//Attach all pages.
	TabHolder->AddPage(GeneralTab,wxT("General"),true);
	TabHolder->AddPage(InternetTab,wxT("Internet"));
	TabHolder->AddPage(DebugTab,wxT("Debugging"));
	TabHolder->AddPage(FiltersTab,wxT("BOSS Log Filters"));
	TabHolder->AddPage(CSSTab,wxT("BOSS Log CSS"));
	
	//Need to add 'OK' and 'Cancel' buttons.
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxButton(this, OPTION_OKExitSettings, wxT("OK"), wxDefaultPosition, wxSize(70, 30)));
	hbox->Add(new wxButton(this, OPTION_CancelExitSettings, wxT("Cancel"), wxDefaultPosition, wxSize(70, 30)), 0, wxLEFT, 20);

	//Now add TabHolder and OK button to window sizer.
	wxBoxSizer *bigBox = new wxBoxSizer(wxVERTICAL);
	bigBox->Add(TabHolder, 1, wxEXPAND);
	bigBox->Add(hbox, 0, wxCENTER|wxALL, 10);
	bigBox->SetMinSize(400,300);

	//Initialise options with values. For checkboxes, they are off by default.
	SetDefaultValues(DebugVerbosity);
	
	//Now set the layout and sizes.
	SetSizerAndFit(bigBox);
}

void SettingsFrame::SetDefaultValues(wxString * DebugVerbosity) {
	//General Settings
	if (gl_do_startup_update_check)
		StartupUpdateCheckBox->SetValue(true);
	if (gl_use_user_rules_editor)
		UseUserRuleEditorBox->SetValue(true);

	//Internet Settings
	ProxyHostBox->SetValue(gl_proxy_host);

	ProxyPortBox->SetValue(wxString::Format(wxT("%i"),gl_proxy_port));

	ProxyUserBox->SetValue(gl_proxy_user);

	ProxyPasswdBox->SetValue(gl_proxy_passwd);

	//Debugging Settings
	if (gl_debug_with_source)
		DebugSourceReferencesBox->SetValue(true);

	if (gl_log_debug_output)
		LogDebugOutputBox->SetValue(true);

	if (gl_debug_verbosity == 0)
		DebugVerbosityChoice->SetSelection(0);
	else if (gl_debug_verbosity == 1)
		DebugVerbosityChoice->SetSelection(1);
	else if (gl_debug_verbosity == 2)
		DebugVerbosityChoice->SetSelection(2);
	else if (gl_debug_verbosity == 3)
		DebugVerbosityChoice->SetSelection(3);

	//BOSS Log Filters Settings
	if (UseDarkColourScheme)
		UseDarkColourSchemeBox->SetValue(true);
	if (HideVersionNumbers)
		HideVersionNumbersBox->SetValue(true);
	if (HideGhostedLabel)
		HideGhostedLabelBox->SetValue(true);
	if (HideChecksums)
		HideChecksumsBox->SetValue(true);
	if (HideMessagelessMods)
		HideMessagelessModsBox->SetValue(true);
	if (HideGhostedMods)
		HideGhostedModsBox->SetValue(true);
	if (HideCleanMods)
		HideCleanModsBox->SetValue(true);
	if (HideRuleWarnings)
		HideRuleWarningsBox->SetValue(true);
	if (HideAllModMessages)
		HideAllModMessagesBox->SetValue(true);
	if (HideNotes)
		HideNotesBox->SetValue(true);
	if (HideBashTagSuggestions)
		HideBashTagSuggestionsBox->SetValue(true);
	if (HideRequirements)
		HideRequirementsBox->SetValue(true);
	if (HideIncompatibilities)
		HideIncompatibilitiesBox->SetValue(true);
	if (HideDoNotCleanMessages)
		HideDoNotCleanMessagesBox->SetValue(true);

	//BOSS Log CSS Settings
	CSSBodyBox->SetValue(CSSBody);
	CSSDarkBodyBox->SetValue(CSSDarkBody);
	CSSDarkLinkBox->SetValue(CSSDarkLink);
	CSSDarkLinkVisitedBox->SetValue(CSSDarkLinkVisited);
	CSSFiltersBox->SetValue(CSSFilters);
	CSSFiltersListBox->SetValue(CSSFiltersList);
	CSSDarkFiltersBox->SetValue(CSSDarkFilters);
	CSSTitleBox->SetValue(CSSTitle);
	CSSCopyrightBox->SetValue(CSSCopyright);
	CSSSectionsBox->SetValue(CSSSections);
	CSSSectionTitleBox->SetValue(CSSSectionTitle);
	CSSSectionPlusMinusBox->SetValue(CSSSectionPlusMinus);
	CSSLastSectionBox->SetValue(CSSLastSection);
	CSSTableBox->SetValue(CSSTable);
	CSSListBox->SetValue(CSSList);
	CSSListItemBox->SetValue(CSSListItem);
	CSSSubListBox->SetValue(CSSSubList);
	CSSCheckboxBox->SetValue(CSSCheckbox);
	CSSBlockquoteBox->SetValue(CSSBlockquote);
	CSSErrorBox->SetValue(CSSError);
	CSSWarningBox->SetValue(CSSWarning);
	CSSSuccessBox->SetValue(CSSSuccess);
	CSSVersionBox->SetValue(CSSVersion);
	CSSGhostBox->SetValue(CSSGhost);
	CSSCRCBox->SetValue(CSSCRC);
	CSSTagPrefixBox->SetValue(CSSTagPrefix);
	CSSDirtyBox->SetValue(CSSDirty);
	CSSQuotedMessageBox->SetValue(CSSQuotedMessage);
	CSSModBox->SetValue(CSSMod);
	CSSTagBox->SetValue(CSSTag);
	CSSNoteBox->SetValue(CSSNote);
	CSSRequirementBox->SetValue(CSSRequirement);
	CSSIncompatibilityBox->SetValue(CSSIncompatibility);

	//Tooltips.
	LogDebugOutputBox->SetToolTip(wxT("The output is logged to the BOSSDebugLog.txt file"));
	DebugSourceReferencesBox->SetToolTip(wxT("Adds source code references to command line output."));
	DebugVerbosityChoice->SetToolTip(wxT("The higher the verbosity level, the more information is outputted to the command line."));
}

void SettingsFrame::OnOKQuit(wxCommandEvent& event) {
	//Make sure the settings are saved.

	//General
	gl_do_startup_update_check = StartupUpdateCheckBox->IsChecked();
	gl_use_user_rules_editor = UseUserRuleEditorBox->IsChecked();

	//Network
	gl_proxy_host = ProxyHostBox->GetValue();
	gl_proxy_port = wxAtoi(ProxyPortBox->GetValue());
	gl_proxy_user = ProxyUserBox->GetValue();
	gl_proxy_passwd = ProxyPasswdBox->GetValue();

	//Debugging
	gl_debug_verbosity = DebugVerbosityChoice->GetSelection();
	gl_debug_with_source = DebugSourceReferencesBox->IsChecked();
	gl_log_debug_output = LogDebugOutputBox->IsChecked();

	//Filters
	UseDarkColourScheme = UseDarkColourSchemeBox->IsChecked();
	HideVersionNumbers = HideVersionNumbersBox->IsChecked();
	HideGhostedLabel = HideGhostedLabelBox->IsChecked();
	HideChecksums = HideChecksumsBox->IsChecked();
	HideMessagelessMods = HideMessagelessModsBox->IsChecked();
	HideGhostedMods = HideGhostedModsBox->IsChecked();
	HideCleanMods = HideCleanModsBox->IsChecked();
	HideRuleWarnings = HideRuleWarningsBox->IsChecked();
	HideAllModMessages = HideAllModMessagesBox->IsChecked();
	HideNotes = HideNotesBox->IsChecked();
	HideBashTagSuggestions = HideBashTagSuggestionsBox->IsChecked();
	HideRequirements = HideRequirementsBox->IsChecked();
	HideIncompatibilities = HideIncompatibilitiesBox->IsChecked();
	HideDoNotCleanMessages = HideDoNotCleanMessagesBox->IsChecked();

	//CSS
	CSSBody = CSSBodyBox->GetValue();
	CSSDarkBody = CSSDarkBodyBox->GetValue();
	CSSDarkLink = CSSDarkLinkBox->GetValue();
	CSSDarkLinkVisited = CSSDarkLinkVisitedBox->GetValue();
	CSSFilters = CSSFiltersBox->GetValue();
	CSSFiltersList = CSSFiltersListBox->GetValue();
	CSSDarkFilters = CSSDarkFiltersBox->GetValue();
	CSSTitle = CSSTitleBox->GetValue();
	CSSCopyright = CSSCopyrightBox->GetValue();
	CSSSections = CSSSectionsBox->GetValue();
	CSSSectionTitle = CSSSectionTitleBox->GetValue();
	CSSSectionPlusMinus = CSSSectionPlusMinusBox->GetValue();
	CSSLastSection = CSSLastSectionBox->GetValue();
	CSSTable = CSSTableBox->GetValue();
	CSSList = CSSListBox->GetValue();
	CSSListItem = CSSListItemBox->GetValue();
	CSSSubList = CSSSubListBox->GetValue();
	CSSCheckbox = CSSCheckboxBox->GetValue();
	CSSBlockquote = CSSBlockquoteBox->GetValue();
	CSSError = CSSErrorBox->GetValue();
	CSSWarning = CSSWarningBox->GetValue();
	CSSSuccess = CSSSuccessBox->GetValue();
	CSSVersion = CSSVersionBox->GetValue();
	CSSGhost = CSSGhostBox->GetValue();
	CSSCRC = CSSCRCBox->GetValue();
	CSSTagPrefix = CSSTagPrefixBox->GetValue();
	CSSDirty = CSSDirtyBox->GetValue();
	CSSQuotedMessage = CSSQuotedMessageBox->GetValue();
	CSSMod = CSSModBox->GetValue();
	CSSTag = CSSTagBox->GetValue();
	CSSNote = CSSNoteBox->GetValue();
	CSSRequirement = CSSRequirementBox->GetValue();
	CSSIncompatibility = CSSIncompatibilityBox->GetValue();

	//Also set the logger settings now.
	g_logger.setOriginTracking(gl_debug_with_source);
	g_logger.setVerbosity(static_cast<LogVerbosity>(LV_WARN + gl_debug_verbosity));
	if (gl_log_debug_output)
		g_logger.setStream(debug_log_path.string().c_str());

	this->Close();
}

void SettingsFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}