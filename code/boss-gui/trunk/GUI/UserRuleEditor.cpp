/*	General User Interface for BOSS (Better Oblivion Sorting Software)
	
	Providing a graphical frontend to BOSS's functions.

    Copyright (C) 2011 WrinklyNinja & the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/


	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#include "GUI/UserRuleEditor.h"
#include "GUI/ElementIDs.h"

#include <boost/spirit/include/karma.hpp>
#include <boost/algorithm/string/case_conv.hpp>

using namespace boss;
using namespace std;
namespace karma = boost::spirit::karma;
namespace unicode = boost::spirit::unicode;

using boost::algorithm::to_upper_copy;

UserRulesEditorFrame::UserRulesEditorFrame(const wxChar *title, wxFrame *parent) : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize) {

	//Need to parse userlist, masterlist and build modlist.
	BuildModlist(Modlist);

	//Parse masterlist/modlist backup into data structure.
	LOG_INFO("Starting to parse sorting file: %s", masterlist_path.string().c_str());
	try {
		parseMasterlist(masterlist_path,Masterlist);
	} catch (boss_error e) {
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString()+" Scanning for plugins aborted. User Rules Editor cannot load.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	//Check if parsing failed. If so, exit with errors.
	if (!masterlistErrorBuffer.empty()) {
		size_t size = masterlistErrorBuffer.size();
		for (size_t i=0; i<size; i++)  //Print parser error messages.
			Output(masterlistErrorBuffer[i]);
		wxMessageBox(wxString::Format(
				wxT("Error: "+masterlistErrorBuffer[0]+" Masterlist parsing aborted. User Rules Editor cannot load.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
		return;
	}

	LOG_INFO("Starting to parse userlist.");
	try {
		bool parsed = parseUserlist(userlist_path,Userlist);
		if (!parsed)
			Userlist.clear();  //If userlist has parsing errors, empty it so no rules are applied.
	} catch (boss_error e) {
		LOG_ERROR("Error: %s", e.getString().c_str());
		wxMessageBox(wxString::Format(
				wxT("Error: "+e.getString()+" Userlist parsing aborted. User Rules Editor cannot load existing rules.")
			),
			wxT("BOSS: Error"),
			wxOK | wxICON_ERROR,
			NULL);
	}
}
/*
struct ruleKeys_ : karma::symbols<keyType, string> {
	ruleKeys_::ruleKeys_() {
		add
			(ADD,"ADD")
			(OVERRIDE,"OVERRIDE")
			(FOR,"FOR")
		;
	}
} ruleKeys;

struct messageKeys_ : karma::symbols<keyType, string> {
	messageKeys_::messageKeys_() {
		add
			(APPEND,"APPEND")
			(REPLACE,"REPLACE")
			(BEFORE,"BEFORE")
			(AFTER,"AFTER")
			(TOP,"TOP")
			(BOTTOM,"BOTTOM")
		;
	}
} sortOrMessageKeys;

struct my_grammar : karma::grammar<back_insert_iterator<string>,  vector<rule>()>
{
	my_grammar() : my_grammar::base_type(list, "userlist generator")
	{
		// Rule definitions
		list = rule % (karma::eol << karma::eol);

		rule = 
			ruleKeys << ": " << unicode::string << karma::eol
			<< ruleline % karma::eol;

		ruleline = sortOrMessageKeys << ": " << unicode::string;

		ruleKey = ruleKeys;

		sortOrMessageKey = sortOrMessageKeys;
	}

	karma::rule<back_insert_iterator<string>, vector<rule>() > list;
	karma::rule<back_insert_iterator<string>, rule()> rule;
	karma::rule<back_insert_iterator<string>, line()> ruleline;
	karma::rule<back_insert_iterator<string>, keyType()> sortOrMessageKey, ruleKey;
};
*/

void UserRulesEditorFrame::OnOKQuit(wxCommandEvent& event) {
	//Use BOOST Spirit.Karma to form userlist output.

	/*my_grammar gram;

	string out;
	back_insert_iterator<string> sink(out);
	bool r = karma::generate(sink,gram,Userlist);
	*/
	ofstream outFile(userlist_path.c_str(),ios_base::trunc);
	//outFile << out;

	size_t size = Userlist.size();
	for (size_t i=0;i<size;i++) {
		outFile << to_upper_copy(KeyToString(Userlist[i].ruleKey)) << ": " << Userlist[i].ruleObject << endl;
		size_t linesSize = Userlist[i].lines.size();
		for (size_t j=0;j<linesSize;j++) {
			outFile << to_upper_copy(KeyToString(Userlist[i].lines[j].key)) << ": " << Userlist[i].lines[j].object << endl;
		}
		outFile << endl;
	}


	outFile.close();

	this->Close();
}

void UserRulesEditorFrame::OnCancelQuit(wxCommandEvent& event) {
	this->Close();
}