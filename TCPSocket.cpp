/*
 *   Copyright (C) 2010-2013,2016 by Jonathan Naylor G4KLX
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

#include "TCPSocket.h"
#include "UDPSocket.h"
#include "Log.h"

#include <cstdio>
#include <cassert>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
typedef int ssize_t;
#else
#include <cerrno>
#endif

CTCPSocket::CTCPSocket(const std::string& address, unsigned int port) :
m_address(address),
m_port(port),
m_fd(-1)
{
	assert(!address.empty());
	assert(port > 0U);

#if defined(_WIN32) || defined(_WIN64)
	WSAData data;
	int wsaRet = ::WSAStartup(MAKEWORD(2, 2), &data);
	if (wsaRet != 0)
		LogError("Error from WSAStartup");
#endif
}

CTCPSocket::~CTCPSocket()
{
#if defined(_WIN32) || defined(_WIN64)
	::WSACleanup();
#endif
}

bool CTCPSocket::open()
{
	if (m_fd != -1)
		return true;

	if (m_address.empty() || m_port == 0U)
		return false;

	m_fd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_fd < 0) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Cannot create the TCP client socket, err=%d", ::GetLastError());
#else
		LogError("Cannot create the TCP client socket, err=%d", errno);
#endif
		return false;
	}

	struct sockaddr_in addr;
	::memset(&addr, 0x00, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(m_port);
	addr.sin_addr   = CUDPSocket::lookup(m_address);

	if (addr.sin_addr.s_addr == INADDR_NONE) {
		close();
		return false;
	}

	if (::connect(m_fd, (sockaddr*)&addr, sizeof(struct sockaddr_in)) == -1) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Cannot connect the TCP client socket, err=%d", ::GetLastError());
#else
		LogError("Cannot connect the TCP client socket, err=%d", errno);
#endif
		close();
		return false;
	}

	int noDelay = 1;
	if (::setsockopt(m_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&noDelay, sizeof(noDelay)) == -1) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Cannot set the TCP client socket option, err=%d", ::GetLastError());
#else
		LogError("Cannot set the TCP client socket option, err=%d", errno);
#endif
		close();
		return false;
	}

	return true;
}

int CTCPSocket::read(unsigned char* buffer, unsigned int length, unsigned int secs, unsigned int msecs)
{
	assert(buffer != NULL);
	assert(length > 0U);
	assert(m_fd != -1);

	// Check that the recv() won't block
	fd_set readFds;
	FD_ZERO(&readFds);
#if defined(_WIN32) || defined(_WIN64)
	FD_SET((unsigned int)m_fd, &readFds);
#else
	FD_SET(m_fd, &readFds);
#endif

	// Return after timeout
	timeval tv;
	tv.tv_sec  = secs;
	tv.tv_usec = msecs * 1000;

	int ret = ::select(m_fd + 1, &readFds, NULL, NULL, &tv);
	if (ret < 0) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Error returned from TCP client select, err=%d", ::GetLastError());
#else
		LogError("Error returned from TCP client select, err=%d", errno);
#endif
		return -1;
	}

#if defined(_WIN32) || defined(_WIN64)
	if (!FD_ISSET((unsigned int)m_fd, &readFds))
		return 0;
#else
	if (!FD_ISSET(m_fd, &readFds))
		return 0;
#endif

	ssize_t len = ::recv(m_fd, (char*)buffer, length, 0);
	if (len == 0) {
		return -2;
	} else if (len < 0) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Error returned from recv, err=%d", ::GetLastError());
#else
		LogError("Error returned from recv, err=%d", errno);
#endif
		return -1;
	}

	return len;
}

int CTCPSocket::readLine(std::string& line, unsigned int secs)
{
	// Maybe there is a better way to do this like reading blocks, pushing them for later calls
	// Nevermind, we'll read one char at a time for the time being.
	int resultCode;
	int len = 0;

	line.clear();

	char c[2U];
	c[1U] = 0x00U;

	do
	{
		resultCode = read((unsigned char*)c, 1U, secs);
		if (resultCode == 1){
			line.append(c);
			len++;
		}
	} while (c[0U] != '\n' && resultCode == 1);

	return resultCode <= 0 ? resultCode : len;
}

bool CTCPSocket::write(const unsigned char* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(length > 0U);
	assert(m_fd != -1);

	ssize_t ret = ::send(m_fd, (char *)buffer, length, 0);
	if (ret != ssize_t(length)) {
#if defined(_WIN32) || defined(_WIN64)
		LogError("Error returned from send, err=%d", ::GetLastError());
#else
		LogError("Error returned from send, err=%d", errno);
#endif
		return false;
	}

	return true;
}

bool CTCPSocket::writeLine(const std::string& line)
{
	std::string lineCopy(line);
	if (lineCopy.length() > 0 && lineCopy.at(lineCopy.length() - 1) != '\n')
		lineCopy.append("\n");
	
	//stupidly write one char after the other
	size_t len = lineCopy.length();
	bool result = true;
	for (size_t i = 0U; i < len && result; i++){
		unsigned char c = lineCopy.at(i);
		result = write(&c , 1);
	}

	return result;
}

void CTCPSocket::close()
{
	if (m_fd != -1) {
#if defined(_WIN32) || defined(_WIN64)
		::closesocket(m_fd);
#else
		::close(m_fd);
#endif
		m_fd = -1;
	}
}
