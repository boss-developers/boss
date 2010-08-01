/*	Better Oblivion Sorting Software
	2.0 Beta
	Quick and Dirty Load Order Utility
	(Making C++ look like the scripting language it isn't.)

    	Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
    	http://creativecommons.org/licenses/by-nc-nd/3.0/

	 $Id: VersionRegex.h 1200 2010-07-29 22:51:09Z leandor@gmail.com $
	$URL: https://better-oblivion-sorting-software.googlecode.com/svn/BOSS%20source%20code/Support/VersionRegex.h $
*/

#ifndef __SUPPORT_VERSIONREGEX__HPP__
#define __SUPPORT_VERSIONREGEX__HPP__

#include <boost/regex.hpp>

namespace boss {

	using namespace boost;

	/// Array used to try each of the expressions defined above using 
	/// an iteration for each of them.
	regex* version_checks[];

};


#endif __SUPPORT_VERSIONREGEX__HPP__