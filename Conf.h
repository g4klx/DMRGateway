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
	bool         getDaemon() const;
	unsigned int getRFTimeout() const;
	unsigned int getNetTimeout() const;
	std::string  getRptAddress() const;
	unsigned int getRptPort() const;
	std::string  getLocalAddress() const;
	unsigned int getLocalPort() const;
	bool         getRuleTrace() const;
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

	// The Info section
	bool         getInfoEnabled() const;
	unsigned int getInfoRXFrequency() const;
	unsigned int getInfoTXFrequency() const;
	unsigned int getInfoPower() const;
	float        getInfoLatitude() const;
	float        getInfoLongitude() const;
	int          getInfoHeight() const;
	std::string  getInfoLocation() const;
	std::string  getInfoDescription() const;
	std::string  getInfoURL() const;

	// The DMR Network 1 section
	bool         getDMRNetwork1Enabled() const;
	std::string  getDMRNetwork1Name() const;
	unsigned int getDMRNetwork1Id() const;
	std::string  getDMRNetwork1Address() const;
	unsigned int getDMRNetwork1Port() const;
	unsigned int getDMRNetwork1Local() const;
	std::string  getDMRNetwork1Password() const;
	std::string  getDMRNetwork1Options() const;
	bool         getDMRNetwork1Location() const;
	bool         getDMRNetwork1Debug() const;
	std::vector<CTGRewriteStruct>   getDMRNetwork1TGRewrites() const;
	std::vector<CPCRewriteStruct>   getDMRNetwork1PCRewrites() const;
	std::vector<CTypeRewriteStruct> getDMRNetwork1TypeRewrites() const;
	std::vector<CSrcRewriteStruct>  getDMRNetwork1SrcRewrites() const;
	std::vector<unsigned int>       getDMRNetwork1PassAllPC() const;
	std::vector<unsigned int>       getDMRNetwork1PassAllTG() const;

	// The DMR Network 2 section
	bool         getDMRNetwork2Enabled() const;
	std::string  getDMRNetwork2Name() const;
	unsigned int getDMRNetwork2Id() const;
	std::string  getDMRNetwork2Address() const;
	unsigned int getDMRNetwork2Port() const;
	unsigned int getDMRNetwork2Local() const;
	std::string  getDMRNetwork2Password() const;
	std::string  getDMRNetwork2Options() const;
	bool         getDMRNetwork2Location() const;
	bool         getDMRNetwork2Debug() const;
	std::vector<CTGRewriteStruct>   getDMRNetwork2TGRewrites() const;
	std::vector<CPCRewriteStruct>   getDMRNetwork2PCRewrites() const;
	std::vector<CTypeRewriteStruct> getDMRNetwork2TypeRewrites() const;
	std::vector<CSrcRewriteStruct>  getDMRNetwork2SrcRewrites() const;
	std::vector<unsigned int>       getDMRNetwork2PassAllPC() const;
	std::vector<unsigned int>       getDMRNetwork2PassAllTG() const;

	// The DMR Network 3 section
	bool         getDMRNetwork3Enabled() const;
	std::string  getDMRNetwork3Name() const;
	unsigned int getDMRNetwork3Id() const;
	std::string  getDMRNetwork3Address() const;
	unsigned int getDMRNetwork3Port() const;
	unsigned int getDMRNetwork3Local() const;
	std::string  getDMRNetwork3Password() const;
	std::string  getDMRNetwork3Options() const;
	bool         getDMRNetwork3Location() const;
	bool         getDMRNetwork3Debug() const;
	std::vector<CTGRewriteStruct>   getDMRNetwork3TGRewrites() const;
	std::vector<CPCRewriteStruct>   getDMRNetwork3PCRewrites() const;
	std::vector<CTypeRewriteStruct> getDMRNetwork3TypeRewrites() const;
	std::vector<CSrcRewriteStruct>  getDMRNetwork3SrcRewrites() const;
	std::vector<unsigned int>       getDMRNetwork3PassAllPC() const;
	std::vector<unsigned int>       getDMRNetwork3PassAllTG() const;

	// The XLX Network section
	bool         getXLXNetworkEnabled() const;
	unsigned int getXLXNetworkId() const;
	std::string  getXLXNetworkFile() const;
    unsigned int getXLXNetworkReloadTime() const;
    unsigned int getXLXNetworkPort() const;
    std::string  getXLXNetworkPassword() const;
	unsigned int getXLXNetworkLocal() const;
	unsigned int getXLXNetworkSlot() const;
	unsigned int getXLXNetworkTG() const;
	unsigned int getXLXNetworkBase() const;
	unsigned int getXLXNetworkStartup() const;
	unsigned int getXLXNetworkRelink() const;
	bool         getXLXNetworkDebug() const;
    bool         getXLXNetworkUserControl() const;
    char         getXLXNetworkModule() const;

private:
	std::string  m_file;
	bool         m_daemon;
	std::string  m_rptAddress;
	unsigned int m_rptPort;
	std::string  m_localAddress;
	unsigned int m_localPort;
	unsigned int m_rfTimeout;
	unsigned int m_netTimeout;
	bool         m_ruleTrace;
	bool         m_debug;

	bool         m_voiceEnabled;
	std::string  m_voiceLanguage;
	std::string  m_voiceDirectory;

	unsigned int m_logDisplayLevel;
	unsigned int m_logFileLevel;
	std::string  m_logFilePath;
	std::string  m_logFileRoot;

	bool         m_infoEnabled;
	unsigned int m_infoRXFrequency;
	unsigned int m_infoTXFrequency;
	unsigned int m_infoPower;
	float        m_infoLatitude;
	float        m_infoLongitude;
	int          m_infoHeight;
	std::string  m_infoLocation;
	std::string  m_infoDescription;
	std::string  m_infoURL;

	bool         m_dmrNetwork1Enabled;
	std::string  m_dmrNetwork1Name;
	unsigned int m_dmrNetwork1Id;
	std::string  m_dmrNetwork1Address;
	unsigned int m_dmrNetwork1Port;
	unsigned int m_dmrNetwork1Local;
	std::string  m_dmrNetwork1Password;
	std::string  m_dmrNetwork1Options;
	bool         m_dmrNetwork1Location;
	bool         m_dmrNetwork1Debug;
	std::vector<CTGRewriteStruct>   m_dmrNetwork1TGRewrites;
	std::vector<CPCRewriteStruct>   m_dmrNetwork1PCRewrites;
	std::vector<CTypeRewriteStruct> m_dmrNetwork1TypeRewrites;
	std::vector<CSrcRewriteStruct>  m_dmrNetwork1SrcRewrites;
	std::vector<unsigned int>       m_dmrNetwork1PassAllPC;
	std::vector<unsigned int>       m_dmrNetwork1PassAllTG;

	bool         m_dmrNetwork2Enabled;
	std::string  m_dmrNetwork2Name;
	unsigned int m_dmrNetwork2Id;
	std::string  m_dmrNetwork2Address;
	unsigned int m_dmrNetwork2Port;
	unsigned int m_dmrNetwork2Local;
	std::string  m_dmrNetwork2Password;
	std::string  m_dmrNetwork2Options;
	bool         m_dmrNetwork2Location;
	bool         m_dmrNetwork2Debug;
	std::vector<CTGRewriteStruct>   m_dmrNetwork2TGRewrites;
	std::vector<CPCRewriteStruct>   m_dmrNetwork2PCRewrites;
	std::vector<CTypeRewriteStruct> m_dmrNetwork2TypeRewrites;
	std::vector<CSrcRewriteStruct>  m_dmrNetwork2SrcRewrites;
	std::vector<unsigned int>       m_dmrNetwork2PassAllPC;
	std::vector<unsigned int>       m_dmrNetwork2PassAllTG;

	bool         m_dmrNetwork3Enabled;
	std::string  m_dmrNetwork3Name;
	unsigned int m_dmrNetwork3Id;
	std::string  m_dmrNetwork3Address;
	unsigned int m_dmrNetwork3Port;
	unsigned int m_dmrNetwork3Local;
	std::string  m_dmrNetwork3Password;
	std::string  m_dmrNetwork3Options;
	bool         m_dmrNetwork3Location;
	bool         m_dmrNetwork3Debug;
	std::vector<CTGRewriteStruct>   m_dmrNetwork3TGRewrites;
	std::vector<CPCRewriteStruct>   m_dmrNetwork3PCRewrites;
	std::vector<CTypeRewriteStruct> m_dmrNetwork3TypeRewrites;
	std::vector<CSrcRewriteStruct>  m_dmrNetwork3SrcRewrites;
	std::vector<unsigned int>       m_dmrNetwork3PassAllPC;
	std::vector<unsigned int>       m_dmrNetwork3PassAllTG;

	bool         m_xlxNetworkEnabled;
	unsigned int m_xlxNetworkId;
	std::string  m_xlxNetworkFile;
    unsigned int m_xlxNetworkReloadTime;
    unsigned int m_xlxNetworkPort;
    std::string  m_xlxNetworkPassword;
	unsigned int m_xlxNetworkLocal;
	unsigned int m_xlxNetworkSlot;
	unsigned int m_xlxNetworkTG;
	unsigned int m_xlxNetworkBase;
	unsigned int m_xlxNetworkStartup;
	unsigned int m_xlxNetworkRelink;
	bool         m_xlxNetworkDebug;
    bool         m_xlxNetworkUserControl;
    char         m_xlxNetworkModule;
};

#endif
