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

struct CRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromTG;
	unsigned int m_toSlot;
	unsigned int m_toTG;
};

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	bool         getDaemon() const;
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

	// The DMR Network 1 section
	bool         getDMRNetwork1Enabled() const;
	unsigned int getDMRNetwork1Id() const;
	std::string  getDMRNetwork1Address() const;
	unsigned int getDMRNetwork1Port() const;
	unsigned int getDMRNetwork1Local() const;
	std::string  getDMRNetwork1Password() const;
	bool         getDMRNetwork1Debug() const;
	std::vector<CRewriteStruct> getDMRNetwork1Rewrites() const;
	bool         getDMRNetwork1PrivateSlot1() const;
	bool         getDMRNetwork1PrivateSlot2() const;

	// The DMR Network 2 section
	bool         getDMRNetwork2Enabled() const;
	unsigned int getDMRNetwork2Id() const;
	std::string  getDMRNetwork2Address() const;
	unsigned int getDMRNetwork2Port() const;
	unsigned int getDMRNetwork2Local() const;
	std::string  getDMRNetwork2Password() const;
	bool         getDMRNetwork2Debug() const;
	std::vector<CRewriteStruct> getDMRNetwork2Rewrites() const;
	bool         getDMRNetwork2PrivateSlot1() const;
	bool         getDMRNetwork2PrivateSlot2() const;

	// The XLX Network section
	bool         getXLXNetworkEnabled() const;
	unsigned int getXLXNetworkId() const;
	std::string  getXLXNetworkAddress() const;
	unsigned int getXLXNetworkPort() const;
	unsigned int getXLXNetworkLocal() const;
	std::string  getXLXNetworkPassword() const;
	unsigned int getXLXNetworkSlot() const;
	unsigned int getXLXNetworkTG() const;
	std::string  getXLXNetworkOptions() const;
	bool         getXLXNetworkDebug() const;

private:
	std::string  m_file;
	bool         m_daemon;
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

	bool         m_dmrNetwork1Enabled;
	unsigned int m_dmrNetwork1Id;
	std::string  m_dmrNetwork1Address;
	unsigned int m_dmrNetwork1Port;
	unsigned int m_dmrNetwork1Local;
	std::string  m_dmrNetwork1Password;
	bool         m_dmrNetwork1Debug;

	bool         m_dmrNetwork2Enabled;
	unsigned int m_dmrNetwork2Id;
	std::string  m_dmrNetwork2Address;
	unsigned int m_dmrNetwork2Port;
	unsigned int m_dmrNetwork2Local;
	std::string  m_dmrNetwork2Password;
	bool         m_dmrNetwork2Debug;
	std::vector<CRewriteStruct> m_dmrNetwork1Rewrites;
	bool         m_dmrNetwork1PrivateSlot1;
	bool         m_dmrNetwork1PrivateSlot2;

	bool         m_xlxNetworkEnabled;
	unsigned int m_xlxNetworkId;
	std::string  m_xlxNetworkAddress;
	unsigned int m_xlxNetworkPort;
	unsigned int m_xlxNetworkLocal;
	std::string  m_xlxNetworkPassword;
	unsigned int m_xlxNetworkSlot;
	unsigned int m_xlxNetworkTG;
	std::string  m_xlxNetworkOptions;
	bool         m_xlxNetworkDebug;
	std::vector<CRewriteStruct> m_dmrNetwork2Rewrites;
	bool         m_dmrNetwork2PrivateSlot1;
	bool         m_dmrNetwork2PrivateSlot2;
};

#endif
