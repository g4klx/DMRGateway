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

#include "RewriteType.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteType::CRewriteType(const std::string& name, unsigned int fromSlot, unsigned int fromTG, unsigned int toSlot, unsigned int toId) :
CRewrite(),
m_name(name),
m_fromSlot(fromSlot),
m_fromTG(fromTG),
m_toSlot(toSlot),
m_toId(toId)
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewriteType::~CRewriteType()
{
}

bool CRewriteType::process(CDMRData& data, bool trace)
{
	FLCO flco           = data.getFLCO();
	unsigned int dstId  = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_GROUP || slotNo != m_fromSlot || dstId != m_fromTG) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteType %s Slot=%u Dst=TG%u: not matched", m_name.c_str(), m_fromSlot, m_fromTG);

		return false;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	data.setDstId(m_toId);
	data.setFLCO(FLCO_USER_USER);

	processMessage(data);

	if (trace)
		LogDebug("Rule Trace,\tRewriteType %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_fromSlot, m_fromTG);

	return true;
}
