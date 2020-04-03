/*
*   Copyright (C) 2017,2020 by Jonathan Naylor G4KLX
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

#include "PassAllPC.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CPassAllPC::CPassAllPC(const std::string& name, unsigned int slot) :
CRewrite(),
m_name(name),
m_slot(slot)
{
	assert(slot == 1U || slot == 2U);
}

CPassAllPC::~CPassAllPC()
{
}

PROCESS_RESULT CPassAllPC::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int slotNo = data.getSlotNo();

	bool ret = (flco == FLCO_USER_USER && slotNo == m_slot);

	if (trace)
		LogDebug("Rule Trace,\tPassAllPC %s Slot=%u: %s", m_name.c_str(), m_slot, ret ? "matched" : "not matched");

	return ret ? RESULT_MATCHED : RESULT_UNMATCHED;
}
