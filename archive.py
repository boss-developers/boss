#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Creates an archive of a BOSS release, putting it in the current folder.

# Files and folders that need to go in (relative to repository root):
#
# code/trunk/bin/Release-32/BOSS.exe
# code/trunk/bin/Release-32/BOSS GUI.exe
# data/boss-common/resources/l10n/es/LC_MESSAGES/boss.mo
# data/boss-common/resources/l10n/es/LC_MESSAGES/wxstd.mo
# data/boss-common/resources/l10n/ru/LC_MESSAGES/boss.mo
# data/boss-common/resources/l10n/ru/LC_MESSAGES/wxstd.mo
# data/boss-common/resources/l10n/zh/LC_MESSAGES/boss.mo
# data/boss-common/resources/l10n/zh/LC_MESSAGES/wxstd.mo
# data/boss-common/resources/script.js
# data/boss-common/resources/style.css
# data/boss-common/BOSS.ini
# data/boss-common/Docs

#   BOSS
#
#   A plugin load order optimiser for games that use the esp/esm plugin system.
#
#   Copyright (C) 2009-2014    WrinklyNinja
#
#   This file is part of BOSS.
#
#   BOSS is free software: you can redistribute
#   it and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   BOSS is distributed in the hope that it will
#   be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with BOSS.  If not, see
#   <http://www.gnu.org/licenses/>.

import sys
import os
import shutil
import zipfile

temp_path = u'archive.tmp'
archive_name = u'BOSS Archive.zip'

def onerror(func, path, exc_info):
    """
    Error handler for ``shutil.rmtree``.

    If the error is due to an access error (read only file)
    it attempts to add write permission and then retries.

    If the error is for another reason it re-raises the error.

    Usage : ``shutil.rmtree(path, onerror=onerror)``
    """
    import stat
    if not os.access(path, os.W_OK):
        # Is the error an access error ?
        os.chmod(path, stat.S_IWUSR)
        func(path)
    else:
        raise

# Set archive name if alternative is given.
if (len(sys.argv) > 1):
    archive_name = sys.argv[1]

# First make sure that the temporary folder for the archive exists.
if not os.path.exists(temp_path):
    os.makedirs(temp_path)

# Now copy everything into the temporary folder.
shutil.copy( os.path.join(u'code', u'trunk', u'bin', u'Release-32', u'BOSS.exe'), temp_path )
shutil.copy( os.path.join(u'code', u'trunk', u'bin', u'Release-32', u'BOSS GUI.exe'), temp_path )

for lang in ['es', 'ru', 'zh']:
    os.makedirs(os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES'))
    shutil.copy( os.path.join('data', 'boss-common', 'resources', 'l10n', lang, 'LC_MESSAGES', 'messages.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )
    shutil.copy( os.path.join('data', 'boss-common', 'resources', 'l10n', lang, 'LC_MESSAGES', 'wxstd.mo'), os.path.join(temp_path, 'resources', 'l10n', lang, 'LC_MESSAGES') )

shutil.copy( os.path.join('data', 'boss-common', 'resources', 'script.js'), os.path.join(temp_path, 'resources') )
shutil.copy( os.path.join('data', 'boss-common', 'resources', 'style.css'), os.path.join(temp_path, 'resources') )
shutil.copy( os.path.join('data', 'boss-common', 'BOSS.ini'), temp_path)

shutil.copytree( os.path.join('data', 'boss-common', 'docs'), os.path.join(temp_path, 'docs') )

# Git repositories.
for game, boss_game in [('oblivion','Oblivion'), ('falloutnv','Fallout New Vegas'), ('skyrim','Skyrim'), ('fallout3','Fallout 3'), ('nehrim','Nehrim')]:
    os.makedirs(os.path.join(temp_path, boss_game, '.git'))
    shutil.copy(os.path.join('..', game, '.git', 'config'), os.path.join(temp_path, boss_game, '.git'))
    shutil.copy(os.path.join('..', game, '.git', 'HEAD'), os.path.join(temp_path, boss_game, '.git'))
    shutil.copy(os.path.join('..', game, '.git', 'index'), os.path.join(temp_path, boss_game, '.git'))
    shutil.copy(os.path.join('..', game, '.git', 'packed-refs'), os.path.join(temp_path, boss_game, '.git'))

    os.makedirs(os.path.join(temp_path, boss_game, '.git', 'refs', 'heads'))
    shutil.copy(os.path.join('..', game, '.git', 'refs', 'heads', 'master'), os.path.join(temp_path, boss_game, '.git', 'refs', 'heads'))

    os.makedirs(os.path.join(temp_path, boss_game, '.git', 'refs', 'remotes', 'origin'))
    shutil.copy(os.path.join('..', game, '.git', 'refs', 'remotes', 'origin', 'HEAD'), os.path.join(temp_path, boss_game, '.git', 'refs', 'remotes', 'origin'))

    shutil.copytree(os.path.join('..', game, '.git', 'objects'), os.path.join(temp_path, boss_game, '.git', 'objects'))

# Now compress the temporary folder. (Creating a zip because I can't get pylzma to work...)
os.chdir(temp_path)
zip = zipfile.ZipFile( os.path.join('..', archive_name), 'w', zipfile.ZIP_DEFLATED )
for root, dirs, files in os.walk('.'):
    for file in files:
        zip.write(os.path.join(root, file))
zip.close()
os.chdir('..')


# And finally, delete the temporary folder.
shutil.rmtree('archive.tmp', onerror=onerror)
