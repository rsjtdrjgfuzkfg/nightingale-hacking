/*
 *=BEGIN SONGBIRD GPL
 *
 * This file is part of the Songbird web player.
 *
 * Copyright(c) 2005-2010 POTI, Inc.
 * http://www.songbirdnest.com
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the ``GPL'').
 *
 * Software distributed under the License is distributed
 * on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *=END SONGBIRD GPL
 */

#include <nsIGenericFactory.h>
#include "sbSongkickDBService.h"


NS_GENERIC_FACTORY_CONSTRUCTOR(sbSongkickDBService)

static nsModuleComponentInfo sbSongkickComponent[] =
{
  {
    SONGBIRD_SONGKICKDBSERVICE_CLASSNAME,
    SONGBIRD_SONGKICKDBSERVICE_CID,
    SONGBIRD_SONGKICKDBSERVICE_CONTRACTID,
    sbSongkickDBServiceConstructor
  }
};

NS_IMPL_NSGETMODULE(SongkickDBServiceComponent, sbSongkickComponent)

