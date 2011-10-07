/*	Better Oblivion Sorting Software API

    Copyright (C) 2011  Random/Random007/jpearce, WrinklyNinja
	& the BOSS development team.
    http://creativecommons.org/licenses/by-nc-nd/3.0/

	$Revision: 2188 $, $Date: 2011-01-20 10:05:16 +0000 (Thu, 20 Jan 2011) $
*/

#ifndef BOSSAPI_H
#define BOSSAPI_H

#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
//MSVC doesn't support C99, so do the stdbool.h definitions ourselves.
//START OF stdbool.h DEFINITIONS. 
#	ifndef __cplusplus
#		define bool	_Bool
#		define true	1
#		define false   0
#	endif
#	define __bool_true_false_are_defined   1
//END OF stdbool.h DEFINITIONS.
#else
#	include <stdbool.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#	ifndef _UNICODE
#		define _UNICODE	// Tell compiler we're using Unicode, notice the _
#	endif

// set up dll import/export decorators
// when compiling the dll on windows, ensure BOSS_EXPORT is defined.  clients
// that use this header do not need to define anything to import the symbols
// properly.
#   ifdef BOSS_EXPORT
#       define BOSS_API __declspec(dllexport)
#   else
#       define BOSS_API __declspec(dllimport)
#   endif
#else
#   define BOSS_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

////////////////////////
// Types
////////////////////////

// All API strings are uint8_t* strings encoded in UTF-8.
// All API numbers and error codes are uint32_t integers.

// Abstracts the definition of BOSS's internal state while still providing
// type safety across the API.
typedef struct _boss_db_int * boss_db;

// BashTag structure gives the Unique ID number (UID) for each Bash Tag and 
// the corresponding Tag name string.
typedef struct {
    uint32_t id;
    const uint8_t * name;  // don't use char for utf-8 since char can be signed
} BashTag;

// The following are the possible error codes that the API can return.
BOSS_API extern const uint32_t BOSS_API_ERROR_OK;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_WRITE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_NOT_UTF8;
BOSS_API extern const uint32_t BOSS_API_ERROR_FILE_NOT_FOUND;
BOSS_API extern const uint32_t BOSS_API_ERROR_PARSE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_NO_MEM;
BOSS_API extern const uint32_t BOSS_API_ERROR_OVERWRITE_FAIL;
BOSS_API extern const uint32_t BOSS_API_ERROR_INVALID_ARGS;
BOSS_API extern const uint32_t BOSS_API_ERROR_MAX;

// The following are the mod cleanliness states that the API can return.
BOSS_API extern const uint32_t BOSS_API_CLEAN_NO;
BOSS_API extern const uint32_t BOSS_API_CLEAN_YES;
BOSS_API extern const uint32_t BOSS_API_CLEAN_UNKNOWN;


//////////////////////////////
// Version Functions
//////////////////////////////

// Returns whether this version of BOSS supports the API from the given 
// BOSS version. Abstracts BOSS API stability policy away from clients.
BOSS_API bool IsCompatibleVersion (uint32_t bossVersionMajor, uint32_t bossVersionMinor, uint32_t bossVersionPatch);

// Returns the version string for this version of BOSS.
// The string exists for the lifetime of the library.
BOSS_API uint32_t GetVersionString (const uint8_t ** bossVersionStr);


////////////////////////////////////
// Lifecycle Management Functions
////////////////////////////////////

// Explicitly manage database lifetime. Allows clients to free memory when
// they want/need to.
BOSS_API uint32_t CreateBossDb  (boss_db * db);
BOSS_API void     DestroyBossDb (boss_db db);


///////////////////////////////////
// Database Loading Functions
///////////////////////////////////

// Loads the masterlist and userlist from the paths specified.
// Can be called multiple times. On error, the database is unchanged.
// Paths are case-sensitive if the underlying filesystem is case-sensitive.
// masterlistPath and userlistPath are files.  dataPath is a directory.
BOSS_API uint32_t Load (boss_db db, const uint8_t * masterlistPath,
									const uint8_t * userlistPath,
									const uint8_t * dataPath);

// Re-evaluates the masterlist's regex entries, so Load() doesn't need to be called
// whenever the mods installed are changed. This doesn't need to be used if Load() is
// called, as the evaluation is incorporated into Load() too.
BOSS_API uint32_t ReEvalRegex(boss_db db, const uint8_t * dataPath);


//////////////////////////
// DB Access Functions
//////////////////////////

// Returns an array of the Bash Tags encounterred when loading the masterlist
// and userlist, and the number of tags in the returned array. The array and
// its contents are static and should not be freed by the client.
BOSS_API uint32_t GetBashTagMap (boss_db db, BashTag ** tagMap, uint32_t * numTags);

// Returns arrays of Bash Tag UIDs for Bash Tags suggested for addition and removal 
// by BOSS's masterlist and userlist, and the number of tags in each array.
// The returned arrays are valid until the db is destroyed or until the Load
// function is called.  The arrays should not be freed by the client. modName is 
// case-insensitive. If no Tags are found for an array, the array pointer (*tagIds)
// will be NULL. The userlistModified bool is true if the userlist contains Bash Tag 
// suggestion message additions.
BOSS_API uint32_t GetModBashTags (boss_db db, const uint8_t * modName, 
									uint32_t ** tagIds_added, 
									uint32_t * numTags_added, 
									uint32_t **tagIds_removed, 
									uint32_t *numTags_removed,
									bool * userlistModified);
									
// Returns the message associated with a dirty mod and whether the mod needs
// cleaning. If a mod has no dirty mmessage, *message will be NULL. modName is
// case-insensitive. The return values for needsCleaning are:
//   BOSS_API_CLEAN_NO
//   BOSS_API_CLEAN_YES
//   BOSS_API_CLEAN_UNKNOWN
// The message string is valid until the db is destroyed or until a Load
// function is called. The string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * modName, 
									const uint8_t ** message, uint32_t * needsCleaning);
									
// Writes a minimal masterlist that only contains mods that have Bash Tag suggestions, 
// plus the Tag suggestions themselves, in order to create the Wrye Bash taglist.
// outputFile is the path to use for output. If outputFile already exists, it will
// only be overwritten if overwrite is true.
BOSS_API uint32_t DumpMinimal (boss_db db, const uint8_t * outputFile, bool overwrite);


#ifdef __cplusplus
}
#endif

#endif // BOSSAPI_H