/*
*   Copyright (C) 2017 by Jonathan Naylor G4KLX
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
#include "DMREMB.h"
#include "Voice.h"
#include "Sync.h"
#include "Log.h"

#include <cstring>
#include <cstdlib>

#include <sys/stat.h>

const unsigned char SILENCE[] = {0xACU, 0xAAU, 0x40U, 0x20U, 0x00U, 0x44U, 0x40U, 0x80U, 0x80U};

const unsigned int POSITION_0 = 0U;
const unsigned int POSITION_1 = 1U;
const unsigned int POSITION_2 = 2U;
const unsigned int POSITION_3 = 3U;
const unsigned int POSITION_4 = 4U;
const unsigned int POSITION_5 = 5U;
const unsigned int POSITION_6 = 6U;
const unsigned int POSITION_7 = 7U;
const unsigned int POSITION_8 = 8U;
const unsigned int POSITION_9 = 9U;
const unsigned int POSITION_CONNECTED    = 10U;
const unsigned int POSITION_DISCONNECTED = 11U;

const unsigned char COLOR_CODE = 3U;

CVoice::CVoice(const std::string& directory, const std::string& language, unsigned int id, unsigned int slot, unsigned int tg) :
m_indxFile(),
m_ambeFile(),
m_slot(slot),
m_lc(FLCO_GROUP, id, tg),
m_embeddedLC(),
m_status(VS_NONE),
m_timer(1000U, 1U),
m_stopWatch(),
m_seqNo(0U),
m_streamId(0U),
m_sent(0U),
m_ambe(NULL),
m_positions(NULL),
m_data(),
m_it()
{
	m_embeddedLC.setLC(m_lc);

	m_positions = new CPositions[12U];
	::memset(m_positions, 0x00U, 12U * sizeof(CPositions));

#if defined(_WIN32) || defined(_WIN64)
	m_indxFile = directory + "\\" + language + ".indx";
	m_ambeFile = directory + "\\" + language + ".ambe";
#else
	m_indxFile = directory + "/" + language + ".indx";
	m_ambeFile = directory + "/" + language + ".ambe";
#endif
}

CVoice::~CVoice()
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_data.clear();

	delete[] m_ambe;
	delete[] m_positions;
}

bool CVoice::open()
{
	FILE* fpindx = ::fopen(m_indxFile.c_str(), "rt");
	if (fpindx == NULL) {
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
	if (fpambe == NULL) {
		LogError("Unable to open the AMBE file - %s", m_ambeFile.c_str());
		::fclose(fpindx);
		return false;
	}

	m_ambe = new unsigned char[statStruct.st_size];

	size_t sizeRead = ::fread(m_ambe, 1U, statStruct.st_size, fpambe);
	if (sizeRead != 0U) {
		char buffer[80U];
		while (::fgets(buffer, 80, fpindx) != NULL) {
			char* p1 = ::strtok(buffer, "\t\r\n");
			char* p2 = ::strtok(NULL, "\t\r\n");
			char* p3 = ::strtok(NULL, "\t\r\n");

			if (p1 != NULL && p2 != NULL && p3 != NULL) {
				unsigned int start = ::atoi(p2) * 9U;
				unsigned int length = ::atoi(p3) * 9U;

				if (::strcmp(p1, "0") == 0) {
					m_positions[POSITION_0].m_start = start;
					m_positions[POSITION_0].m_length = length;
				} else if (::strcmp(p1, "1") == 0) {
					m_positions[POSITION_1].m_start = start;
					m_positions[POSITION_1].m_length = length;
				} else if (::strcmp(p1, "2") == 0) {
					m_positions[POSITION_2].m_start = start;
					m_positions[POSITION_2].m_length = length;
				} else if (::strcmp(p1, "3") == 0) {
					m_positions[POSITION_3].m_start = start;
					m_positions[POSITION_3].m_length = length;
				} else if (::strcmp(p1, "4") == 0) {
					m_positions[POSITION_4].m_start = start;
					m_positions[POSITION_4].m_length = length;
				} else if (::strcmp(p1, "5") == 0) {
					m_positions[POSITION_5].m_start = start;
					m_positions[POSITION_5].m_length = length;
				} else if (::strcmp(p1, "6") == 0) {
					 m_positions[POSITION_6].m_start = start;
					 m_positions[POSITION_6].m_length = length;
				} else if (::strcmp(p1, "7") == 0) {
					 m_positions[POSITION_7].m_start = start;
					 m_positions[POSITION_7].m_length = length;
				} else if (::strcmp(p1, "8") == 0) {
					 m_positions[POSITION_8].m_start = start;
					 m_positions[POSITION_8].m_length = length;
				} else if (::strcmp(p1, "9") == 0) {
					 m_positions[POSITION_9].m_start = start;
					 m_positions[POSITION_9].m_length = length;
				} else if (::strcmp(p1, "connected") == 0) {
					 m_positions[POSITION_CONNECTED].m_start = start;
					 m_positions[POSITION_CONNECTED].m_length = length;
				} else if (::strcmp(p1, "disconnected") == 0) {
					 m_positions[POSITION_DISCONNECTED].m_start = start;
					 m_positions[POSITION_DISCONNECTED].m_length = length;
				}
			}
		}
	}

	::fclose(fpindx);
	::fclose(fpambe);

	return true;
}

void CVoice::linkedTo(unsigned int number, unsigned int room)
{
	char letters[10U];
	::sprintf(letters, "%03u%02u", number, room % 100U);

	std::vector<unsigned int> words;
	words.push_back(POSITION_CONNECTED);
	words.push_back(letters[0U] - '0');
	words.push_back(letters[1U] - '0');
	words.push_back(letters[2U] - '0');
	words.push_back(letters[3U] - '0');
	words.push_back(letters[4U] - '0');

	createVoice(words);
}

void CVoice::unlinked()
{
	std::vector<unsigned int> words;
	words.push_back(POSITION_DISCONNECTED);

	createVoice(words);
}

void CVoice::createVoice(const std::vector<unsigned int>& words)
{
	unsigned int ambeLength = 0U;
	for (std::vector<unsigned int>::const_iterator it = words.begin(); it != words.end(); ++it)
		ambeLength += m_positions[*it].m_length;

	// Ensure that the AMBE is an integer number of DMR frames
	if ((ambeLength % 27U) != 0U) {
		unsigned int frames = ambeLength / 27U;
		frames++;
		ambeLength = frames * 27U;
	}

	unsigned char* ambeData = new unsigned char[ambeLength];

	// Fill the AMBE data with silence
	for (unsigned int i = 0U; i < ambeLength; i += 9U)
		::memcpy(ambeData + i, SILENCE, 9U);

	unsigned int pos = 0U;
	for (std::vector<unsigned int>::const_iterator it = words.begin(); it != words.end(); ++it) {
		unsigned int start  = m_positions[*it].m_start;
		unsigned int length = m_positions[*it].m_length;
		::memcpy(ambeData + pos, m_ambe + start, length);
		pos += length;
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
	for (unsigned int i = 0U; i < ambeLength; i += 27U) {
		unsigned char* p = ambeData + i;

		CDMRData* data = new CDMRData;

		data->setSlotNo(m_slot);
		data->setFLCO(FLCO_GROUP);
		data->setSrcId(m_lc.getSrcId());
		data->setDstId(m_lc.getDstId());
		data->setN(n);
		data->setSeqNo(m_seqNo++);
		data->setStreamId(m_streamId);

		::memcpy(buffer + 0U, p + 0U, 9U);
		::memcpy(buffer + 9U, p + 9U, 9U);
		::memcpy(buffer + 15U, p + 9U, 9U);
		::memcpy(buffer + 24U, p + 18U, 9U);

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

	m_status = VS_WAITING;
	m_timer.start();
}

bool CVoice::read(CDMRData& data)
{
	if (m_status != VS_SENDING)
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
			m_status = VS_NONE;
		}

		return true;
	}

	return false;
}

void CVoice::clock(unsigned int ms)
{
	m_timer.clock(ms);
	if (m_timer.isRunning() && m_timer.hasExpired()) {
		if (m_status == VS_WAITING) {
			m_stopWatch.start();
			m_status = VS_SENDING;
			m_it = m_data.begin();
			m_sent = 0U;
		}
	}
}

void CVoice::createHeaderTerminator(unsigned char type)
{
	CDMRData* data = new CDMRData;

	data->setSlotNo(m_slot);
	data->setFLCO(FLCO_GROUP);
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
