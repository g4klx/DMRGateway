/*
 *   Copyright (C) 2015-2020,2025 by Jonathan Naylor G4KLX
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

#include "Conf.h"
#include "Log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>

const int BUFFER_SIZE = 500;

enum class SECTION {
	NONE,
	GENERAL,
	LOG,
	VOICE,
	INFO,
	DMR_NETWORK_N,
	XLX_NETWORK,
	GPSD,
	APRS,
	DYNAMIC_TG_CONTROL,
	REMOTE_CONTROL
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_rptAddress("127.0.0.1"),
m_rptPort(62032U),
m_localAddress("127.0.0.1"),
m_localPort(62031U),
m_rfTimeout(10U),
m_netTimeout(10U),
m_ruleTrace(false),
m_debug(false),
m_voiceEnabled(true),
m_voiceLanguage("en_GB"),
m_voiceDirectory(),
m_logDisplayLevel(0U),
m_logFileLevel(0U),
m_logFilePath(),
m_logFileRoot(),
m_logFileRotate(true),
m_infoLatitude(0.0F),
m_infoLongitude(0.0F),
m_infoHeight(0),
m_infoLocation(),
m_infoDescription(),
m_infoURL(),
m_dmrNetworks(),
m_xlxNetworkEnabled(false),
m_xlxNetworkId(0U),
m_xlxNetworkFile(),
m_xlxNetworkReloadTime(0U),
m_xlxNetworkPort(62030U),
m_xlxNetworkPassword("passw0rd"),
m_xlxNetworkLocal(0U),
m_xlxNetworkSlot(1U),
m_xlxNetworkTG(8U),
m_xlxNetworkBase(84000U),
m_xlxNetworkStartup("4000"),
m_xlxNetworkRelink(0U),
m_xlxNetworkDebug(false),
m_xlxNetworkUserControl(true),
m_xlxNetworkModule(),
m_gpsdEnabled(false),
m_gpsdAddress(),
m_gpsdPort(),
m_aprsEnabled(false),
m_aprsAddress(),
m_aprsPort(0U),
m_aprsSuffix(),
m_aprsDescription(),
m_aprsSymbol(),
m_dynamicTGControlEnabled(false),
m_dynamicTGControlPort(3769U),
m_remoteControlEnabled(false),
m_remoteControlAddress("127.0.0.1"),
m_remoteControlPort(0U)
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
	FILE* fp = ::fopen(m_file.c_str(), "rt");
	if (fp == nullptr) {
		::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
		return false;
	}

	SECTION section = SECTION::NONE;
	CDMRNetConfStruct *lastDmrNet = nullptr;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0) {
				section = SECTION::GENERAL;
			} else if (::strncmp(buffer, "[Log]", 5U) == 0) {
				section = SECTION::LOG;
			} else if (::strncmp(buffer, "[Voice]", 7U) == 0) {
				section = SECTION::VOICE;
			} else if (::strncmp(buffer, "[Info]", 6U) == 0) {
				section = SECTION::INFO;
			} else if (::strncmp(buffer, "[XLX Network]", 13U) == 0) {
				section = SECTION::XLX_NETWORK;
			} else if (::strncmp(buffer, "[DMR Network", 12U) == 0) {
				section = SECTION::DMR_NETWORK_N;
				lastDmrNet = addDMRNetwork();
			} else if (::strncmp(buffer, "[GPSD]", 6U) == 0) {
				section = SECTION::GPSD;
			} else if (::strncmp(buffer, "[APRS]", 6U) == 0) {
				section = SECTION::APRS;
			} else if (::strncmp(buffer, "[Dynamic TG Control]", 20U) == 0) {
				section = SECTION::DYNAMIC_TG_CONTROL;
			} else if (::strncmp(buffer, "[Remote Control]", 16U) == 0) {
				section = SECTION::REMOTE_CONTROL;
			} else {
				section = SECTION::NONE;
			}

			continue;
		}

		char* key = ::strtok(buffer, " \t=\r\n");
		if (key == nullptr)
			continue;

		char* value = ::strtok(nullptr, "\r\n");
		if (value == nullptr)
			continue;

		// Remove quotes from the value
		size_t len = ::strlen(value);
		if (len > 1U && *value == '"' && value[len - 1U] == '"') {
			value[len - 1U] = '\0';
			value++;
		} else {
			char *p;

			// if value is not quoted, remove after # (to make comment)
			if ((p = strchr(value, '#')) != nullptr)
				*p = '\0';

			// remove trailing tab/space
			for (p = value + strlen(value) - 1U; p >= value && (*p == '\t' || *p == ' '); p--)
				*p = '\0';
		}

		if (section == SECTION::GENERAL) {
			if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
			else if (::strcmp(key, "Timeout") == 0)
				m_rfTimeout = m_netTimeout = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RFTimeout") == 0)
				m_rfTimeout = (unsigned int)::atoi(value);
			else if (::strcmp(key, "NetTimeout") == 0)
				m_netTimeout = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RptAddress") == 0)
				m_rptAddress = value;
			else if (::strcmp(key, "RptPort") == 0)
				m_rptPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "LocalAddress") == 0)
				m_localAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_localPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "RuleTrace") == 0)
				m_ruleTrace = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_debug = ::atoi(value) == 1;
		} else if (section == SECTION::LOG) {
			if (::strcmp(key, "FilePath") == 0)
				m_logFilePath = value;
			else if (::strcmp(key, "FileRoot") == 0)
				m_logFileRoot = value;
			else if (::strcmp(key, "FileLevel") == 0)
				m_logFileLevel = (unsigned int)::atoi(value);
			else if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (unsigned int)::atoi(value);
			else if (::strcmp(key, "FileRotate") == 0)
				m_logFileRotate = ::atoi(value) == 1;
		} else if (section == SECTION::VOICE) {
			if (::strcmp(key, "Enabled") == 0)
				m_voiceEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Language") == 0)
				m_voiceLanguage = value;
			else if (::strcmp(key, "Directory") == 0)
				m_voiceDirectory = value;
		} else if (section == SECTION::INFO) {
			if (::strcmp(key, "Latitude") == 0)
				m_infoLatitude = float(::atof(value));
			else if (::strcmp(key, "Longitude") == 0)
				m_infoLongitude = float(::atof(value));
			else if (::strcmp(key, "Height") == 0)
				m_infoHeight = ::atoi(value);
			else if (::strcmp(key, "Location") == 0)
				m_infoLocation = value;
			else if (::strcmp(key, "Description") == 0)
				m_infoDescription = value;
			else if (::strcmp(key, "URL") == 0)
				m_infoURL = value;
		} else if (section == SECTION::XLX_NETWORK) {
			if (::strcmp(key, "Enabled") == 0)
				m_xlxNetworkEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Id") == 0)
				m_xlxNetworkId = (unsigned int)::atoi(value);
			else if (::strcmp(key, "File") == 0)
				m_xlxNetworkFile = value;
			else if (::strcmp(key, "ReloadTime") == 0)
				m_xlxNetworkReloadTime = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Port") == 0)
				m_xlxNetworkPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_xlxNetworkPassword = value;
			else if (::strcmp(key, "Local") == 0)
				m_xlxNetworkLocal = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Slot") == 0)
				m_xlxNetworkSlot = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TG") == 0)
				m_xlxNetworkTG = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Base") == 0)
				m_xlxNetworkBase = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Startup") == 0) {
				char buffer[16];
				char *p = buffer;

				// Right align, then pads with zeros, as XLX IDs are 3 characters length
				snprintf(buffer, sizeof(buffer), "%3s", value);
				while ((*p != '\0') && (*p == ' '))
					 *p++ = '0';

				m_xlxNetworkStartup = std::string(buffer);
			}
			else if (::strcmp(key, "Relink") == 0)
				m_xlxNetworkRelink = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Debug") == 0)
				m_xlxNetworkDebug = ::atoi(value) == 1;
			else if (::strcmp(key, "UserControl") == 0)
				m_xlxNetworkUserControl = ::atoi(value) ==1;
			else if (::strcmp(key, "Module") == 0)
				m_xlxNetworkModule = ::toupper(value[0]);
		} else if (section == SECTION::DMR_NETWORK_N) {
			assert(lastDmrNet != nullptr);
			if (::strcmp(key, "Enabled") == 0)
				lastDmrNet->m_Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				lastDmrNet->m_Name = value;
			else if (::strcmp(key, "Id") == 0)
				lastDmrNet->m_Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				lastDmrNet->m_Address = value;
			else if (::strcmp(key, "Port") == 0)
				lastDmrNet->m_Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				lastDmrNet->m_Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				lastDmrNet->m_Password = value;
			else if (::strcmp(key, "Options") == 0)
				lastDmrNet->m_Options = value;
			else if (::strcmp(key, "Location") == 0)
				lastDmrNet->m_Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				lastDmrNet->m_Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(nullptr, ", ");
				char* p3 = ::strtok(nullptr, ", ");
				char* p4 = ::strtok(nullptr, ", ");
				char* p5 = ::strtok(nullptr, " \r\n");
				if (p1 != nullptr && p2 != nullptr && p3 != nullptr && p4 != nullptr && p5 != nullptr) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					lastDmrNet->m_TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(nullptr, ", ");
				char* p3 = ::strtok(nullptr, ", ");
				char* p4 = ::strtok(nullptr, ", ");
				char* p5 = ::strtok(nullptr, " \r\n");
				if (p1 != nullptr && p2 != nullptr && p3 != nullptr && p4 != nullptr && p5 != nullptr) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					lastDmrNet->m_PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(nullptr, ", ");
				char* p3 = ::strtok(nullptr, ", ");
				char* p4 = ::strtok(nullptr, ", \r\n");
				if (p1 != nullptr && p2 != nullptr && p3 != nullptr && p4 != nullptr) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(nullptr, " \r\n");
					rewrite.m_range    = p5 != nullptr ? ::atoi(p5) : 1;
					lastDmrNet->m_TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(nullptr, ", ");
				char* p3 = ::strtok(nullptr, ", ");
				char* p4 = ::strtok(nullptr, ", ");
				char* p5 = ::strtok(nullptr, " \r\n");
				if (p1 != nullptr && p2 != nullptr && p3 != nullptr && p4 != nullptr && p5 != nullptr) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					lastDmrNet->m_SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(nullptr, ", ");
				char* p3 = ::strtok(nullptr, ", ");
				char* p4 = ::strtok(nullptr, ", ");
				char* p5 = ::strtok(nullptr, ", ");
				char* p6 = ::strtok(nullptr, ", \r\n");
				char* p;
				while ((p = ::strtok(nullptr, ", \r\n")) != nullptr)
					p7.push_back(p);
				if (p1 != nullptr && p2 != nullptr && p3 != nullptr && p4 != nullptr && p5 != nullptr && p6 != nullptr) {
					CTGDynRewriteStruct rewrite;
					rewrite.m_slot     = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_discPC   = ::atoi(p3);
					rewrite.m_statusPC = ::atoi(p4);
					rewrite.m_toTG     = ::atoi(p5);
					rewrite.m_range    = ::atoi(p6);
					for (std::vector<char*>::const_iterator it = p7.cbegin(); it != p7.cend(); ++it) {
						unsigned int tg = ::atoi(*it);
						rewrite.m_exclTGs.push_back(tg);
					}
					lastDmrNet->m_TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(nullptr, " \r\n");
				if (rfId != nullptr && netId != nullptr) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					lastDmrNet->m_IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				lastDmrNet->m_PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				lastDmrNet->m_PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION::GPSD) {
			if (::strcmp(key, "Enable") == 0)
				m_gpsdEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_gpsdAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_gpsdPort = value;
		} else if (section == SECTION::APRS) {
			if (::strcmp(key, "Enable") == 0)
				m_aprsEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_aprsAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_aprsPort = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Suffix") == 0)
				m_aprsSuffix = value;
			else if (::strcmp(key, "Description") == 0)
				m_aprsDescription = value;
						else if (::strcmp(key, "Symbol") == 0)
								m_aprsSymbol = value;
		} else if (section == SECTION::DYNAMIC_TG_CONTROL) {
			if (::strcmp(key, "Enabled") == 0)
				m_dynamicTGControlEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Port") == 0)
				m_dynamicTGControlPort = (unsigned short)::atoi(value);
		} else if (section == SECTION::REMOTE_CONTROL) {
			if (::strcmp(key, "Enable") == 0)
				m_remoteControlEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_remoteControlAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_remoteControlPort = (unsigned short)::atoi(value);
		}
	}

	::fclose(fp);

	return true;
}

bool CConf::getDaemon() const
{
	return m_daemon;
}

std::string CConf::getRptAddress() const
{
	return m_rptAddress;
}

unsigned short CConf::getRptPort() const
{
	return m_rptPort;
}

std::string CConf::getLocalAddress() const
{
	return m_localAddress;
}

unsigned short CConf::getLocalPort() const
{
	return m_localPort;
}

unsigned int CConf::getRFTimeout() const
{
	return m_rfTimeout;
}

unsigned int CConf::getNetTimeout() const
{
	return m_netTimeout;
}

bool CConf::getRuleTrace() const
{
	return m_ruleTrace;
}

bool CConf::getDebug() const
{
	return m_debug;
}

unsigned int CConf::getLogDisplayLevel() const
{
	return m_logDisplayLevel;
}

unsigned int CConf::getLogFileLevel() const
{
	return m_logFileLevel;
}

std::string CConf::getLogFilePath() const
{
	return m_logFilePath;
}

std::string CConf::getLogFileRoot() const
{
	return m_logFileRoot;
}

bool CConf::getLogFileRotate() const
{
	return m_logFileRotate;
}

bool CConf::getVoiceEnabled() const
{
	return m_voiceEnabled;
}

std::string CConf::getVoiceLanguage() const
{
	return m_voiceLanguage;
}

std::string CConf::getVoiceDirectory() const
{
	return m_voiceDirectory;
}

float CConf::getInfoLatitude() const
{
	return m_infoLatitude;
}

float CConf::getInfoLongitude() const
{
	return m_infoLongitude;
}

int CConf::getInfoHeight() const
{
	return m_infoHeight;
}

std::string CConf::getInfoLocation() const
{
	return m_infoLocation;
}

std::string CConf::getInfoDescription() const
{
	return m_infoDescription;
}

std::string CConf::getInfoURL() const
{
	return m_infoURL;
}

bool CConf::getXLXNetworkEnabled() const
{
	return m_xlxNetworkEnabled;
}

unsigned int CConf::getXLXNetworkId() const
{
	return m_xlxNetworkId;
}

std::string CConf::getXLXNetworkFile() const
{
	return m_xlxNetworkFile;
}

unsigned int CConf::getXLXNetworkReloadTime() const
{
	return m_xlxNetworkReloadTime;
}

unsigned short CConf::getXLXNetworkPort() const
{
	return m_xlxNetworkPort;
}

std::string CConf::getXLXNetworkPassword() const
{
	return m_xlxNetworkPassword;
}

unsigned short CConf::getXLXNetworkLocal() const
{
	return m_xlxNetworkLocal;
}

unsigned int CConf::getXLXNetworkSlot() const
{
	return m_xlxNetworkSlot;
}

unsigned int CConf::getXLXNetworkTG() const
{
	return m_xlxNetworkTG;
}

unsigned int CConf::getXLXNetworkBase() const
{
	return m_xlxNetworkBase;
}

std::string CConf::getXLXNetworkStartup() const
{
	return m_xlxNetworkStartup;
}

unsigned int CConf::getXLXNetworkRelink() const
{
	return m_xlxNetworkRelink;
}

bool CConf::getXLXNetworkDebug() const
{
	return m_xlxNetworkDebug;
}
bool CConf::getXLXNetworkUserControl() const
{
	return m_xlxNetworkUserControl;
}
char CConf::getXLXNetworkModule() const
{
	return m_xlxNetworkModule;
}

CDMRNetConfStruct *CConf::addDMRNetwork()
{
	m_dmrNetworks.emplace_back();
	CDMRNetConfStruct& dmrNetwork = m_dmrNetworks.back();

	// set defaults
	dmrNetwork.m_Enabled = false;
	dmrNetwork.m_Id = 0U;
	dmrNetwork.m_Port = 0U;
	dmrNetwork.m_Local = 0U;
	dmrNetwork.m_Location = true;
	dmrNetwork.m_Debug = false;

	return &dmrNetwork;
}

unsigned int CConf::getDMRNetworksCount() const
{
	return m_dmrNetworks.size();
}

bool CConf::getDMRNetworkEnabled(unsigned int index) const
{
	return m_dmrNetworks[index].m_Enabled;
}

std::string CConf::getDMRNetworkName(unsigned int index) const
{
	if (m_dmrNetworks[index].m_Name.empty())
		return "DMR-" + std::to_string(index + 1);
	else
		return m_dmrNetworks[index].m_Name;
}

unsigned int CConf::getDMRNetworkId(unsigned int index) const
{
	return m_dmrNetworks[index].m_Id;
}

std::string CConf::getDMRNetworkAddress(unsigned int index) const
{
	return m_dmrNetworks[index].m_Address;
}

unsigned short CConf::getDMRNetworkPort(unsigned int index) const
{
	return m_dmrNetworks[index].m_Port;
}

unsigned short CConf::getDMRNetworkLocal(unsigned int index) const
{
	return m_dmrNetworks[index].m_Local;
}

std::string CConf::getDMRNetworkPassword(unsigned int index) const
{
	return m_dmrNetworks[index].m_Password;
}

std::string CConf::getDMRNetworkOptions(unsigned int index) const
{
	return m_dmrNetworks[index].m_Options;
}

bool CConf::getDMRNetworkLocation(unsigned int index) const
{
	return m_dmrNetworks[index].m_Location;
}

bool CConf::getDMRNetworkDebug(unsigned int index) const
{
	return m_dmrNetworks[index].m_Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetworkTGRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetworkPCRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetworkTypeRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetworkSrcRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetworkTGDynRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetworkIdRewrites(unsigned int index) const
{
	return m_dmrNetworks[index].m_IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetworkPassAllPC(unsigned int index) const
{
	return m_dmrNetworks[index].m_PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetworkPassAllTG(unsigned int index) const
{
	return m_dmrNetworks[index].m_PassAllTG;
}

bool CConf::getGPSDEnabled() const
{
	return m_gpsdEnabled;
}

std::string CConf::getGPSDAddress() const
{
	return m_gpsdAddress;
}

std::string CConf::getGPSDPort() const
{
	return m_gpsdPort;
}

bool CConf::getAPRSEnabled() const
{
	return m_aprsEnabled;
}

std::string CConf::getAPRSAddress() const
{
	return m_aprsAddress;
}

unsigned short CConf::getAPRSPort() const
{
	return m_aprsPort;
}

std::string CConf::getAPRSSuffix() const
{
	return m_aprsSuffix;
}

std::string CConf::getAPRSDescription() const
{
	return m_aprsDescription;
}

std::string CConf::getAPRSSymbol() const
{
	return m_aprsSymbol;
}

bool CConf::getDynamicTGControlEnabled() const
{
	return m_dynamicTGControlEnabled;
}

unsigned short CConf::getDynamicTGControlPort() const
{
	return m_dynamicTGControlPort;
}

bool CConf::getRemoteControlEnabled() const
{
	return m_remoteControlEnabled;
}

std::string CConf::getRemoteControlAddress() const
{
	return m_remoteControlAddress;
}

unsigned short CConf::getRemoteControlPort() const
{
	return m_remoteControlPort;
}
