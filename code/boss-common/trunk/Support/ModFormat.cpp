/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#include <cstring>
#include <fstream>

#include <Support/Types.h>
#include <Support/Helpers.h>
#include <Support/ModFormat.h>

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

	ModHeader ReadHeader(string filename) {
		char		buffer[MAXLENGTH];
		char*		bufptr = buffer;
		ModHeader	modHeader;
		ifstream	file(filename.c_str(), ios_base::binary | ios_base::in);
	
		modHeader.Name = filename;

		// Reads the first MAXLENGHT bytes into the buffer
		file.read(&buffer[0], sizeof(buffer));

		// Check for the 'magic' marker at start
		if (Read<ulong>(bufptr) != Record::TES4){
			return modHeader;
		}

		// Next field is the total header size
		ulong headerSize = Read<ulong>(bufptr);

		// Next comes the header record Flags
		ulong flags = Read<ulong>(bufptr);

		// LSb of this record's flags is used to indicate if the 
		//	mod is a master or a plugin
		modHeader.IsMaster = (flags & 0x1) != 0;

		// Next comes the FormID...
		ulong dummy = Read<ulong>(bufptr); // skip formID

		// ...and extra flags
		dummy = Read<ulong>(bufptr); // skip flags2
	
		// Here the Header record starts, check for its signature 'HEDR'
		if (Read<ulong>(bufptr) != Record::HEDR){
			return modHeader;
		}
	
		// HEDR record has fields: DataSize, Version (0.8 o 1.0), Record Count 
		//	and Next Object Id
		ushort dataSize = Read<ushort>(bufptr);
		float version = Read<float>(bufptr);
		long numRecords = Read<long>(bufptr);
		ulong nextObjId = Read<ulong>(bufptr);

		// Then comes the sub-records
		ulong signature = Read<ulong>(bufptr);

		// OFST records are optional, skip it if it's present.
		if ( signature == Record::OFST){
			ushort size = Read<ushort>(bufptr);
			bufptr += size; // skip
			signature = Read<ulong>(bufptr);
		}

		// Same for DELE...
		if ( signature == Record::DELE){
			ushort size = Read<ushort>(bufptr);
			bufptr += size; // skip
			signature = Read<ulong>(bufptr);
		}

		// Then check for CNAM, which is author name
		if ( signature == Record::CNAM){
			ushort size = Read<ushort>(bufptr);
			modHeader.Author = ReadString(bufptr, size);
			signature = Read<ulong>(bufptr);
		}

		// Then check for SNAM, which is the mod's description field.
		if ( signature == Record::SNAM){
			ushort size = Read<ushort>(bufptr);
			modHeader.Description = ReadString(bufptr, size);
			modHeader.Version = ParseVersion(modHeader.Description);
		}

		// We should have all the required information.
		return modHeader;
	}

};