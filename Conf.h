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

#if !defined(CONF_H)
#define	CONF_H

#include <string>
#include <vector>

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	bool         getDaemon() const;
	unsigned int getXLXSlot() const;
	unsigned int getXLXTG() const;
	unsigned int getTimeout() const;
	std::string  getRptAddress() const;
	unsigned int getRptPort() const;
	std::string  getLocalAddress() const;
	unsigned int getLocalPort() const;
	bool         getDebug() const;

	// The Log section
	unsigned int getLogDisplayLevel() const;
	unsigned int getLogFileLevel() const;
	std::string  getLogFilePath() const;
	std::string  getLogFileRoot() const;

	// The Voice section
	bool         getVoiceEnabled() const;
	std::string  getVoiceLanguage() const;
	std::string  getVoiceDirectory() const;

	// The DMR Network section
	std::string  getDMRNetworkAddress() const;
	unsigned int getDMRNetworkPort() const;
	unsigned int getDMRNetworkLocal() const;
	std::string  getDMRNetworkPassword() const;
	bool         getDMRNetworkDebug() const;

	// The XLX Network section
	std::string  getXLXNetworkAddress() const;
	unsigned int getXLXNetworkPort() const;
	unsigned int getXLXNetworkLocal() const;
	std::string  getXLXNetworkPassword() const;
	std::string  getXLXNetworkOptions() const;
	bool         getXLXNetworkDebug() const;

private:
	std::string  m_file;
	bool         m_daemon;
	unsigned int m_xlxSlot;
	unsigned int m_xlxTG;
	std::string  m_rptAddress;
	unsigned int m_rptPort;
	std::string  m_localAddress;
	unsigned int m_localPort;
	unsigned int m_timeout;
	bool         m_debug;

	bool         m_voiceEnabled;
	std::string  m_voiceLanguage;
	std::string  m_voiceDirectory;

	unsigned int m_logDisplayLevel;
	unsigned int m_logFileLevel;
	std::string  m_logFilePath;
	std::string  m_logFileRoot;

	std::string  m_dmrNetworkAddress;
	unsigned int m_dmrNetworkPort;
	unsigned int m_dmrNetworkLocal;
	std::string  m_dmrNetworkPassword;
	bool         m_dmrNetworkDebug;

	std::string  m_xlxNetworkAddress;
	unsigned int m_xlxNetworkPort;
	unsigned int m_xlxNetworkLocal;
	std::string  m_xlxNetworkPassword;
	std::string  m_xlxNetworkOptions;
	bool         m_xlxNetworkDebug;
};

#endif
