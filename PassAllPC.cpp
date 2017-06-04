/*
*   Copyright (C) 2017 by Jonathan Naylor G4KLX
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

CPassAllPC::CPassAllPC(const char* name, unsigned int slot, bool trace) :
m_name(name),
m_slot(slot),
m_trace(trace)
{
	assert(slot == 1U || slot == 2U);
}

CPassAllPC::~CPassAllPC()
{
}

bool CPassAllPC::processRF(CDMRData& data)
{
	bool ret = process(data);

	if (m_trace)
		LogDebug("Rule Trace,\tPassAllPC %s Slot=%u: %s", m_name, m_slot, ret ? "matched" : "not matched");

	return ret;
}

bool CPassAllPC::processNet(CDMRData& data)
{
	bool ret = process(data);

	if (m_trace)
		LogDebug("Rule Trace,\tPassAllPC %s Slot=%u: %s", m_name, m_slot, ret ? "matched" : "not matched");

	return ret;
}

bool CPassAllPC::process(CDMRData& data)
{
	FLCO flco = data.getFLCO();
	unsigned int slotNo = data.getSlotNo();

	return flco == FLCO_USER_USER && slotNo == m_slot;
}
