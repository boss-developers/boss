/* 
 * Logger.cpp
 *
 * Better Oblivion Sorting Software
 *
 *
 * Quick and Dirty Load Order Utility
 * (Making C++ look like the scripting language it isn't.)
 *
 * Copyright (C) 2009-2010  Random/Random007/jpearce & the BOSS development team
 * http://creativecommons.org/licenses/by-nc-nd/3.0/
 *
 * $Revision$, $Date$
 */


#include "Logger.h"

#include <stdio.h>


// the values in the LogVerbosity enum refer to indices in this array
static const char * LOG_VERBOSITY_NAMES[] =
    { "OFF  ", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE" };


namespace boss
{
    // the global logger instance
    Logger g_logger;

    // ensures the given verbosity is within the valid range
    // returns false if the verbosity is beyond LV_OFF
    // if the verbosity is beyond LV_TRACE, sets the verbosity to LV_TRACE
    // returns true if, after limiting the upper bound, the verbosity is valid
    static inline bool _checkVerbosity (LogVerbosity & verbosity)
    {
        if (LV_OFF > verbosity)
        {
            LOG_WARN("invalid verbosity: %d", verbosity);
            return false;
        }

        if (LV_TRACE < verbosity)
        {
            LOG_DEBUG("verbosity not defined: %d;"
                      " bumping down to LV_TRACE", verbosity);

            verbosity = LV_TRACE;
        }

        return true;
    }

    // sets the default verbosity to WARN and origin tracking off
    Logger::Logger ()
        : m_verbosity(LV_WARN), m_originTracking(false)
    { }

    // sets the verbosity to the given value
    // bounds are checked and boxed by _checkVerbosity (above)
    void Logger::setVerbosity (LogVerbosity verbosity)
    {
        if (!_checkVerbosity(verbosity))
        {
            return;
        }

        m_verbosity = verbosity;
    }

    // sets whether to output log message origin information
    void Logger::setOriginTracking (bool enabled)
    {
        m_originTracking = enabled;
    }

    // formats the message and prints to stdout
    void Logger::_log (LogVerbosity verbosity, const char * fileName, 
                       int lineNo, const char * formatStr, va_list ap)
    {
        if (!_checkVerbosity(verbosity))
        {
            return;
        }

        // assumes single thread -- multithread could interleave the lines below
        // a thread safe version would use vasprintf to print to a temporary string first
        printf("%s", LOG_VERBOSITY_NAMES[verbosity]);

        // if enabled, print the log origin
        if (m_originTracking) { printf(" %s:%d", fileName, lineNo); }

        printf(": ");
        vprintf(formatStr, ap);
        printf("\n");
    }
}
