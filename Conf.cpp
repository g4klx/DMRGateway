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
  SECTION_DMR_NETWORK_1,
  SECTION_DMR_NETWORK_2,
  SECTION_XLX_NETWORK_1,
  SECTION_XLX_NETWORK_2
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_rptAddress("127.0.0.1"),
m_rptPort(62032U),
m_localAddress("127.0.0.1"),
m_localPort(62031U),
m_timeout(10U),
m_ruleTrace(false),
m_debug(false),
m_voiceEnabled(true),
m_voiceLanguage("en_GB"),
m_voiceDirectory(),
m_logDisplayLevel(0U),
m_logFileLevel(0U),
m_logFilePath(),
m_logFileRoot(),
m_dmrNetwork1Enabled(false),
m_dmrNetwork1Id(0U),
m_dmrNetwork1Address(),
m_dmrNetwork1Port(0U),
m_dmrNetwork1Local(0U),
m_dmrNetwork1Password(),
m_dmrNetwork1Options(),
m_dmrNetwork1Debug(false),
m_dmrNetwork1TGRewrites(),
m_dmrNetwork1PCRewrites(),
m_dmrNetwork1TypeRewrites(),
m_dmrNetwork1SrcRewrites(),
m_dmrNetwork1PassAllPC(),
m_dmrNetwork1PassAllTG(),
m_dmrNetwork2Enabled(false),
m_dmrNetwork2Id(0U),
m_dmrNetwork2Address(),
m_dmrNetwork2Port(0U),
m_dmrNetwork2Local(0U),
m_dmrNetwork2Password(),
m_dmrNetwork2Options(),
m_dmrNetwork2Debug(false),
m_dmrNetwork2TGRewrites(),
m_dmrNetwork2PCRewrites(),
m_dmrNetwork2TypeRewrites(),
m_dmrNetwork2SrcRewrites(),
m_dmrNetwork2PassAllPC(),
m_dmrNetwork2PassAllTG(),
m_xlxNetwork1Enabled(false),
m_xlxNetwork1Id(0U),
m_xlxNetwork1Address(),
m_xlxNetwork1Port(0U),
m_xlxNetwork1Local(0U),
m_xlxNetwork1Password(),
m_xlxNetwork1Slot(1U),
m_xlxNetwork1TG(8U),
m_xlxNetwork1Base(84000U),
m_xlxNetwork1Startup(4000U),
m_xlxNetwork1Relink(0U),
m_xlxNetwork1Options(),
m_xlxNetwork1Debug(false),
m_xlxNetwork2Enabled(false),
m_xlxNetwork2Id(0U),
m_xlxNetwork2Address(),
m_xlxNetwork2Port(0U),
m_xlxNetwork2Local(0U),
m_xlxNetwork2Password(),
m_xlxNetwork2Slot(1U),
m_xlxNetwork2TG(7U),
m_xlxNetwork2Base(74000U),
m_xlxNetwork2Startup(4000U),
m_xlxNetwork2Relink(0U),
m_xlxNetwork2Options(),
m_xlxNetwork2Debug(false)
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
		else if (::strncmp(buffer, "[XLX Network 1]", 15U) == 0)
			section = SECTION_XLX_NETWORK_1;
		else if (::strncmp(buffer, "[XLX Network 2]", 15U) == 0)
			section = SECTION_XLX_NETWORK_2;
		else if (::strncmp(buffer, "[DMR Network 1]", 15U) == 0)
			section = SECTION_DMR_NETWORK_1;
		else if (::strncmp(buffer, "[DMR Network 2]", 15U) == 0)
			section = SECTION_DMR_NETWORK_2;
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

		if (section == SECTION_GENERAL) {
			if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
			else if (::strcmp(key, "Timeout") == 0)
				m_timeout = (unsigned int)::atoi(value);
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
		} else if (section == SECTION_XLX_NETWORK_1) {
			if (::strcmp(key, "Enabled") == 0)
				m_xlxNetwork1Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Id") == 0)
				m_xlxNetwork1Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_xlxNetwork1Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_xlxNetwork1Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_xlxNetwork1Local = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_xlxNetwork1Password = value;
			else if (::strcmp(key, "Slot") == 0)
				m_xlxNetwork1Slot = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TG") == 0)
				m_xlxNetwork1TG = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Base") == 0)
				m_xlxNetwork1Base = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Startup") == 0)
				m_xlxNetwork1Startup = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Relink") == 0)
				m_xlxNetwork1Relink = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Options") == 0)
				m_xlxNetwork1Options = value;
			else if (::strcmp(key, "Debug") == 0)
				m_xlxNetwork1Debug = ::atoi(value) == 1;
		} else if (section == SECTION_XLX_NETWORK_2) {
			if (::strcmp(key, "Enabled") == 0)
				m_xlxNetwork2Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Id") == 0)
				m_xlxNetwork2Id = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Address") == 0)
				m_xlxNetwork2Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_xlxNetwork2Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_xlxNetwork2Local = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_xlxNetwork2Password = value;
			else if (::strcmp(key, "Slot") == 0)
				m_xlxNetwork2Slot = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TG") == 0)
				m_xlxNetwork2TG = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Base") == 0)
				m_xlxNetwork2Base = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Startup") == 0)
				m_xlxNetwork2Startup = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Relink") == 0)
				m_xlxNetwork2Relink = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Options") == 0)
				m_xlxNetwork2Options = value;
			else if (::strcmp(key, "Debug") == 0)
				m_xlxNetwork2Debug = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_NETWORK_1) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork1Enabled = ::atoi(value) == 1;
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
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork1Debug = ::atoi(value) == 1;
			else if (::strcmp(key, "TGRewrite") == 0) {
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
			} else if (::strcmp(key, "PCRewrite") == 0) {
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
			} else if (::strcmp(key, "TypeRewrite") == 0) {
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
			} else if (::strcmp(key, "SrcRewrite") == 0) {
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
			} else if (::strcmp(key, "PassAllPC") == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork1PassAllPC.push_back(slotNo);
			} else if (::strcmp(key, "PassAllTG") == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork1PassAllTG.push_back(slotNo);
			}
		} else if (section == SECTION_DMR_NETWORK_2) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork2Enabled = ::atoi(value) == 1;
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
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork2Debug = ::atoi(value) == 1;
			else if (::strcmp(key, "TGRewrite") == 0) {
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
			} else if (::strcmp(key, "PCRewrite") == 0) {
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
			} else if (::strcmp(key, "TypeRewrite") == 0) {
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
			} else if (::strcmp(key, "SrcRewrite") == 0) {
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
			} else if (::strcmp(key, "PassAllPC") == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork2PassAllPC.push_back(slotNo);
			} else if (::strcmp(key, "PassAllTG") == 0) {
				unsigned int slotNo = (unsigned int)::atoi(value);
				m_dmrNetwork2PassAllTG.push_back(slotNo);
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

unsigned int CConf::getTimeout() const
{
	return m_timeout;
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

bool CConf::getXLXNetwork1Enabled() const
{
	return m_xlxNetwork1Enabled;
}

unsigned int CConf::getXLXNetwork1Id() const
{
	return m_xlxNetwork1Id;
}

std::string CConf::getXLXNetwork1Address() const
{
	return m_xlxNetwork1Address;
}

unsigned int CConf::getXLXNetwork1Port() const
{
	return m_xlxNetwork1Port;
}

unsigned int CConf::getXLXNetwork1Local() const
{
	return m_xlxNetwork1Local;
}

unsigned int CConf::getXLXNetwork1Slot() const
{
	return m_xlxNetwork1Slot;
}

unsigned int CConf::getXLXNetwork1TG() const
{
	return m_xlxNetwork1TG;
}

unsigned int CConf::getXLXNetwork1Base() const
{
	return m_xlxNetwork1Base;
}

unsigned int CConf::getXLXNetwork1Startup() const
{
	return m_xlxNetwork1Startup;
}

unsigned int CConf::getXLXNetwork1Relink() const
{
	return m_xlxNetwork1Relink;
}

std::string CConf::getXLXNetwork1Password() const
{
	return m_xlxNetwork1Password;
}

std::string CConf::getXLXNetwork1Options() const
{
	return m_xlxNetwork1Options;
}

bool CConf::getXLXNetwork1Debug() const
{
	return m_xlxNetwork1Debug;
}

bool CConf::getXLXNetwork2Enabled() const
{
	return m_xlxNetwork2Enabled;
}

unsigned int CConf::getXLXNetwork2Id() const
{
	return m_xlxNetwork2Id;
}

std::string CConf::getXLXNetwork2Address() const
{
	return m_xlxNetwork2Address;
}

unsigned int CConf::getXLXNetwork2Port() const
{
	return m_xlxNetwork2Port;
}

unsigned int CConf::getXLXNetwork2Local() const
{
	return m_xlxNetwork2Local;
}

unsigned int CConf::getXLXNetwork2Slot() const
{
	return m_xlxNetwork2Slot;
}

unsigned int CConf::getXLXNetwork2TG() const
{
	return m_xlxNetwork2TG;
}

unsigned int CConf::getXLXNetwork2Base() const
{
	return m_xlxNetwork2Base;
}

unsigned int CConf::getXLXNetwork2Startup() const
{
	return m_xlxNetwork2Startup;
}

unsigned int CConf::getXLXNetwork2Relink() const
{
	return m_xlxNetwork2Relink;
}

std::string CConf::getXLXNetwork2Password() const
{
	return m_xlxNetwork2Password;
}

std::string CConf::getXLXNetwork2Options() const
{
	return m_xlxNetwork2Options;
}

bool CConf::getXLXNetwork2Debug() const
{
	return m_xlxNetwork2Debug;
}

bool CConf::getDMRNetwork1Enabled() const
{
	return m_dmrNetwork1Enabled;
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
