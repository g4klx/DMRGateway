/*
 *   Copyright (C) 2018,2020 by Jonathan Naylor G4KLX
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

#include "GPSD.h"

#if defined(USE_GPSD)

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

CGPSD::CGPSD(const std::string& address, const std::string& port) :
m_gpsdAddress(address),
m_gpsdPort(port),
m_gpsdData(),
m_idTimer(1000U, 60U),
m_networks(),
m_aprs(NULL)
{
	assert(!address.empty());
	assert(!port.empty());
}

CGPSD::~CGPSD()
{
}

void CGPSD::addNetwork(CDMRNetwork* network)
{
	assert(network != NULL);

	m_networks.push_back(network);
}

void CGPSD::setAPRS(CAPRSWriter* aprs)
{
	assert(aprs != NULL);

	m_aprs = aprs;
}

bool CGPSD::open()
{
	int ret = ::gps_open(m_gpsdAddress.c_str(), m_gpsdPort.c_str(), &m_gpsdData);
	if (ret != 0) {
		LogError("Error when opening access to gpsd - %d - %s", errno, ::gps_errstr(errno));
		return false;
	}

	::gps_stream(&m_gpsdData, WATCH_ENABLE | WATCH_JSON, NULL);

	LogMessage("Connected to GPSD");

	m_idTimer.start();

	return true;
}

void CGPSD::clock(unsigned int ms)
{
	m_idTimer.clock(ms);

	if (m_idTimer.hasExpired()) {
		sendReport();
		m_idTimer.start();
	}
}

void CGPSD::close()
{
	::gps_stream(&m_gpsdData, WATCH_DISABLE, NULL);
	::gps_close(&m_gpsdData);
}

void CGPSD::sendReport()
{
	if (!::gps_waiting(&m_gpsdData, 0))
		return;

#if GPSD_API_MAJOR_VERSION >= 7
	if (::gps_read(&m_gpsdData, NULL, 0) <= 0)
		return;
#else
	if (::gps_read(&m_gpsdData) <= 0)
		return;
#endif

	if (m_gpsdData.status != STATUS_FIX)
		return;

	bool latlonSet = (m_gpsdData.set & LATLON_SET) == LATLON_SET;
	if (!latlonSet)
		return;

	bool altitudeSet = (m_gpsdData.set & ALTITUDE_SET) == ALTITUDE_SET;

	float latitude  = float(m_gpsdData.fix.latitude);
	float longitude = float(m_gpsdData.fix.longitude);
#if GPSD_API_MAJOR_VERSION >= 9
	float altitude  = float(m_gpsdData.fix.altMSL);
#else
	float altitude  = float(m_gpsdData.fix.altitude);
#endif

	if (m_aprs != NULL)
		m_aprs->setLocation(latitude, longitude, altitudeSet ? altitude : 0.0F);

	for (std::vector<CDMRNetwork*>::const_iterator it = m_networks.begin(); it != m_networks.end(); ++it)
		(*it)->writeHomePosition(latitude, longitude);
}

#endif
