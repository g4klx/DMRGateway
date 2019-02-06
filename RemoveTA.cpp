/*
*   Copyright (C) 2017,2019 by Jonathan Naylor G4KLX
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

#include "RemoveTA.h"
#include "DMREMB.h"
#include "DMRLC.h"

#include <cstdio>

CRemoveTA::CRemoveTA() :
m_embeddedLC()
{
}

CRemoveTA::~CRemoveTA()
{
}

void CRemoveTA::process(CDMRData& data)
{
	unsigned char dataType = data.getDataType();

	switch (dataType) {
	case DT_VOICE_LC_HEADER:
		processHeader(data);
		break;

	case DT_VOICE:
		processVoice(data);
		break;

	case DT_CSBK:
	case DT_DATA_HEADER:
	case DT_RATE_12_DATA:
	case DT_RATE_34_DATA:
	case DT_RATE_1_DATA:
	case DT_VOICE_SYNC:
	case DT_TERMINATOR_WITH_LC:
		// Nothing to do
		break;

	case DT_VOICE_PI_HEADER:
	default:
		// Not sure what to do
		break;
	}
}

void CRemoveTA::processHeader(CDMRData& data)
{
	CDMRLC lc;
	lc.setFLCO(data.getFLCO());
	lc.setSrcId(data.getSrcId());
	lc.setDstId(data.getDstId());

	m_embeddedLC.setLC(lc);
}

void CRemoveTA::processVoice(CDMRData& data)
{
	unsigned char n = data.getN();

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];
	data.getData(buffer);

	CDMREMB emb;
	emb.putData(buffer);

	unsigned char lcss = m_embeddedLC.getData(buffer, n);

	emb.setLCSS(lcss);
	emb.getData(buffer);

	data.setData(buffer);
}
