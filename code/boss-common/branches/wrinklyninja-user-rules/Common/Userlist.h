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
		vector<string> l1substr,l2substr,modgroupcontent;
		vector<char> l1symb,l2symb;
		void AddRules(char * file);
		void CheckRules(vector<int> toremove);
		void RemoveRules(vector<int> indices,ofstream& output);		
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
		string line;
		userlist.open(filename);
		while(GetLine(userlist,line)) {
			if (line[0]=='*' || line[0]=='\?' || line[0]==':' || line[0]=='\"') {
				l1substr.push_back(Tidy(line).substr(1));
				l1symb.push_back(Tidy(line)[0]);
			} else if (line[0]=='<' || line[0]=='>' || line[0]=='/' || line[0]=='|') {
				l2substr.push_back(Tidy(line).substr(1));
				l2symb.push_back(Tidy(line)[0]);
			}
		}
		modgroupcontent.resize(l1substr.size());
		userlist.close();
	}

	//Checks the rules against the masterlist.
	//Adds already-present mod addition rules' indicies to the toremove vector for later removal.
	//Fills the modgroupcontent vector with the contents of each sorting override rule's mod or group.
	void Rules::CheckRules(vector<int> toremove) {
		for (int i=0;i<(int)l1substr.size();i++) {
			if (l1symb[i]=='\?') {		//plugin addition

			} else if (l1symb[i]==':' || l1symb[i]=='*') {	//plugin or group override

			}
		}
	}

	//Removes rules from the rules object as specified by the values of the vector input.
	//Only needed for removing mod addition rules that already exist in the masterlist.
	void Rules::RemoveRules(vector<int> indicies,ofstream& output) {
		for (int i=0;i<(int)indicies.size();i++) {
			output << "\"" << l1substr[indicies[i]-i] << "\" already present in masterlist.txt. Sorting rule skipped." << endl << endl;
			l1substr.erase(l1substr.begin()+indicies[i]-i);
			l2substr.erase(l2substr.begin()+indicies[i]-i);
			modgroupcontent.erase(modgroupcontent.begin()+indicies[i]-i);
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

	class Modlist {
	public:
		vector<string> mod;
		void AddMods();
		void RemoveMods(vector<int> indicies);
		void PrintModList(ofstream& out);
	};

	//Adds mods in directory to Modlist.
	void Modlist::AddMods() {
		//First list mods in file.
		if (FileExists ("BOSS\\modlist.txt")) {	//add an additional undo level just in case.
			if (FileExists ("BOSS\\modlist.old")) {
				system ("attrib -r BOSS\\modlist.old");	//Clears read only attribute of modlist.old if present, so we can delete the file.
				system ("del BOSS\\modlist.old");
			}
			system ("attrib -r BOSS\\modlist.txt");	//Clears read only attribute of modlist.txt if present, so we can rename the file.
			system ("ren BOSS\\modlist.txt modlist.old");
		}
		system ("dir *.es? /a:-d /b /o:d /t:w > BOSS\\modlist.txt");
		ifstream list;
		list.open("BOSS\\modlist.txt");
		string line;
		while(GetLine(list,line)) {
			mod.push_back(line);
		}
	}

	void Modlist::RemoveMods(vector<int> indicies) {
		for (int i=0;i<(int)indicies.size();i++) {
			mod.erase(mod.begin()+indicies[i]-i);
		}
	}

	//Debug output function.
	void Modlist::PrintModList(ofstream& out) {
		for (int i=0;i<(int)mod.size();i++) {
			out << mod[i] << endl;
		}
	}

}

#endif __BOSS_USERLIST_H__