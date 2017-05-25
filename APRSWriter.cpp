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

#include "APRSWriter.h"

#include "YSFDefines.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

CAPRSWriter::CAPRSWriter(const std::string& callsign, const std::string& suffix, const std::string& password, const std::string& address, unsigned int port) :
m_thread(NULL),
m_enabled(false),
m_idTimer(1000U, 20U * 60U),		// 20 minutes
m_callsign(callsign),
m_txFrequency(0U),
m_rxFrequency(0U),
m_latitude(0.0F),
m_longitude(0.0F),
m_height(0)
{
	assert(!callsign.empty());
	assert(!password.empty());
	assert(!address.empty());
	assert(port > 0U);

	if (!suffix.empty()) {
		m_callsign.append("-");
		m_callsign.append(suffix.substr(0U, 1U));
	}

	m_thread = new CAPRSWriterThread(m_callsign, password, address, port);
}

CAPRSWriter::~CAPRSWriter()
{
}

void CAPRSWriter::setInfo(unsigned int txFrequency, unsigned int rxFrequency, float latitude, float longitude, int height)
{
	m_txFrequency = txFrequency;
	m_rxFrequency = rxFrequency;
	m_latitude    = latitude;
	m_longitude   = longitude;
	m_height      = height;
}

bool CAPRSWriter::open()
{
    ::fprintf(stdout, "Opening\n");
	return m_thread->start();
}

void CAPRSWriter::write(const unsigned char* source, const char* type, unsigned char radio, float fLatitude, float fLongitude)
{
    ::fprintf(stdout, "Startig to Write!!\n");
	assert(source != NULL);
	assert(type != NULL);

	char callsign[11U];
	::memcpy(callsign, source, YSF_CALLSIGN_LENGTH);
	callsign[YSF_CALLSIGN_LENGTH] = 0x00U;

	size_t n = ::strspn(callsign, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	callsign[n] = 0x00U;

	double tempLat = ::fabs(fLatitude);
	double tempLong = ::fabs(fLongitude);

	double latitude = ::floor(tempLat);
	double longitude = ::floor(tempLong);

	latitude = (tempLat - latitude)  * 60.0 + latitude  * 100.0;
	longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

	char lat[20U];
	::sprintf(lat, "%07.2lf", latitude);

	char lon[20U];
	::sprintf(lon, "%08.2lf", longitude);

	char symbol;
	switch (radio) {
	case 0x24U:
	case 0x28U:
		symbol = '[';
		break;
	case 0x25U:
	case 0x29U:
		symbol = '>';
		break;
	case 0x26U:
		symbol = 'r';
		break;
	default:
		symbol = '-';
		break;
	}

	char output[300U];
	::sprintf(output, "%s-Y>APDPRS,WIDE1-1,qAR,%s:!%s%c/%s%c%c %s",
		callsign, m_callsign.c_str(),
		lat, (fLatitude < 0.0F) ? 'S' : 'N',
		lon, (fLongitude < 0.0F) ? 'W' : 'E',
		symbol, type);

        ::fprintf(stdout, "Sending: %s\n", output);
        
	m_thread->write(output);
        ::fprintf(stdout, "Finished Writing to thread\n");
}

void CAPRSWriter::clock(unsigned int ms)
{
	m_idTimer.clock(ms);

	if (m_idTimer.hasExpired()) {
		sendIdFrames();
		m_idTimer.start();
	}
}

void CAPRSWriter::close()
{
	m_thread->stop();
}

void CAPRSWriter::sendIdFrames()
{
	if (!m_thread->isConnected())
		return;

	// Default values aren't passed on
	if (m_latitude == 0.0F && m_longitude == 0.0F)
		return;

	char desc[100U];
	if (m_txFrequency != 0U) {
		float offset = float(int(m_rxFrequency) - int(m_txFrequency)) / 1000000.0F;
		::sprintf(desc, "MMDVM Voice %.5lfMHz %c%.4lfMHz",
			float(m_txFrequency) / 1000000.0F,
			offset < 0.0F ? '-' : '+',
			::fabs(offset));
	} else {
		::strcpy(desc, "MMDVM Voice");
	}

	const char* band = "4m";
	if (m_txFrequency >= 1200000000U)
		band = "1.2";
	else if (m_txFrequency >= 420000000U)
		band = "440";
	else if (m_txFrequency >= 144000000U)
		band = "2m";
	else if (m_txFrequency >= 50000000U)
		band = "6m";
	else if (m_txFrequency >= 28000000U)
		band = "10m";

	double tempLat  = ::fabs(m_latitude);
	double tempLong = ::fabs(m_longitude);

	double latitude  = ::floor(tempLat);
	double longitude = ::floor(tempLong);

	latitude  = (tempLat  - latitude)  * 60.0 + latitude  * 100.0;
	longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

	char lat[20U];
	::sprintf(lat, "%04.2lf", latitude);

	char lon[20U];
	::sprintf(lon, "%05.2lf", longitude);

	std::string server = m_callsign;
	size_t pos = server.find_first_of('-');
	if (pos == std::string::npos)
		server.append("-S");
	else
		server.append("S");

	char output[500U];
	::sprintf(output, "%s>APDG03,TCPIP*,qAC,%s:!%s%cD%s%c&/A=%06.0f%s %s",
		m_callsign.c_str(), server.c_str(),
		lat, (m_latitude < 0.0F)  ? 'S' : 'N',
		lon, (m_longitude < 0.0F) ? 'W' : 'E',
		float(m_height) * 3.28F, band, desc);

	m_thread->write(output);

	m_idTimer.start();
}
