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

#include "RewriteDynTGNet.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteDynTGNet::CRewriteDynTGNet(const std::string& name, unsigned int slot, unsigned int toTG) :
CRewrite(),
m_name(name),
m_slot(slot),
m_toTG(toTG),
m_currentTG(0U)
{
	assert(slot == 1U || slot == 2U);
}

CRewriteDynTGNet::~CRewriteDynTGNet()
{
}

PROCESS_RESULT CRewriteDynTGNet::process(CDMRData& data, bool trace)
{
	FLCO flco           = data.getFLCO();
	unsigned int dstId  = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_GROUP || slotNo != m_slot || dstId != m_currentTG) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteDynTGNet from %s Slot=%u Dst=TG%u: not matched", m_name.c_str(), m_slot, m_currentTG);

		return RESULT_UNMATCHED;
	}

	data.setDstId(m_toTG);

	processMessage(data);

	if (trace)
		LogDebug("Rule Trace,\tRewriteDynTGNet from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_slot, m_currentTG);

	return RESULT_MATCHED;
}

void CRewriteDynTGNet::setCurrentTG(unsigned int currentTG)
{
	m_currentTG = currentTG;
}
