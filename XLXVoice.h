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

#if !defined(XLXVoice_H)
#define	XLXVoice_H

#include "DMREmbeddedData.h"
#include "StopWatch.h"
#include "DMRData.h"
#include "DMRLC.h"
#include "Timer.h"

#include <string>
#include <vector>
#include <unordered_map>

enum XLXVOICE_STATUS {
	XLXVS_NONE,
	XLXVS_WAITING,
	XLXVS_SENDING
};

struct CXLXPositions {
	unsigned int m_start;
	unsigned int m_length;
};

class CXLXVoice {
public:
	CXLXVoice(const std::string& directory, const std::string& language, unsigned int id, unsigned int slot, unsigned int tg);
	~CXLXVoice();

	bool open();

	void linkedTo(unsigned int number, unsigned int room);
	void unlinked();

	void reset();

	bool read(CDMRData& data);

	void clock(unsigned int ms);

private:
	std::string                            m_indxFile;
	std::string                            m_ambeFile;
	unsigned int                           m_slot;
	CDMRLC                                 m_lc;
	CDMREmbeddedData                       m_embeddedLC;
	XLXVOICE_STATUS                        m_status;
	CTimer                                 m_timer;
	CStopWatch                             m_stopWatch;
	unsigned int                           m_seqNo;
	unsigned int                           m_streamId;
	unsigned int                           m_sent;
	unsigned char*                         m_ambe;
	std::unordered_map<std::string, CXLXPositions*> m_positions;
	std::vector<CDMRData*>                 m_data;
	std::vector<CDMRData*>::const_iterator m_it;

	void createHeaderTerminator(unsigned char type);
	void createVoice(const std::vector<std::string>& words);
};

#endif
