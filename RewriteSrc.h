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

#if !defined(REWRITESRC_H)
#define	REWRITESRC_H

#include "DMREmbeddedData.h"
#include "Rewrite.h"
#include "DMRData.h"
#include "DMRLC.h"

class CRewriteSrc : public IRewrite {
public:
	CRewriteSrc(const char* name, unsigned int fromSlot, unsigned int fromId, unsigned int toSlot, unsigned int toTG, unsigned int range);
	virtual ~CRewriteSrc();

	virtual bool processRF(CDMRData& data);
	virtual bool processNet(CDMRData& data);

private:
	const char*      m_name;
	unsigned int     m_fromSlot;
	unsigned int     m_fromIdStart;
	unsigned int     m_fromIdEnd;
	unsigned int     m_toSlot;
	unsigned int     m_toTG;
	CDMRLC           m_lc;
	CDMREmbeddedData m_embeddedLC;

	bool process(CDMRData& data);
	void processHeader(CDMRData& data, unsigned char dataType);
	void processVoice(CDMRData& data);
};


#endif
