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

#include "RewritePC.h"

#include "DMRDefines.h"
#include "DMRFullLC.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewritePC::CRewritePC(const std::string& name, unsigned int fromSlot, unsigned int fromId, unsigned int toSlot, unsigned int toId, unsigned int range) :
m_name(name),
m_fromSlot(fromSlot),
m_fromIdStart(fromId),
m_fromIdEnd(fromId + range - 1U),
m_toSlot(toSlot),
m_toIdStart(toId),
m_toIdEnd(toId + range - 1U),
m_lc(FLCO_USER_USER, 0U, 0U),
m_embeddedLC(),
m_dataHeader(),
m_csbk()
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewritePC::~CRewritePC()
{
}

bool CRewritePC::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_USER_USER || slotNo != m_fromSlot || dstId < m_fromIdStart || dstId > m_fromIdEnd) {
		if (trace)
			LogDebug("Rule Trace,\tRewritePC from %s Slot=%u Dst=%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);
		return false;
	}

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	if (m_fromIdStart != m_toIdStart) {
		unsigned int newDstId = dstId + m_toIdStart - m_fromIdStart;

		data.setDstId(newDstId);

		unsigned char dataType = data.getDataType();

		switch (dataType) {
		case DT_VOICE_LC_HEADER:
		case DT_TERMINATOR_WITH_LC:
			processHeader(data, newDstId, dataType);
			break;
		case DT_VOICE:
			processVoice(data, newDstId);
			break;
		case DT_CSBK:
			processCSBK(data, newDstId);
			break;
		case DT_DATA_HEADER:
			processDataHeader(data, newDstId);
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
	}

	if (trace) {
		LogDebug("Rule Trace,\tRewritePC from %s Slot=%u Dst=%u-%u: not matched", m_name.c_str(), m_fromSlot, m_fromIdStart, m_fromIdEnd);
		LogDebug("Rule Trace,\tRewritePC to %s Slot=%u Dst=%u-%u", m_name.c_str(), m_toSlot, m_toIdStart, m_toIdEnd);
	}

	return true;
}

void CRewritePC::processHeader(CDMRData& data, unsigned int dstId, unsigned char dataType)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId() || dstId != m_lc.getDstId()) {
		m_lc.setSrcId(srcId);
		m_lc.setDstId(dstId);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewritePC::processVoice(CDMRData& data, unsigned int dstId)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId() || dstId != m_lc.getDstId()) {
		m_lc.setSrcId(srcId);
		m_lc.setDstId(dstId);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	unsigned char n = data.getN();
	m_embeddedLC.getData(buffer, n);

	data.setData(buffer);
}

void CRewritePC::processDataHeader(CDMRData& data, unsigned int dstId)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	bool ret = m_dataHeader.put(buffer);
	if (!ret)
		return;

	m_dataHeader.setDstId(dstId);	

	m_dataHeader.get(buffer);
	
	data.setData(buffer);
}

void CRewritePC::processCSBK(CDMRData& data, unsigned int dstId)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	bool ret = m_csbk.put(buffer);
	if (!ret)
		return;

	m_csbk.setDstId(dstId);	

	m_csbk.get(buffer);
	
	data.setData(buffer);
}
