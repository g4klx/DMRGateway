/*
 *   Copyright (C) 2010-2014,2016,2017,2018,2020,2023 by Jonathan Naylor G4KLX
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
#include "APRSWriter.h"
#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

// In Log.cpp
extern CMQTTConnection* m_mqtt;

CAPRSWriter::CAPRSWriter(const std::string& callsign, const std::string& suffix, bool debug) :
m_idTimer(1000U),
m_callsign(callsign),
m_debug(debug),
m_txFrequency(0U),
m_rxFrequency(0U),
m_latitude(0.0F),
m_longitude(0.0F),
m_height(0),
m_desc(),
m_symbol()
{
	assert(!callsign.empty());

	if (!suffix.empty()) {
		m_callsign.append("-");
		m_callsign.append(suffix.substr(0U, 1U));
	}
}

CAPRSWriter::~CAPRSWriter()
{
}

void CAPRSWriter::setInfo(unsigned int txFrequency, unsigned int rxFrequency, const std::string& desc, const std::string& symbol)

{
	m_txFrequency = txFrequency;
	m_rxFrequency = rxFrequency;
	m_desc        = desc;
	m_symbol      = symbol;
}

void CAPRSWriter::setLocation(float latitude, float longitude, int height)
{
	m_latitude  = latitude;
	m_longitude = longitude;
	m_height    = height;
}

bool CAPRSWriter::open()
{
	m_idTimer.setTimeout(60U);
	m_idTimer.start();

	return true;
}

void CAPRSWriter::clock(unsigned int ms)
{
	m_idTimer.clock(ms);
	if (m_idTimer.hasExpired()) {
		sendIdFrame();
		m_idTimer.setTimeout(20U * 60U);
		m_idTimer.start();
	}
}

void CAPRSWriter::close()
{
}

void CAPRSWriter::sendIdFrame()
{
	// Default values aren't passed on
	if (m_latitude == 0.0F && m_longitude == 0.0F)
		return;

	char desc[200U];
	if (m_txFrequency != 0U) {
		float offset = float(int(m_rxFrequency) - int(m_txFrequency)) / 1000000.0F;
		::sprintf(desc, "MMDVM Voice (DMR) %.5LfMHz %c%.4lfMHz%s%s",
			(long double)(m_txFrequency) / 1000000.0F,
			offset < 0.0F ? '-' : '+',
			::fabs(offset), m_desc.empty() ? "" : ", ", m_desc.c_str());
	} else {
		::sprintf(desc, "MMDVM Voice (DMR)%s%s", m_desc.empty() ? "" : ", ", m_desc.c_str());
	}

	const char* band = "4m";
	if (m_txFrequency >= 1200000000U)
		band = "23cm/1.2GHz";
	else if (m_txFrequency >= 420000000U)
		band = "70cm";
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
	::sprintf(lat, "%07.2lf", latitude);

	char lon[20U];
	::sprintf(lon, "%08.2lf", longitude);

	std::string server = m_callsign;
	std::string symbol = m_symbol;
	size_t pos = server.find_first_of('-');
	if (pos == std::string::npos)
		server.append("-S");
	else
		server.append("S");

        if (symbol.empty())
                symbol.append("D&");

	char output[500U];
	::sprintf(output, "%s>APDG03,TCPIP*,qAC,%s:!%s%c%c%s%c%c/A=%06.0f%s %s\r\n",
		m_callsign.c_str(), server.c_str(),
		lat, (m_latitude < 0.0F)  ? 'S' : 'N', symbol[0],
		lon, (m_longitude < 0.0F) ? 'W' : 'E', symbol[1],
		float(m_height) * 3.28F, band, desc);

	if (m_debug)
		LogDebug("APRS ==> %s", output);

	m_mqtt->publish("aprs-gateway/aprs", output);
}

