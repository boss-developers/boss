//Contains BOSS masterlist.txt update function stuff, so that the main BOSS.cpp doesn't get cluttered with it.
//Once it works, it's a simple matter of including it in BOSS.cpp and uncommenting the function call there.

//Need to statically link libcurl.lib. (actually curllib.lib, for some reason the former doesn't work properly.)
#define CURL_STATICLIB
#include <stdio.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

int writer(char *data, size_t size, size_t nmemb, string *buffer){
	int result = 0;
	if(buffer != NULL) {
		buffer -> append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
} 

int UpdateMasterlist(string path, int game) {
	char *url;
	if (game == 1) url = "http://better-oblivion-sorting-software.googlecode.com/svn/masterlist.txt";
	else if (game == 2) url = "http://better-oblivion-sorting-software.googlecode.com/svn/FO3Masterlist/masterlist.txt";
	else if (game == 3) url = "http://better-oblivion-sorting-software.googlecode.com/svn/MWmasterlist/masterlist.txt";

	//Get SVN masterlist.
    CURL *curl;
	string buffer,revision;
    curl = curl_easy_init();
	ofstream file;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
		file.open((path+".tmp").c_str());
		file << buffer;
		file.close();
    }
	//Get HEAD revision number from http://better-oblivion-sorting-software.googlecode.com/svn/ page text.
	curl = curl_easy_init();
	if (curl){
		curl_easy_setopt(curl, CURLOPT_URL, "http://better-oblivion-sorting-software.googlecode.com/svn/");
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);	 /* No we don't need the Header of the web content. Set to 0 and curl ignores the first line */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0); /* Don't follow anything else than the particular url requested*/
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	/* Function Pointer "writer" manages the required buffer size */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer ); /* Data Pointer &buffer stores downloaded web content */
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	int start = buffer.find("Revision ");
	int end = buffer.find(": /");
	end = end - (start+9);
	revision = buffer.substr(start+9,end);
	//Add revision number to masterlist and fix the line breaks.
	string oldline = "? Masterlist Information: $Revision$, $Date$, $LastChangedBy$";
	string newline = "? Masterlist Revision: "+revision;
	ifstream in;
	ofstream out;
	in.open((path+".tmp").c_str());
	out.open(path.c_str());
	while (!in.eof()) {	
		char cbuffer[4096];
		in.getline(cbuffer,4096);
		string textbuf=(string)cbuffer;
		if (textbuf.length()>0) {
			int pos = textbuf.find(oldline);
			if (pos > -1) {
				out << newline << endl;
			} else {
				pos = textbuf.find("\r");
				textbuf.replace(pos,2,"\n");
				out << textbuf;
			}
		}
	}
	in.close();
	out.close();
	//Remove temporary masterlist file.
	system (("del "+path+".tmp").c_str());
	//Return revision number
	stringstream ss(revision);
	ss >> end;
	return end;
}