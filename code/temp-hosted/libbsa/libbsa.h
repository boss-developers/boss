/*	libbsa
	
	A library for reading and writing BSA files.

    Copyright (C) 2012    WrinklyNinja

	This file is part of libbsa.

    libbsa is free software: you can redistribute 
	it and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation, either version 3 of 
	the License, or (at your option) any later version.

    libbsa is distributed in the hope that it will 
	be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libbsa.  If not, see 
	<http://www.gnu.org/licenses/>.
*/

/* License above not final - hope to re-use code from BSAOpt, will adapt licensing/copyright to work with that. */

#ifndef LIBBSA_H
#define LIBBSA_H

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
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

// set up dll import/export decorators
// when compiling the dll on windows, ensure LIBLO_EXPORT is defined.  clients
// that use this header do not need to define anything to import the symbols
// properly.
#if defined(_WIN32) || defined(_WIN64)
#   ifdef LIBBSA_EXPORT
#       define LIBBSA __declspec(dllexport)
#   else
#       define LIBBSA __declspec(dllimport)
#   endif
#else
#   define LIBBSA
#endif

#ifdef __cplusplus
extern "C"
{
#endif


/*------------------------------
   Types
------------------------------*/

/* Abstraction of BSA info structure while providing type safety. */
typedef struct bsa_handle_int * bsa_handle;

/* Structure containing all required information about a BSA asset that can 
   be obtained from the BSA (I think). */
typedef struct {
	uint8_t * extPath;  //Path of extracted asset, relative to temp directory. NULL if not extracted.
	uint8_t * intPath;  //Path of asset inside BSA.
	uint32_t crc;		//CRC of asset.
	time_t timestamp;	//Timestamp of asset.
} bsa_asset;

/* Return codes */
LIBBSA extern const uint32_t LIBBSA_OK;
LIBBSA extern const uint32_t LIBBSA_ERROR_INVALID_ARGUMENTS;
LIBBSA extern const uint32_t LIBBSA_ERROR_NO_MEM;
LIBBSA extern const uint32_t LIBBSA_ERROR_FILE_NOT_FOUND;
LIBBSA extern const uint32_t LIBBSA_ERROR_FILE_WRITE_FAIL;
LIBBSA extern const uint32_t LIBBSA_ERROR_FILE_READ_FAIL;
LIBBSA extern const uint32_t LIBBSA_RETURN_MAX;
/* No doubt there will be more... */


/*------------------------------
   Version Functions
------------------------------*/

/* Checks if this version of the API is compatible with the given version 
   of the API. Useful for checking if the interface code you wrote for a 
   previous version still works. */
LIBBSA bool IsCompatibleVersion (const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch);

/* Gets the version numbers for the API. */
LIBBSA void GetVersionNums(uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch);


/*----------------------------------
   Lifecycle Management Functions
----------------------------------*/

/* Sets a directory to which all BSA extractions shall take place. Each 
   extracted BSA's content is put in a subfolder of this directory, with 
   the folder name being the same as the BSA filename. */
LIBBSA uint32_t SetTempDirectory(const uint8_t * path);

/* Opens a BSA at path, returning a handle bh. If the BSA doesn't exist 
   then the function will return with an error. */
LIBBSA uint32_t OpenBSA(const uint8_t * path, bsa_handle * bh);

/* Create a BSA at path with the specified version and compression level. 
   If the compression level is not very high (>95%) then the compression 
   will be turned off. This behaviour can be stopped by setting 
   forceCompression to true. */
LIBBSA uint32_t CreateBSA(const uint8_t * path, const uint32_t version, const uint32_t compressionLevel, const bool forceCompression, bsa_handle * bh);

/* Closes the BSA associated with the given handle. This just frees up any 
   memory and deletes the BSA's folder in the temp dir, since writes to BSA 
   content are performed to-disk rather than to-memory by the relevant functions. */
LIBBSA uint32_t CloseBSA(const bsa_handle bh);


/*------------------------------
   Error Handling Functions
------------------------------*/

/* Gets a string with details about the last error returned. */
LIBBSA uint32_t GetLastErrorDetails(uint8_t ** details);

/* Frees the memory allocated to the last error details string. */
LIBBSA void CleanUpErrorDetails();


/*------------------------------
   Content Reading Functions
------------------------------*/

/* Gets an array of all the assets in the given BSA that match the contentPath 
   given. contentPath is a POSIX Extended regular expression that all asset 
   paths within the BSA will be compared to. */
LIBBSA uint32_t GetBSAContent(const bsa_handle bh, const uint8_t * contentPath, bsa_asset ** content);

/* Checks if a specific asset, found within the BSA at assetPath, is in the given BSA. */
LIBBSA uint32_t IsAssetInBSA(const bsa_handle bh, const uint8_t * assetPath, bool * result);


/*------------------------------
   Content Writing Functions
------------------------------*/

/* Replaces all the assets in the given BSA with the given assets. */
LIBBSA uint32_t SetBSAContent(const bsa_handle bh, const bsa_asset * content);

/* Adds a specific asset, found at assetSource, to a BSA, placing it in assetDest. */
LIBBSA uint32_t AddAssetToBSA(bsa_handle bh, const uint8_t * assetSource, const uint8_t * assetDest);


/*--------------------------------
   Content Extraction Functions
--------------------------------*/

/* Gets an array of all the assets in the given BSA that match the contentPath 
   given. contentPath is a POSIX Extended regular expression that all asset 
   paths within the BSA will be compared to. */
LIBBSA uint32_t ExtractBSAContent(const bsa_handle bh, const uint8_t * contentPath, bsa_asset ** content);

/* Fetches a specific asset, found at assetPath, from a given BSA. */
LIBBSA uint32_t ExtractAssetFromBSA(const bsa_handle bh, const uint8_t * assetPath, bsa_asset * asset);

#endif