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

#include "Rewrite.h"

#include "DMRDefines.h"
#include "DMRFullLC.h"

#include <cstdio>

CRewrite::CRewrite() :
m_slot(0U),
m_tg(0U),
m_lc(NULL),
m_embeddedLC()
{
}

CRewrite::~CRewrite()
{
}

void CRewrite::setParams(unsigned int slot, unsigned int tg)
{
	m_slot = slot;
	m_tg   = tg;
}

void CRewrite::process(CDMRData& data)
{
	data.setSlotNo(m_slot);

	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();

	if (flco == FLCO_GROUP && dstId != m_tg) {
		data.setDstId(m_tg);

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
	}
}

void CRewrite::processHeader(CDMRData& data, unsigned char dataType)
{
	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	delete m_lc;
	m_lc = NULL;

	CDMRFullLC fullLC;
	m_lc = fullLC.decode(buffer, dataType);
	if (m_lc == NULL)
		return;

	m_lc->setDstId(m_tg);

	m_embeddedLC.setLC(*m_lc);

	fullLC.encode(*m_lc, buffer, dataType);

	data.setData(buffer);
}

void CRewrite::processVoice(CDMRData& data)
{
	if (m_lc == NULL) {
		m_lc = new CDMRLC(FLCO_GROUP, data.getSrcId(), m_tg);
		m_embeddedLC.setLC(*m_lc);
	}

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	unsigned char n = data.getN();
	m_embeddedLC.getData(buffer, n);

	data.setData(buffer);
}
