/***************************************************************************
 *                                                                         *
 *  This file is part of DSOrganize.                                       *
 *                                                                         *
 *  DSOrganize is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  DSOrganize is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with DSOrganize.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                         *
 ***************************************************************************/
 
#ifndef __soundplayerShared_h__
#define __soundplayerShared_h__

#include "ipcfifoTGDSUser.h"

//Lower quality
#define MIC_8

//Better quality. Segfaults!!
//#define MIC_16

//TGDS Shared Sound Player Context
static inline struct sSoundPlayerStruct * soundIPC(){
	struct sIPCSharedTGDSSpecific * TGDSUSERIPC = (struct sIPCSharedTGDSSpecific *)TGDSIPCUserStartAddress;	
	struct sSoundPlayerStruct * sndPlayerctx = (struct sSoundPlayerStruct *)&TGDSUSERIPC->sndPlayerCtx;
	return sndPlayerctx;
}

#endif