/*
 *   Copyright (C) 2019,2021,2023,2025 by Jonathan Naylor G4KLX
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

#include "MQTTConnection.h"
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

// In Log.cpp
extern CMQTTConnection* m_mqtt;

CRemoteControl::CRemoteControl(CDMRGateway* host) :
m_host(host),
m_command(REMOTE_COMMAND::NONE),
m_args()
{
}

CRemoteControl::~CRemoteControl()
{
}

REMOTE_COMMAND CRemoteControl::processCommand(const std::string& command)
{
	m_command = REMOTE_COMMAND::NONE;
	m_args.clear();

	std::string replyStr = "OK";

	// Parse the original command into a vector of strings.
	size_t start = command.find_first_not_of(' ');
	while (start != std::string::npos) {
		size_t end = command.find_first_of(' ', start);

		m_args.push_back(command.substr(start, end - start));

		start = command.find_first_not_of(' ', end);
	}

	if (m_args.at(0U) == "enable" && m_args.size() >= ENABLE_ARGS) {
		if (m_args.at(1U) == "net1")
			m_command = REMOTE_COMMAND::ENABLE_NETWORK1;
		else if (m_args.at(1U) == "net2")
			m_command = REMOTE_COMMAND::ENABLE_NETWORK2;
		else if (m_args.at(1U) == "net3")
			m_command = REMOTE_COMMAND::ENABLE_NETWORK3;
		else if (m_args.at(1U) == "net4")
			m_command = REMOTE_COMMAND::ENABLE_NETWORK4;
		else if (m_args.at(1U) == "net5")
			m_command = REMOTE_COMMAND::ENABLE_NETWORK5;
		else if (m_args.at(1U) == "xlx")
			m_command = REMOTE_COMMAND::ENABLE_XLX;
		else
			replyStr = "KO";
	} else if (m_args.at(0U) == "disable" && m_args.size() >= DISABLE_ARGS) {
		if (m_args.at(1U) == "net1")
			m_command = REMOTE_COMMAND::DISABLE_NETWORK1;
		else if (m_args.at(1U) == "net2")
			m_command = REMOTE_COMMAND::DISABLE_NETWORK2;
		else if (m_args.at(1U) == "net3")
			m_command = REMOTE_COMMAND::DISABLE_NETWORK3;
		else if (m_args.at(1U) == "net4")
			m_command = REMOTE_COMMAND::DISABLE_NETWORK4;
		else if (m_args.at(1U) == "net5")
			m_command = REMOTE_COMMAND::DISABLE_NETWORK5;
		else if (m_args.at(1U) == "xlx")
			m_command = REMOTE_COMMAND::DISABLE_XLX;
		else
			replyStr = "KO";
	} else if (m_args.at(0U) == "status") {
		if (m_host != nullptr)
			m_host->buildNetworkStatusString(replyStr);
		else
			replyStr = "KO";

		m_command = REMOTE_COMMAND::CONNECTION_STATUS;
	} else if (m_args.at(0U) == "hosts") {
		if (m_host != nullptr)
			m_host->buildNetworkHostsString(replyStr);
		else
			replyStr = "KO";

		m_command = REMOTE_COMMAND::CONFIG_HOSTS;
	} else {
		replyStr = "KO";
	}

	char buffer[200U];
	::snprintf(buffer, 200, "%s remote command of \"%s\" received", ((m_command == REMOTE_COMMAND::NONE) ? "Invalid" : "Valid"), command.c_str());

	if (m_command == REMOTE_COMMAND::NONE) {
		m_args.clear();
		LogWarning(buffer);
	} else {
#if !defined(REMOTE_COMMAND_NO_LOG)
		LogMessage(buffer);
#endif
	}

	m_mqtt->publish("response", replyStr);
	
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

