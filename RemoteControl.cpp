/*
 *   Copyright (C) 2019,2021,2023,2025,2026 by Jonathan Naylor G4KLX
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
#include <cctype>

const unsigned int ENABLE_ARGS = 2U;
const unsigned int DISABLE_ARGS = 2U;
const unsigned int LINK_ARGS = 3U;
const unsigned int UNLINK_ARGS = 2U;

const unsigned int BUFFER_LENGTH = 100U;

// In Log.cpp
extern CMQTTConnection* m_mqtt;

CRemoteControl::CRemoteControl(CDMRGateway* host) :
m_host(host),
m_command(REMOTE_COMMAND::NONE),
m_args(),
m_xlxNumber(),
m_xlxModule(0)
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

	if (m_args.empty()) {
		if (m_mqtt != nullptr)
			m_mqtt->publish("response", "KO");
		return REMOTE_COMMAND::NONE;
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
	} else if (m_args.at(0U) == "link" && m_args.size() >= LINK_ARGS && m_args.at(1U) == "xlx") {
		// link xlx <reflector> [<module>]: as a private call to Base+4000+reflector,
		// then to Base+module, from the radio.
		const std::string& number = m_args.at(2U);
		bool valid = number.length() >= 1U && number.length() <= 3U && number.find_first_not_of("0123456789") == std::string::npos;

		char module = 0;
		if (valid && m_args.size() > LINK_ARGS) {
			const std::string& m = m_args.at(3U);
			valid = m.length() == 1U && ::isalpha((unsigned char)m.at(0U));
			if (valid)
				module = (char)::toupper((unsigned char)m.at(0U));
		}

		if (valid) {
			char buffer[16U];
			::snprintf(buffer, sizeof(buffer), "%03u", (unsigned int)::atoi(number.c_str()));
			valid = m_host != nullptr && m_host->canLinkXLX(buffer);
			if (valid) {
				m_xlxNumber = buffer;
				m_xlxModule = module;
				m_command   = REMOTE_COMMAND::LINK_XLX;
			}
		}

		if (!valid)
			replyStr = "KO";
	} else if (m_args.at(0U) == "unlink" && m_args.size() >= UNLINK_ARGS && m_args.at(1U) == "xlx") {
		// unlink xlx: from the module, as a private call to Base.
		if (m_host != nullptr && m_host->canLinkXLX(""))
			m_command = REMOTE_COMMAND::UNLINK_XLX;
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

	if (m_mqtt != nullptr)
		m_mqtt->publish("response", replyStr);
	
	return m_command;
}

std::string CRemoteControl::getXLXNumber() const
{
	return m_xlxNumber;
}

char CRemoteControl::getXLXModule() const
{
	return m_xlxModule;
}
