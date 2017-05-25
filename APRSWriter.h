/*
 *   Copyright (C) 2010,2011,2012,2016 by Jonathan Naylor G4KLX
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

#ifndef	APRSWriter_H
#define	APRSWriter_H

#include "APRSWriterThread.h"
#include "Timer.h"

#include <string>

class CAPRSWriter {
public:
	CAPRSWriter(const std::string& callsign, const std::string& suffix, const std::string& password, const std::string& address, unsigned int port);
	~CAPRSWriter();

	bool open();

	void setInfo(unsigned int txFrequency, unsigned int rxFrequency, float latitude, float longitude, int height);

	void write(const unsigned char* source, const char* type, unsigned char radio, float latitude, float longitude);

	void clock(unsigned int ms);

	void close();

private:
	CAPRSWriterThread* m_thread;
	bool               m_enabled;
	CTimer             m_idTimer;
	std::string        m_callsign;
	unsigned int       m_txFrequency;
	unsigned int       m_rxFrequency;
	float              m_latitude;
	float              m_longitude;
	int                m_height;

	void sendIdFrames();
};

#endif
