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
#include "DMRFullLC.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

CRewriteType::CRewriteType(const char* name, unsigned int fromSlot, unsigned int fromId, unsigned int toSlot, unsigned int toTG) :
m_name(name),
m_fromSlot(fromSlot),
m_fromId(fromId),
m_toSlot(toSlot),
m_toTG(toTG),
m_lc(FLCO_GROUP, 0U, toTG),
m_embeddedLC()
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewriteType::~CRewriteType()
{
}

bool CRewriteType::process(CDMRData& data)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_USER_USER || slotNo != m_fromSlot || dstId != m_fromId)
		return false;

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
	case DT_VOICE_SYNC:
		// Nothing to do
		break;
	default:
		// Not sure what to do
		break;
	}

	return true;
}

void CRewriteType::processHeader(CDMRData& data, unsigned char dataType)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId()) {
		m_lc.setSrcId(srcId);
		m_embeddedLC.setLC(m_lc);
	}

	LogDebug("%s, Private call to TG rewrite of id: %u", m_name, m_fromId);

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewriteType::processVoice(CDMRData& data)
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
