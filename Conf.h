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

struct CTGRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromTG;
	unsigned int m_toSlot;
	unsigned int m_toTG;
	unsigned int m_range;
};

struct CPCRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromId;
	unsigned int m_toSlot;
	unsigned int m_toId;
	unsigned int m_range;
};

struct CTypeRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromTG;
	unsigned int m_toSlot;
	unsigned int m_toId;
};

struct CSrcRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromId;
	unsigned int m_toSlot;
	unsigned int m_toTG;
	unsigned int m_range;
};

class CConf
{
public:
	CConf(const std::string& file);
	~CConf();

	bool read();

	// The General section
	std::string  getCallsign() const;
    std::string  getSuffix() const;
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

	// The DMR Id section
	std::string  getDMRIdLookupFile() const;
	unsigned int getDMRIdLookupTime() const;


	// The aprs.fi section
	bool         getAPRSEnabled() const;
	std::string  getAPRSServer() const;
	unsigned int getAPRSPort() const;
	std::string  getAPRSPassword() const;

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
	std::string  getDMRNetwork1Options() const;
	bool         getDMRNetwork1Debug() const;
	std::vector<CTGRewriteStruct>   getDMRNetwork1TGRewrites() const;
	std::vector<CPCRewriteStruct>   getDMRNetwork1PCRewrites() const;
	std::vector<CTypeRewriteStruct> getDMRNetwork1TypeRewrites() const;
	std::vector<CSrcRewriteStruct>  getDMRNetwork1SrcRewrites() const;
	std::vector<unsigned int>       getDMRNetwork1PassAllPC() const;
	std::vector<unsigned int>       getDMRNetwork1PassAllTG() const;

	// The DMR Network 2 section
	bool         getDMRNetwork2Enabled() const;
	unsigned int getDMRNetwork2Id() const;
	std::string  getDMRNetwork2Address() const;
	unsigned int getDMRNetwork2Port() const;
	unsigned int getDMRNetwork2Local() const;
	std::string  getDMRNetwork2Password() const;
	std::string  getDMRNetwork2Options() const;
	bool         getDMRNetwork2Debug() const;
	std::vector<CTGRewriteStruct>   getDMRNetwork2TGRewrites() const;
	std::vector<CPCRewriteStruct>   getDMRNetwork2PCRewrites() const;
	std::vector<CTypeRewriteStruct> getDMRNetwork2TypeRewrites() const;
	std::vector<CSrcRewriteStruct>  getDMRNetwork2SrcRewrites() const;
	std::vector<unsigned int>       getDMRNetwork2PassAllPC() const;
	std::vector<unsigned int>       getDMRNetwork2PassAllTG() const;

	// The XLX Network 1 section
	bool         getXLXNetwork1Enabled() const;
	unsigned int getXLXNetwork1Id() const;
	std::string  getXLXNetwork1Address() const;
	unsigned int getXLXNetwork1Port() const;
	unsigned int getXLXNetwork1Local() const;
	std::string  getXLXNetwork1Password() const;
	unsigned int getXLXNetwork1Slot() const;
	unsigned int getXLXNetwork1TG() const;
	unsigned int getXLXNetwork1Base() const;
	unsigned int getXLXNetwork1Startup() const;
	std::string  getXLXNetwork1Options() const;
	bool         getXLXNetwork1Debug() const;

	// The XLX Network 2 section
	bool         getXLXNetwork2Enabled() const;
	unsigned int getXLXNetwork2Id() const;
	std::string  getXLXNetwork2Address() const;
	unsigned int getXLXNetwork2Port() const;
	unsigned int getXLXNetwork2Local() const;
	std::string  getXLXNetwork2Password() const;
	unsigned int getXLXNetwork2Slot() const;
	unsigned int getXLXNetwork2TG() const;
	unsigned int getXLXNetwork2Base() const;
	unsigned int getXLXNetwork2Startup() const;
	std::string  getXLXNetwork2Options() const;
	bool         getXLXNetwork2Debug() const;

private:
	std::string  m_file;
	std:: string m_callsign;
	std:: string m_suffix;
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

	std::string  m_dmrIdLookupFile;
  	unsigned int m_dmrIdLookupTime;

	bool         m_aprsEnabled;
	std::string  m_aprsServer;
	unsigned int m_aprsPort;
	std::string  m_aprsPassword;

	bool         m_dmrNetwork1Enabled;
	unsigned int m_dmrNetwork1Id;
	std::string  m_dmrNetwork1Address;
	unsigned int m_dmrNetwork1Port;
	unsigned int m_dmrNetwork1Local;
	std::string  m_dmrNetwork1Password;
	std::string  m_dmrNetwork1Options;
	bool         m_dmrNetwork1Debug;
	std::vector<CTGRewriteStruct>   m_dmrNetwork1TGRewrites;
	std::vector<CPCRewriteStruct>   m_dmrNetwork1PCRewrites;
	std::vector<CTypeRewriteStruct> m_dmrNetwork1TypeRewrites;
	std::vector<CSrcRewriteStruct>  m_dmrNetwork1SrcRewrites;
	std::vector<unsigned int>       m_dmrNetwork1PassAllPC;
	std::vector<unsigned int>       m_dmrNetwork1PassAllTG;

	bool         m_dmrNetwork2Enabled;
	unsigned int m_dmrNetwork2Id;
	std::string  m_dmrNetwork2Address;
	unsigned int m_dmrNetwork2Port;
	unsigned int m_dmrNetwork2Local;
	std::string  m_dmrNetwork2Password;
	std::string  m_dmrNetwork2Options;
	bool         m_dmrNetwork2Debug;
	std::vector<CTGRewriteStruct>   m_dmrNetwork2TGRewrites;
	std::vector<CPCRewriteStruct>   m_dmrNetwork2PCRewrites;
	std::vector<CTypeRewriteStruct> m_dmrNetwork2TypeRewrites;
	std::vector<CSrcRewriteStruct>  m_dmrNetwork2SrcRewrites;
	std::vector<unsigned int>       m_dmrNetwork2PassAllPC;
	std::vector<unsigned int>       m_dmrNetwork2PassAllTG;

	bool         m_xlxNetwork1Enabled;
	unsigned int m_xlxNetwork1Id;
	std::string  m_xlxNetwork1Address;
	unsigned int m_xlxNetwork1Port;
	unsigned int m_xlxNetwork1Local;
	std::string  m_xlxNetwork1Password;
	unsigned int m_xlxNetwork1Slot;
	unsigned int m_xlxNetwork1TG;
	unsigned int m_xlxNetwork1Base;
	unsigned int m_xlxNetwork1Startup;
	std::string  m_xlxNetwork1Options;
	bool         m_xlxNetwork1Debug;

	bool         m_xlxNetwork2Enabled;
	unsigned int m_xlxNetwork2Id;
	std::string  m_xlxNetwork2Address;
	unsigned int m_xlxNetwork2Port;
	unsigned int m_xlxNetwork2Local;
	std::string  m_xlxNetwork2Password;
	unsigned int m_xlxNetwork2Slot;
	unsigned int m_xlxNetwork2TG;
	unsigned int m_xlxNetwork2Base;
	unsigned int m_xlxNetwork2Startup;
	std::string  m_xlxNetwork2Options;
	bool         m_xlxNetwork2Debug;
};

#endif
