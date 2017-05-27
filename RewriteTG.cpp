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

#include "RewriteTG.h"

#include "DMRDefines.h"
#include "DMRFullLC.h"

#include <cstdio>
#include <cassert>

CRewriteTG::CRewriteTG(const char* name, unsigned int fromSlot, unsigned int fromTG, unsigned int toSlot, unsigned int toTG, unsigned int range) :
m_name(name),
m_fromSlot(fromSlot),
m_fromTGStart(fromTG),
m_fromTGEnd(fromTG + range),
m_toSlot(toSlot),
m_toTGStart(toTG),
m_lc(FLCO_GROUP, 0U, toTG),
m_embeddedLC()
{
	assert(fromSlot == 1U || fromSlot == 2U);
	assert(toSlot == 1U || toSlot == 2U);
}

CRewriteTG::~CRewriteTG()
{
}

bool CRewriteTG::processRF(CDMRData& data)
{
	return process(data);
}

bool CRewriteTG::processNet(CDMRData& data)
{
	return process(data);
}

bool CRewriteTG::process(CDMRData& data)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();
	unsigned int slotNo = data.getSlotNo();

	if (flco != FLCO_GROUP || slotNo != m_fromSlot || dstId < m_fromTGStart || dstId >= m_fromTGEnd)
		return false;

	if (m_fromSlot != m_toSlot)
		data.setSlotNo(m_toSlot);

	if (m_fromTGStart != m_toTGStart) {
		unsigned int newTG = dstId + m_toTGStart - m_fromTGStart;

		data.setDstId(newTG);

		unsigned char dataType = data.getDataType();

		switch (dataType) {
		case DT_VOICE_LC_HEADER:
		case DT_TERMINATOR_WITH_LC:
			processHeader(data, newTG, dataType);
			break;
		case DT_VOICE:
			processVoice(data, newTG);
			break;
		case DT_VOICE_SYNC:
			// Nothing to do
			break;
		default:
			// Not sure what to do
			break;
		}
	}

	return true;
}

void CRewriteTG::processHeader(CDMRData& data, unsigned int tg, unsigned char dataType)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId() || tg != m_lc.getDstId()) {
		m_lc.setSrcId(srcId);
		m_lc.setDstId(tg);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewriteTG::processVoice(CDMRData& data, unsigned int tg)
{
	unsigned int srcId = data.getSrcId();
	if (srcId != m_lc.getSrcId() || tg != m_lc.getDstId()) {
		m_lc.setSrcId(srcId);
		m_lc.setDstId(tg);
		m_embeddedLC.setLC(m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	unsigned char n = data.getN();
	m_embeddedLC.getData(buffer, n);

	data.setData(buffer);
}
