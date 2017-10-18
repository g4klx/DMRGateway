/*
* Contribution by Simon Adlem G7RZU, based on existing code written by G4KLX
* 
* Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
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

#include "DNSThread.h"
#include "Timer.h"
#include "Log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

CDNSThread::CDNSThread() :
CThread(),
m_mutex_query(),
m_mutex_address(),
m_stop(false),
m_query(false),
m_address(),
m_hostname()
{
}

CDNSThread::~CDNSThread()
{
}

void CDNSThread::entry()
{
	LogInfo("Started the DNS lookup thread");

	while (!m_stop) {
		sleep(1000U);
		if (m_query) {
		      LogInfo("DNS Lookup: %s,m_hostname");
		      m_mutex_address.lock();
		      m_address = CUDPSocket::lookup(m_hostname);
		      m_mutex_address.unlock();
		      
		      m_mutex_query.lock();
		      m_query = false;
		      m_mutex_query.unlock();
		}
	}

	LogInfo("Stopped the DNS lookup thread");
}

void CDNSThread::stop()
{
	m_stop = true;
	wait();
}

in_addr CDNSThread::lookup(std::string hostname)
{

	m_mutex_query.lock();
	m_query = true;
	m_mutex_query.unlock();

	m_mutex_address.lock();
	return(m_address);
	m_mutex_address.unlock();
}
