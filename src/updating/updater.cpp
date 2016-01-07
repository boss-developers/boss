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

	$Revision: 3184 $, $Date: 2011-08-26 20:52:13 +0100 (Fri, 26 Aug 2011) $
*/

#include "updating/updater.h"

#include <cstddef>

#include <fstream>
#include <string>

#include <boost/filesystem.hpp>

#include <git2.h>

#include "base/fstream.h"
#include "common/error.h"
#include "common/game.h"
#include "common/globals.h"
#include "support/helpers.h"
#include "support/logger.h"

namespace boss {

pointers_struct::pointers_struct()
    : repo(NULL),
      remote(NULL),
      cfg(NULL),
      obj(NULL),
      commit(NULL),
      ref(NULL),
      sig(NULL),
      blob(NULL) {
	git_libgit2_init();
}

void pointers_struct::free() {
	git_commit_free(commit);
	git_object_free(obj);
	git_config_free(cfg);
	git_remote_free(remote);
	git_repository_free(repo);
	git_reference_free(ref);
	git_signature_free(sig);
	git_blob_free(blob);
}

void handle_error(int error_code, pointers_struct &pointers) {
	if (!error_code)
		return;

	const git_error *error = giterr_last();
	std::string error_message;
	if (error == NULL)
		error_message = IntToString(error_code) + ".";
	else
		error_message = IntToString(error_code) + "; " + error->message;
	pointers.free();
	giterr_clear();

	LOG_ERROR("Git operation failed. Error: %s", error_message.c_str());
	throw boss_error(error_message, BOSS_ERROR_GIT_ERROR);
}

std::string RepoURL(const Game &game) {
	// TODO(MCP): Look at converting this to a switch-statement
	// MCP Note: The last else-statement should be an else-if with a default of invalid or similar
	if (game.Id() == OBLIVION)
		return gl_oblivion_repo_url;
	else if (game.Id() == NEHRIM)
		return gl_nehrim_repo_url;
	else if (game.Id() == SKYRIM)
		return gl_skyrim_repo_url;
	else if (game.Id() == FALLOUT3)
		return gl_fallout3_repo_url;
	else
		return gl_falloutnv_repo_url;
}

bool are_files_equal(const void *buf1, std::size_t buf1_size,
                            const void *buf2, std::size_t buf2_size) {
	if (buf1_size != buf2_size)
		return false;

	std::size_t pos = 0;
	while (pos < buf1_size) {
		if (*((char*)buf1 + pos) != *((char*)buf2 + pos))
			return false;
		++pos;
	}
	return true;
}

// Gets the revision SHA (first 9 characters) for the currently checked-out masterlist, or "unknown".
std::string GetMasterlistVersion(Game &game) {
	if (!boost::filesystem::exists(game.Masterlist().parent_path() / ".git" / "HEAD")) {
		return "Unknown: Git repository missing";
	}
	std::string rev;
	// Naive check, ignoring working directory changes.

	/*
	 * Better check, which compares HEAD to the working dir.
	 *
	 * 1. Get an object for the masterlist in HEAD.
	 * 2. Get the blob for that object.
	 * 3. Open the masterlist file in the working dir in a file buffer.
	 * 4. Compare the file and blob buffers.
	 */
	pointers_struct ptrs;
	LOG_INFO("Existing repository found, attempting to open it.");
	handle_error(git_repository_open(&ptrs.repo, game.Masterlist().parent_path().string().c_str()), ptrs);

	LOG_INFO("Getting HEAD masterlist object.");
	handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, "HEAD:masterlist.txt"), ptrs);

	LOG_INFO("Getting blob for masterlist object.");
	handle_error(git_blob_lookup(&ptrs.blob, ptrs.repo, git_object_id(ptrs.obj)), ptrs);

	LOG_INFO("Opening masterlist in working directory.");
	std::string mlist;
	fileToBuffer(game.Masterlist(), mlist);

	LOG_INFO("Comparing files.");
	if (are_files_equal(git_blob_rawcontent(ptrs.blob),
	                    git_blob_rawsize(ptrs.blob),
	                    mlist.data(),
	                    mlist.length())) {
		ptrs.free();
		// For some reason trying to get the revision of HEAD:masterlist.txt using libgit2 gives me 18efbc9d8 instead.
		std::string revision;
		//std::ifstream head((game.Masterlist().parent_path() / ".git" / "HEAD").string());
		boss_fstream::ifstream head((game.Masterlist().parent_path() / ".git" / "HEAD"));
		head >> revision;
		head.close();
		revision.resize(9);
		return revision;
	}
	ptrs.free();
	return "Unknown: Masterlist edited";
}

int ValidateCertificate(git_cert *certificate, int is_valid, const char *host_name, void *payload_data) {
	if(!is_valid)
		return 1;
	return -1;
}

}  // namespace boss
