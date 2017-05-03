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
  SECTION_XLX_NETWORK
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_rptAddress("127.0.0.1"),
m_rptPort(62032U),
m_localAddress("127.0.0.1"),
m_localPort(62031U),
m_timeout(10U),
m_debug(false),
m_logDisplayLevel(0U),
m_logFileLevel(0U),
m_logFilePath(),
m_logFileRoot(),
m_voiceEnabled(true),
m_voiceLanguage("en_GB"),
m_voiceDirectory(),
m_dmrNetwork1Enabled(false),
m_dmrNetwork1Address(),
m_dmrNetwork1Port(0U),
m_dmrNetwork1Local(0U),
m_dmrNetwork1Password(),
m_dmrNetwork1Debug(false),
m_dmrNetwork1Rewrites(),
m_dmrNetwork1PrivateSlot1(false),
m_dmrNetwork1PrivateSlot2(false),
m_dmrNetwork2Enabled(false),
m_dmrNetwork2Address(),
m_dmrNetwork2Port(0U),
m_dmrNetwork2Local(0U),
m_dmrNetwork2Password(),
m_dmrNetwork2Debug(false),
m_dmrNetwork2Rewrites(),
m_dmrNetwork2PrivateSlot1(false),
m_dmrNetwork2PrivateSlot2(false),
m_xlxNetworkEnabled(false),
m_xlxNetworkAddress(),
m_xlxNetworkPort(0U),
m_xlxNetworkLocal(0U),
m_xlxNetworkPassword(),
m_xlxNetworkSlot(2U),
m_xlxNetworkTG(9U),
m_xlxNetworkOptions(),
m_xlxNetworkDebug(false)
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
		else if (::strncmp(buffer, "[XLX Network]", 13U) == 0)
			section = SECTION_XLX_NETWORK;
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
		} else if (section == SECTION_XLX_NETWORK) {
			if (::strcmp(key, "Enabled") == 0)
				m_xlxNetworkEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_xlxNetworkAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_xlxNetworkPort = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_xlxNetworkLocal = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_xlxNetworkPassword = value;
			else if (::strcmp(key, "Slot") == 0)
				m_xlxNetworkSlot = (unsigned int)::atoi(value);
			else if (::strcmp(key, "TG") == 0)
				m_xlxNetworkTG = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Options") == 0)
				m_xlxNetworkOptions = value;
			else if (::strcmp(key, "Debug") == 0)
				m_xlxNetworkDebug = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_NETWORK_1) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork1Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork1Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork1Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork1Local = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork1Password = value;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork1Debug = ::atoi(value) == 1;
			else if (::strcmp(key, "Rewrites") == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG = ::atoi(p2);
					rewrite.m_toSlot = ::atoi(p3);
					rewrite.m_toTG = ::atoi(p4);
					m_dmrNetwork1Rewrites.push_back(rewrite);
				}
			} else if (::strcmp(key, "PrivateSlot1") == 0)
				m_dmrNetwork1PrivateSlot1 = ::atoi(value) == 1;
			else if (::strcmp(key, "PrivateSlot2") == 0)
				m_dmrNetwork1PrivateSlot2 = ::atoi(value) == 1;
		} else if (section == SECTION_DMR_NETWORK_2) {
			if (::strcmp(key, "Enabled") == 0)
				m_dmrNetwork2Enabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Address") == 0)
				m_dmrNetwork2Address = value;
			else if (::strcmp(key, "Port") == 0)
				m_dmrNetwork2Port = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Local") == 0)
				m_dmrNetwork2Local = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Password") == 0)
				m_dmrNetwork2Password = value;
			else if (::strcmp(key, "Debug") == 0)
				m_dmrNetwork2Debug = ::atoi(value) == 1;
			else if (::strcmp(key, "Rewrites") == 0) {
				char* p1 = ::strtok(value, ", ");
				char* p2 = ::strtok(NULL, ", ");
				char* p3 = ::strtok(NULL, ", ");
				char* p4 = ::strtok(NULL, " \r\n");
				if (p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL) {
					CRewriteStruct rewrite;
					rewrite.m_fromSlot = ::atoi(p1);
					rewrite.m_fromTG = ::atoi(p2);
					rewrite.m_toSlot = ::atoi(p3);
					rewrite.m_toTG = ::atoi(p4);
					m_dmrNetwork2Rewrites.push_back(rewrite);
				}
			} else if (::strcmp(key, "PrivateSlot1") == 0)
				m_dmrNetwork2PrivateSlot1 = ::atoi(value) == 1;
			else if (::strcmp(key, "PrivateSlot2") == 0)
				m_dmrNetwork2PrivateSlot2 = ::atoi(value) == 1;
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

bool CConf::getXLXNetworkEnabled() const
{
	return m_xlxNetworkEnabled;
}

std::string CConf::getXLXNetworkAddress() const
{
	return m_xlxNetworkAddress;
}

unsigned int CConf::getXLXNetworkPort() const
{
	return m_xlxNetworkPort;
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

std::string CConf::getXLXNetworkPassword() const
{
	return m_xlxNetworkPassword;
}

std::string CConf::getXLXNetworkOptions() const
{
	return m_xlxNetworkOptions;
}

bool CConf::getXLXNetworkDebug() const
{
	return m_xlxNetworkDebug;
}

bool CConf::getDMRNetwork1Enabled() const
{
	return m_dmrNetwork1Enabled;
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

bool CConf::getDMRNetwork1Debug() const
{
	return m_dmrNetwork1Debug;
}

std::vector<CRewriteStruct> CConf::getDMRNetwork1Rewrites() const
{
	return m_dmrNetwork1Rewrites;
}

bool CConf::getDMRNetwork1PrivateSlot1() const
{
	return m_dmrNetwork1PrivateSlot1;
}

bool CConf::getDMRNetwork1PrivateSlot2() const
{
	return m_dmrNetwork1PrivateSlot2;
}

bool CConf::getDMRNetwork2Enabled() const
{
	return m_dmrNetwork2Enabled;
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

bool CConf::getDMRNetwork2Debug() const
{
	return m_dmrNetwork2Debug;
}

std::vector<CRewriteStruct> CConf::getDMRNetwork2Rewrites() const
{
	return m_dmrNetwork2Rewrites;
}

bool CConf::getDMRNetwork2PrivateSlot1() const
{
	return m_dmrNetwork2PrivateSlot1;
}

bool CConf::getDMRNetwork2PrivateSlot2() const
{
	return m_dmrNetwork2PrivateSlot2;
}
