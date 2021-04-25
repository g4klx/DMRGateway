/*
 *   Copyright (C) 2010,2011,2012,2016,2017,2018,2020 by Jonathan Naylor G4KLX
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

#include "UDPSocket.h"
#include "Timer.h"

#include <string>

#if !defined(_WIN32) && !defined(_WIN64)
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#else
#include <WS2tcpip.h>
#endif

class CAPRSWriter {
public:
	CAPRSWriter(const std::string& callsign, const std::string& suffix, const std::string& address, unsigned short port, bool debug);
	~CAPRSWriter();

	bool open();

	void setInfo(unsigned int txFrequency, unsigned int rxFrequency, const std::string& desc);

	void setLocation(float latitude, float longitude, int height);

	void clock(unsigned int ms);

	void close();

private:
	CTimer            m_idTimer;
	std::string       m_callsign;
	bool              m_debug;
	unsigned int      m_txFrequency;
	unsigned int      m_rxFrequency;
	float             m_latitude;
	float             m_longitude;
	int               m_height;
	std::string       m_desc;
	sockaddr_storage  m_aprsAddr;
	unsigned int      m_aprsLen;
	CUDPSocket        m_aprsSocket;

	void sendIdFrame();
};

#endif
