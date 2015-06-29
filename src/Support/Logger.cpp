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

	$Revision: 1946 $, $Date: 2010-11-27 23:12:48 +0000 (Sat, 27 Nov 2010) $
*/


#include "Support/Logger.h"


// The values in the LogVerbosity enum refer to indices in this array
static const char * LOG_VERBOSITY_NAMES[] = {"OFF  ", "ERROR", "WARN ",
                                             "INFO ", "DEBUG", "TRACE"};


namespace boss {
	// The global logger instance
	BOSS_COMMON Logger g_logger;

	/*
	 * Ensures the given verbosity is within the valid range
	 * Returns false if the verbosity is beyond LV_OFF
	 * If the verbosity is beyond LV_TRACE, sets the verbosity to LV_TRACE
	 * Returns true if, after limiting the upper bound, the verbosity is valid
	 */
	static inline bool _checkVerbosity(LogVerbosity & verbosity) {
		if (LV_OFF > verbosity) {
			LOG_WARN("invalid verbosity: %d", verbosity);
			return false;
		}

		if (LV_TRACE < verbosity) {
			LOG_DEBUG("verbosity not defined: %d;"
			          " bumping down to LV_TRACE", verbosity);

			verbosity = LV_TRACE;
		}

		return true;
	}

	// Sets the default verbosity to WARN and origin tracking off
	Logger::Logger() : m_verbosity(LV_WARN), m_out(stdout) {}

	// Sets the verbosity to the given value
	// Bounds are checked and boxed by _checkVerbosity (above)
	void Logger::setVerbosity(LogVerbosity verbosity) {
		if (!_checkVerbosity(verbosity)) {
			return;
		}

		m_verbosity = verbosity;
	}

	// Formats the message and prints to stdout
	void Logger::_log(LogVerbosity verbosity, const char * formatStr,
	                  va_list ap) {
		if (!_checkVerbosity(verbosity)) {
			return;
		}

		// Assumes single thread -- multithread could interleave the lines below
		// A thread safe version would use vasprintf to print to a temporary string first
		printf("%s", LOG_VERBOSITY_NAMES[verbosity]);

		printf(": ");
		vprintf(formatStr, ap);
		printf("\n");
		fflush(stdout);

		if (m_out != stdout) {
			fprintf(m_out, "%s: ", LOG_VERBOSITY_NAMES[verbosity]);
			vfprintf(m_out, formatStr, ap);
			fprintf(m_out, "\n");
			fflush(m_out);
		}
	}
}  // namespace boss
