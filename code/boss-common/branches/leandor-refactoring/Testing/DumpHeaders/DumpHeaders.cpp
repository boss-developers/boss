//
// DumpHeaders.cpp


#include "Support/Types.h"
#include "Support/Helpers.h"
#include "Support/ModFormat.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <string>
#include <iomanip>
#include <fstream>
#include <iostream>


using namespace std;
using namespace boost::filesystem;
using namespace boost::algorithm;
using namespace boss;


int main(int argc, char* argv[])
{
	directory_iterator end;
	path root = current_path();

	cout << "Running on: " << root << endl << endl;

	path outname = root / "modinfo.txt";
	remove(outname);

	ofstream out(outname.filename().c_str());

	for (directory_iterator it(root); it != end; it++) {

		if (is_directory(it->status())) {
			continue;
		}

		path item = it->path();

		string ext = to_lower_copy(extension(*it));

		if (ext == ".esp" || ext == ".esm" || ext == ".esp.ghosted" || ext == ".esm.ghosted") {

			ModHeader header = ReadHeader(item.filename());
			string desc = header.Description;

			replace_all(desc, "\r", "");
			replace_all(desc, "\n", "|");
	
			out << "FILENAME:" << item.filename() << endl 
				<< " DESCRIP:" << trim_copy(desc) << endl 
				<< " VERSION:" << trim_copy(header.Version) << endl
				<< endl;
		}
	}

	return 0;
}
