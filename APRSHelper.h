/*
*   Copyright (C) 2017 by Tony Bailey
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

#if !defined(APRSHelper_H)
#define	APRSHelper_H

#include "APRSWriter.h"

#include <string>

class CAPRSHelper {
public:
	CAPRSHelper(const std::string& callsign, const std::string& suffix, const std::string& password, const std::string& address, unsigned int port);
	~CAPRSHelper();

	void open();

    void send(std::string callsign, float latitude, float longitude );
    
	void close();

private:
	CAPRSWriter    m_writer;
	unsigned char* m_buffer;
};

#endif
