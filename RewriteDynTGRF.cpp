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

CRewriteDynTGRF::CRewriteDynTGRF(const std::string& name, unsigned int slot, unsigned int fromTG, unsigned int toTG, unsigned int discTG, unsigned int statusTG, unsigned int range, CRewriteDynTGNet* rewriteNet, CDynVoice* voice) :
CRewrite(),
m_name(name),
m_slot(slot),
m_fromTGStart(fromTG),
m_fromTGEnd(fromTG + range - 1U),
m_toTG(toTG),
m_discTG(discTG),
m_statusTG(statusTG),
m_rewriteNet(rewriteNet),
m_voice(voice),
m_currentTG(0U)
{
	assert(slot == 1U || slot == 2U);
	assert(rewriteNet != NULL);

	if (voice != NULL)
		voice->setSlotAndTG(slot, toTG);
}

CRewriteDynTGRF::~CRewriteDynTGRF()
{
}

PROCESS_RESULT CRewriteDynTGRF::process(CDMRData& data, bool trace)
{
	FLCO flco           = data.getFLCO();
	unsigned int dstId  = data.getDstId();
	unsigned int slotNo = data.getSlotNo();
	unsigned char type  = data.getDataType();

	if (flco == FLCO_GROUP && slotNo == m_slot && dstId == m_toTG && m_currentTG != 0U) {
		data.setDstId(m_currentTG);

		processMessage(data);

		if (trace)
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_slot, m_toTG);

		return RESULT_MATCHED;
	}

	if (flco == FLCO_GROUP && slotNo == m_slot && dstId == m_discTG && m_currentTG != 0U) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_slot, m_discTG);

		if (type == DT_TERMINATOR_WITH_LC) {
			m_rewriteNet->setCurrentTG(0U);
			m_currentTG = 0U;
			if (m_voice != NULL)
				m_voice->unlinked();
		}

		return RESULT_MATCHED;
	}

	if (slotNo == m_slot && dstId >= m_fromTGStart && dstId <= m_fromTGEnd) {
		if (trace) {
			if (m_fromTGStart == m_fromTGEnd)
				LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_slot, m_fromTGStart);
			else
				LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u-TG%u: matched", m_name.c_str(), m_slot, m_fromTGStart, m_fromTGEnd);
		}

		data.setFLCO(FLCO_GROUP);

		if (type == DT_TERMINATOR_WITH_LC) {
			m_rewriteNet->setCurrentTG(dstId);
			m_currentTG = dstId;
			if (m_voice != NULL)
				m_voice->linkedTo(dstId);
		}

		return RESULT_MATCHED;
	}

	if (flco == FLCO_GROUP && slotNo == m_slot && dstId == m_statusTG) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u: matched", m_name.c_str(), m_slot, m_statusTG);

		if (type == DT_TERMINATOR_WITH_LC && m_voice != NULL) {
			if (m_currentTG == 0U)
				m_voice->unlinked();
			else
				m_voice->linkedTo(dstId);
		}

		return RESULT_IGNORED;
	}

	if (trace) {
		if (m_fromTGStart == m_fromTGEnd)
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u or Dst=TG%u or Dst=TG%u or Dst=TG%u: not matched", m_name.c_str(), m_slot, m_fromTGStart, m_toTG, m_discTG, m_statusTG);
		else
			LogDebug("Rule Trace,\tRewriteDynTGRF from %s Slot=%u Dst=TG%u-TG%u or Dst=TG%u or Dst=TG%u or Dst=TG%u: not matched", m_name.c_str(), m_slot, m_fromTGStart, m_fromTGEnd, m_toTG, m_discTG, m_statusTG);
	}

	return RESULT_UNMATCHED;
}
