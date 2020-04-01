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

#include "RewriteDynTGRF.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteDynTGRF::CRewriteDynTGRF(const std::string& name, unsigned int slot, unsigned int fromTG, unsigned int toTG, unsigned int discTG, unsigned int range, CRewriteDynTGNet* rewriteNet) :
CRewrite(),
m_name(name),
m_slot(slot),
m_fromTGStart(fromTG),
m_fromTGEnd(fromTG + range - 1U),
m_toTG(toTG),
m_discTG(discTG),
m_rewriteNet(rewriteNet),
m_currentTG(0U)
{
	assert(slot == 1U || slot == 2U);
	assert(rewriteNet != NULL);
}

CRewriteDynTGRF::~CRewriteDynTGRF()
{
}

bool CRewriteDynTGRF::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_GROUP || slotNo != m_fromSlot || dstId < m_fromTGStart || dstId > m_fromTGEnd) {
		if (trace) {
			if (m_fromTGStart == m_fromTGEnd)
				LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart);
			else
				LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u-TG%u: not matched", m_name.c_str(), m_fromSlot, m_fromTGStart, m_fromTGEnd);
		}

		return false;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	if (m_fromTGStart != m_toTGStart) {
		unsigned int newTG = dstId + m_toTGStart - m_fromTGStart;
		data.setDstId(newTG);

		processMessage(data);
	}

	if (trace) {
		if (m_fromTGStart == m_fromTGEnd)
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_fromSlot, m_fromTGStart);
		else
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u-TG%u: matched", m_name.c_str(), m_fromSlot, m_fromTGStart, m_fromTGEnd);

		if (m_toTGStart == m_toTGEnd)
			LogDebug("Rule Trace,\tRewriteDynTGRF to %s Slot=%u Dst=TG%u", m_name.c_str(), m_toSlot, m_toTGStart);
		else
			LogDebug("Rule Trace,\tRewriteDynTGRF to %s Slot=%u Dst=TG%u-TG%u", m_name.c_str(), m_toSlot, m_toTGStart, m_toTGEnd);
	}

	return true;
}
