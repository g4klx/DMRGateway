/*
 *   Copyright (C) 2015,2016,2017,2025 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Sync.h"

#include "DMRDefines.h"

#include <cstdio>
#include <cassert>
#include <cstring>


void CSync::addDMRDataSync(unsigned char* data, bool duplex)
{
	assert(data != nullptr);

	if (duplex) {
		for (unsigned int i = 0U; i < 7U; i++)
			data[i + 13U] = (data[i + 13U] & ~SYNC_MASK[i]) | BS_SOURCED_DATA_SYNC[i];
	} else {
		for (unsigned int i = 0U; i < 7U; i++)
			data[i + 13U] = (data[i + 13U] & ~SYNC_MASK[i]) | MS_SOURCED_DATA_SYNC[i];
	}
}

void CSync::addDMRAudioSync(unsigned char* data, bool duplex)
{
	assert(data != nullptr);

	if (duplex) {
		for (unsigned int i = 0U; i < 7U; i++)
			data[i + 13U] = (data[i + 13U] & ~SYNC_MASK[i]) | BS_SOURCED_AUDIO_SYNC[i];
	} else {
		for (unsigned int i = 0U; i < 7U; i++)
			data[i + 13U] = (data[i + 13U] & ~SYNC_MASK[i]) | MS_SOURCED_AUDIO_SYNC[i];
	}
}
