//
// BOSSAPI.h
//
// <copyright header>

#ifndef BOSSAPI_H
#define BOSSAPI_H

#include <boost/cstdint.hpp>

//MSVC doesn't support C99, so do the stdbool.h definitions ourselves.

#ifndef __cplusplus

#define bool	_Bool
#define true	1
#define false   0

#else /* __cplusplus */

/* Supporting <stdbool.h> in C++ is a GCC extension.  */
#define _Bool   bool
#define bool	bool
#define false   false
#define true	true

#endif /* __cplusplus */

/* Signal that all the definitions are present.  */
#define __bool_true_false_are_defined   1

//Test definition of BOSS_EXPORT
#define BOSS_EXPORT

// set up dll import/export decorators
// when compiling the dll on windows, ensure BOSS_EXPORT is defined.  clients
// that use this header do not need to define anything to import the symbols
// properly.
#if defined(_WIN32) || defined(_WIN64)
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

//////////////////////////////
// types

// abstracts the definition of BOSS's internal state while still providing
// type safety across the API
typedef struct _boss_db_int * boss_db;

// Bash Tags are added to a hashset internally which holds their string name 
// against the string's index in the hashset.
// The API then creates a BashTag structure from the index and string which it can
// serve. The GetBashTagMap() function returns an array of all these BashTag structures
// and the GetModBashTags() returns only the indexes of tags found in the internal hashset.

// Clients then use the array of BashTag structures to lookup the corresponding string name
// of a given index.

// The BashTag structure is a frontend for a hashset which holds  

// name is encoded in UTF-8 (though in practice it won't be more than ASCII)
typedef struct
{
    uint32_t id;
    uint8_t * name;  // don't use char for utf-8 since char can be signed
} BashTag;

// These are returned from the functions that return an uint32_t.  We define
// error codes instead of error messages so client programs can deal
// with the message localization.
extern BOSS_API const uint32_t BOSS_ERROR_SUCCESS;
extern BOSS_API const uint32_t BOSS_ERROR_BAD_ARGUMENT;
extern BOSS_API const uint32_t BOSS_ERROR_FILE_NOT_FOUND;
extern BOSS_API const uint32_t BOSS_ERROR_FILE_ACCESS_DENIED;
extern BOSS_API const uint32_t BOSS_ERROR_FILE_INVALID_SYNTAX;
extern BOSS_API const uint32_t BOSS_ERROR_NO_MEM;
extern BOSS_API const uint32_t BOSS_ERROR_INTERNAL;
// ... other error codes
extern BOSS_API const uint32_t BOSS_ERROR_MAX;


//////////////////////////////
// version functions

// returns whether this version of the BOSS library supports the API from the
// given BOSS library version.  this abstracts the BOSS API stability policy
// away from clients that use this library.
BOSS_API bool IsCompatibleVersion (uint32_t bossVersionMajor, uint32_t bossVersionMinor);

// returns the version string for the library in utf-8, used for display
// the string exists for the lifetime of the library
BOSS_API uint32_t GetVersionString (uint8_t ** bossVersionStr);


//////////////////////////////
// data definition functions

// returns an array of BashTags and the number of tags in the returned array
// the array and its contents are static and should not be freed by the client
BOSS_API void GetBashTagMap (boss_db db, BashTag ** tagMap, uint32_t * numTags);


////////////////////////////////////
// lifecycle management functions

// explicitly manage the lifetime of the database.  this way the client can
// free up the memory when it wants/needs to, for example when the process is
// low on memory.
BOSS_API uint32_t CreateBossDb  (boss_db * db);
BOSS_API void     DestroyBossDb (boss_db db);


/////////////////////////////////////
// path setting/getting functions.

// These functions are required to interface with the BOSS internal code correctly and
// avoid any issues that may be caused by not globally setting the correct paths.
// Path strings are in UTF-8. Path is case sensitive if the underlying file system is 
// case sensitive

BOSS_API uint32_t GetDataPath(uint8_t * path);
BOSS_API uint32_t GetBOSSPath(uint8_t * path);
BOSS_API uint32_t GetMasterlistFilename(uint8_t * filename);

BOSS_API uint32_t SetDataPath(const uint8_t * path);
BOSS_API uint32_t SetBOSSPath(const uint8_t * path);
BOSS_API uint32_t SetMasterlistFilename(const uint8_t * filename);


/////////////////////////////////////
// database loading functions.

// can be called multiple times.  if masterlist is loaded, then userlist is
// loaded, then masterlist is loaded again, the previously-loaded userlist
// should still be applied over the new masterlist.  path strings are in
// UTF-8.  on error, database is expected to be unchanged.  path is case
// sensitive if the underlying file system is case sensitive
BOSS_API uint32_t LoadMasterlist (boss_db db);
BOSS_API uint32_t LoadUserlist   (boss_db db);


/////////////////////////////////////
// db access functions

// returns an array of tagIds and the number of tags.  if there are no tags,
// *tagIds can be NULL.  modName is encoded in utf-8 and is not case sensitive
// the returned arrays are valid until the db is destroyed or until a Load
// function is called.  The arrays should not be freed by the client.
BOSS_API uint32_t GetModBashTags (boss_db db, const uint8_t * modName, uint32_t ** tagIds_added, uint32_t * numTags_added, uint32_t **tagIds_removed, uint32_t *numTags_removed);

// returns the message associated with a dirty mod and whether the mod needs
// cleaning.  if a mod has no dirty message, *message should be NULL.  modName
// is encoded in utf-8 and is not case sensitive.  message, if not NULL, must
// be encoded in utf-8
// needsCleaning:
//   0 == no
//   1 == yes
//   2 == unknown
// the message string is valid until the db is destroyed or until a Load
// function is called.  the string should not be freed by the client.
BOSS_API uint32_t GetDirtyMessage (boss_db db, const uint8_t * modName, uint8_t ** message, uint32_t * needsCleaning);

// writes out a minimal masterlist that only contains mods that have tags (plus
// the tags themselves) in order to create the Wrye Bash taglist.  outputFile
// is a UTF-8 string specifying the path to use for output.  if it already
// exists, outputFile will be overwritten iff overwrite is true
BOSS_API uint32_t DumpTags (boss_db db, const uint8_t * outputFile, bool overwrite);


#ifdef __cplusplus
}
#endif

#endif // BOSSAPI_H