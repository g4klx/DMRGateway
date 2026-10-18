/*
 *   Copyright (C) 2018,2020,2025 by Jonathan Naylor G4KLX
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

#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

// Report on first fix, then 60 s while moving, hourly when parked.
// 50 m clears GPS wander and the ~18 m APRS position resolution.
const unsigned int RETRY_SECS               = 30U;
const unsigned int FIX_TIMEOUT_SECS         = 30U;
const unsigned int MOBILE_MIN_INTERVAL_SECS = 60U;
const unsigned int STATIONARY_INTERVAL_SECS = 3600U;
const double       MOVED_DISTANCE_METRES    = 50.0;

CGPSD::CGPSD(const std::string& address, const std::string& port) :
m_gpsdAddress(address),
m_gpsdPort(port),
m_gpsdData(),
m_connected(false),
m_warned(false),
m_retryTimer(1000U, RETRY_SECS),
m_fixTimer(1000U, FIX_TIMEOUT_SECS),
m_idTimer(1000U, MOBILE_MIN_INTERVAL_SECS),
m_refreshTimer(1000U, STATIONARY_INTERVAL_SECS),
m_fixValid(false),
m_latitude(0.0),
m_longitude(0.0),
m_altitudeValid(false),
m_altitude(0.0),
m_reportSent(false),
m_sentLatitude(0.0),
m_sentLongitude(0.0),
m_networks(),
m_aprs(nullptr)
{
	assert(!address.empty());
	assert(!port.empty());
}

CGPSD::~CGPSD()
{
}

void CGPSD::addNetwork(CDMRNetwork* network)
{
	assert(network != nullptr);

	m_networks.push_back(network);
}

void CGPSD::setAPRS(CAPRSWriter* aprs)
{
	assert(aprs != nullptr);

	m_aprs = aprs;

	// gpsd drives the APRS beacon from now on
	m_aprs->setMobile();
}

bool CGPSD::open()
{
	// gpsd may not be up yet; clock() retries
	if (!connect())
		m_retryTimer.start();

	return true;
}

void CGPSD::clock(unsigned int ms)
{
	if (m_connected) {
		read();
	} else {
		m_retryTimer.clock(ms);
		if (m_retryTimer.hasExpired()) {
			if (connect())
				m_retryTimer.stop();
			else
				m_retryTimer.start();
		}
	}

	// Don't report a stale fix
	m_fixTimer.clock(ms);
	if (m_fixTimer.hasExpired()) {
		m_fixValid = false;
		m_fixTimer.stop();
	}

	m_idTimer.clock(ms);
	m_refreshTimer.clock(ms);

	if (m_fixValid && reportDue()) {
		sendReport();

		m_reportSent    = true;
		m_sentLatitude  = m_latitude;
		m_sentLongitude = m_longitude;

		m_idTimer.start();
		m_refreshTimer.start();
	}
}

void CGPSD::close()
{
	if (m_connected) {
		::gps_stream(&m_gpsdData, WATCH_DISABLE, nullptr);
		disconnect();
	}
}

bool CGPSD::connect()
{
	int ret = ::gps_open(m_gpsdAddress.c_str(), m_gpsdPort.c_str(), &m_gpsdData);
	if (ret != 0) {
		// -1: reason in errno (system error, or a libgps code on old libgps)
		int err = (ret == -1) ? errno : ret;
		const char* reason = (err < 0) ? ::gps_errstr(err) : ::strerror(err);

		// Warn once per outage
		if (!m_warned)
			LogWarning("Unable to connect to gpsd at %s:%s - %s, retrying every %u seconds", m_gpsdAddress.c_str(), m_gpsdPort.c_str(), reason, RETRY_SECS);
		m_warned = true;
		return false;
	}

	::gps_stream(&m_gpsdData, WATCH_ENABLE | WATCH_JSON, nullptr);

	m_connected = true;
	m_warned    = false;

	LogMessage("Connected to GPSD");

	return true;
}

void CGPSD::disconnect()
{
	::gps_close(&m_gpsdData);

	m_connected = false;
	m_fixValid  = false;
	m_fixTimer.stop();
}

// Drain gpsd and keep the latest fix: gps_read() returns one message.
void CGPSD::read()
{
	// Bounded; the rest is read next time
	for (unsigned int n = 0U; n < 50U && ::gps_waiting(&m_gpsdData, 0); n++) {
		// Non-TPV messages leave stale flags set
		m_gpsdData.set = 0U;

#if GPSD_API_MAJOR_VERSION >= 7
		int ret = ::gps_read(&m_gpsdData, nullptr, 0);
#else
		int ret = ::gps_read(&m_gpsdData);
#endif
		if (ret < 0) {
			LogWarning("Lost the connection to gpsd, retrying every %u seconds", RETRY_SECS);
			disconnect();
			m_warned = true;
			m_retryTimer.start();
			return;
		}

		if (ret == 0)
			return;

		// TPV reports only
		if ((m_gpsdData.set & MODE_SET) != MODE_SET)
			continue;

		// Any 2D/3D fix (GPS, DGPS, RTK...). gpsd doesn't range-check.
		bool latlonSet = (m_gpsdData.set & LATLON_SET) == LATLON_SET;
		double lat     = m_gpsdData.fix.latitude;
		double lon     = m_gpsdData.fix.longitude;
		bool latlonOK  = std::isfinite(lat) && std::isfinite(lon) &&
		                 ::fabs(lat) <= 90.0 && ::fabs(lon) <= 180.0;
		if (m_gpsdData.fix.mode < MODE_2D || !latlonSet || !latlonOK) {
			m_fixValid = false;
			m_fixTimer.stop();
			continue;
		}

		m_latitude  = lat;
		m_longitude = lon;

#if GPSD_API_MAJOR_VERSION >= 9
		m_altitude = m_gpsdData.fix.altMSL;
#else
		m_altitude = m_gpsdData.fix.altitude;
#endif
		m_altitudeValid = (m_gpsdData.fix.mode >= MODE_3D) && std::isfinite(m_altitude) &&
		                  m_altitude >= -1000.0 && m_altitude <= 100000.0;

		m_fixValid = true;
		m_fixTimer.start();
	}
}

bool CGPSD::reportDue()
{
	// First fix
	if (!m_reportSent)
		return true;

	// Parked: hourly refresh
	if (m_refreshTimer.hasExpired())
		return true;

	// Moving: at most once a minute
	if (!m_idTimer.hasExpired())
		return false;

	// Distance from the last position sent (equirectangular)
	const double DEG_TO_RAD   = 0.017453292519943295;
	const double EARTH_RADIUS = 6371000.0;
	const double PI           = 3.141592653589793;

	double dLon = (m_longitude - m_sentLongitude) * DEG_TO_RAD;
	if (dLon > PI)
		dLon -= 2.0 * PI;
	else if (dLon < -PI)
		dLon += 2.0 * PI;

	double dLat   = (m_latitude - m_sentLatitude) * DEG_TO_RAD;
	double midLat = (m_latitude + m_sentLatitude) * 0.5 * DEG_TO_RAD;

	double x = dLon * ::cos(midLat);
	double distance = ::sqrt(x * x + dLat * dLat) * EARTH_RADIUS;

	return distance > MOVED_DISTANCE_METRES;
}

void CGPSD::sendReport()
{
	float latitude  = float(m_latitude);
	float longitude = float(m_longitude);

	if (m_aprs != nullptr)
		m_aprs->sendLocation(latitude, longitude, m_altitudeValid ? int(::lround(m_altitude)) : 0);

	for (std::vector<CDMRNetwork*>::const_iterator it = m_networks.begin(); it != m_networks.end(); ++it)
		(*it)->writeHomePosition(latitude, longitude);
}

#endif
