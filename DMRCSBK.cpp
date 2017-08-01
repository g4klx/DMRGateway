/*
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

#include "DMRCSBK.h"
#include "BPTC19696.h"
#include "Utils.h"
#include "CRC.h"

#include <cstdio>
#include <cassert>

CDMRCSBK::CDMRCSBK() :
m_data(NULL),
m_CSBKO(CSBKO_NONE)
{
	m_data = new unsigned char[12U];
}

CDMRCSBK::~CDMRCSBK()
{
	delete[] m_data;
}

bool CDMRCSBK::put(const unsigned char* bytes)
{
	assert(bytes != NULL);

	CBPTC19696 bptc;
	bptc.decode(bytes, m_data);

	m_data[10U] ^= CSBK_CRC_MASK[0U];
	m_data[11U] ^= CSBK_CRC_MASK[1U];

	bool valid = CCRC::checkCCITT162(m_data, 12U);
	if (!valid)
		return false;

	m_CSBKO = CSBKO(m_data[0U] & 0x3FU);

	return true;
}

void CDMRCSBK::get(unsigned char* bytes) const
{
	assert(bytes != NULL);

	CCRC::addCCITT162(m_data, 12U);
	
	m_data[10U] ^= CSBK_CRC_MASK[0U];
	m_data[11U] ^= CSBK_CRC_MASK[1U];

	CBPTC19696 bptc;
	bptc.encode(m_data, bytes);
}

CSBKO CDMRCSBK::getCSBKO() const
{
	return m_CSBKO;
}

bool CDMRCSBK::getGI() const
{
	if (m_CSBKO == CSBKO_PRECCSBK)
		return (m_data[2U] & 0x40U) == 0x40U;
	else
		return false;
}

unsigned int CDMRCSBK::getSrcId() const
{
	if (m_CSBKO == CSBKO_NACKRSP)
		return m_data[4U] << 16 | m_data[5U] << 8 | m_data[6U];		
	else
		return m_data[7U] << 16 | m_data[8U] << 8 | m_data[9U];
}

unsigned int CDMRCSBK::getDstId() const
{
	if (m_CSBKO == CSBKO_NACKRSP)
		return m_data[7U] << 16 | m_data[8U] << 8 | m_data[9U];
	else
		return m_data[4U] << 16 | m_data[5U] << 8 | m_data[6U];		
}

void CDMRCSBK::setGI(bool group)
{
	if (m_CSBKO == CSBKO_PRECCSBK) {
		if (group)
			m_data[2U] |= 0x40U;
		else
			m_data[2U] &= ~0x40U;
	}
}

void CDMRCSBK::setSrcId(unsigned int id)
{
	if (m_CSBKO == CSBKO_NACKRSP) {
		m_data[4U] = id >> 16;
		m_data[5U] = id >> 8;
		m_data[6U] = id >> 0;
	} else {
		m_data[7U] = id >> 16;
		m_data[8U] = id >> 8;
		m_data[9U] = id >> 0;
	}
}

void CDMRCSBK::setDstId(unsigned int id)
{
	if (m_CSBKO == CSBKO_NACKRSP) {
		m_data[7U] = id >> 16;
		m_data[8U] = id >> 8;
		m_data[9U] = id >> 0;
	} else {
		m_data[4U] = id >> 16;
		m_data[5U] = id >> 8;
		m_data[6U] = id >> 0;
	}
}
