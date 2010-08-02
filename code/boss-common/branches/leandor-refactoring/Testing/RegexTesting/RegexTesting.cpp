// RegexTesting.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <iomanip>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <Support/Types.h>
#include <Support/Helpers.h>
#include <Support/ModFormat.h>

using namespace std;
using namespace boost::filesystem;
using namespace boost::algorithm;
using namespace boss;

boost::format InvalidInputError = boost::format("Invalid input file format. Expected: '%1%' but found '%2%'");

int main(int argc, char* argv[])
{
	try
	{
		path root = current_path();
	
		path inname = root / "VersionCheck.txt";
	
		ifstream in(inname.filename());
	
		cout << "Reading: " << inname.filename() << endl << endl;
	
		string line;
		while (in) {
	
			while (in) {
				if (! ReadLine(in, line)) {
					return 0;
				}

				trim(line);

				if (! line.empty()) 
					break;
			}
	
			if (!starts_with(line, "FILENAME:")) throw exception((InvalidInputError % "FILENAME:" % line).str().c_str());
			replace_first(line, "FILENAME:", "");
			trim(line);
			string modfile = line;
	
			if (! ReadLine(in, line)) {
				throw exception("No more data to read.");
			}
	
			trim(line);
			if (!starts_with(line, "DESCRIP:")) throw exception((InvalidInputError % "DESCRIP:" % line).str().c_str());
			replace_first(line, "DESCRIP:", "");
			replace_all(line, "|", "\n");
			string descr = line;
	
			if (! ReadLine(in, line)) {
				throw exception("No more data to read.");
			}
	
			trim(line);
			if (!starts_with(line, "VERSION:")) throw exception((InvalidInputError % "VERSION:" % line).str().c_str());
			replace_first(line, "VERSION:", "");
			string version = line;		
	
			string text = ParseVersion(descr);
	
			if (!equals(text, version)) {
				cout << (boost::format("%1%: Failed.\n - Expected: [%2%]\n - Extracted [%3%]") % modfile % version % text).str() << endl << endl;
			}
		}
		
		return 0;
	}
	catch (exception& e)
	{
		cerr << "Error: " << e.what() << endl;
		return 1;
	}
}

