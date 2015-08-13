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

	$Revision: 1783 $, $Date: 2010-10-31 23:05:28 +0000 (Sun, 31 Oct 2010) $
*/

#ifndef SUPPORT_LOGGER_H_
#define SUPPORT_LOGGER_H_

#include <cstdarg>
#include <cstdio>

#include "common/dll_def.h"
#include "support/platform.h"

/*
 * TODO(MCP): Look at replacing this with a portable version so it's not relying on extensions.
 * Possibly make formatStr part of the variable arguments?
 */
#define _LOG_IMPL(verbosity, formatStr, ...) \
	boss::g_logger.log(verbosity, formatStr, ##__VA_ARGS__)

// Convenience macros
#define LOG_ERROR(formatStr, ...) _LOG_IMPL(boss::LV_ERROR, formatStr, ##__VA_ARGS__)
#define LOG_WARN(formatStr,  ...) _LOG_IMPL(boss::LV_WARN,  formatStr, ##__VA_ARGS__)
#define LOG_INFO(formatStr,  ...) _LOG_IMPL(boss::LV_INFO,  formatStr, ##__VA_ARGS__)
#define LOG_DEBUG(formatStr, ...) _LOG_IMPL(boss::LV_DEBUG, formatStr, ##__VA_ARGS__)
#define LOG_TRACE(formatStr, ...) _LOG_IMPL(boss::LV_TRACE, formatStr, ##__VA_ARGS__)


namespace boss {

enum LogVerbosity {
	LV_OFF   = 0,
	LV_ERROR = 1,
	LV_WARN  = 2,
	LV_INFO  = 3,
	LV_DEBUG = 4,
	LV_TRACE = 5
};

// A simple logging class.  Not implemented to be thread safe.
class BOSS_COMMON Logger {
 public:
	Logger();

	// Sets the verbosity limit
	void setVerbosity(LogVerbosity verbosity);

	// Sets the output stream
	inline void setStream(const char *file) {
		m_out = std::fopen(file, "w");
	}

	// For use when calculating the arguments to a LOG macro would be expensive
	inline bool isDebugEnabled() {
		return _isVerbosityEnabled(LV_DEBUG);
	}
	inline bool isTraceEnabled() {
		return _isVerbosityEnabled(LV_TRACE);
	}

	// If a message is of a sufficient verbosity, outputs the given message
	inline void log(LogVerbosity verbosity, const char *formatStr, ...) __attribute__((__format__ (__printf__, 5, 6)))
	{
		if (_isVerbosityEnabled(verbosity)) {
			std::va_list ap;
			va_start(ap, formatStr);
			_log(verbosity, formatStr, ap);
			va_end(ap);
		}
	}

 private:
	LogVerbosity m_verbosity;
	FILE *m_out;

	inline bool _isVerbosityEnabled(LogVerbosity verbosity) {
		return verbosity <= m_verbosity;
	}

	void _log(LogVerbosity verbosity, const char *formatStr, std::va_list ap);
};

// Declare global logger
BOSS_COMMON extern Logger g_logger;

}  // namespace boss
#endif  // SUPPORT_LOGGER_H_
