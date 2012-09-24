/*	libloadorder

	A library for reading and writing the load order of plugin files for
	TES III: Morrowind, TES IV: Oblivion, TES V: Skyrim, Fallout 3 and
	Fallout: New Vegas.

    Copyright (C) 2012    WrinklyNinja

	This file is part of libloadorder.

    libloadorder is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    libloadorder is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libloadorder.  If not, see 
	<http://www.gnu.org/licenses/>.
*/

#ifndef LIBLO_EXCEPTION_H
#define LIBLO_EXCEPTION_H

#include <stdint.h>
#include <string>

namespace liblo {

	class error {
	public:
		error();
		error(uint32_t code);
		error(uint32_t code, std::string subject);
		error(uint32_t code, std::string subject, std::string detail);

		std::string	what()			const;
		uint32_t	code()			const;
	private:
		uint32_t errCode;
		std::string errSubject;
		std::string errDetail;
	};

	extern error lastException;
}

#endif