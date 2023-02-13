/*
 *   Copyright (C) 2015-2020 by Jonathan Naylor G4KLX
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

const int BUFFER_SIZE = 500;

enum SECTION {
	SECTION_NONE,
	SECTION_GENERAL,
	SECTION_LOG,
	SECTION_VOICE,
	SECTION_INFO,
	SECTION_DMR_NETWORK_1,
	SECTION_DMR_NETWORK_2,
	SECTION_DMR_NETWORK_3,
	SECTION_DMR_NETWORK_4,
	SECTION_DMR_NETWORK_5,
	SECTION_XLX_NETWORK,
	SECTION_GPSD,
	SECTION_APRS,
	SECTION_DYNAMIC_TG_CONTROL,
	SECTION_REMOTE_CONTROL
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
m_dmrNetwork1Enabled(false),
m_dmrNetwork1Name(),
m_dmrNetwork1Id(0U),
m_dmrNetwork1Address(),
m_dmrNetwork1Port(0U),
m_dmrNetwork1Local(0U),
m_dmrNetwork1Password(),
m_dmrNetwork1Options(),
m_dmrNetwork1Location(true),
m_dmrNetwork1Debug(false),
m_dmrNetwork1TGRewrites(),
m_dmrNetwork1PCRewrites(),
m_dmrNetwork1TypeRewrites(),
m_dmrNetwork1SrcRewrites(),
m_dmrNetwork1TGDynRewrites(),
m_dmrNetwork1IdRewrites(),
m_dmrNetwork1PassAllPC(),
m_dmrNetwork1PassAllTG(),
m_dmrNetwork2Enabled(false),
m_dmrNetwork2Name(),
m_dmrNetwork2Id(0U),
m_dmrNetwork2Address(),
m_dmrNetwork2Port(0U),
m_dmrNetwork2Local(0U),
m_dmrNetwork2Password(),
m_dmrNetwork2Options(),
m_dmrNetwork2Location(true),
m_dmrNetwork2Debug(false),
m_dmrNetwork2TGRewrites(),
m_dmrNetwork2PCRewrites(),
m_dmrNetwork2TypeRewrites(),
m_dmrNetwork2SrcRewrites(),
m_dmrNetwork2TGDynRewrites(),
m_dmrNetwork2IdRewrites(),
m_dmrNetwork2PassAllPC(),
m_dmrNetwork2PassAllTG(),
m_dmrNetwork3Enabled(false),
m_dmrNetwork3Name(),
m_dmrNetwork3Id(0U),
m_dmrNetwork3Address(),
m_dmrNetwork3Port(0U),
m_dmrNetwork3Local(0U),
m_dmrNetwork3Password(),
m_dmrNetwork3Options(),
m_dmrNetwork3Location(true),
m_dmrNetwork3Debug(false),
m_dmrNetwork3TGRewrites(),
m_dmrNetwork3PCRewrites(),
m_dmrNetwork3TypeRewrites(),
m_dmrNetwork3SrcRewrites(),
m_dmrNetwork3TGDynRewrites(),
m_dmrNetwork3IdRewrites(),
m_dmrNetwork3PassAllPC(),
m_dmrNetwork3PassAllTG(),
m_dmrNetwork4Enabled(false),
m_dmrNetwork4Name(),
m_dmrNetwork4Id(0U),
m_dmrNetwork4Address(),
m_dmrNetwork4Port(0U),
m_dmrNetwork4Local(0U),
m_dmrNetwork4Password(),
m_dmrNetwork4Options(),
m_dmrNetwork4Location(true),
m_dmrNetwork4Debug(false),
m_dmrNetwork4TGRewrites(),
m_dmrNetwork4PCRewrites(),
m_dmrNetwork4TypeRewrites(),
m_dmrNetwork4SrcRewrites(),
m_dmrNetwork4TGDynRewrites(),
m_dmrNetwork4IdRewrites(),
m_dmrNetwork4PassAllPC(),
m_dmrNetwork4PassAllTG(),
m_dmrNetwork5Enabled(false),
m_dmrNetwork5Name(),
m_dmrNetwork5Id(0U),
m_dmrNetwork5Address(),
m_dmrNetwork5Port(0U),
m_dmrNetwork5Local(0U),
m_dmrNetwork5Password(),
m_dmrNetwork5Options(),
m_dmrNetwork5Location(true),
m_dmrNetwork5Debug(false),
m_dmrNetwork5TGRewrites(),
m_dmrNetwork5PCRewrites(),
m_dmrNetwork5TypeRewrites(),
m_dmrNetwork5SrcRewrites(),
m_dmrNetwork5TGDynRewrites(),
m_dmrNetwork5IdRewrites(),
m_dmrNetwork5PassAllPC(),
m_dmrNetwork5PassAllTG(),
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
  if (fp == NULL) {
    ::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
    return false;
  }

  SECTION section = SECTION_NONE;

  char buffer[BUFFER_SIZE];
  while (::fgets(buffer, BUFFER_SIZE, fp) != NULL) {
    if (buffer[0U] == '#')
      continue;

	if (buffer[0U] == '[') {
		if (::strncmp(buffer, "[General]", 9U) == 0)
			section = SECTION_GENERAL;
		else if (::strncmp(buffer, "[Log]", 5U) == 0)
			section = SECTION_LOG;
		else if (::strncmp(buffer, "[Voice]", 7U) == 0)
			section = SECTION_VOICE;
		else if (::strncmp(buffer, "[Info]", 6U) == 0)
			section = SECTION_INFO;
		else if (::strncmp(buffer, "[XLX Network]", 13U) == 0)
			section = SECTION_XLX_NETWORK;
		else if (::strncmp(buffer, "[DMR Network 1]", 15U) == 0)
			section = SECTION_DMR_NETWORK_1;
		else if (::strncmp(buffer, "[DMR Network 2]", 15U) == 0)
			section = SECTION_DMR_NETWORK_2;
		else if (::strncmp(buffer, "[DMR Network 3]", 15U) == 0)
			section = SECTION_DMR_NETWORK_3;
		else if (::strncmp(buffer, "[DMR Network 4]", 15U) == 0)
			section = SECTION_DMR_NETWORK_4;
		else if (::strncmp(buffer, "[DMR Network 5]", 15U) == 0)
			section = SECTION_DMR_NETWORK_5;
		else if (::strncmp(buffer, "[GPSD]", 6U) == 0)
			section = SECTION_GPSD;
		else if (::strncmp(buffer, "[APRS]", 6U) == 0)
			section = SECTION_APRS;
		else if (::strncmp(buffer, "[Dynamic TG Control]", 20U) == 0)
			section = SECTION_DYNAMIC_TG_CONTROL;
		else if (::strncmp(buffer, "[Remote Control]", 16U) == 0)
			section = SECTION_REMOTE_CONTROL;
		else
			section = SECTION_NONE;

		continue;
	}

	char* key = ::strtok(buffer, " \t=\r\n");
	if (key == NULL)
		continue;

	char* value = ::strtok(NULL, "\r\n");
	if (value == NULL)
		continue;

	// Remove quotes from the value
	size_t len = ::strlen(value);
	if (len > 1U && *value == '"' && value[len - 1U] == '"') {
		value[len - 1U] = '\0';
		value++;
	} else {
		char *p;

		// if value is not quoted, remove after # (to make comment)
		if ((p = strchr(value, '#')) != NULL)
			*p = '\0';

		// remove trailing tab/space
		for (p = value + strlen(value) - 1U; p >= value && (*p == '\t' || *p == ' '); p--)
			*p = '\0';
	}

	if (section == SECTION_GENERAL) {
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
		} else if (section == SECTION_LOG) {
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
		} else if (section == SECTION_VOICE) {
			if (::strcmp(key, "Enabled") == 0)
				m_voiceEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Language") == 0)
				m_voiceLanguage = value;
			else if (::strcmp(key, "Directory") == 0)
				m_voiceDirectory = value;
		} else if (section == SECTION_INFO) {
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
		} else if (section == SECTION_XLX_NETWORK) {
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
		} else if (section == SECTION_DMR_NETWORK_1) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork1Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				m_dmrNetwork1Name = value;
			else if (::strcmp(key, "Id") == 0)
				m_dmrNetwork1Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork1Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork1Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork1Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork1Password = value;
			else if (::strcmp(key, "Options") == 0)
				m_dmrNetwork1Options = value;
			else if (::strcmp(key, "Location") == 0)
				m_dmrNetwork1Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork1Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork1TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork1PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(NULL, " \r\n");
					rewrite.m_range    = p5 != NULL ? ::atoi(p5) : 1;
					m_dmrNetwork1TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork1SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, ", ");
				char* p6 = ::strtok(NULL, ", \r\n");
				char* p;
				while ((p = ::strtok(NULL, ", \r\n")) != NULL)
					p7.push_back(p);
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL && p6 != NULL) {
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
					m_dmrNetwork1TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(NULL, " \r\n");
				if (rfId != NULL && netId != NULL) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					m_dmrNetwork1IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork1PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork1PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_DMR_NETWORK_2) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork2Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				m_dmrNetwork2Name = value;
			else if (::strcmp(key, "Id") == 0)
				m_dmrNetwork2Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork2Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork2Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork2Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork2Password = value;
			else if (::strcmp(key, "Options") == 0)
				m_dmrNetwork2Options = value;
			else if (::strcmp(key, "Location") == 0)
				m_dmrNetwork2Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork2Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork2TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork2PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(NULL, " \r\n");
					rewrite.m_range    = p5 != NULL ? ::atoi(p5) : 1;
					m_dmrNetwork2TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork2SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, ", ");
				char* p6 = ::strtok(NULL, ", \r\n");
				char* p;
				while ((p = ::strtok(NULL, ", \r\n")) != NULL)
					p7.push_back(p);
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL && p6 != NULL) {
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
					m_dmrNetwork2TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(NULL, " \r\n");
				if (rfId != NULL && netId != NULL) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					m_dmrNetwork2IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork2PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork2PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_DMR_NETWORK_3) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork3Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				m_dmrNetwork3Name = value;
			else if (::strcmp(key, "Id") == 0)
				m_dmrNetwork3Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork3Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork3Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork3Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork3Password = value;
			else if (::strcmp(key, "Options") == 0)
				m_dmrNetwork3Options = value;
			else if (::strcmp(key, "Location") == 0)
				m_dmrNetwork3Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork3Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork3TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork3PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(NULL, " \r\n");
					rewrite.m_range    = p5 != NULL ? ::atoi(p5) : 1;
					m_dmrNetwork3TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork3SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, ", ");
				char* p6 = ::strtok(NULL, ", \r\n");
				char* p;
				while ((p = ::strtok(NULL, ", \r\n")) != NULL)
					p7.push_back(p);
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL && p6 != NULL) {
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
					m_dmrNetwork3TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(NULL, " \r\n");
				if (rfId != NULL && netId != NULL) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					m_dmrNetwork3IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork3PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork3PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_DMR_NETWORK_4) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork4Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				m_dmrNetwork4Name = value;
			else if (::strcmp(key, "Id") == 0)
				m_dmrNetwork4Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork4Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork4Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork4Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork4Password = value;
			else if (::strcmp(key, "Options") == 0)
				m_dmrNetwork4Options = value;
			else if (::strcmp(key, "Location") == 0)
				m_dmrNetwork4Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork4Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork4TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork4PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(NULL, " \r\n");
					rewrite.m_range    = p5 != NULL ? ::atoi(p5) : 1;
					m_dmrNetwork4TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork4SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, ", ");
				char* p6 = ::strtok(NULL, ", \r\n");
				char* p;
				while ((p = ::strtok(NULL, ", \r\n")) != NULL)
					p7.push_back(p);
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL && p6 != NULL) {
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
					m_dmrNetwork4TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(NULL, " \r\n");
				if (rfId != NULL && netId != NULL) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					m_dmrNetwork4IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork4PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork4PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_DMR_NETWORK_5) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork5Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Name") == 0)
				m_dmrNetwork5Name = value;
			else if (::strcmp(key, "Id") == 0)
				m_dmrNetwork5Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork5Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork5Port = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork5Local = (unsigned short)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork5Password = value;
			else if (::strcmp(key, "Options") == 0)
				m_dmrNetwork5Options = value;
			else if (::strcmp(key, "Location") == 0)
				m_dmrNetwork5Location = ::atoi(value) == 1;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork5Debug = ::atoi(value) == 1;
			else if (::strncmp(key, "TGRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CTGRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork5TGRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PCRewrite", 9U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CPCRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork5PCRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TypeRewrite", 11U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
					char* p5 = ::strtok(NULL, " \r\n");
					rewrite.m_range    = p5 != NULL ? ::atoi(p5) : 1;
					m_dmrNetwork5TypeRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "SrcRewrite", 10U) == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL) {
					CSrcRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromId   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toTG     = ::atoi(p4);
					rewrite.m_range    = ::atoi(p5);
					m_dmrNetwork5SrcRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "TGDynRewrite", 12U) == 0) {
				std::vector<char*> p7;
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, ", ");
				char* p5 = ::strtok(NULL, ", ");
				char* p6 = ::strtok(NULL, ", \r\n");
				char* p;
				while ((p = ::strtok(NULL, ", \r\n")) != NULL)
					p7.push_back(p);
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL && p6 != NULL) {
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
					m_dmrNetwork5TGDynRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "IdRewrite", 9U) == 0) {
				char* rfId = ::strtok(value, ", ");
				char* netId = ::strtok(NULL, " \r\n");
				if (rfId != NULL && netId != NULL) {
					CIdRewriteStruct rewrite;
					rewrite.m_rfId  = ::atoi(rfId);
					rewrite.m_netId = ::atoi(netId);
					m_dmrNetwork5IdRewrites.push_back(rewrite);
				}
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork5PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork5PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_GPSD) {
			if (::strcmp(key, "Enable") == 0)
				m_gpsdEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_gpsdAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_gpsdPort = value;
		} else if (section == SECTION_APRS) {
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
		} else if (section == SECTION_DYNAMIC_TG_CONTROL) {
			if (::strcmp(key, "Enabled") == 0)
				m_dynamicTGControlEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Port") == 0)
				m_dynamicTGControlPort = (unsigned short)::atoi(value);
		} else if (section == SECTION_REMOTE_CONTROL) {
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

bool CConf::getDMRNetwork1Enabled() const
{
	return m_dmrNetwork1Enabled;
}

std::string CConf::getDMRNetwork1Name() const
{
	if (m_dmrNetwork1Name.empty())
		return "DMR-1";
	else
		return m_dmrNetwork1Name;
}

unsigned int CConf::getDMRNetwork1Id() const
{
	return m_dmrNetwork1Id;
}

std::string CConf::getDMRNetwork1Address() const
{
	return m_dmrNetwork1Address;
}

unsigned short CConf::getDMRNetwork1Port() const
{
	return m_dmrNetwork1Port;
}

unsigned short CConf::getDMRNetwork1Local() const
{
	return m_dmrNetwork1Local;
}

std::string CConf::getDMRNetwork1Password() const
{
	return m_dmrNetwork1Password;
}

std::string CConf::getDMRNetwork1Options() const
{
	return m_dmrNetwork1Options;
}

bool CConf::getDMRNetwork1Location() const
{
	return m_dmrNetwork1Location;
}

bool CConf::getDMRNetwork1Debug() const
{
	return m_dmrNetwork1Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetwork1TGRewrites() const
{
	return m_dmrNetwork1TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetwork1PCRewrites() const
{
	return m_dmrNetwork1PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetwork1TypeRewrites() const
{
	return m_dmrNetwork1TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetwork1SrcRewrites() const
{
	return m_dmrNetwork1SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetwork1TGDynRewrites() const
{
	return m_dmrNetwork1TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetwork1IdRewrites() const
{
	return m_dmrNetwork1IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetwork1PassAllPC() const
{
	return m_dmrNetwork1PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork1PassAllTG() const
{
	return m_dmrNetwork1PassAllTG;
}

bool CConf::getDMRNetwork2Enabled() const
{
	return m_dmrNetwork2Enabled;
}

std::string CConf::getDMRNetwork2Name() const
{
	if (m_dmrNetwork2Name.empty())
		return "DMR-2";
	else
		return m_dmrNetwork2Name;
}

unsigned int CConf::getDMRNetwork2Id() const
{
	return m_dmrNetwork2Id;
}

std::string CConf::getDMRNetwork2Address() const
{
	return m_dmrNetwork2Address;
}

unsigned short CConf::getDMRNetwork2Port() const
{
	return m_dmrNetwork2Port;
}

unsigned short CConf::getDMRNetwork2Local() const
{
	return m_dmrNetwork2Local;
}

std::string CConf::getDMRNetwork2Password() const
{
	return m_dmrNetwork2Password;
}

std::string CConf::getDMRNetwork2Options() const
{
	return m_dmrNetwork2Options;
}

bool CConf::getDMRNetwork2Location() const
{
	return m_dmrNetwork2Location;
}

bool CConf::getDMRNetwork2Debug() const
{
	return m_dmrNetwork2Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetwork2TGRewrites() const
{
	return m_dmrNetwork2TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetwork2PCRewrites() const
{
	return m_dmrNetwork2PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetwork2TypeRewrites() const
{
	return m_dmrNetwork2TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetwork2SrcRewrites() const
{
	return m_dmrNetwork2SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetwork2TGDynRewrites() const
{
	return m_dmrNetwork2TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetwork2IdRewrites() const
{
	return m_dmrNetwork2IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetwork2PassAllPC() const
{
	return m_dmrNetwork2PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork2PassAllTG() const
{
	return m_dmrNetwork2PassAllTG;
}

bool CConf::getDMRNetwork3Enabled() const
{
	return m_dmrNetwork3Enabled;
}

std::string CConf::getDMRNetwork3Name() const
{
	if (m_dmrNetwork3Name.empty())
		return "DMR-3";
	else
		return m_dmrNetwork3Name;
}

unsigned int CConf::getDMRNetwork3Id() const
{
	return m_dmrNetwork3Id;
}

std::string CConf::getDMRNetwork3Address() const
{
	return m_dmrNetwork3Address;
}

unsigned short CConf::getDMRNetwork3Port() const
{
	return m_dmrNetwork3Port;
}

unsigned short CConf::getDMRNetwork3Local() const
{
	return m_dmrNetwork3Local;
}

std::string CConf::getDMRNetwork3Password() const
{
	return m_dmrNetwork3Password;
}

std::string CConf::getDMRNetwork3Options() const
{
	return m_dmrNetwork3Options;
}

bool CConf::getDMRNetwork3Location() const
{
	return m_dmrNetwork3Location;
}

bool CConf::getDMRNetwork3Debug() const
{
	return m_dmrNetwork3Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetwork3TGRewrites() const
{
	return m_dmrNetwork3TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetwork3PCRewrites() const
{
	return m_dmrNetwork3PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetwork3TypeRewrites() const
{
	return m_dmrNetwork3TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetwork3SrcRewrites() const
{
	return m_dmrNetwork3SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetwork3TGDynRewrites() const
{
	return m_dmrNetwork3TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetwork3IdRewrites() const
{
	return m_dmrNetwork3IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetwork3PassAllPC() const
{
	return m_dmrNetwork3PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork3PassAllTG() const
{
	return m_dmrNetwork3PassAllTG;
}

bool CConf::getDMRNetwork4Enabled() const
{
	return m_dmrNetwork4Enabled;
}

std::string CConf::getDMRNetwork4Name() const
{
	if (m_dmrNetwork4Name.empty())
		return "DMR-4";
	else
		return m_dmrNetwork4Name;
}

unsigned int CConf::getDMRNetwork4Id() const
{
	return m_dmrNetwork4Id;
}

std::string CConf::getDMRNetwork4Address() const
{
	return m_dmrNetwork4Address;
}

unsigned short CConf::getDMRNetwork4Port() const
{
	return m_dmrNetwork4Port;
}

unsigned short CConf::getDMRNetwork4Local() const
{
	return m_dmrNetwork4Local;
}

std::string CConf::getDMRNetwork4Password() const
{
	return m_dmrNetwork4Password;
}

std::string CConf::getDMRNetwork4Options() const
{
	return m_dmrNetwork4Options;
}

bool CConf::getDMRNetwork4Location() const
{
	return m_dmrNetwork4Location;
}

bool CConf::getDMRNetwork4Debug() const
{
	return m_dmrNetwork4Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetwork4TGRewrites() const
{
	return m_dmrNetwork4TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetwork4PCRewrites() const
{
	return m_dmrNetwork4PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetwork4TypeRewrites() const
{
	return m_dmrNetwork4TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetwork4SrcRewrites() const
{
	return m_dmrNetwork4SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetwork4TGDynRewrites() const
{
	return m_dmrNetwork4TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetwork4IdRewrites() const
{
	return m_dmrNetwork4IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetwork4PassAllPC() const
{
	return m_dmrNetwork4PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork4PassAllTG() const
{
	return m_dmrNetwork4PassAllTG;
}

bool CConf::getDMRNetwork5Enabled() const
{
	return m_dmrNetwork5Enabled;
}

std::string CConf::getDMRNetwork5Name() const
{
	if (m_dmrNetwork5Name.empty())
		return "DMR-5";
	else
		return m_dmrNetwork5Name;
}

unsigned int CConf::getDMRNetwork5Id() const
{
	return m_dmrNetwork5Id;
}

std::string CConf::getDMRNetwork5Address() const
{
	return m_dmrNetwork5Address;
}

unsigned short CConf::getDMRNetwork5Port() const
{
	return m_dmrNetwork5Port;
}

unsigned short CConf::getDMRNetwork5Local() const
{
	return m_dmrNetwork5Local;
}

std::string CConf::getDMRNetwork5Password() const
{
	return m_dmrNetwork5Password;
}

std::string CConf::getDMRNetwork5Options() const
{
	return m_dmrNetwork5Options;
}

bool CConf::getDMRNetwork5Location() const
{
	return m_dmrNetwork5Location;
}

bool CConf::getDMRNetwork5Debug() const
{
	return m_dmrNetwork5Debug;
}

std::vector<CTGRewriteStruct> CConf::getDMRNetwork5TGRewrites() const
{
	return m_dmrNetwork5TGRewrites;
}

std::vector<CPCRewriteStruct> CConf::getDMRNetwork5PCRewrites() const
{
	return m_dmrNetwork5PCRewrites;
}

std::vector<CTypeRewriteStruct> CConf::getDMRNetwork5TypeRewrites() const
{
	return m_dmrNetwork5TypeRewrites;
}

std::vector<CSrcRewriteStruct> CConf::getDMRNetwork5SrcRewrites() const
{
	return m_dmrNetwork5SrcRewrites;
}

std::vector<CTGDynRewriteStruct> CConf::getDMRNetwork5TGDynRewrites() const
{
	return m_dmrNetwork5TGDynRewrites;
}

std::vector<CIdRewriteStruct> CConf::getDMRNetwork5IdRewrites() const
{
	return m_dmrNetwork5IdRewrites;
}

std::vector<unsigned int> CConf::getDMRNetwork5PassAllPC() const
{
	return m_dmrNetwork5PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork5PassAllTG() const
{
	return m_dmrNetwork5PassAllTG;
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
