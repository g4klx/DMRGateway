/*
*   Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
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

#if !defined(Reflectors_H)
#define	Reflectors_H

#include "Timer.h"

#include <vector>
#include <string>

class CReflector {
public:
	CReflector() :
	m_id(0U),
	m_address(),
	m_startup(0U)
	{
	}

	unsigned int m_id;
	std::string  m_address;
	unsigned int m_startup;
};

class CReflectors {
public:
	CReflectors(const std::string& hostsFile, unsigned int reloadTime);
	~CReflectors();

	bool load();

	CReflector* find(unsigned int id);

    void clock(unsigned int ms);

private:
	std::string              m_hostsFile;
	std::vector<CReflector*> m_reflectors;
    CTimer                   m_timer;
};

#endif
