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

#ifndef __BOSS_UPDATER_H__
#define __BOSS_UPDATER_H__

#include <string>
#include <vector>
#include <fstream>
#include "Common/Game.h"
#include "Support/Logger.h"

#include <git2.h>

namespace boss {

    struct pointers_struct {
        pointers_struct() : repo(NULL), remote(NULL), cfg(NULL), obj(NULL), commit(NULL) {}

        void free() {
            git_commit_free(commit);
            git_object_free(obj);
            git_config_free(cfg);
            git_remote_free(remote);
            git_repository_free(repo);
        }

        git_repository * repo;
        git_remote * remote;
        git_config * cfg;
        git_object * obj;
        git_commit * commit;
    };

    inline void handle_error(int error_code, pointers_struct& pointers) {
        if (!error_code)
            return;

        const git_error * error = giterr_last();
        std::string error_message;
        if (error == NULL)
            error_message = IntToString(error_code) + ".";
        else
            error_message = IntToString(error_code) + "; " + error->message;
        pointers.free();
        giterr_clear();

        LOG_ERROR("Git operation failed. Error: %s", error_message);
        throw boss_error(error_message, BOSS_ERROR_GIT_ERROR);
    }

    inline std::string RepoURL(const Game& game) {
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

    //Progress has form prog(const char *str, int len, void *data)
    template<class Progress>
    std::string UpdateMasterlist(Game& game, Progress prog, void * out) {
        pointers_struct ptrs;
        const git_transfer_progress * stats = NULL;

        LOG_INFO("Checking for a Git repository.");

        //Checking for a ".git" folder.
        if (fs::exists(game.Masterlist().parent_path() / ".git")) {
            //Repository exists. Open it.
            LOG_INFO("Existing repository found, attempting to open it.");
            handle_error(git_repository_open(&ptrs.repo, game.Masterlist().parent_path().string().c_str()), ptrs);

            LOG_INFO("Attempting to get info on the repository remote.");

            //Now get remote info.
            handle_error(git_remote_load(&ptrs.remote, ptrs.repo, "origin"), ptrs);

            LOG_INFO("Getting the remote URL.");

            //Get the remote URL.
            const char * url = git_remote_url(ptrs.remote);

            LOG_INFO("Checking to see if remote URL matches URL in settings.");

            //Check if the repo URLs match.
            LOG_INFO("Remote URL given: %s", RepoURL(game));
            LOG_INFO("Remote URL in repository settings: %s", url);
            if (url != RepoURL(game)) {
                LOG_INFO("URLs do not match, setting repository URL to URL in settings.");
                //The URLs don't match. Change the remote URL to match the one BOSS has.
                handle_error(git_remote_set_url(ptrs.remote, RepoURL(game).c_str()), ptrs);

                //Now save change.
                handle_error(git_remote_save(ptrs.remote), ptrs);
            }
        }
        else {
            LOG_INFO("Repository doesn't exist, initialising a new repository.");
            //Repository doesn't exist. Set up a repository.
            handle_error(git_repository_init(&ptrs.repo, game.Masterlist().parent_path().string().c_str(), false), ptrs);

            LOG_INFO("Setting the new repository's remote to: %s", RepoURL(game));

            //Now set the repository's remote.
            handle_error(git_remote_create(&ptrs.remote, ptrs.repo, "origin", RepoURL(game).c_str()), ptrs);
        }

        //WARNING: This is generally a very bad idea, since it makes HTTPS a little bit pointless, but in this case because we're only reading data and not really concerned about its integrity, it's acceptable. A better solution would be to figure out why GitHub's certificate appears to be invalid to OpenSSL.
#ifndef _WIN32
        git_remote_check_cert(ptrs.remote, 0);
#endif

        LOG_INFO("Fetching updates from remote.");

        //Now pull from the remote repository. This involves a fetch followed by a merge. First perform the fetch.

        //Set up callbacks.
        git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
        callbacks.transfer_progress = prog;
        callbacks.payload = out;
        git_remote_set_callbacks(ptrs.remote, &callbacks);

        //Fetch from remote.
        LOG_INFO("Fetching from remote.");
        handle_error(git_remote_fetch(ptrs.remote), ptrs);

        // Now start the merging. Not entirely sure what's going on here, but it looks like libgit2's merge API is incomplete, you can create some git_merge_head objects, but can't do anything with them...
        // Thankfully, we don't really need a merge, we just need to replace whatever's in the working directory with the relevant file from FETCH_HEAD, which was updated in the fetching step before.
        // The porcelain equivalent is `git checkout refs/remotes/origin/gh-pages masterlist.txt`

        LOG_INFO("Setting up checkout parameters.");

        char * paths[] = { "masterlist.txt" };

        git_checkout_opts opts = GIT_CHECKOUT_OPTS_INIT;
        opts.checkout_strategy = GIT_CHECKOUT_FORCE;  //Make sure the existing file gets overwritten.
        opts.paths.strings = paths;
        opts.paths.count = 1;

        //Next, we need to do a looping checkout / parsing check / roll-back.

        bool parsingFailed = false;
        unsigned int rollbacks = 0;
        char revision[10];
        do {
            LOG_INFO("Getting the Git object for the tree at refs/remotes/origin/master~%i.", rollbacks);

            //Get the commit hash so that we can report the revision if there is an error.
            string filespec = "refs/remotes/origin/master~" + IntToString(rollbacks);

            handle_error(git_revparse_single(&ptrs.obj, ptrs.repo, filespec.c_str()), ptrs);

            LOG_INFO("Checking out the tree at refs/remotes/origin/master~%i.", rollbacks);

            //Now we can do the checkout.
            handle_error(git_checkout_tree(ptrs.repo, ptrs.obj, &opts), ptrs);

            LOG_INFO("Getting the hash for the tree.");

            LOG_INFO("Converting and recording the first 10 hex characters of the hash.");

            git_oid_tostr(revision, 10, git_object_id(ptrs.obj));

            LOG_INFO("Tree hash is: %s", revision);
            LOG_INFO("Freeing the masterlist object.");

            git_object_free(ptrs.obj);
            /*
            BOOST_LOG_TRIVIAL(trace) << "Testing masterlist parsing.";

            //Now try parsing the masterlist.
            list<boss::Message> messages;
            list<boss::Plugin> plugins;
            try {
                boss::ifstream in(game.MasterlistPath());
                YAML::Node mlist = YAML::Load(in);
                in.close();

                if (mlist["globals"])
                    messages = mlist["globals"].as< list<boss::Message> >();
                if (mlist["plugins"])
                    plugins = mlist["plugins"].as< list<boss::Plugin> >();

                for (list<boss::Plugin>::iterator it = plugins.begin(), endIt = plugins.end(); it != endIt; ++it) {
                    it->EvalAllConditions(game, g_lang_any);
                }

                for (list<boss::Message>::iterator it = messages.begin(), endIt = messages.end(); it != endIt; ++it) {
                    it->EvalCondition(game, g_lang_any);
                }

                parsingFailed = false;

            }
            catch (std::exception& e) {
                parsingFailed = true;
                rollbacks++;

                //Roll back one revision if there's an error.
                BOOST_LOG_TRIVIAL(error) << "Masterlist parsing failed. Masterlist revision " + string(revision) + ": " + e.what();

                parsingErrors.push_back(boss::Message(boss::g_message_error, boost::locale::translate("Masterlist revision").str() + " " + string(revision) + ": " + e.what() + " " + boost::locale::translate("Rolled back to the previous revision.").str()));
            }
            */
        } while (parsingFailed);

        //Finally, free memory.
        ptrs.free();

        return string(revision);
    }
}
#endif