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

#include "Voice.h"

CVoice::CVoice(const std::string& language, unsigned int slot, unsigned int tg) :
m_language(language),
m_slot(slot),
m_tg(tg),
m_status(VS_NONE),
m_timer(1000U, 1U),
m_stopWatch()
{
}

CVoice::~CVoice()
{
}

bool CVoice::open()
{
	return true;
}

void CVoice::linkedTo(unsigned int id)
{
	m_status = VS_WAITING;
	m_timer.start();
}

void CVoice::unlinked()
{
	m_status = VS_WAITING;
	m_timer.start();
}

bool CVoice::read(CDMRData& data)
{
	if (m_status != VS_SENDING)
		return false;

	return false;
}

void CVoice::clock(unsigned int ms)
{
	m_timer.clock(ms);
	if (m_timer.isRunning() && m_timer.hasExpired()) {
		if (m_status == VS_WAITING) {
			m_stopWatch.start();
			m_status = VS_SENDING;
		}
	}
}
