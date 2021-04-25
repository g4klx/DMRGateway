/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021 by Jonathan Naylor G4KLX
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

#include "DMRNetwork.h"

#include "StopWatch.h"
#include "SHA256.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

const unsigned int BUFFER_LENGTH = 500U;

const unsigned int HOMEBREW_DATA_PACKET_LENGTH = 55U;


CDMRNetwork::CDMRNetwork(const std::string& address, unsigned short port, unsigned short local, unsigned int id, const std::string& password, const std::string& name, bool location, bool debug) :
m_addr(),
m_addrLen(0U),
m_id(NULL),
m_password(password),
m_name(name),
m_location(location),
m_debug(debug),
m_socket(local),
m_enabled(false),
m_status(WAITING_CONNECT),
m_retryTimer(1000U, 10U),
m_timeoutTimer(1000U, 60U),
m_buffer(NULL),
m_salt(NULL),
m_rxData(1000U, "DMR Network"),
m_options(),
m_configData(NULL),
m_configLen(0U),
m_beacon(false)
{
	assert(!address.empty());
	assert(port > 0U);
	assert(id > 1000U);
	assert(!password.empty());

	if (CUDPSocket::lookup(address, port, m_addr, m_addrLen) != 0)
		m_addrLen = 0U;

	m_buffer   = new unsigned char[BUFFER_LENGTH];
	m_salt     = new unsigned char[sizeof(uint32_t)];
	m_id       = new uint8_t[4U];

	m_id[0U] = id >> 24;
	m_id[1U] = id >> 16;
	m_id[2U] = id >> 8;
	m_id[3U] = id >> 0;

	CStopWatch stopWatch;
	::srand(stopWatch.start());
}

CDMRNetwork::~CDMRNetwork()
{
	delete[] m_buffer;
	delete[] m_salt;
	delete[] m_id;
}

void CDMRNetwork::setOptions(const std::string& options)
{
	m_options = options;
}

void CDMRNetwork::setConfig(const unsigned char* data, unsigned int len)
{
	m_configData = new unsigned char[len];
	::memcpy(m_configData, data, len);

	m_configLen = len;
}

bool CDMRNetwork::open()
{
	if (m_addrLen == 0U) {
		LogError("%s, Could not lookup the address of the master", m_name.c_str());
		return false;
	}

	LogMessage("%s, Opening DMR Network", m_name.c_str());

	m_status = WAITING_CONNECT;
	m_timeoutTimer.stop();
	m_retryTimer.start();

	return true;
}

void CDMRNetwork::enable(bool enabled)
{
        if (!enabled && m_enabled)
                m_rxData.clear();

        m_enabled = enabled;
}

bool CDMRNetwork::read(CDMRData& data)
{
	if (m_status != RUNNING)
		return false;

	if (m_rxData.isEmpty())
		return false;

	unsigned char length = 0U;

	m_rxData.getData(&length, 1U);
	m_rxData.getData(m_buffer, length);

	// Is this a data packet?
	if (::memcmp(m_buffer, "DMRD", 4U) != 0)
		return false;

	unsigned char seqNo = m_buffer[4U];

	unsigned int srcId = (m_buffer[5U] << 16) | (m_buffer[6U] << 8) | (m_buffer[7U] << 0);

	unsigned int dstId = (m_buffer[8U] << 16) | (m_buffer[9U] << 8) | (m_buffer[10U] << 0);

	unsigned int slotNo = (m_buffer[15U] & 0x80U) == 0x80U ? 2U : 1U;

	FLCO flco = (m_buffer[15U] & 0x40U) == 0x40U ? FLCO_USER_USER : FLCO_GROUP;

	unsigned int streamId;
	::memcpy(&streamId, m_buffer + 16U, 4U);

	unsigned char ber  = m_buffer[53U];

	unsigned char rssi = m_buffer[54U];

	data.setSeqNo(seqNo);
	data.setSlotNo(slotNo);
	data.setSrcId(srcId);
	data.setDstId(dstId);
	data.setFLCO(flco);
	data.setStreamId(streamId);
	data.setBER(ber);
	data.setRSSI(rssi);

	bool dataSync = (m_buffer[15U] & 0x20U) == 0x20U;
	bool voiceSync = (m_buffer[15U] & 0x10U) == 0x10U;

	if (dataSync) {
		unsigned char dataType = m_buffer[15U] & 0x0FU;
		data.setData(m_buffer + 20U);
		data.setDataType(dataType);
		data.setN(0U);
	} else if (voiceSync) {
		data.setData(m_buffer + 20U);
		data.setDataType(DT_VOICE_SYNC);
		data.setN(0U);
	} else {
		unsigned char n = m_buffer[15U] & 0x0FU;
		data.setData(m_buffer + 20U);
		data.setDataType(DT_VOICE);
		data.setN(n);
	}

	return true;
}

bool CDMRNetwork::write(const CDMRData& data)
{
	if (m_status != RUNNING)
		return false;

	unsigned char buffer[HOMEBREW_DATA_PACKET_LENGTH];
	::memset(buffer, 0x00U, HOMEBREW_DATA_PACKET_LENGTH);

	buffer[0U]  = 'D';
	buffer[1U]  = 'M';
	buffer[2U]  = 'R';
	buffer[3U]  = 'D';

	unsigned int srcId = data.getSrcId();
	buffer[5U]  = srcId >> 16;
	buffer[6U]  = srcId >> 8;
	buffer[7U]  = srcId >> 0;

	unsigned int dstId = data.getDstId();
	buffer[8U]  = dstId >> 16;
	buffer[9U]  = dstId >> 8;
	buffer[10U] = dstId >> 0;

	::memcpy(buffer + 11U, m_id, 4U);

	unsigned int slotNo = data.getSlotNo();

	buffer[15U] = slotNo == 1U ? 0x00U : 0x80U;

	FLCO flco = data.getFLCO();
	buffer[15U] |= flco == FLCO_GROUP ? 0x00U : 0x40U;

	unsigned char dataType = data.getDataType();
	if (dataType == DT_VOICE_SYNC) {
		buffer[15U] |= 0x10U;
	} else if (dataType == DT_VOICE) {
		buffer[15U] |= data.getN();
	} else {
		buffer[15U] |= (0x20U | dataType);
	}

	buffer[4U] = data.getSeqNo();

	unsigned int streamId = data.getStreamId();
	::memcpy(buffer + 16U, &streamId, 4U);

	data.getData(buffer + 20U);

	buffer[53U] = data.getBER();

	buffer[54U] = data.getRSSI();

	write(buffer, HOMEBREW_DATA_PACKET_LENGTH);

	return true;
}

bool CDMRNetwork::writeRadioPosition(const unsigned char* data, unsigned int length)
{
	if (m_status != RUNNING)
		return false;

	if (!m_location)
		return false;

	unsigned char buffer[50U];

	::memcpy(buffer + 0U, "DMRG", 4U);

	::memcpy(buffer + 4U, m_id, 4U);

	::memcpy(buffer + 8U, data + 4U, length - 4U);

	return write(buffer, length);
}

bool CDMRNetwork::writeTalkerAlias(const unsigned char* data, unsigned int length)
{
	if (m_status != RUNNING)
		return false;

	unsigned char buffer[50U];

	::memcpy(buffer + 0U, "DMRA", 4U);

	::memcpy(buffer + 4U, m_id, 4U);

	::memcpy(buffer + 8U, data + 4U, length - 4U);

	return write(buffer, length);
}

bool CDMRNetwork::writeHomePosition(float latitude, float longitude)
{
	if (m_status != RUNNING)
		return false;

	if (!m_location)
		return false;

	char buffer[50U];

	::memcpy(buffer + 0U, "RPTG", 4U);

	::memcpy(buffer + 4U, m_id, 4U);

	::sprintf(buffer + 8U, "%+08.4f%+09.4f", latitude, longitude);

	return write((unsigned char*)buffer, 25U);
}

bool CDMRNetwork::isConnected() const
{
	return m_status == RUNNING;
}

void CDMRNetwork::close(bool sayGoodbye)
{
	LogMessage("%s, Closing DMR Network", m_name.c_str());

	if (sayGoodbye && (m_status == RUNNING)) {
		unsigned char buffer[9U];
		::memcpy(buffer + 0U, "RPTCL", 5U);
		::memcpy(buffer + 5U, m_id, 4U);
		write(buffer, 9U);
	}

	m_socket.close();

	m_retryTimer.stop();
	m_timeoutTimer.stop();
}

void CDMRNetwork::clock(unsigned int ms)
{
	if (m_status == WAITING_CONNECT) {
		m_retryTimer.clock(ms);
		if (m_retryTimer.isRunning() && m_retryTimer.hasExpired()) {
			bool ret = m_socket.open(m_addr);
			if (ret) {
				ret = writeLogin();
				if (!ret)
					return;

				m_status = WAITING_LOGIN;
				m_timeoutTimer.start();
			}

			m_retryTimer.start();
		}

		return;
	}

	sockaddr_storage address;
	unsigned int addrlen;
	int length = m_socket.read(m_buffer, BUFFER_LENGTH, address, addrlen);
	if (length < 0) {
		LogError("%s, Socket has failed, retrying connection to the master", m_name.c_str());
		close(false);
		open();
		return;
	}

	if (m_debug && length > 0)
		CUtils::dump(1U, "Network Received", m_buffer, length);

	if (length > 0 && CUDPSocket::match(m_addr, address)) {
		if (::memcmp(m_buffer, "DMRD", 4U) == 0) {
			if (m_debug)
				CUtils::dump(1U, "Network Received", m_buffer, length);

			if (m_enabled) {
				unsigned char len = length;
				m_rxData.addData(&len, 1U);
				m_rxData.addData(m_buffer, len);
			}
		} else if (::memcmp(m_buffer, "MSTNAK",  6U) == 0) {
			if (m_status == RUNNING) {
				LogWarning("%s, Login to the master has failed, retrying login ...", m_name.c_str());
				m_status = WAITING_LOGIN;
				m_timeoutTimer.start();
				m_retryTimer.start();
			} else {
				/* Once the modem death spiral has been prevented in Modem.cpp
				   the Network sometimes times out and reaches here.
				   We want it to reconnect so... */
				LogError("%s, Login to the master has failed, retrying network ...", m_name.c_str());
				close(false);
				open();
				return;
			}
		} else if (::memcmp(m_buffer, "RPTACK",  6U) == 0) {
			switch (m_status) {
				case WAITING_LOGIN:
					LogDebug("%s, Sending authorisation", m_name.c_str());
					::memcpy(m_salt, m_buffer + 6U, sizeof(uint32_t));
					writeAuthorisation();
					m_status = WAITING_AUTHORISATION;
					m_timeoutTimer.start();
					m_retryTimer.start();
					break;
				case WAITING_AUTHORISATION:
					LogDebug("%s, Sending configuration", m_name.c_str());
					writeConfig();
					m_status = WAITING_CONFIG;
					m_timeoutTimer.start();
					m_retryTimer.start();
					break;
				case WAITING_CONFIG:
					if (m_options.empty()) {
						LogMessage("%s, Logged into the master successfully", m_name.c_str());
						m_status = RUNNING;
					} else {
						LogDebug("%s, Sending options", m_name.c_str());
						writeOptions();
						m_status = WAITING_OPTIONS;
					}
					m_timeoutTimer.start();
					m_retryTimer.start();
					break;
				case WAITING_OPTIONS:
					LogMessage("%s, Logged into the master successfully", m_name.c_str());
					m_status = RUNNING;
					m_timeoutTimer.start();
					m_retryTimer.start();
					break;
				default:
					break;
			}
		} else if (::memcmp(m_buffer, "MSTCL",   5U) == 0) {
			LogError("%s, Master is closing down", m_name.c_str());
			close(false);
			open();
		} else if (::memcmp(m_buffer, "MSTPONG", 7U) == 0) {
			m_timeoutTimer.start();
		} else if (::memcmp(m_buffer, "RPTSBKN", 7U) == 0) {
			m_beacon = true;
		} else {
			char buffer[100U];
			::sprintf(buffer, "%s, Unknown packet from the master", m_name.c_str());
			CUtils::dump(buffer, m_buffer, length);
		}
	}

	m_retryTimer.clock(ms);
	if (m_retryTimer.isRunning() && m_retryTimer.hasExpired()) {
		switch (m_status) {
			case WAITING_LOGIN:
				writeLogin();
				break;
			case WAITING_AUTHORISATION:
				writeAuthorisation();
				break;
			case WAITING_OPTIONS:
				writeOptions();
				break;
			case WAITING_CONFIG:
				writeConfig();
				break;
			case RUNNING:
				writePing();
				break;
			default:
				break;
		}

		m_retryTimer.start();
	}

	m_timeoutTimer.clock(ms);
	if (m_timeoutTimer.isRunning() && m_timeoutTimer.hasExpired()) {
		LogError("%s, Connection to the master has timed out, retrying connection", m_name.c_str());
		close(false);
		open();
	}
}

bool CDMRNetwork::writeLogin()
{
	unsigned char buffer[8U];

	::memcpy(buffer + 0U, "RPTL", 4U);
	::memcpy(buffer + 4U, m_id, 4U);

	return write(buffer, 8U);
}

bool CDMRNetwork::writeAuthorisation()
{
	size_t size = m_password.size();

	unsigned char* in = new unsigned char[size + sizeof(uint32_t)];
	::memcpy(in, m_salt, sizeof(uint32_t));
	for (size_t i = 0U; i < size; i++)
		in[i + sizeof(uint32_t)] = m_password.at(i);

	unsigned char out[40U];
	::memcpy(out + 0U, "RPTK", 4U);
	::memcpy(out + 4U, m_id, 4U);

	CSHA256 sha256;
	sha256.buffer(in, (unsigned int)(size + sizeof(uint32_t)), out + 8U);

	delete[] in;

	return write(out, 40U);
}

bool CDMRNetwork::writeOptions()
{
	char buffer[300U];

	::memcpy(buffer + 0U, "RPTO", 4U);
	::memcpy(buffer + 4U, m_id, 4U);
	::strcpy(buffer + 8U, m_options.c_str());

	return write((unsigned char*)buffer, (unsigned int)m_options.length() + 8U);
}

bool CDMRNetwork::writeConfig()
{
	char buffer[400U];

	::memcpy(buffer + 0U, "RPTC", 4U);
	::memcpy(buffer + 4U, m_id, 4U);
	::memcpy(buffer + 8U, m_configData, m_configLen);

	if (!m_location)
		::memcpy(buffer + 38U, "0.00000000.000000", 17U);

	return write((unsigned char*)buffer, m_configLen + 8U);
}

bool CDMRNetwork::writePing()
{
	unsigned char buffer[11U];

	::memcpy(buffer + 0U, "RPTPING", 7U);
	::memcpy(buffer + 7U, m_id, 4U);

	return write(buffer, 11U);
}

bool CDMRNetwork::wantsBeacon()
{
	bool beacon = m_beacon;

	m_beacon = false;

	return beacon;
}

bool CDMRNetwork::write(const unsigned char* data, unsigned int length)
{
	assert(data != NULL);
	assert(length > 0U);

	if (m_debug)
		CUtils::dump(1U, "Network Transmitted", data, length);

	bool ret = m_socket.write(data, length, m_addr, m_addrLen);
	if (!ret) {
		LogError("%s, Socket has failed when writing data to the master, retrying connection", m_name.c_str());
		close(false);
		open();
		return false;
	}

	return true;
}
