/*
*  
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

#ifndef	DNSThread_H
#define	DNSThread_H

#include "Thread.h"
#include "Mutex.h"
#include "UDPSocket.h"

#include <string>
#include <unordered_map>

class CDNSThread : public CThread {
public:
	CDNSThread();
	virtual ~CDNSThread();
	virtual void entry();
	
	void stop();
	
	in_addr lookup(std::string hostname);

private:
	CMutex                                        m_mutex_query;
	CMutex					      m_mutex_address;
	bool                                          m_stop;
	bool					      m_query;
	in_addr					      m_address;
	std::string				      m_hostname;

};

#endif
