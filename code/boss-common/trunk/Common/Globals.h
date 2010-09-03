/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision$, $Date$
*/

#ifndef __BOSS_GLOBALS_H__
#define __BOSS_GLOBALS_H__

#include <string>
#include <fstream>

namespace boss {
	using namespace std;

	extern ifstream order;						//masterlist.txt - the grand mod order list
	extern ofstream bosslog;					//BOSSlog.txt - output file.
	extern bool fcom;							//true if key FCOM files are found.
	extern bool ooo;                      	 	//true if OOO esm is found.
	extern bool bc;                        	//true if Better Cities esm is found.
	extern bool fook2;							//true if key FOOK2 files are found.
	extern bool fwe;							//true if FWE esm is found
};

#endif __BOSS_GLOBALS_H__
