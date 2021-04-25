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

#include "RemoteControl.h"
#include "Log.h"
#include "DMRGateway.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

const unsigned int ENABLE_ARGS = 2U;
const unsigned int DISABLE_ARGS = 2U;

const unsigned int BUFFER_LENGTH = 100U;

CRemoteControl::CRemoteControl(CDMRGateway* host, const std::string address, unsigned short port) :
m_host(host),
m_socket(address, port),
m_command(RCD_NONE),
m_args()
{
	assert(port > 0U);
}

CRemoteControl::~CRemoteControl()
{
}

bool CRemoteControl::open()
{
	return m_socket.open();
}

REMOTE_COMMAND CRemoteControl::getCommand()
{
	m_command = RCD_NONE;
	m_args.clear();

	char command[BUFFER_LENGTH];
	char buffer[BUFFER_LENGTH * 2];
	std::string replyStr = "OK";
	sockaddr_storage address;
	unsigned int addrlen;
	int ret = m_socket.read((unsigned char*)buffer, BUFFER_LENGTH, address, addrlen);
	if (ret > 0) {
		buffer[ret] = '\0';

		// Make a copy of the original command for logging.
		::strcpy(command, buffer);

		// Parse the original command into a vector of strings.
		char* b = buffer;
		char* p = NULL;
		while ((p = ::strtok(b, " ")) != NULL) {
			b = NULL;
			m_args.push_back(std::string(p));
		}
		if (m_args.at(0U) == "enable" && m_args.size() >= ENABLE_ARGS) {
			if (m_args.at(1U) == "net1")
				m_command = RCD_ENABLE_NETWORK1;
			else if (m_args.at(1U) == "net2")
				m_command = RCD_ENABLE_NETWORK2;
			else if (m_args.at(1U) == "net3")
				m_command = RCD_ENABLE_NETWORK3;
			else if (m_args.at(1U) == "net4")
				m_command = RCD_ENABLE_NETWORK4;
			else if (m_args.at(1U) == "net5")
				m_command = RCD_ENABLE_NETWORK5;
			else if (m_args.at(1U) == "xlx")
				m_command = RCD_ENABLE_XLX;
			else
				replyStr = "KO";
		} else if (m_args.at(0U) == "disable" && m_args.size() >= DISABLE_ARGS) {
			if (m_args.at(1U) == "net1")
				m_command = RCD_DISABLE_NETWORK1;
			else if (m_args.at(1U) == "net2")
				m_command = RCD_DISABLE_NETWORK2;
			else if (m_args.at(1U) == "net3")
				m_command = RCD_DISABLE_NETWORK3;
			else if (m_args.at(1U) == "net4")
				m_command = RCD_DISABLE_NETWORK4;
			else if (m_args.at(1U) == "net5")
				m_command = RCD_DISABLE_NETWORK5;
			else if (m_args.at(1U) == "xlx")
				m_command = RCD_DISABLE_XLX;
			else
				replyStr = "KO";
		} else if (m_args.at(0U) == "status") {
			if (m_host != NULL) {
				m_host->buildNetworkStatusString(replyStr);
			}
			else {
				replyStr = "KO";
			}

			m_command = RCD_CONNECTION_STATUS;
		} else {
			replyStr = "KO";
		}

		::snprintf(buffer, BUFFER_LENGTH * 2, "%s remote command of \"%s\" received", ((m_command == RCD_NONE) ? "Invalid" : "Valid"), command);
		if (m_command == RCD_NONE) {
			m_args.clear();
			LogWarning(buffer);
		} else {
			LogMessage(buffer);
		}

		m_socket.write((unsigned char*)replyStr.c_str(), (unsigned int)replyStr.length(), address, addrlen);
	}
	
	return m_command;
}

unsigned int CRemoteControl::getArgCount() const
{
	switch (m_command) {
		default:
			return 0U;
	}
}

std::string CRemoteControl::getArgString(unsigned int n) const
{
	switch (m_command) {
		default:
			return "";
	}

	if (n >= m_args.size())
		return "";

	return m_args.at(n);
}

unsigned int CRemoteControl::getArgUInt(unsigned int n) const
{
	return (unsigned int)::atoi(getArgString(n).c_str());
}

int CRemoteControl::getArgInt(unsigned int n) const
{
	return ::atoi(getArgString(n).c_str());
}

void CRemoteControl::close()
{
	m_socket.close();
}
