/* vim: le=unix sw=2 : */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Songbird Distribution Stub Helper.
 *
 * The Initial Developer of the Original Code is
 * POTI <http://www.songbirdnest.com/>.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Mook <mook@songbirdnest.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef _DISTHERLPER_COMMANDS_H__
#define _DISTHERLPER_COMMANDS_H__

#include <string>
#include "readini.h"
#include "stringconvert.h"

/* all commands return DH_ERROR_OK on success, non-zero on failure.
 * all std::string paramters are expected to be in UTF-8
 * see error.h
 */

/**
 * Copy a file from aSrc to aDest, attempting to overwrite if the file already
 * exists.
 */
int CommandCopyFile(std::string aSrc, std::string aDest, bool aRecursive = false);

/**
 * Move a file from aSrc to aDest, attempting to overwrite if the destination
 * already exists.  This may involve a copy followed by a delete.
 */
int CommandMoveFile(std::string aSrc, std::string aDest, bool aRecursive = false);

/**
 * Delete a give file.  Results are undefined if the file does not exist.
 */
int CommandDeleteFile(std::string aFile, bool aRecursive = false);

/**
 * Change the icon for the given executable to the image(s) from aIconFile.
 * This may not make sense on all platforms; in that case, this is a no-op.
 *
 * \param aExecutable the full path to the executable file to modify
 * \param aIconFile the new icon to embed into the executable
 * \param aIconName the icon to replace
 */
int CommandSetIcon(std::string aExecutable, std::string aIconFile, std::string aIconName);

/**
 * Set auxilary version information for a given executable file to the key/value
 * pairs found in the INI file section.  This may not make sense on all
 * platforms; this is a no-op in those cases.
 */
int CommandSetVersionInfo(std::string aExecutable, IniEntry_t& aSection);

/**
 * Execute a file with some number of arguments which will be parsed by this
 * function, in some OS-dependent way
 */
int CommandExecuteFile(std::string aExecutable, std::string aArg);

/**
 * Get the path to the application directory (where disthelper.exe lives).
 * Must end with a path separator (\ on Windows, / on Unix)
 */
tstring GetAppDirectory();

/**
 * Get the path to the directory containing distribution.ini file
 * If aPath is given, use it as a basis to set further calls.
 * The result must end with a path separator.
 */
tstring GetDistributionDirectory(const TCHAR* aPath = NULL);

/**
 * Resolve a path name.
 * All paths starting with $/ or $\ are relative to the application directory.
 * All other paths are assumed to be relative to be the distribution.ini used.
 */
tstring ResolvePathName(std::string aSrc);

#endif /* _DISTHERLPER_COMMANDS_H__ */
