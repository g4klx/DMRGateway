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

#include <sys/stat.h>

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
m_data(),
m_it(),
m_start(),
m_length()
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

CVoice::~CVoice()
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_data.clear();

	delete[] m_ambe;
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

	::fread(m_ambe, 1U, statStruct.st_size, fpambe);

	char buffer[80U];
	while (::fgets(buffer, 80, fpindx) != NULL) {
		char* p1 = ::strtok(buffer, "\t\r\n");
		char* p2 = ::strtok(NULL, "\t\r\n");
		char* p3 = ::strtok(NULL, "\t\r\n");

		if (p1 != NULL && p2 != NULL && p3 != NULL) {
			unsigned int start = ::atoi(p2) * 9U;
			unsigned int length = ::atoi(p3) * 9U;

			m_start[p1]  = start;
			m_length[p1] = length;
		}
	}

	::fclose(fpindx);
	::fclose(fpambe);

	return true;
}

void CVoice::linkedTo(unsigned int id)
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_data.clear();

	m_streamId = ::rand() + 1U;
	m_seqNo = 0U;

	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);

	unsigned int length = 0U;
	unsigned char* ambe = new unsigned char[10000U];


	delete[] ambe;

	createHeaderTerminator(DT_TERMINATOR_WITH_LC);

	m_status = VS_WAITING;
	m_timer.start();
}

void CVoice::unlinked()
{
	for (std::vector<CDMRData*>::iterator it = m_data.begin(); it != m_data.end(); ++it)
		delete *it;

	m_data.clear();

	m_streamId = ::rand() + 1U;
	m_seqNo = 0U;

	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);
	createHeaderTerminator(DT_VOICE_LC_HEADER);

	unsigned int start = m_start["disconnected"];
	unsigned int length = m_length["disconnected"] / 9U;

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];

	unsigned char* p = m_ambe + start;
	for (unsigned int i = 0U; i < length; i++, p += 27U) {
		CDMRData* data = new CDMRData;

		data->setSlotNo(m_slot);
		data->setFLCO(FLCO_GROUP);
		data->setSrcId(m_lc.getSrcId());
		data->setDstId(m_lc.getDstId());
		data->setN();
		data->setSeqNo(m_seqNo++);
		data->setStreamId(m_streamId);

		::memcpy(buffer + 0U, p + 0U, 9U);
		::memcpy(buffer + 9U, p + 9U, 9U);
		::memcpy(buffer + 15U, p + 9U, 9U);
		::memcpy(buffer + 24U, p + 18U, 9U);

		if (n == 0U) {
			CSync::addDMRAudioSync(buffer);
			data->setDataType(DT_VOICE_SYNC);
		} else {
			CDMREMB emb;
			emb.setColorCode(COLOR_CODE);
			emb.setPI(false);
			emb.setLCSS();
			emb.getData(buffer);

			m_embeddedLC.getData(buffer, n);

			data->setDataType(DT_VOICE);
		}

		data->setData(buffer);

		m_data.push_back(data);
	}

	createHeaderTerminator(DT_TERMINATOR_WITH_LC);

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
