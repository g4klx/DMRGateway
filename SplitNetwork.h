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

#if !defined(SplitNetwork_H)
#define	SplitNetwork_H

#include "MMDVMNetwork.h"

#include <cstdint>

class CSplitNetwork : public IMMDVMNetwork
{
public:
	CSplitNetwork(IMMDVMNetwork* network1, IMMDVMNetwork* network2, unsigned int slotNo, bool debug);
	virtual ~CSplitNetwork();

	virtual unsigned int getShortConfig(unsigned char* config) const;

	virtual unsigned int getId() const;

	virtual bool open();

	virtual bool read(CDMRData& data);

	virtual bool write(CDMRData& data);

	virtual bool readRadioPosition(unsigned char* data, unsigned int& length);

	virtual bool readTalkerAlias(unsigned char* data, unsigned int& length);

	virtual bool writeBeacon();

	virtual void clock(unsigned int ms);

	virtual void close();

private:
	IMMDVMNetwork* m_network1;
	IMMDVMNetwork* m_network2;
	unsigned int   m_slotNo;
	bool           m_debug;
};

#endif
