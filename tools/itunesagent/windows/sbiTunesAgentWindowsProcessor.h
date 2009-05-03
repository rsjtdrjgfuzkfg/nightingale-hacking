/*
 //
 // BEGIN SONGBIRD GPL
 //
 // This file is part of the Songbird web player.
 //
 // Copyright(c) 2005-2009 POTI, Inc.
 // http://songbirdnest.com
 //
 // This file may be licensed under the terms of of the
 // GNU General Public License Version 2 (the "GPL").
 //
 // Software distributed under the License is distributed
 // on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
 // express or implied. See the GPL for the specific language
 // governing rights and limitations.
 //
 // You should have received a copy of the GPL along with this
 // program. If not, go to http://www.gnu.org/licenses/gpl.html
 // or write to the Free Software Foundation, Inc.,
 // 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 //
 // END SONGBIRD GPL
 //
 */

#ifndef SBITUNESAGENTWINDOWSPROCESSOR_H_
#define SBITUNESAGENTWINDOWSPROCESSOR_H_

#include <fstream>

#include "sbiTunesAgentProcessor.h"
#include "sbiTunesLibrary.h"

class sbiTunesAgentWindowsProcessor : public sbiTunesAgentProcessor
{
public:
  /**
   * Initialize any state
   */
  sbiTunesAgentWindowsProcessor();
  
  /**
   * Cleanup resources
   */
  virtual ~sbiTunesAgentWindowsProcessor();
  
  /**
   * Removes the task file
   */
  virtual void RemoveTaskFile();
  
  /**
   * Reports the error
   */
  virtual bool ErrorHandler(sbError const & aError);
  
  /**
   * Registers the application to startup when the user logs in
   */
  virtual sbError RegisterForStartOnLogin();
  
  /**
   * Returns true if there are any tasks file ready to process
   */
  virtual bool TaskFileExists();
  
  /**
   * Unregisters the application to startup when the user logs in
   */
  virtual sbError UnregisterForStartOnLogin();
  
  /**
   * Returns what to do with the file given it's version
   */
  virtual VersionAction VersionCheck(std::string const & aVersion);
  
  /**
   * Waits for the iTunes process to start
   */
  virtual sbError WaitForiTunes();
protected:
  /**
   * Adds a track to the iTunes database given a path
   */
  virtual sbError AddTracks(std::string const & aSource,
                            Tracks const & aPaths);
  
  /**
   * Creates a playlist (Recreates it if it already exists)
   */
  sbError CreatePlaylist(std::string const & aPlaylistName);

  /**
   * Performs any initialization necessary. Optional to implement
   */
  virtual sbError Initialize();
  
  /**
   * Retrieve the path to the task file
   */
  virtual bool OpenTaskFile(std::ifstream & aStream);
  
  /**
   * Logs the message to the platform specific log device
   */
  virtual void Log(std::string const & aMsg);
  
  /**
   * Removes a playlist from the iTunes database
   */
  virtual sbError RemovePlaylist(std::string const & aPlaylist);
  
  /**
   * Returns true if we should shutdown
   */
  virtual bool Shutdown();
  
  /**
   * Sleep for x milliseconds
   */
  virtual void Sleep(unsigned long aMilliseconds);
private:
  sbiTunesLibrary miTunesLibrary;
  std::ofstream mLog;
  std::wstring mCurrentTaskFile;

  enum LogState {
    DEACTIVATED,
    ACTIVE,
    OPENED
  };
  LogState mLogState;
  
  bool ShutdownCallback(bool);
};

#endif /* SBITUNESAGENTWINDOWSPROCESSOR_H_ */
