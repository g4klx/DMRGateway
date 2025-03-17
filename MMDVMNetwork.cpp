/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2025 by Jonathan Naylor G4KLX
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

#include "MMDVMNetwork.h"

#include "StopWatch.h"
#include "Utils.h"
#include "Log.h"

#include <cstdio>
#include <cassert>

const unsigned int BUFFER_LENGTH = 500U;

const unsigned int HOMEBREW_DATA_PACKET_LENGTH = 55U;


CMMDVMNetwork::CMMDVMNetwork(const std::string& rptAddress, unsigned short rptPort, const std::string& localAddress, unsigned short localPort, bool debug) :
m_rptAddr(),
m_rptAddrLen(0U),
m_id(0U),
m_netId(nullptr),
m_debug(debug),
m_socket(localAddress, localPort),
m_buffer(nullptr),
m_rxData(1000U, "MMDVM Network"),
m_configData(nullptr),
m_configLen(0U),
m_radioPositionData(nullptr),
m_radioPositionLen(0U),
m_talkerAliasData(nullptr),
m_talkerAliasLen(0U)
{
	assert(!rptAddress.empty());
	assert(rptPort > 0U);

	if (CUDPSocket::lookup(rptAddress, rptPort, m_rptAddr, m_rptAddrLen) != 0)
		m_rptAddrLen = 0U;

	m_buffer = new unsigned char[BUFFER_LENGTH];
	m_netId  = new unsigned char[4U];

	m_radioPositionData = new unsigned char[50U];
	m_talkerAliasData   = new unsigned char[50U];

	CStopWatch stopWatch;
	::srand((unsigned int)stopWatch.start());
}

CMMDVMNetwork::~CMMDVMNetwork()
{
	delete[] m_netId;
	delete[] m_buffer;
	delete[] m_configData;
	delete[] m_radioPositionData;
	delete[] m_talkerAliasData;
}

unsigned int CMMDVMNetwork::getShortConfig(unsigned char* config) const
{
	if (m_configData == 0U)
		return 0U;

	::memcpy(config, m_configData, m_configLen);

	return m_configLen;
}

unsigned int CMMDVMNetwork::getId() const
{
	return m_id;
}

bool CMMDVMNetwork::open()
{
	if (m_rptAddrLen == 0U) {
		LogError("Could not lookup the address of the MMDVM Host");
		return false;
	}

	LogMessage("MMDVM Network, Opening");

	return m_socket.open(m_rptAddr);
}

bool CMMDVMNetwork::read(CDMRData& data)
{
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

	FLCO flco = (m_buffer[15U] & 0x40U) == 0x40U ? FLCO::USER_USER : FLCO::GROUP;

	unsigned int streamId;
	::memcpy(&streamId, m_buffer + 16U, 4U);

	unsigned char ber = m_buffer[53U];

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

bool CMMDVMNetwork::write(const CDMRData& data)
{
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

	::memcpy(buffer + 11U, m_netId, 4U);

	unsigned int slotNo = data.getSlotNo();

	buffer[15U] = slotNo == 1U ? 0x00U : 0x80U;

	FLCO flco = data.getFLCO();
	buffer[15U] |= flco == FLCO::GROUP ? 0x00U : 0x40U;

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

	if (m_debug)
		CUtils::dump(1U, "Network Transmitted", buffer, HOMEBREW_DATA_PACKET_LENGTH);

	m_socket.write(buffer, HOMEBREW_DATA_PACKET_LENGTH, m_rptAddr, m_rptAddrLen);

	return true;
}

bool CMMDVMNetwork::readRadioPosition(unsigned char* data, unsigned int& length)
{
	if (m_radioPositionLen == 0U)
		return false;

	::memcpy(data, m_radioPositionData, m_radioPositionLen);
	length = m_radioPositionLen;

	m_radioPositionLen = 0U;

	return true;
}

bool CMMDVMNetwork::readTalkerAlias(unsigned char* data, unsigned int& length)
{
	if (m_talkerAliasLen == 0U)
		return false;

	::memcpy(data, m_talkerAliasData, m_talkerAliasLen);
	length = m_talkerAliasLen;

	m_talkerAliasLen = 0U;

	return true;
}

bool CMMDVMNetwork::writeBeacon()
{
	return m_socket.write((unsigned char*)"DMRB", 4U, m_rptAddr, m_rptAddrLen);
}

void CMMDVMNetwork::close()
{
	LogMessage("MMDVM Network, Closing");

	m_socket.close();
}

void CMMDVMNetwork::clock(unsigned int ms)
{
	sockaddr_storage address;
	unsigned int addrlen;
	int length = m_socket.read(m_buffer, BUFFER_LENGTH, address, addrlen);
	if (length <= 0)
		return;

	if (!CUDPSocket::match(m_rptAddr, address)) {
		LogMessage("MMDVM packet received from an invalid source");
		return;
	}

	if (m_debug)
		CUtils::dump(1U, "Network Received", m_buffer, length);

	if (::memcmp(m_buffer, "DMRD", 4U) == 0) {
		unsigned char len = length;
		m_rxData.addData(&len, 1U);
		m_rxData.addData(m_buffer, len);
	} else if (::memcmp(m_buffer, "DMRG", 4U) == 0) {
		::memcpy(m_radioPositionData, m_buffer, length);
		m_radioPositionLen = length;
	} else if (::memcmp(m_buffer, "DMRA", 4U) == 0) {
		::memcpy(m_talkerAliasData, m_buffer, length);
		m_talkerAliasLen = length;
	} else if (::memcmp(m_buffer, "DMRC", 4U) == 0) {
		m_id = (m_buffer[4U] << 24) | (m_buffer[5U] << 16) | (m_buffer[6U] << 8) | (m_buffer[7U] << 0);

		if (m_configData == nullptr) {
			m_configLen = length - 8U;
			m_configData = new unsigned char[m_configLen];
			::memcpy(m_configData, m_buffer + 8U, m_configLen);
		}

		m_socket.write((unsigned char*)"DMRP", 4U, m_rptAddr, m_rptAddrLen);
	} else {
		CUtils::dump("Unknown packet from the MMDVM", m_buffer, length);
	}
}
