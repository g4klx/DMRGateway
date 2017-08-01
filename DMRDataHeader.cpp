/*
 *   Copyright (C) 2012 by Ian Wraith
 *   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
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

#include "DMRDataHeader.h"
#include "DMRDefines.h"
#include "BPTC19696.h"
#include "RS129.h"
#include "Utils.h"
#include "CRC.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

CDMRDataHeader::CDMRDataHeader() :
m_data(NULL)
{
	m_data = new unsigned char[12U];
}

CDMRDataHeader::~CDMRDataHeader()
{
	delete[] m_data;
}

bool CDMRDataHeader::put(const unsigned char* bytes)
{
	assert(bytes != NULL);

	CBPTC19696 bptc;
	bptc.decode(bytes, m_data);

	m_data[10U] ^= DATA_HEADER_CRC_MASK[0U];
	m_data[11U] ^= DATA_HEADER_CRC_MASK[1U];

	bool valid = CCRC::checkCCITT162(m_data, 12U);
	if (!valid)
		return false;

	unsigned char dpf = m_data[0U] & 0x0FU;
	if (dpf == DPF_PROPRIETARY)
		return false;

	return true;
}

void CDMRDataHeader::get(unsigned char* bytes) const
{
	assert(bytes != NULL);

	CCRC::addCCITT162(m_data, 12U);
	
	m_data[10U] ^= DATA_HEADER_CRC_MASK[0U];
	m_data[11U] ^= DATA_HEADER_CRC_MASK[1U];

	CBPTC19696 bptc;
	bptc.encode(m_data, bytes);
}

bool CDMRDataHeader::getGI() const
{
	return (m_data[0U] & 0x80U) == 0x80U;
}

unsigned int CDMRDataHeader::getSrcId() const
{
	return m_data[5U] << 16 | m_data[6U] << 8 | m_data[7U];
}

unsigned int CDMRDataHeader::getDstId() const
{
	return m_data[2U] << 16 | m_data[3U] << 8 | m_data[4U];
}

void CDMRDataHeader::setGI(bool group)
{
	if (group)
		m_data[0U] |= 0x80U;
	else
		m_data[0U] &= ~0x80U;
}

void CDMRDataHeader::setSrcId(unsigned int id)
{
	m_data[5U] = id >> 16;
	m_data[6U] = id >> 8;
	m_data[7U] = id >> 0;
}

void CDMRDataHeader::setDstId(unsigned int id)
{
	m_data[2U] = id >> 16;
	m_data[3U] = id >> 8;
	m_data[4U] = id >> 0;
}
