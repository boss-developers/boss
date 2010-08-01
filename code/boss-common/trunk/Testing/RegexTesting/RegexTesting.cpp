// RegexTesting.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <iomanip>
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


string Trim(string& text) {
	
	return text;
}

int main(int argc, char* argv[])
{
	directory_iterator end;
	path root = current_path();

	path outname = root / "testing.txt";
	remove(outname);

	ofstream out(outname.filename());
	out << "Running on: " << root << endl << endl;

	for (directory_iterator it(root); it != end; it++) {

		if (is_directory(it->status())) {
			continue;
		}

		path item = it->path();
		
		string ext = extension(*it);
		to_lower(ext);

		if (ext == ".esp" || ext == ".esm") {

			ModHeader header = ReadHeader(item.filename());

			out << " * " << item.filename() << endl;

			if (!header.Version.empty()) {
				out << "  * VERSION: [" << trim_copy(header.Version) << "]" << endl;
			}

			out << "  * DESCR: [" << replace_all_copy(header.Description, "\r\n", "|") << "]" << endl << endl;
		}
	}
	
	return 0;
}

