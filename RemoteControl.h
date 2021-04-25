/*
 *   Copyright (C) 2019,2021 by Jonathan Naylor G4KLX
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

enum REMOTE_COMMAND {
	RCD_ENABLE_NETWORK1,
	RCD_ENABLE_NETWORK2,
	RCD_ENABLE_NETWORK3,
	RCD_ENABLE_NETWORK4,
	RCD_ENABLE_NETWORK5,
	RCD_ENABLE_XLX,
	RCD_DISABLE_NETWORK1,
	RCD_DISABLE_NETWORK2,
	RCD_DISABLE_NETWORK3,
	RCD_DISABLE_NETWORK4,
	RCD_DISABLE_NETWORK5,
	RCD_DISABLE_XLX,
	RCD_CONNECTION_STATUS,
	RCD_NONE
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
