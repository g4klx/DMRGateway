/*
*   Copyright (C) 2017 Tony Bailey
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
*
*
************************************************************
*
*   A wrapper class for the APRS utilities send GPS data to an APRS server
*
*/

#include "APRSHelper.h"

#include <cstdio>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <iostream>

CAPRSHelper::CAPRSHelper(const std::string& callsign, const std::string& suffix, const std::string& password, const std::string& address, unsigned int port)
:
m_writer(callsign, suffix, password, address, port),
m_buffer(NULL)
{
   m_buffer = new unsigned char[300U];
}

CAPRSHelper::~CAPRSHelper()
{
	delete[] m_buffer;    
}

void CAPRSHelper::setInfo(unsigned int txFrequency, unsigned int rxFrequency, float latitude, float longitude, int height)
{
    m_writer.setInfo(rxFrequency, rxFrequency, latitude, longitude, height);
}


bool CAPRSHelper::open()
{
    return m_writer.open();
}

void CAPRSHelper::send(std::string callsign, float latitude, float longitude, int altitude )
{    
    unsigned char source[10U];    
    copy( callsign.begin(), callsign.end(), source );
    
    char type[11U];
    ::strcpy(type, "MD-390/RT8");
    
    // Source: Callsign
    // Type: Radio Type
    // symbol?:
    //  default (unset) is a House '/-'
    //  0x24U 
    //  0x25U
    //  0x26U - ??
    //  0x28U - Human
    //  0x29U - Normal Car '/>'
    //  lat
    //  long
    //::sprintf(m_buffer[4U], 0x28U);
    m_writer.write(source, type, 0x28U, latitude, longitude, altitude); 
}

void CAPRSHelper::clock(unsigned int ms)
{
    m_writer.clock(ms);
}

void CAPRSHelper::close()
{
    m_writer.close();
}

/*
 *   Uncomment this section for local testing and build it without MMDVMHost in the Makefile
 */


/*
int main(void)
{
    std::string callsign = "N0CALL";
    std::string suffix = "11";
    std::string password = "-1";
    std::string address = "rotate.aprs2.net";
    int port = 14580;    
	CAPRSHelper* helper = new CAPRSHelper(callsign, suffix, password, address, port);

	helper->open();
    // Wait 6 seconds for the server to connect before sending data
    usleep(1000*1000*6);
    
    // Send the GPS packet
    helper->send();
    
    // Need to go into an endless loop and give the APRSWriterThread time to cycle through the message queue
    for (;;)
    {
        // Ctrl-C out of the test program once the packet is sent
    }

	delete helper;
    
    return 0;
}
*/
