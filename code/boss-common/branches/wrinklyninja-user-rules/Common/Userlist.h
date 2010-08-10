/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 1284 $, $Date: 2010-08-05 03:43:59 +0100 (Thu, 05 Aug 2010) $
*/

#ifndef __BOSS_USERLIST_H__
#define __BOSS_USERLIST_H__

#include <string>
#include <fstream>
#include "Globals.h"
#include <Support/Types.h>
#include <Support/Helpers.h>
#include <vector>

namespace boss {
	using namespace std;
		class Rules {
	public:
		vector<string> l1substr,l2substr,groupcontent;
		vector<char> l1symb,l2symb;
		void AddRules(char * file);									
		void RemoveRules(vector<int> indices,ofstream output);		
		string GroupOpener(string groupname);		//Returns the BeginGroup line for a group, as it appears in the masterlist. For comparison.
		string GetGroupContent(string groupname);	//Stores the content of the given group, as in the masterlist.
		int FindRule(string search);
		int FindRule(string search,int line);
		void PrintRules(ofstream& output);
	};

	//Debug function, just prints the object contents to the output file stream given.
	void Rules::PrintRules(ofstream& output) {
		output << "Rule dump commencing..." << endl << endl;
		for (int i=0;i<(int)l1substr.size();i++) {
			output << l1symb[i] << l1substr[i] << endl
					<< l2symb[i] << l2substr[i] << endl << endl;
		}
		output << "Rule dump finished.";
	}

	//Add rules from a file into the rules object.
	void Rules::AddRules(char * filename) {
		ifstream userlist;
		userlist.open(filename);
		string firstline,secondline,line;
		bool foundfirstline = false;
		while(GetLine(userlist,line)) {
			if (line[0]=='*' || line[0]=='\?' || line[0]==':' || line[0]=='\"') {
				l1substr.push_back(Tidy(line).substr(1));
				l1symb.push_back(Tidy(line)[0]);
			} else if (line[0]=='<' || line[0]=='>' || line[0]=='/' || line[0]=='|') {
				l2substr.push_back(Tidy(line).substr(1));
				l2symb.push_back(Tidy(line)[0]);
			}
		}
		groupcontent.resize(l1substr.size());
		userlist.close();
	}

	//Removes rules from the rules object as specified by the values of the vector input.
	//Only needed for removing mod addition rules that already exist in the masterlist.
	void Rules::RemoveRules(vector<int> indicies,ofstream output) {
		for (int i=0;i<(int)indicies.size();i++) {
			output << "\"" << l1substr[indicies[i]-i] << "\" already present in masterlist.txt. Sorting rule skipped." << endl << endl;
			l1substr.erase(l1substr.begin()+indicies[i]-i);
			l2substr.erase(l2substr.begin()+indicies[i]-i);
			groupcontent.erase(groupcontent.begin()+indicies[i]-i);
			l1symb.erase(l1symb.begin()+indicies[i]-i);
			l2symb.erase(l2symb.begin()+indicies[i]-i);
		}
	}

	//Returns formatted opening tag of the specified group.
	string Rules::GroupOpener(string groupname) {
		return ("\\BeginGroup\\: "+groupname.substr(1));
	}

	//returns the index of the rule that contains the search string, or -1 if it is not found.
	int Rules::FindRule(string masterlistline) {
		for (int i=0;i<(int)l1substr.size();i++) {
			if (l1substr[i]==masterlistline) return i;
			if (l2substr[i]==masterlistline) return i;
		}
		return -1;
	}

	//returns the index of the rule that contains the search string on the given line, or -1 if not found.
	int Rules::FindRule(string masterlistline,int line) {
		for (int i=0;i<(int)l1substr.size();i++) {
			if (line=1) {
				if (l1substr[i]==masterlistline) return i;
			} else if (line=2) {
				if (l2substr[i]==masterlistline) return i;
			}
		}
		return -1;
	}
}

#endif __BOSS_USERLIST_H__