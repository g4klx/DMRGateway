/*
 *   Copyright (C) 2021 by Jonathan Naylor G4KLX
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

#include "SplitNetwork.h"

#include <cstdio>
#include <cassert>

CSplitNetwork::CSplitNetwork(IMMDVMNetwork* network1, IMMDVMNetwork* network2, unsigned int slotNo, bool debug) :
m_network1(network1),
m_network2(network2),
m_slotNo(slotNo),
m_debug(debug)
{
	assert(network1 != NULL);
	assert(network2 != NULL);
	assert(slotNo == 1U || slotNo == 2U);
}

CSplitNetwork::~CSplitNetwork()
{
	delete m_network1;
	delete m_network2;
}

unsigned int CSplitNetwork::getShortConfig(unsigned char* config) const
{
	assert(config != NULL);

	return m_network1->getShortConfig(config);
}

unsigned int CSplitNetwork::getId() const
{
	return m_network1->getId();
}

bool CSplitNetwork::open()
{
	bool ret = m_network1->open();
	if (!ret)
		return false;

	ret = m_network2->open();
	if (!ret) {
		m_network1->close();
		return false;
	}

	return true;
}

bool CSplitNetwork::read(CDMRData& data)
{
	bool ret = m_network1->read(data);
	if (ret) {
		data.setSlotNo(1U);
		return true;
	}

	ret = m_network2->read(data);
	if (ret) {
		data.setSlotNo(2U);
		return true;
	}

	return false;
}

bool CSplitNetwork::write(CDMRData& data)
{
	unsigned int slot = data.getSlotNo();
	if (slot == 1U) {
		data.setSlotNo(m_slotNo);
		return m_network1->write(data);
	} else {
		data.setSlotNo(m_slotNo);
		return m_network2->write(data);
	}
}

bool CSplitNetwork::readRadioPosition(unsigned char* data, unsigned int& length)
{
	assert(data != NULL);

	bool ret = m_network1->readRadioPosition(data, length);
	if (ret)
		return true;

	return m_network2->readRadioPosition(data, length);
}

bool CSplitNetwork::readTalkerAlias(unsigned char* data, unsigned int& length)
{
	assert(data != NULL);

	bool ret = m_network1->readTalkerAlias(data, length);
	if (ret)
		return true;

	return m_network2->readTalkerAlias(data, length);
}

bool CSplitNetwork::writeBeacon()
{
	bool ret = m_network1->writeBeacon();
	if (!ret)
		return false;

	return m_network2->writeBeacon();
}

void CSplitNetwork::clock(unsigned int ms)
{
	m_network1->clock(ms);
	m_network2->clock(ms);
}

void CSplitNetwork::close()
{
	m_network1->close();
	m_network2->close();
}
