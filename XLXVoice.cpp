/*
*   Copyright (C) 2017,2020,2025 by Jonathan Naylor G4KLX
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

#include "DMRSlotType.h"
#include "DMRFullLC.h"
#include "XLXVoice.h"
#include "DMREMB.h"
#include "Sync.h"
#include "Log.h"

#include <cstring>
#include <cstdlib>

#include <sys/stat.h>

const unsigned char SILENCE[] = {0xACU, 0xAAU, 0x40U, 0x20U, 0x00U, 0x44U, 0x40U, 0x80U, 0x80U};

const unsigned char COLOR_CODE = 3U;

const unsigned int SILENCE_LENGTH = 9U;
const unsigned int AMBE_LENGTH = 9U;

CXLXVoice::CXLXVoice(const std::string& directory, const std::string& language, unsigned int id, unsigned int slot, unsigned int tg) :
m_indxFile(),
m_ambeFile(),
m_slot(slot),
m_lc(FLCO::GROUP, id, tg),
m_embeddedLC(),
m_status(XLXVOICE_STATUS::NONE),
m_timer(1000U, 1U),
m_stopWatch(),
m_seqNo(0U),
m_streamId(0U),
m_sent(0U),
m_ambe(nullptr),
m_positions(),
m_data(),
m_it()
{
	m_embeddedLC.setLC(m_lc);

#if defined(_WIN32) || defined(_WIN64)
	m_indxFile = directory + "\\" + language + ".indx";
	m_ambeFile = directory + "\\" + language + ".ambe";
#else
	m_indxFile = directory + "/" + language + ".indx";
	m_ambeFile = directory + "/" + language + ".ambe";
#endif
}

CXLXVoice::~CXLXVoice()
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	for (std::unordered_map<std::string, CXLXPositions*>::iterator it = m_positions.begin(); it != m_positions.end(); ++it)
		delete it->second;

	m_data.clear();
	m_positions.clear();

	delete[] m_ambe;
}

bool CXLXVoice::open()
{
	FILE* fpindx = ::fopen(m_indxFile.c_str(), "rt");
	if (fpindx == nullptr) {
		LogError("Unable to open the index file - %s", m_indxFile.c_str());
		return false;
	}

	struct stat statStruct;
	int ret = ::stat(m_ambeFile.c_str(), &statStruct);
	if (ret != 0) {
		LogError("Unable to stat the AMBE file - %s", m_ambeFile.c_str());
		::fclose(fpindx);
		return false;
	}

	FILE* fpambe = ::fopen(m_ambeFile.c_str(), "rb");
	if (fpambe == nullptr) {
		LogError("Unable to open the AMBE file - %s", m_ambeFile.c_str());
		::fclose(fpindx);
		return false;
	}

	m_ambe = new unsigned char[statStruct.st_size];

	size_t sizeRead = ::fread(m_ambe, 1U, statStruct.st_size, fpambe);
	if (sizeRead != 0U) {
		char buffer[80U];
		while (::fgets(buffer, 80, fpindx) != nullptr) {
			char* p1 = ::strtok(buffer, "\t\r\n");
			char* p2 = ::strtok(nullptr, "\t\r\n");
			char* p3 = ::strtok(nullptr, "\t\r\n");

			if (p1 != nullptr && p2 != nullptr && p3 != nullptr) {
				std::string symbol  = std::string(p1);
				unsigned int start  = ::atoi(p2) * AMBE_LENGTH;
				unsigned int length = ::atoi(p3) * AMBE_LENGTH;

				CXLXPositions* pos = new CXLXPositions;
				pos->m_start = start;
				pos->m_length = length;

				m_positions[symbol] = pos;
			}
		}
	}

	::fclose(fpindx);
	::fclose(fpambe);

	return true;
}

void CXLXVoice::linkedTo(const std::string &number, unsigned int room)
{
	std::vector<std::string> words;
	if (m_positions.count("linkedto") == 0U) {
		words.push_back("linked");
		words.push_back("2");
	} else {
		words.push_back("linkedto");
	}
	words.push_back("X");
	words.push_back("L");
	words.push_back("X");
	words.push_back(number.substr(0U, 1U));
	words.push_back(number.substr(1U, 1U));
	words.push_back(number.substr(2U, 1U));

	// 4001 => 1 => A, 4002 => 2 => B, etc.
	room %= 100U;

	if (room >= 1U && room <= 26U)
		words.push_back(std::string(1U, 'A' + room - 1U));

	createVoice(words);
}

void CXLXVoice::unlinked()
{
	std::vector<std::string> words;
	words.push_back("notlinked");

	createVoice(words);
}

void CXLXVoice::createVoice(const std::vector<std::string>& words)
{
	unsigned int ambeLength = 0U;
	for (std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it) {
		if (m_positions.count(*it) > 0U) {
			CXLXPositions* position = m_positions.at(*it);
			ambeLength += position->m_length;
		} else {
			LogWarning("Unable to find character/phrase \"%s\" in the index", (*it).c_str());
		}
	}

	// Ensure that the AMBE is an integer number of DMR frames
	if ((ambeLength % (3U * AMBE_LENGTH)) != 0U) {
		unsigned int frames = ambeLength / (3U * AMBE_LENGTH);
		frames++;
		ambeLength = frames * (3U * AMBE_LENGTH);
	}

	// Add space for silence before and after the voice
	ambeLength += SILENCE_LENGTH * AMBE_LENGTH;
	ambeLength += SILENCE_LENGTH * AMBE_LENGTH;

	unsigned char* ambeData = new unsigned char[ambeLength];

	// Fill the AMBE data with silence
	for (unsigned int i = 0U; i < ambeLength; i += AMBE_LENGTH)
		::memcpy(ambeData + i, SILENCE, AMBE_LENGTH);

	// Put offset in for silence at the beginning
	unsigned int pos = SILENCE_LENGTH * AMBE_LENGTH;
	for (std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it) {
		if (m_positions.count(*it) > 0U) {
			CXLXPositions* position = m_positions.at(*it);
			unsigned int start = position->m_start;
			unsigned int length = position->m_length;
			::memcpy(ambeData + pos, m_ambe + start, length);
			pos += length;
		}
	}
		
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_data.clear();

	m_streamId = ::rand() + 1U;
	m_seqNo = 0U;

	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];

	unsigned int n = 0U;
	for (unsigned int i = 0U; i < ambeLength; i += (3U * AMBE_LENGTH)) {
		unsigned char* p = ambeData + i;

		CDMRData* data = new CDMRData;

		data->setSlotNo(m_slot);
		data->setFLCO(FLCO::GROUP);
		data->setSrcId(m_lc.getSrcId());
		data->setDstId(m_lc.getDstId());
		data->setN(n);
		data->setSeqNo(m_seqNo++);
		data->setStreamId(m_streamId);

		::memcpy(buffer + 0U, p + 0U, AMBE_LENGTH);
		::memcpy(buffer + 9U, p + 9U, AMBE_LENGTH);
		::memcpy(buffer + 15U, p + 9U, AMBE_LENGTH);
		::memcpy(buffer + 24U, p + 18U, AMBE_LENGTH);

		if (n == 0U) {
			CSync::addDMRAudioSync(buffer, true);
			data->setDataType(DT_VOICE_SYNC);
		} else {
			unsigned char lcss = m_embeddedLC.getData(buffer, n);

			CDMREMB emb;
			emb.setColorCode(COLOR_CODE);
			emb.setPI(false);
			emb.setLCSS(lcss);
			emb.getData(buffer);

			data->setDataType(DT_VOICE);
		}

		n++;
		if (n >= 6U)
			n = 0U;

		data->setData(buffer);

		m_data.push_back(data);
	}

	createHeaderTerminator(DT_TERMINATOR_WITH_LC);
	createHeaderTerminator(DT_TERMINATOR_WITH_LC);

	delete[] ambeData;

	m_status = XLXVOICE_STATUS::WAITING;
	m_timer.start();
}

void CXLXVoice::reset()
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_timer.stop();
	m_status = XLXVOICE_STATUS::NONE;
	m_data.clear();
	m_seqNo = 0U;
	m_streamId = 0U;
	m_sent = 0U;
}

bool CXLXVoice::read(CDMRData& data)
{
	if (m_status != XLXVOICE_STATUS::SENDING)
		return false;

	unsigned int count = m_stopWatch.elapsed() / DMR_SLOT_TIME;

	if (m_sent < count) {
		data = *(*m_it);

		++m_sent;
		++m_it;

		if (m_it == m_data.end()) {
			for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
				delete *it;
			m_data.clear();
			m_timer.stop();
			m_status = XLXVOICE_STATUS::NONE;
		}

		return true;
	}

	return false;
}

void CXLXVoice::clock(unsigned int ms)
{
	m_timer.clock(ms);
	if (m_timer.isRunning() && m_timer.hasExpired()) {
		if (m_status == XLXVOICE_STATUS::WAITING) {
			m_stopWatch.start();
			m_status = XLXVOICE_STATUS::SENDING;
			m_it = m_data.begin();
			m_sent = 0U;
		}
	}
}

void CXLXVoice::createHeaderTerminator(unsigned char type)
{
	CDMRData* data = new CDMRData;

	data->setSlotNo(m_slot);
	data->setFLCO(FLCO::GROUP);
	data->setSrcId(m_lc.getSrcId());
	data->setDstId(m_lc.getDstId());
	data->setDataType(type);
	data->setN(0U);
	data->setSeqNo(m_seqNo++);
	data->setStreamId(m_streamId);

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];

	CDMRFullLC fullLC;
	fullLC.encode(m_lc, buffer, type);

	CDMRSlotType slotType;
	slotType.setColorCode(COLOR_CODE);
	slotType.setDataType(type);
	slotType.getData(buffer);

	CSync::addDMRDataSync(buffer, true);

	data->setData(buffer);

	m_data.push_back(data);
}
