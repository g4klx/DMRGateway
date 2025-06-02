/*
 *   Copyright (C) 2015,2016,2017,2019,2020 by Jonathan Naylor G4KLX
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
	unsigned int m_range;
};

struct CSrcRewriteStruct {
	unsigned int m_fromSlot;
	unsigned int m_fromId;
	unsigned int m_toSlot;
	unsigned int m_toTG;
	unsigned int m_range;
};

struct CTGDynRewriteStruct {
	unsigned int m_slot;
	unsigned int m_fromTG;
	unsigned int m_discPC;
	unsigned int m_statusPC;
	unsigned int m_toTG;
	unsigned int m_range;
	std::vector<unsigned int> m_exclTGs;
};

struct CIdRewriteStruct {
	unsigned int m_rfId;
	unsigned int m_netId;
};

struct CDMRNetConfStruct {
	bool           m_Enabled;
	std::string    m_Name;
	unsigned int   m_Id;
	std::string    m_Address;
	unsigned short m_Port;
	unsigned short m_Local;
	std::string    m_Password;
	std::string    m_Options;
	bool           m_Location;
	bool           m_Debug;
	std::vector<CTGRewriteStruct>    m_TGRewrites;
	std::vector<CPCRewriteStruct>    m_PCRewrites;
	std::vector<CTypeRewriteStruct>  m_TypeRewrites;
	std::vector<CSrcRewriteStruct>   m_SrcRewrites;
	std::vector<CTGDynRewriteStruct> m_TGDynRewrites;
	std::vector<CIdRewriteStruct>    m_IdRewrites;
	std::vector<unsigned int>        m_PassAllPC;
	std::vector<unsigned int>        m_PassAllTG;
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
	unsigned short getRptPort() const;
	std::string  getLocalAddress() const;
	unsigned short getLocalPort() const;
	bool         getRuleTrace() const;
	bool         getDebug() const;

	// The Log section
	unsigned int getLogDisplayLevel() const;
	unsigned int getLogFileLevel() const;
	std::string  getLogFilePath() const;
	std::string  getLogFileRoot() const;
	bool         getLogFileRotate() const;

	// The Voice section
	bool         getVoiceEnabled() const;
	std::string  getVoiceLanguage() const;
	std::string  getVoiceDirectory() const;

	// The Info section
	float        getInfoLatitude() const;
	float        getInfoLongitude() const;
	int          getInfoHeight() const;
	std::string  getInfoLocation() const;
	std::string  getInfoDescription() const;
	std::string  getInfoURL() const;

	// DMR Network sections
	unsigned int getDMRNetworksCount() const;  // get [DMR Network X] sections count
	bool         getDMRNetworkEnabled(unsigned int index) const;
	std::string  getDMRNetworkName(unsigned int index) const;
	unsigned int getDMRNetworkId(unsigned int index) const;
	std::string  getDMRNetworkAddress(unsigned int index) const;
	unsigned short getDMRNetworkPort(unsigned int index) const;
	unsigned short getDMRNetworkLocal(unsigned int index) const;
	std::string  getDMRNetworkPassword(unsigned int index) const;
	std::string  getDMRNetworkOptions(unsigned int index) const;
	bool         getDMRNetworkLocation(unsigned int index) const;
	bool         getDMRNetworkDebug(unsigned int index) const;
	std::vector<CTGRewriteStruct>    getDMRNetworkTGRewrites(unsigned int index) const;
	std::vector<CPCRewriteStruct>    getDMRNetworkPCRewrites(unsigned int index) const;
	std::vector<CTypeRewriteStruct>  getDMRNetworkTypeRewrites(unsigned int index) const;
	std::vector<CSrcRewriteStruct>   getDMRNetworkSrcRewrites(unsigned int index) const;
	std::vector<CTGDynRewriteStruct> getDMRNetworkTGDynRewrites(unsigned int index) const;
	std::vector<CIdRewriteStruct>    getDMRNetworkIdRewrites(unsigned int index) const;
	std::vector<unsigned int>        getDMRNetworkPassAllPC(unsigned int index) const;
	std::vector<unsigned int>        getDMRNetworkPassAllTG(unsigned int index) const;

	// The XLX Network section
	bool         getXLXNetworkEnabled() const;
	unsigned int getXLXNetworkId() const;
	std::string  getXLXNetworkFile() const;
	unsigned int getXLXNetworkReloadTime() const;
	unsigned short getXLXNetworkPort() const;
	std::string  getXLXNetworkPassword() const;
	unsigned short getXLXNetworkLocal() const;
	unsigned int getXLXNetworkSlot() const;
	unsigned int getXLXNetworkTG() const;
	unsigned int getXLXNetworkBase() const;
	std::string getXLXNetworkStartup() const;
	unsigned int getXLXNetworkRelink() const;
	bool         getXLXNetworkDebug() const;
	bool         getXLXNetworkUserControl() const;
	char         getXLXNetworkModule() const;

	// The GPSD section
	bool         getGPSDEnabled() const;
	std::string  getGPSDAddress() const;
	std::string  getGPSDPort() const;

	// The APRS section
	bool         getAPRSEnabled() const;
	std::string  getAPRSAddress() const;
	unsigned short getAPRSPort() const;
	std::string  getAPRSSuffix() const;
	std::string  getAPRSDescription() const;
	std::string  getAPRSSymbol() const;

	// The Dynamic TG Control section
	bool         getDynamicTGControlEnabled() const;
	unsigned short getDynamicTGControlPort() const;

	// The Remote Control section
	bool         getRemoteControlEnabled() const;
	std::string  getRemoteControlAddress() const;
	unsigned short getRemoteControlPort() const;

private:
	std::string  m_file;
	bool         m_daemon;
	std::string  m_rptAddress;
	unsigned short m_rptPort;
	std::string  m_localAddress;
	unsigned short m_localPort;
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
	bool         m_logFileRotate;

	float        m_infoLatitude;
	float        m_infoLongitude;
	int          m_infoHeight;
	std::string  m_infoLocation;
	std::string  m_infoDescription;
	std::string  m_infoURL;

	std::vector<CDMRNetConfStruct> m_dmrNetworks;
	CDMRNetConfStruct *addDMRNetwork();

	bool         m_xlxNetworkEnabled;
	unsigned int m_xlxNetworkId;
	std::string  m_xlxNetworkFile;
	unsigned int m_xlxNetworkReloadTime;
	unsigned short m_xlxNetworkPort;
	std::string  m_xlxNetworkPassword;
	unsigned short m_xlxNetworkLocal;
	unsigned int m_xlxNetworkSlot;
	unsigned int m_xlxNetworkTG;
	unsigned int m_xlxNetworkBase;
	std::string m_xlxNetworkStartup;
	unsigned int m_xlxNetworkRelink;
	bool         m_xlxNetworkDebug;
	bool         m_xlxNetworkUserControl;
	char         m_xlxNetworkModule;

	bool         m_gpsdEnabled;
	std::string  m_gpsdAddress;
	std::string  m_gpsdPort;

	bool         m_aprsEnabled;
	std::string  m_aprsAddress;
	unsigned short m_aprsPort;
	std::string  m_aprsSuffix;
	std::string  m_aprsDescription;
	std::string  m_aprsSymbol;

	bool         m_dynamicTGControlEnabled;
	unsigned short m_dynamicTGControlPort;

	bool         m_remoteControlEnabled;
	std::string  m_remoteControlAddress;
	unsigned short m_remoteControlPort;
};

#endif
