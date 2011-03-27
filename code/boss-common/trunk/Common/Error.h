/*	Better Oblivion Sorting Software
	
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/
//Contains the BOSS exception class.

#ifndef __BOSS_ERROR_H__
#define __BOSS_ERROR_H__

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <string>

namespace boss {
	using namespace std;

	struct boss_error: virtual exception, virtual boost::exception {};
	typedef boost::error_info<struct tag_errno,string> err_detail;
}
#endif