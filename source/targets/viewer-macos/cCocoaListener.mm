/*
 *  cCocoaListener.mm
 *  Avida
 *
 *  Created by David on 11/11/10.
 *  Copyright 2010 Michigan State University. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; version 2
 *  of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "cCocoaListener.h"

#include <cassert>


void cCocoaListener::NotifyMap(cCoreView_Map* map)
{
  m_map = map;
  [m_win_ctrl performSelectorOnMainThread:@selector(handleMap:) withObject:nil waitUntilDone:NO];
}


void cCocoaListener::NotifyUpdate(int update)
{
  m_win_ctrl.update = update;
  [m_win_ctrl performSelectorOnMainThread:@selector(handleUpdate:) withObject:nil waitUntilDone:NO];
}
