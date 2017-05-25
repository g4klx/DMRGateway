/*
 *   Copyright (C) 2010-2014,2016 by Jonathan Naylor G4KLX
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

#include "APRSWriterThread.h"
#include "Utils.h"
#include "Log.h"

#include <algorithm>
#include <functional>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cassert>

// #define	DUMP_TX

const unsigned int CALLSIGN_LENGTH = 8U;

const unsigned int APRS_TIMEOUT = 10U;

CAPRSWriterThread::CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, unsigned int port) :
CThread(),
m_username(callsign),
m_password(password),
m_socket(address, port),
m_queue(20U, "APRS Queue"),
m_exit(false),
m_connected(false),
m_APRSReadCallback(NULL),
m_filter(),
m_clientName("YSFGateway")
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!address.empty());
	assert(port > 0U);

	m_username.resize(CALLSIGN_LENGTH, ' ');
	m_username.erase(std::find_if(m_username.rbegin(), m_username.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), m_username.end());
	std::transform(m_username.begin(), m_username.end(), m_username.begin(), ::toupper);
}

CAPRSWriterThread::CAPRSWriterThread(const std::string& callsign, const std::string& password, const std::string& address, unsigned int port, const std::string& filter, const std::string& clientName) :
CThread(),
m_username(callsign),
m_password(password),
m_socket(address, port),
m_queue(20U, "APRS Queue"),
m_exit(false),
m_connected(false),
m_APRSReadCallback(NULL),
m_filter(filter),
m_clientName(clientName)
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!address.empty());
	assert(port > 0U);

	m_username.resize(CALLSIGN_LENGTH, ' ');
	m_username.erase(std::find_if(m_username.rbegin(), m_username.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), m_username.end());
	std::transform(m_username.begin(), m_username.end(), m_username.begin(), ::toupper);
}

CAPRSWriterThread::~CAPRSWriterThread()
{
	m_username.clear();
}

bool CAPRSWriterThread::start()
{
	run();

	return true;
}

void CAPRSWriterThread::entry()
{
	LogMessage("Starting the APRS Writer thread");

	m_connected = connect();

	try {
		while (!m_exit) {
			if (!m_connected) {
				m_connected = connect();

				if (!m_connected){
					LogError("Reconnect attempt to the APRS server has failed");
					sleep(10000UL);		// 10 secs
				}
			}

			if (m_connected) {
				if (!m_queue.isEmpty()){
                                    
					char* p = NULL;
					m_queue.getData(&p, 1U);

					LogMessage("APRS ==> %s", p);

					::strcat(p, "\r\n");

					bool ret = m_socket.write((unsigned char*)p, ::strlen(p));
					if (!ret) {
						m_connected = false;
						m_socket.close();
						LogError("Connection to the APRS thread has failed");
					}

					delete[] p;
				}
				{
					std::string line;
					int length = m_socket.readLine(line, APRS_TIMEOUT);

					if (length < 0) {
						m_connected = false;
						m_socket.close();
						LogError("Error when reading from the APRS server");
					}

					if(length > 0 && line.at(0U) != '#'//check if we have something and if that something is an APRS frame
					    && m_APRSReadCallback != NULL)//do we have someone wanting an APRS Frame?
					{	
						//wxLogMessage(wxT("Received APRS Frame : ") + line);
						m_APRSReadCallback(std::string(line));
					}
				}

			}
		}

		if (m_connected)
			m_socket.close();

		while (!m_queue.isEmpty()) {
			char* p = NULL;
			m_queue.getData(&p, 1U);
			delete[] p;
		}
	}
	catch (std::exception& e) {
		LogError("Exception raised in the APRS Writer thread - \"%s\"", e.what());
	}
	catch (...) {

            LogError("Unknown exception raised in the APRS Writer thread");
	}
        

	LogMessage("Stopping the APRS Writer thread");
}

void CAPRSWriterThread::setReadAPRSCallback(ReadAPRSFrameCallback cb)
{
	m_APRSReadCallback = cb;
}

void CAPRSWriterThread::write(const char* data)
{
	assert(data != NULL);

	if (!m_connected)
        {
		return;
        }

	unsigned int len = ::strlen(data);

	char* p = new char[len + 5U];
	::strcpy(p, data);

	m_queue.addData(&p, 1U);
}

bool CAPRSWriterThread::isConnected() const
{
	return m_connected;
}

void CAPRSWriterThread::stop()
{
	m_exit = true;

	wait();
}

bool CAPRSWriterThread::connect()
{
	bool ret = m_socket.open();
	if (!ret)
		return false;

	//wait for lgin banner
	int length;
	std::string serverResponse;
	length = m_socket.readLine(serverResponse, APRS_TIMEOUT);
	if (length == 0) {
		LogError("No reply from the APRS server after %u seconds", APRS_TIMEOUT);
		m_socket.close();
		return false;
	}

	LogMessage("Received login banner : %s", serverResponse.c_str());
        

	std::string filter(m_filter);
	if (filter.length() > 0)
		filter.insert(0U, " filter ");

	char connectString[200U];
	::sprintf(connectString, "user %s pass %s vers %s%s\n", m_username.c_str(), m_password.c_str(), (m_clientName.length() ? m_clientName : "YSFGateway").c_str(), filter.c_str());

	ret = m_socket.writeLine(std::string(connectString));
	if (!ret) {
		m_socket.close();
		return false;
	}

	length = m_socket.readLine(serverResponse, APRS_TIMEOUT);
	if (length == 0) {
		LogError("No reply from the APRS server after %u seconds", APRS_TIMEOUT);
		m_socket.close();
		return false;
	}

	if (length < 0) {
		LogError("Error when reading from the APRS server");
		m_socket.close();
		return false;
	}

	LogMessage("Response from APRS server: %s", serverResponse.c_str());

	LogMessage("Connected to the APRS server");

	return true;
}
