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

#include <windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <stdlib.h>
#include <vector>

#include "readini.h"
#include "stringconvert.h"
#include "commands.h"
#include "error.h"
#include "debug.h"

/*
 * disthelper
 * usage: $0 <mode> [<ini file>]
 * the mode is used to find the [steps:<mode>] section in the ini file
 * steps are sorted in ascii order.  (not numerical order, 2 > 10!)
 * if the ini file is not given, the environment variable DISTHELPER_DISTINI 
 * is used (must be a platform-native absolute path)
 */

int main(int argc, LPTSTR *argv) {
  int result = 0;
  
  if (argc != 2 && argc != 3) {
    OutputDebugString(_T("Incorrect number of arguments"));
    return DH_ERROR_USER;
  }

  LPTSTR ininame;
  tstring distIni;
  if (argc > 2) {
    ininame = argv[2];
  } else {
    ininame = _tgetenv(_T("DISTHELPER_DISTINI"));
    if (!ininame) {
      DebugMessage("No ini file specified");
      return DH_ERROR_USER;
    }
  }
  distIni = GetDistributionDirectory(ininame);
  if (distIni.empty()) {
    DebugMessage("Failed to find distribution.ini");
    return DH_ERROR_USER;
  }
  distIni = ininame;
  
  std::wstring appIni(GetDistributionDirectory());
  appIni.append(L"application.ini");
  
  IniFile_t iniFile;
  result = ReadIniFile(distIni.c_str(), iniFile);
  if (result) {
    LogMessage("Failed to read INI file: %i", result);
    return result;
  }
  std::string section;
  section.assign(ConvertUTFnToUTF8(argv[1]));
  section.insert(0, "steps:");
  IniEntry_t::const_iterator it, end = iniFile[section].end();
  
  // Always copy the distribution.ini / application.ini files to the appdir
  // don't abort if this fails
  result = CommandCopyFile(ConvertUTFnToUTF8(distIni), "$/distribution/");
  if (result) {
    LogMessage("Failed to copy distribution.ini file %S", distIni.c_str());
  }
  result = CommandCopyFile(ConvertUTFnToUTF8(appIni), "$/");
  if (result) {
    LogMessage("Failed to copy application.ini file %S", appIni.c_str());
  }

  for (it = iniFile[section].begin(); it != end; ++it) {
    std::vector<std::string> command;
    std::string line = it->second;
    std::string::size_type prev = 0, offset;
    LogMessage("Executing command %s", line.c_str());
    while ((offset = line.find_first_of(" \t\r\n", prev)) != std::string::npos) {
      if (offset > prev) {
        // skip empty params (e.g. more than one space in a row)
        command.push_back(line.substr(prev, offset - prev));
      }
      prev = offset + 1;
      if (prev > line.length()) {
        break;
      }
      if (command.size() > 1 && command[0] == "exec") {
        prev = line.find_first_not_of(" \t\r\n", prev);
        if (prev != std::string::npos) {
          offset = line.find_last_not_of(" \t\r\n");
          command.push_back(line.substr(prev, offset - prev + 1));
        }
        prev = line.length();
        break;
      }
    }
    if (prev < line.length()) {
      command.push_back(line.substr(prev));
    }
    if (command[0] == "copy") {
      if (command.size() < 3) {
        OutputDebugString(_T("Not enough arguments for copy"));
        result = DH_ERROR_UNKNOWN;
      } else {
        result = CommandCopyFile(command[1], command[2]);
      }
    } else if (command[0] == "move") {
      if (command.size() < 3) {
        OutputDebugString(_T("Not enough arguments for move"));
        result = DH_ERROR_UNKNOWN;
      } else {
        result = CommandMoveFile(command[1], command[2]);
      }
    } else if (command[0] == "delete") {
      if (command.size() < 2) {
        OutputDebugString(_T("Not enough arguments for delete"));
        result = DH_ERROR_UNKNOWN;
      } else {
        result = CommandDeleteFile(command[1]);
      }
    } else if (command[0] == "icon") {
      if (command.size() < 3) {
        OutputDebugString(_T("Not enough arguments for icon"));
        result = DH_ERROR_UNKNOWN;
      } else {
        std::string iconname;
        if (command.size() > 3) {
          iconname.assign(command[3]);
        }
        result = CommandSetIcon(command[1], command[2], iconname);
      }
    } else if (command[0] == "versioninfo") {
      if (command.size() < 3) {
        OutputDebugString(_T("Not enough arguments for versioninfo"));
        result = DH_ERROR_UNKNOWN;
      } else {
        result = CommandSetVersionInfo(command[1], iniFile[command[2]]);
      }
    } else if (command[0] == "exec") {
      command.erase(command.begin()); // the command name
      if (command.size() < 2) {
        command.push_back("");
      }
      result = CommandExecuteFile(command[0], command[1]);
    } else {
      OutputDebugString(_T("bad command!"));
      result = DH_ERROR_UNKNOWN;
    }
    
    if (result) {
      LogMessage("Command failed.");
      return result;
    }
  }
  
  LogMessage("Disthelper successfully completed");
  
  if (ConvertUTFnToUTF8(argv[1]) == "uninstall") {
    // uninstall mode, explicitly delete the log
    gEnableLogging = false;
    CommandDeleteFile("$/disthelper.log");
  }
  return 0;
}
