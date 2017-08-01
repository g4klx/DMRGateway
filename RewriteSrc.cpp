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

#include "RewriteSrc.h"

#include "DMRDefines.h"
#include "DMRFullLC.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteSrc::CRewriteSrc(const std::string& name, unsigned int fromSlot, unsigned int fromId, unsigned int toSlot, unsigned int toTG, unsigned int range) :
m_name(name),
m_fromSlot(fromSlot),
m_fromIdStart(fromId),
m_fromIdEnd(fromId + range - 1U),
m_toSlot(toSlot),
m_toTG(toTG),
m_lc(FLCO_GROUP, 0U, toTG),
m_embeddedLC(),
m_dataHeader(),
m_csbk()
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);

	m_embeddedLC.setLC(m_lc);
}

CRewriteSrc::~CRewriteSrc()
{
}

bool CRewriteSrc::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int srcId = data.getSrcId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_USER_USER || slotNo != m_fromSlot || srcId < m_fromIdStart || srcId > m_fromIdEnd) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteSrc from %s Slot=%u Src=%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);
		return false;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	data.setDstId(m_toTG);
	data.setFLCO(FLCO_GROUP);

	unsigned char dataType = data.getDataType();

	switch (dataType) {
	case DT_VOICE_LC_HEADER:
	case DT_TERMINATOR_WITH_LC:
		processHeader(data, dataType);
		break;
	case DT_VOICE:
		processVoice(data);
		break;
	case DT_CSBK:
		processCSBK(data);
		break;
	case DT_DATA_HEADER:
		processDataHeader(data);
		break;
	case DT_RATE_12_DATA:
	case DT_RATE_34_DATA:
	case DT_RATE_1_DATA:
	case DT_VOICE_SYNC:
		// Nothing to do
		break;
	default:
		// Not sure what to do
		break;
	}

	if (trace) {
		LogDebug("Rule Trace,\tRewriteSrc from %s Slot=%u Src=%u-%u: matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);
		LogDebug("Rule Trace,\tRewriteSrc to %s Slot=%u Dst=TG%u", m_name.c_str(), m_toSlot, m_toTG);
	}

	return true;
}

void CRewriteSrc::processHeader(CDMRData& data, unsigned char dataType)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId()) {
		m_lc.setSrcId(srcId);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewriteSrc::processVoice(CDMRData& data)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId()) {
		m_lc.setSrcId(srcId);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	unsigned char n = data.getN();
	m_embeddedLC.getData(buffer, n);

	data.setData(buffer);
}

void CRewriteSrc::processDataHeader(CDMRData& data)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	bool ret = m_dataHeader.put(buffer);
	if (!ret)
		return;

	m_dataHeader.setGI(true);
	m_dataHeader.setDstId(m_toTG);	

	m_dataHeader.get(buffer);
	
	data.setData(buffer);
}

void CRewriteSrc::processCSBK(CDMRData& data)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	bool ret = m_csbk.put(buffer);
	if (!ret)
		return;

	m_csbk.setGI(true);
	m_csbk.setDstId(m_toTG);	

	m_csbk.get(buffer);
	
	data.setData(buffer);
}
