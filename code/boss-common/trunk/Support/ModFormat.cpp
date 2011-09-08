/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2488 $, $Date: 2011-03-27 14:31:33 +0100 (Sun, 27 Mar 2011) $
*/


#include "Types.h"
#include "Helpers.h"
#include "ModFormat.h"

#include <cstring>
#include <fstream>

namespace boss {
	using namespace std;

	//-
	// ModHeader ReadHeader(string):
	//	- Parses the mod file contents and extract the header information 
	//	returning the most important data using a ModHeader struct.
	//	--> see: 
	//			http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format, 
	//
	//	and in particular this link: 
	//			http://www.uesp.net/wiki/Tes4Mod:Mod_File_Format/TES4
	//

	ModHeader ReadHeader(boost::filesystem::path filename) {
		char		buffer[MAXLENGTH];
		char*		bufptr = buffer;
		ModHeader	modHeader;
		ifstream	file(filename.native().c_str(), ios_base::binary | ios_base::in);
	
		modHeader.Name = filename.string();

		// Reads the first MAXLENGTH bytes into the buffer
		file.read(&buffer[0], sizeof(buffer));

		// Check for the 'magic' marker at start
		if (Read<uint>(bufptr) != Record::TES4){
			return modHeader;
		}

		// Next field is the total header size
		/*uint headerSize =*/ Read<uint>(bufptr);

		// Next comes the header record Flags
		uint flags = Read<uint>(bufptr);

		// LSb of this record's flags is used to indicate if the 
		//	mod is a master or a plugin
		modHeader.IsMaster = (flags & 0x1) != 0;

		// Next comes the FormID...
		/*uint formId =*/ Read<uint>(bufptr); // skip formID

		// ...and extra flags
		/*uint flags2 =*/ Read<uint>(bufptr); // skip flags2
	
		// Here the Header record starts, check for its signature 'HEDR'
		if (Read<uint>(bufptr) != Record::HEDR){
			return modHeader;
		}
	
		// HEDR record has fields: DataSize, Version (0.8 o 1.0), Record Count 
		//	and Next Object Id
		/*ushort dataSize =*/ Read<ushort>(bufptr);
		/*float version =*/ Read<float>(bufptr);
		/*int numRecords =*/ Read<int>(bufptr);
		/*uint nextObjId =*/ Read<uint>(bufptr);

		// Then comes the sub-records
		uint signature = Read<uint>(bufptr);

        // skip optional records
        bool loop = true;
        while (loop){
            switch (signature)
            {
            case Record::OFST:
            case Record::DELE:
                bufptr += Read<ushort>(bufptr); // skip
                signature = Read<uint>(bufptr);
                break;

		    // extract author name, if present
            case Record::CNAM:
                modHeader.Author = ReadString(bufptr, Read<ushort>(bufptr));
                signature = Read<uint>(bufptr);
                break;

		    // extract description and version, if present
            case Record::SNAM:
                modHeader.Description = ReadString(bufptr, Read<ushort>(bufptr));
			    modHeader.Version     = ParseVersion(modHeader.Description);
                signature = Read<uint>(bufptr);
                break;

            default:
                loop = false;
            }
        }

		// We should have all the required information.
		return modHeader;
	}
}