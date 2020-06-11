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

#if !defined(REWRITE_H)
#define	REWRITE_H

#include "DMREmbeddedData.h"
#include "DMRData.h"
#include "DMRLC.h"

enum PROCESS_RESULT {
	RESULT_UNMATCHED,
	RESULT_MATCHED,
	RESULT_IGNORED
};

class CRewrite {
public:
	CRewrite();
	virtual ~CRewrite();

	virtual PROCESS_RESULT process(CDMRData& data, bool trace) = 0;

protected:
	void processMessage(CDMRData& data);

private:
	CDMRLC            m_lc;
	CDMREmbeddedData  m_embeddedLC;
	CDMREmbeddedData* m_data;
	unsigned int      m_writeNum;
	unsigned int      m_readNum;
	unsigned char     m_lastN;

	void processHeader(CDMRData& data, unsigned char dataType);
	void processVoiceSync(CDMRData& data);
	void processVoice(CDMRData& data);
	void processDataHeader(CDMRData& data);
	void processData(CDMRData& data);
	void processCSBK(CDMRData& data);
	void swap();

	void setLC(FLCO flco, unsigned int srcId, unsigned int dstId);

	void processEmbeddedData(unsigned char* data, unsigned char n);
};

#endif
