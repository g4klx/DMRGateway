/*
 *   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
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

#if !defined(DMRNetwork_H)
#define	DMRNetwork_H

#include "UDPSocket.h"
#include "Timer.h"
#include "RingBuffer.h"
#include "DMRData.h"

#include <string>
#include <cstdint>

class CDMRNetwork
{
public:
	CDMRNetwork(const std::string& address, unsigned int port, unsigned int local, unsigned int id, const std::string& password, const std::string& name, bool debug);
	~CDMRNetwork();

	void setOptions(const std::string& options);

	void setConfig(const unsigned char* config, unsigned int len);

	bool open();

	bool read(CDMRData& data);

	bool write(const CDMRData& data);

	bool writePosition(const unsigned char* data, unsigned int length);

	bool writeTalkerAlias(const unsigned char* data, unsigned int length);

	bool wantsBeacon();

	void clock(unsigned int ms);

	bool isConnected() const;

	void close();

private: 
	in_addr      m_address;
	std::string  m_hostname;
	unsigned int m_port;
	uint8_t*     m_id;
	std::string  m_password;
	std::string  m_name;
	bool         m_debug;
	CUDPSocket   m_socket;

	enum STATUS {
		WAITING_CONNECT,
		WAITING_LOGIN,
		WAITING_AUTHORISATION,
		WAITING_CONFIG,
		WAITING_OPTIONS,
		RUNNING
	};

	STATUS         m_status;
	CTimer         m_retryTimer;
	CTimer         m_timeoutTimer;
	unsigned char* m_buffer;
	unsigned char* m_salt;

	CRingBuffer<unsigned char> m_rxData;

	std::string    m_options;

	unsigned char* m_configData;
	unsigned int   m_configLen;

	bool           m_beacon;

	bool writeLogin();
	bool writeAuthorisation();
	bool writeOptions();
	bool writeConfig();
	bool writePing();

	bool write(const unsigned char* data, unsigned int length);
};

#endif
