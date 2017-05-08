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

#if !defined(REWRITE_H)
#define	REWRITE_H

#include "DMREmbeddedData.h"
#include "DMRData.h"
#include "DMRLC.h"

class CRewrite {
public:
	CRewrite(unsigned int fromSLot, unsigned int fromTG, unsigned int toSlot, unsigned int toTG);
	~CRewrite();

	bool process(CDMRData& data);

private:
	unsigned int     m_fromSlot;
	unsigned int     m_fromTG;
	unsigned int     m_toSlot;
	unsigned int     m_toTG;
	CDMRLC           m_lc;
	CDMREmbeddedData m_embeddedLC;

	void processHeader(CDMRData& data, unsigned char dataType);
	void processVoice(CDMRData& data);
};


#endif
