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

#include "RewritePC.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewritePC::CRewritePC(const std::string& name, unsigned int fromSlot, unsigned int fromId, unsigned int toSlot, unsigned int toId, unsigned int range) :
CRewrite(),
m_name(name),
m_fromSlot(fromSlot),
m_fromIdStart(fromId),
m_fromIdEnd(fromId + range - 1U),
m_toSlot(toSlot),
m_toIdStart(toId),
m_toIdEnd(toId + range - 1U)
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewritePC::~CRewritePC()
{
}

PROCESS_RESULT CRewritePC::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_USER_USER || slotNo != m_fromSlot || dstId < m_fromIdStart || dstId > m_fromIdEnd) {
		if (trace)
			LogDebug("Rule Trace,\tRewritePC from %s Slot=%u Dst=%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);

		return RESULT_UNMATCHED;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	if (m_fromIdStart != m_toIdStart) {
		unsigned int newDstId = dstId + m_toIdStart - m_fromIdStart;
		data.setDstId(newDstId);

		processMessage(data);
	}

	if (trace) {
		LogDebug("Rule Trace,\tRewritePC from %s Slot=%u Dst=%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);
		LogDebug("Rule Trace,\tRewritePC to %s Slot=%u Dst=%u-%u", m_name.c_str(), m_toSlot, m_toIdStart, m_toIdEnd);
	}

	return RESULT_MATCHED;
}
