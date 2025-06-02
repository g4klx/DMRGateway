/*
 *   Copyright (C) 2019,2021,2025 by Jonathan Naylor G4KLX
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

#ifndef	RemoteControl_H
#define	RemoteControl_H

#include "UDPSocket.h"

#include <vector>
#include <string>

enum class REMOTE_COMMAND {
	//!!TODO: make command with argument to remove networks limit
	ENABLE_NETWORK1,
	ENABLE_NETWORK2,
	ENABLE_NETWORK3,
	ENABLE_NETWORK4,
	ENABLE_NETWORK5,
	ENABLE_NETWORK6,
	ENABLE_NETWORK7,
	ENABLE_NETWORK8,
	ENABLE_XLX,
	DISABLE_NETWORK1,
	DISABLE_NETWORK2,
	DISABLE_NETWORK3,
	DISABLE_NETWORK4,
	DISABLE_NETWORK5,
	DISABLE_NETWORK6,
	DISABLE_NETWORK7,
	DISABLE_NETWORK8,
	DISABLE_XLX,
	CONNECTION_STATUS,
	CONFIG_HOSTS,
	NONE
};

class CDMRGateway;
class CDMRNetwork;

class CRemoteControl {
public:
	CRemoteControl(CDMRGateway* host, const std::string address, unsigned short port);
	~CRemoteControl();

	bool open();

	REMOTE_COMMAND getCommand();

	unsigned int getArgCount() const;

	std::string  getArgString(unsigned int n) const;
	unsigned int getArgUInt(unsigned int n) const;
	signed int   getArgInt(unsigned int n) const;

	void close();

private:
	CDMRGateway*             m_host;
	CUDPSocket               m_socket;
	REMOTE_COMMAND           m_command;
	std::vector<std::string> m_args;
};

#endif
