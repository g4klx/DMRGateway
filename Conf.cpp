/*
 *   Copyright (C) 2015,2016,2017,2018 by Jonathan Naylor G4KLX
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
	SECTION_XLX_NETWORK
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
m_infoEnabled(false),
m_infoRXFrequency(0U),
m_infoTXFrequency(0U),
m_infoPower(0U),
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
m_dmrNetwork3PassAllPC(),
m_dmrNetwork3PassAllTG(),
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
m_xlxNetworkStartup(4000U),
m_xlxNetworkRelink(0U),
m_xlxNetworkDebug(false),
m_xlxNetworkUserControl(true),
m_xlxNetworkModule()
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
				m_rptPort = (unsigned int)::atoi(value);
			else if (::strcmp(key, "LocalAddress") == 0)
				m_localAddress = value;
			else if (::strcmp(key, "LocalPort") == 0)
				m_localPort = (unsigned int)::atoi(value);
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
		} else if (section == SECTION_VOICE) {
			if (::strcmp(key, "Enabled") == 0)
				m_voiceEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Language") == 0)
				m_voiceLanguage = value;
			else if (::strcmp(key, "Directory") == 0)
				m_voiceDirectory = value;
		} else if (section == SECTION_INFO) {
			if (::strcmp(key, "Enabled") == 0)
				m_infoEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "TXFrequency") == 0)
				m_infoTXFrequency = (unsigned int)::atoi(value);
			else if (::strcmp(key, "RXFrequency") == 0)
				m_infoRXFrequency = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Power") == 0)
				m_infoPower = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Latitude") == 0)
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
                m_xlxNetworkPort = (unsigned int)::atoi(value);
            else if (::strcmp(key, "Password") == 0)
                m_xlxNetworkPassword = value;
            else if (::strcmp(key, "Local") == 0)
				m_xlxNetworkLocal = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Slot") == 0)
				m_xlxNetworkSlot = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TG") == 0)
				m_xlxNetworkTG = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Base") == 0)
				m_xlxNetworkBase = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Startup") == 0)
				m_xlxNetworkStartup = (unsigned int)::atoi(value);
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
				m_dmrNetwork1Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork1Local = (unsigned int)::atoi(value);
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
				char* p4 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
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
				m_dmrNetwork2Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork2Local = (unsigned int)::atoi(value);
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
				char* p4 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
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
				m_dmrNetwork3Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork3Local = (unsigned int)::atoi(value);
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
				char* p4 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CTypeRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG   = ::atoi(p2);
					rewrite.m_toSlot   = ::atoi(p3);
					rewrite.m_toId     = ::atoi(p4);
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
			} else if (::strncmp(key, "PassAllPC", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork3PassAllPC.push_back(slotNo);
			} else if (::strncmp(key, "PassAllTG", 9U) == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork3PassAllTG.push_back(slotNo);
			}
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

unsigned int CConf::getRptPort() const
{
	return m_rptPort;
}

std::string CConf::getLocalAddress() const
{
	return m_localAddress;
}

unsigned int CConf::getLocalPort() const
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

bool CConf::getInfoEnabled() const
{
	return m_infoEnabled;
}

unsigned int CConf::getInfoRXFrequency() const
{
	return m_infoRXFrequency;
}

unsigned int CConf::getInfoTXFrequency() const
{
	return m_infoTXFrequency;
}

unsigned int CConf::getInfoPower() const
{
	return m_infoPower;
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

unsigned int CConf::getXLXNetworkPort() const
{
    return m_xlxNetworkPort;
}

std::string CConf::getXLXNetworkPassword() const
{
    return m_xlxNetworkPassword;
}

unsigned int CConf::getXLXNetworkLocal() const
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

unsigned int CConf::getXLXNetworkStartup() const
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

unsigned int CConf::getDMRNetwork1Port() const
{
	return m_dmrNetwork1Port;
}

unsigned int CConf::getDMRNetwork1Local() const
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

unsigned int CConf::getDMRNetwork2Port() const
{
	return m_dmrNetwork2Port;
}

unsigned int CConf::getDMRNetwork2Local() const
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

unsigned int CConf::getDMRNetwork3Port() const
{
	return m_dmrNetwork3Port;
}

unsigned int CConf::getDMRNetwork3Local() const
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

std::vector<unsigned int> CConf::getDMRNetwork3PassAllPC() const
{
	return m_dmrNetwork3PassAllPC;
}

std::vector<unsigned int> CConf::getDMRNetwork3PassAllTG() const
{
	return m_dmrNetwork3PassAllTG;
}
