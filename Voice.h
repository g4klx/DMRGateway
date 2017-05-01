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

#if !defined(Voice_H)
#define	Voice_H

#include "StopWatch.h"
#include "DMRData.h"
#include "Timer.h"

#include <string>

enum VOICE_STATUS {
	VS_NONE,
	VS_WAITING,
	VS_SENDING
};

class CVoice {
public:
	CVoice(const std::string& language, unsigned int slot, unsigned int tg);
	~CVoice();

	bool open();

	void linkedTo(unsigned int id);
	void unlinked();

	bool read(CDMRData& data);

	void clock(unsigned int ms);

private:
	std::string  m_language;
	unsigned int m_slot;
	unsigned int m_tg;
	VOICE_STATUS m_status;
	CTimer       m_timer;
	CStopWatch   m_stopWatch;
};

#endif
