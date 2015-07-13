/*	BOSS

	A "one-click" program for users that quickly optimises and avoids
	detrimental conflicts in their TES IV: Oblivion, Nehrim - At Fate's Edge,
	TES V: Skyrim, Fallout 3 and Fallout: New Vegas mod load orders.

	Copyright (C) 2009-2012    BOSS Development Team.

	This file is part of BOSS.

	BOSS is free software: you can redistribute
	it and/or modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	BOSS is distributed in the hope that it will
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with BOSS.  If not, see
	<http://www.gnu.org/licenses/>.

	$Revision: 2488 $, $Date: 2011-03-27 14:31:33 +0100 (Sun, 27 Mar 2011) $
*/

#ifndef SUPPORT_MOD_FORMAT_H_
#define SUPPORT_MOD_FORMAT_H_

#include <string>

#include <boost/filesystem.hpp>

namespace boss {

namespace fs = boost::filesystem;

// Structure for grouping the information gathered from each mod's header.
struct ModHeader {
	std::string Name;
	std::string Description;
	std::string Author;
	std::string Version;
	bool IsMaster;
};


struct Record {
	static const unsigned long TES4 = '4SET';
	static const unsigned long HEDR = 'RDEH';
	static const unsigned long OFST = 'TSFO';
	static const unsigned long DELE = 'ELED';
	static const unsigned long CNAM = 'MANC';
	static const unsigned long SNAM = 'MANS';
};

ModHeader ReadHeader(fs::path filename);

bool IsPluginMaster(fs::path filename);  // Shorter version of the above, to only get master flag.

}  // namespace boss
#endif  // SUPPORT_MOD_FORMAT_H_
