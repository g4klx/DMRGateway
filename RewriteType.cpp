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

#include "RewriteType.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteType::CRewriteType(const std::string& name, unsigned int fromSlot, unsigned int fromTG, unsigned int toSlot, unsigned int toId, unsigned int range) :
CRewrite(),
m_name(name),
m_fromSlot(fromSlot),
m_fromTGStart(fromTG),
m_fromTGEnd(fromTG + range - 1U),
m_toSlot(toSlot),
m_toIdStart(toId),
m_toIdEnd(toId + range - 1U)
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewriteType::~CRewriteType()
{
}

PROCESS_RESULT CRewriteType::process(CDMRData& data, bool trace)
{
	FLCO flco           = data.getFLCO();
	unsigned int dstId  = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_GROUP || slotNo != m_fromSlot || dstId < m_fromTGStart || dstId > m_fromTGEnd) {
		if (trace) {
			if (m_fromTGStart == m_fromTGEnd)
				LogDebug("Rule Trace,\tRewriteType from \"%s\" Slot=%u Dst=TG%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart);
			else
				LogDebug("Rule Trace,\tRewriteType from \"%s\" Slot=%u Dst=TG%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart, m_fromTGEnd);
		}
		return RESULT_UNMATCHED;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	if (m_fromTGStart != m_toIdStart) {
		unsigned int newDstId = dstId + m_toIdStart - m_fromTGStart;
		data.setDstId(newDstId);
	}
	data.setFLCO(FLCO_USER_USER);

	processMessage(data);

	if (trace) {
		if (m_fromTGStart == m_fromTGEnd)
			LogDebug("Rule Trace,\tRewriteType from \"%s\" Slot=%u Dst=TG%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart);
		else
			LogDebug("Rule Trace,\tRewriteType from \"%s\" Slot=%u Dst=TG%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart, m_fromTGEnd);
		if (m_toIdStart == m_toIdEnd)
			LogDebug("Rule Trace,\tRewriteType  to  \"%s\" Slot=%u Dst=%u: matched", m_name.c_str(), m_toSlot, m_toIdStart);
		else
			LogDebug("Rule Trace,\tRewriteType  to  \"%s\" Slot=%u Dst=%u-%u: matched", m_name.c_str(), m_toSlot, m_toIdStart, m_toIdEnd);
	}

	return RESULT_MATCHED;
}
