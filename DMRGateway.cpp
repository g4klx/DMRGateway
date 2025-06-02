/*
 *   Copyright (C) 2015-2021,2024,2025 by Jonathan Naylor G4KLX
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

#include "RewriteType.h"
#include "DMRSlotType.h"
#include "RewriteSrc.h"
#include "DMRGateway.h"
#include "StopWatch.h"
#include "RewritePC.h"
#include "RewriteSrcId.h"
#include "RewriteDstId.h"
#include "PassAllPC.h"
#include "PassAllTG.h"
#include "DMRFullLC.h"
#include "Version.h"
#include "Thread.h"
#include "DMRLC.h"
#include "Sync.h"
#include "Log.h"
#include "GitVersion.h"

#include <cstdio>
#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
const char* DEFAULT_INI_FILE = "DMRGateway.ini";
#else
const char* DEFAULT_INI_FILE = "/etc/DMRGateway.ini";
#endif

const unsigned int XLX_SLOT = 2U;
const unsigned int XLX_TG   = 9U;

const unsigned char COLOR_CODE = 3U;

static bool m_killed = false;
static int  m_signal = 0;

#if !defined(_WIN32) && !defined(_WIN64)
static void sigHandler(int signum)
{
	m_killed = true;
	m_signal = signum;
}
#endif

const char* HEADER1 = "This software is for use on amateur radio networks only,";
const char* HEADER2 = "it is to be used for educational purposes only. Its use on";
const char* HEADER3 = "commercial networks is strictly prohibited.";
const char* HEADER4 = "Copyright(C) 2017-2024 by Jonathan Naylor, G4KLX and others";

int main(int argc, char** argv)
{
	const char* iniFile = DEFAULT_INI_FILE;

	if (argc > 1) {
		for (int currentArg = 1; currentArg < argc; ++currentArg) {
			std::string arg = argv[currentArg];
			if ((arg == "-v") || (arg == "--version")) {
				::fprintf(stdout, "DMRGateway version %s git #%.7s\n", VERSION, gitversion);
				return 0;
			} else if (arg.substr(0,1) == "-") {
				::fprintf(stderr, "Usage: DMRGateway [-v|--version] [filename]\n");
				return 1;
			} else {
				iniFile = argv[currentArg];
			}
		}
	}

#if !defined(_WIN32) && !defined(_WIN64)
	::signal(SIGINT,  sigHandler);
	::signal(SIGTERM, sigHandler);
	::signal(SIGHUP,  sigHandler);
#endif

	int ret = 0;

	do {
		m_signal = 0;
		m_killed = false;

		CDMRGateway* host = new CDMRGateway(std::string(iniFile));
		ret = host->run();

		delete host;

		switch (m_signal) {
			case 0:
				break;
			case 2:
				::LogInfo("DMRGateway-%s exited on receipt of SIGINT", VERSION);
				break;
			case 15:
				::LogInfo("DMRGateway-%s exited on receipt of SIGTERM", VERSION);
				break;
			case 1:
				::LogInfo("DMRGateway-%s is restarting on receipt of SIGHUP", VERSION);
				break;
			default:
				::LogInfo("DMRGateway-%s exited on receipt of an unknown signal", VERSION);
				break;
		}
	} while (m_signal == 1);

	::LogFinalise();

	return ret;
}

CDMRGateway::CDMRGateway(const std::string& confFile) :
m_conf(confFile),
m_extStatus(nullptr),
m_repeater(nullptr),
m_config(nullptr),
m_configLen(0U),
m_dmrNetworkCount(0U),
m_dmrNetworks(),
m_dmrName(),
m_xlxReflectors(nullptr),
m_xlxNetwork(nullptr),
m_xlxId(0U),
m_xlxNumber("000"),
m_xlxReflector(4000U),
m_xlxSlot(0U),
m_xlxTG(0U),
m_xlxBase(0U),
m_xlxLocal(0U),
m_xlxPort(62030U),
m_xlxPassword("passw0rd"),
m_xlxStartup("950"),
m_xlxRoom(4000U),
m_xlxRelink(1000U),
m_xlxConnected(false),
m_xlxDebug(false),
m_xlxUserControl(true),
m_xlxModule(),
m_rptRewrite(nullptr),
m_xlxRewrite(nullptr),
m_xlxVoice(nullptr),
m_dmrNetRewrites(),
m_dmrRFRewrites(),
m_dmrSrcRewrites(),
m_dmrPassalls(),
m_dynVoices(),
m_dynRF(),
m_socket(nullptr),
m_writer(nullptr),
m_callsign(),
m_txFrequency(0U),
m_rxFrequency(0U),
#if defined(USE_GPSD)
m_gpsd(nullptr),
#endif
m_networkEnabled(nullptr),
m_networkXlxEnabled(false),
m_remoteControl(nullptr)
{
	CUDPSocket::startup();

	m_extStatus = new CDMRGWExtStatus[3U];
	m_extStatus[1U].m_status = DMRGW_STATUS::NONE;
	m_extStatus[2U].m_status = DMRGW_STATUS::NONE;

	m_config = new unsigned char[400U];
}

CDMRGateway::~CDMRGateway()
{
	for (auto& dmrNetRewrites: m_dmrNetRewrites)
		for (CRewrite* rewrite: dmrNetRewrites)
			delete rewrite;

	for (auto& dmrNetRewrites: m_dmrRFRewrites)
		for (CRewrite* rewrite: dmrNetRewrites)
			delete rewrite;

	for (auto& dmrNetRewrites: m_dmrSrcRewrites)
		for (CRewrite* rewrite: dmrNetRewrites)
			delete rewrite;

	for (auto& dmrNetRewrites: m_dmrPassalls)
		for (CRewrite* rewrite: dmrNetRewrites)
			delete rewrite;

	for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it)
		delete* it;

	delete m_rptRewrite;
	delete m_xlxRewrite;

	delete[] m_extStatus;
	delete[] m_config;
	delete[] m_networkEnabled;

	CUDPSocket::shutdown();
}

int CDMRGateway::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "DMRGateway: cannot read the .ini file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	bool m_daemon = m_conf.getDaemon();
	if (m_daemon) {
		// Create new process
		pid_t pid = ::fork();
		if (pid == -1) {
			::fprintf(stderr, "Couldn't fork() , exiting\n");
			return -1;
		} else if (pid != 0) {
			exit(EXIT_SUCCESS);
		}

		// Create new session and process group
		if (::setsid() == -1){
			::fprintf(stderr, "Couldn't setsid(), exiting\n");
			return -1;
		}

		// Set the working directory to the root directory
		if (::chdir("/") == -1){
			::fprintf(stderr, "Couldn't cd /, exiting\n");
			return -1;
		}

		// If we are currently root...
		if (getuid() == 0) {
			struct passwd* user = ::getpwnam("mmdvm");
			if (user == nullptr) {
				::fprintf(stderr, "Could not get the mmdvm user, exiting\n");
				return -1;
			}

			uid_t mmdvm_uid = user->pw_uid;
			gid_t mmdvm_gid = user->pw_gid;

			// Set user and group ID's to mmdvm:mmdvm
			if (setgid(mmdvm_gid) != 0) {
				::fprintf(stderr, "Could not set mmdvm GID, exiting\n");
				return -1;
			}

			if (setuid(mmdvm_uid) != 0) {
				::fprintf(stderr, "Could not set mmdvm UID, exiting\n");
				return -1;
			}

			// Double check it worked (AKA Paranoia) 
			if (setuid(0) != -1) {
				::fprintf(stderr, "It's possible to regain root - something is wrong!, exiting\n");
				return -1;
			}
		}
	}
#endif

#if !defined(_WIN32) && !defined(_WIN64)
	ret = ::LogInitialise(m_daemon, m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), m_conf.getLogDisplayLevel(), m_conf.getLogFileRotate());
#else
	ret = ::LogInitialise(false, m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), m_conf.getLogDisplayLevel(), m_conf.getLogFileRotate());
#endif
	if (!ret) {
		::fprintf(stderr, "DMRGateway: unable to open the log file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	if (m_daemon) {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
		::close(STDERR_FILENO);
	}
#endif

	m_dmrNetworkCount = m_conf.getDMRNetworksCount();

	m_dmrNetworks.resize(m_dmrNetworkCount, nullptr);
	m_dmrName.resize(m_dmrNetworkCount, "");
	m_networkEnabled = new bool[m_dmrNetworkCount];

	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		m_networkEnabled[i] = m_conf.getDMRNetworkEnabled(i);
	}

	m_networkXlxEnabled = m_conf.getXLXNetworkEnabled();

	m_dmrNetRewrites.resize(m_dmrNetworkCount);
	m_dmrRFRewrites.resize(m_dmrNetworkCount);
	m_dmrSrcRewrites.resize(m_dmrNetworkCount);
	m_dmrPassalls.resize(m_dmrNetworkCount);

	LogInfo(HEADER1);
	LogInfo(HEADER2);
	LogInfo(HEADER3);
	LogInfo(HEADER4);

	LogMessage("DMRGateway-%s is starting", VERSION);
	LogMessage("Built %s %s (GitID #%.7s)", __TIME__, __DATE__, gitversion);

	ret = createMMDVM();
	if (!ret)
		return 1;

	LogMessage("Waiting for MMDVM to connect.....");

	while (!m_killed) {
		m_configLen = m_repeater->getShortConfig(m_config);
		if (m_configLen > 0U && m_repeater->getId() > 1000U)
			break;

		m_repeater->clock(10U);

		CThread::sleep(10U);
	}

	if (m_killed) {
		m_repeater->close();
		delete m_repeater;
		return 0;
	}

	LogMessage("MMDVM has connected");

#if defined(USE_GPSD)
	bool gpsdEnabled = m_conf.getGPSDEnabled();
	if (gpsdEnabled) {
		std::string gpsdAddress = m_conf.getGPSDAddress();
		std::string gpsdPort    = m_conf.getGPSDPort();

		LogInfo("GPSD Parameters");
		LogInfo("    Address: %s", gpsdAddress.c_str());
		LogInfo("    Port: %s", gpsdPort.c_str());

		m_gpsd = new CGPSD(gpsdAddress, gpsdPort);

		ret = m_gpsd->open();
		if (!ret) {
			delete m_gpsd;
			m_gpsd = nullptr;
		}
	}
#endif

	bool ruleTrace = m_conf.getRuleTrace();
	LogInfo("Rule trace: %s", ruleTrace ? "yes" : "no");

	if (m_networkXlxEnabled && m_conf.getXLXNetworkEnabled()) {
		ret = createXLXNetwork();
		if (!ret)
			return 1;
	}

	if (m_conf.getVoiceEnabled()) {
		std::string language = m_conf.getVoiceLanguage();
		std::string directory = m_conf.getVoiceDirectory();

		LogInfo("Voice Parameters");
		LogInfo("    Enabled: yes");
		LogInfo("    Language: %s", language.c_str());
		LogInfo("    Directory: %s", directory.c_str());

		if (m_xlxNetwork != nullptr) {
			m_xlxVoice = new CXLXVoice(directory, language, m_repeater->getId(), m_xlxSlot, m_xlxTG);
			bool ret = m_xlxVoice->open();
			if (!ret) {
				delete m_xlxVoice;
				m_xlxVoice = nullptr;
			}
		}
	}

	bool remoteControlEnabled = m_conf.getRemoteControlEnabled();
	if (remoteControlEnabled) {
		std::string address = m_conf.getRemoteControlAddress();
		unsigned short port = m_conf.getRemoteControlPort();

		LogInfo("Remote Control Parameters");
		LogInfo("    Address: %s", address.c_str());
		LogInfo("    Port: %hu", port);

		m_remoteControl = new CRemoteControl(this, address, port);

		ret = m_remoteControl->open();
		if (!ret) {
			LogInfo("Failed to open Remove Control Socket");
			delete m_remoteControl;
			m_remoteControl = nullptr;
		}
	}

	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		if (m_networkEnabled[i] && m_conf.getDMRNetworkEnabled(i)) {
			ret = createDMRNetwork(i);
			if (!ret)
				return 1;
		}
	}

	if (m_conf.getDynamicTGControlEnabled()) {
		bool ret = createDynamicTGControl();
		if (!ret)
			return 1;
	}

	createAPRS();

	unsigned int rfTimeout  = m_conf.getRFTimeout();
	unsigned int netTimeout = m_conf.getNetTimeout();

	CTimer* timer[3U];
	timer[1U] = new CTimer(1000U);
	timer[2U] = new CTimer(1000U);

	unsigned int rfSrcId[3U];
	unsigned int rfDstId[3U];
	rfSrcId[1U] = rfSrcId[2U] = rfDstId[1U] = rfDstId[2U] = 0U;

	std::vector<std::vector<unsigned int>> dmrSrcId(m_dmrNetworkCount, std::vector<unsigned int>(3U, 0U));
	std::vector<std::vector<unsigned int>> dmrDstId(m_dmrNetworkCount, std::vector<unsigned int>(3U, 0U));

	CStopWatch stopWatch;
	stopWatch.start();

	LogMessage("DMRGateway-%s is running", VERSION);

	while (!m_killed) {
		if (m_networkXlxEnabled && (m_xlxNetwork != nullptr)) {
			bool connected = m_xlxNetwork->isConnected();
			if (connected && !m_xlxConnected) {
				if (m_xlxReflector >= 4001U && m_xlxReflector <= 4026U) {
					writeXLXLink(m_xlxId, m_xlxReflector, m_xlxNetwork);
					char c = ('A' + (m_xlxReflector % 100U)) - 1U;
					LogMessage("XLX, Linking to reflector XLX%s %c", m_xlxNumber.c_str(), c);
					if (m_xlxVoice != nullptr)
						m_xlxVoice->linkedTo(m_xlxNumber, m_xlxReflector);
				} else if (m_xlxRoom >= 4001U && m_xlxRoom <= 4026U) {
					writeXLXLink(m_xlxId, m_xlxRoom, m_xlxNetwork);
					char c = ('A' + (m_xlxRoom % 100U)) - 1U;
					LogMessage("XLX, Linking to reflector XLX%s %c", m_xlxNumber.c_str(), c);
					if (m_xlxVoice != nullptr)
						m_xlxVoice->linkedTo(m_xlxNumber, m_xlxRoom);
					m_xlxReflector = m_xlxRoom;
				} else {
					if (m_xlxVoice != nullptr)
						m_xlxVoice->linkedTo(m_xlxNumber, 0U);
				}

				m_xlxConnected = true;

				if (m_xlxNumber == m_xlxStartup && m_xlxRoom == m_xlxReflector)
					m_xlxRelink.stop();
				else
					m_xlxRelink.start();
			} else if (!connected && m_xlxConnected) {
				LogMessage("XLX, Unlinking from XLX%s due to loss of connection", m_xlxNumber.c_str());

				if (m_xlxVoice != nullptr)
					m_xlxVoice->unlinked();

				m_xlxConnected = false;
				m_xlxRelink.stop();
			} else if (connected && m_xlxRelink.isRunning() && m_xlxRelink.hasExpired()) {
				m_xlxRelink.stop();

				if (m_xlxNumber != m_xlxStartup) {
					if (m_xlxStartup != "4000") {
						m_xlxReflector = 4000U;
						char c = ('A' + (m_xlxRoom % 100U)) - 1U;
						LogMessage("XLX, Re-linking to startup reflector XLX%s %c due to RF inactivity timeout", m_xlxNumber.c_str(), c);
						linkXLX(m_xlxStartup);
					} else {
						LogMessage("XLX, Unlinking from XLX%s due to RF inactivity timeout", m_xlxNumber.c_str());
						unlinkXLX();
					}
				} else {
					if (m_xlxReflector >= 4001U && m_xlxReflector <= 4026U)
						writeXLXLink(m_xlxId, 4000U, m_xlxNetwork);

					if (m_xlxRoom >= 4001U && m_xlxRoom <= 4026U) {
						writeXLXLink(m_xlxId, m_xlxRoom, m_xlxNetwork);
						char c = ('A' + (m_xlxRoom % 100U)) - 1U;
						LogMessage("XLX, Re-linking to startup reflector XLX%s %c due to RF inactivity timeout", m_xlxNumber.c_str(), c);
					} else if (m_xlxReflector >= 4001U && m_xlxReflector <= 4026U) {
						char c = ('A' + (m_xlxReflector % 100U)) - 1U;
						LogMessage("XLX, Unlinking from reflector XLX%s %c due to RF inactivity timeout", m_xlxNumber.c_str(), c);
					}

					m_xlxReflector = m_xlxRoom;
					if (m_xlxVoice != nullptr) {
						if (m_xlxReflector < 4001U || m_xlxReflector > 4026U)
							m_xlxVoice->linkedTo(m_xlxNumber, 0U);
						else
							m_xlxVoice->linkedTo(m_xlxNumber, m_xlxReflector);
					}
				}
			}
		}

		CDMRData data;

		bool ret = m_repeater->read(data);
		if (ret) {
			unsigned int slotNo = data.getSlotNo();
			unsigned int srcId = data.getSrcId();
			unsigned int dstId = data.getDstId();
			FLCO flco = data.getFLCO();

			if ((flco == FLCO::GROUP) && (slotNo == m_xlxSlot) && (dstId == m_xlxTG)) {
				if ((m_xlxReflector != m_xlxRoom) || (m_xlxNumber != m_xlxStartup))
					m_xlxRelink.start();

				m_xlxRewrite->process(data, false);
				if (m_networkXlxEnabled) {
					m_xlxNetwork->write(data);
				}
				m_extStatus[slotNo].m_status = DMRGW_STATUS::XLXREFLECTOR;
				timer[slotNo]->setTimeout(rfTimeout);
				timer[slotNo]->start();
			} else if ((dstId <= (m_xlxBase + 26U) || dstId == (m_xlxBase + 1000U)) && flco == FLCO::USER_USER && slotNo == m_xlxSlot && dstId >= m_xlxBase && m_xlxUserControl) {
				dstId += 4000U;
				dstId -= m_xlxBase;

				if (dstId != m_xlxReflector) {
					if (dstId == 4000U) {
						writeXLXLink(srcId, 4000U, m_xlxNetwork);
						m_xlxReflector = 4000U;
						char c = ('A' + (m_xlxRoom % 100U)) - 1U;
						LogMessage("XLX, Unlinking from reflector XLX%s %c", m_xlxNumber.c_str(), c);
					} else if (dstId != 5000U) {
						if (m_xlxReflector != 4000U)
							writeXLXLink(srcId, 4000U, m_xlxNetwork);
						writeXLXLink(srcId, dstId, m_xlxNetwork);
						m_xlxReflector = dstId;
						char c = ('A' + (dstId % 100U)) - 1U;
						LogMessage("XLX, Linking to reflector XLX%s %c", m_xlxNumber.c_str(), c);
					}

					if (m_xlxReflector != m_xlxRoom)
						m_xlxRelink.start();
					else
						m_xlxRelink.stop();
				}

				m_extStatus[slotNo].m_status = DMRGW_STATUS::XLXREFLECTOR;
				timer[slotNo]->setTimeout(rfTimeout);
				timer[slotNo]->start();

				if (m_xlxVoice != nullptr) {
					unsigned char type = data.getDataType();
					if (type == DT_TERMINATOR_WITH_LC) {
						if (m_xlxConnected) {
							if (m_xlxReflector != 4000U)
								m_xlxVoice->linkedTo(m_xlxNumber, m_xlxReflector);
							else
								m_xlxVoice->linkedTo(m_xlxNumber, 0U);
						} else {
							m_xlxVoice->unlinked();
						}
					}
				}
			} else if (dstId >= (m_xlxBase + 4000U) && dstId < (m_xlxBase + 5000U) && flco == FLCO::USER_USER && slotNo == m_xlxSlot && m_xlxUserControl) {
				char dstIdBuf[16];

				dstId -= 4000U;
				dstId -= m_xlxBase;

				// it's all 3 characters IDS, and not digits.
				snprintf(dstIdBuf, sizeof(dstIdBuf), "%03u", dstId);
				if (std::string(dstIdBuf) != m_xlxNumber)
					linkXLX(dstIdBuf);
			} else {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId  = data.getSrcId();
				unsigned int dstId  = data.getDstId();
				FLCO flco           = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != rfSrcId[slotNo] || dstId != rfDstId[slotNo])) {
					rfSrcId[slotNo] = srcId;
					rfDstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, RF transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO::GROUP ? "TG" : "", dstId);

				PROCESS_RESULT result = PROCESS_RESULT::UNMATCHED;

				// Match by m_dmrRFRewrites
				for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
					if (result != PROCESS_RESULT::UNMATCHED)
						break;

					if (m_networkEnabled[i] && (m_dmrNetworks[i] != nullptr)) {
						// Rewrite the slot and/or TG or neither
						for (CRewrite* rewrite: m_dmrRFRewrites[i]) {
							PROCESS_RESULT res = rewrite->process(data, trace);
							if (res != PROCESS_RESULT::UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == PROCESS_RESULT::MATCHED) {
							if (m_extStatus[slotNo].m_status == DMRGW_STATUS::NONE || (
									m_extStatus[slotNo].m_status == DMRGW_STATUS::DMRNETWORK &&
									m_extStatus[slotNo].m_dmrNetwork == i)
							) {
								rewrite(m_dmrSrcRewrites[i], data, trace);
								m_dmrNetworks[i]->write(data);
								m_extStatus[slotNo].m_status = DMRGW_STATUS::DMRNETWORK;
								m_extStatus[slotNo].m_dmrNetwork = i;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				// Match by m_dmrPassalls
				for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
					if (result != PROCESS_RESULT::UNMATCHED)
						break;

					if (m_networkEnabled[i] && (m_dmrNetworks[i] != nullptr)) {
						for (CRewrite* rewrite: m_dmrPassalls[i]) {
							PROCESS_RESULT res = rewrite->process(data, trace);
							if (res != PROCESS_RESULT::UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == PROCESS_RESULT::MATCHED) {
							if (m_extStatus[slotNo].m_status == DMRGW_STATUS::NONE || (
									m_extStatus[slotNo].m_status == DMRGW_STATUS::DMRNETWORK &&
									m_extStatus[slotNo].m_dmrNetwork == i)
							) {
								rewrite(m_dmrSrcRewrites[i], data, trace);
								m_dmrNetworks[i]->write(data);
								m_extStatus[slotNo].m_status = DMRGW_STATUS::DMRNETWORK;
								m_extStatus[slotNo].m_dmrNetwork = i;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == PROCESS_RESULT::UNMATCHED && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}
		}

		if (m_networkXlxEnabled && (m_xlxNetwork != nullptr)) {
			ret = m_xlxNetwork->read(data);
			if (ret) {
				if (m_extStatus[m_xlxSlot].m_status == DMRGW_STATUS::NONE ||
					m_extStatus[m_xlxSlot].m_status == DMRGW_STATUS::XLXREFLECTOR
				) {
					PROCESS_RESULT ret = m_rptRewrite->process(data, false);
					if (ret == PROCESS_RESULT::MATCHED) {
						m_repeater->write(data);
						m_extStatus[m_xlxSlot].m_status = DMRGW_STATUS::XLXREFLECTOR;
						timer[m_xlxSlot]->setTimeout(netTimeout);
						timer[m_xlxSlot]->start();
					} else {
						unsigned int slotNo = data.getSlotNo();
						unsigned int dstId  = data.getDstId();
						FLCO flco           = data.getFLCO();
						LogWarning("XLX%s, Unexpected data from slot %u %s%u", m_xlxNumber.c_str(), slotNo, flco == FLCO::GROUP ? "TG" : "", dstId);
					}
				}
			}
		}

		//!!0
		for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
			if (m_networkEnabled[i] && (m_dmrNetworks[i] != nullptr)) {
				ret = m_dmrNetworks[i]->read(data);
				if (ret) {
					unsigned int slotNo = data.getSlotNo();
					unsigned int srcId  = data.getSrcId();
					unsigned int dstId  = data.getDstId();
					FLCO flco           = data.getFLCO();

					bool trace = false;
					if (ruleTrace && (srcId != dmrSrcId[i][slotNo] || dstId != dmrDstId[i][slotNo])) {
						dmrSrcId[i][slotNo] = srcId;
						dmrDstId[i][slotNo] = dstId;
						trace = true;
					}

					if (trace)
						LogDebug("Rule Trace, network %i transmission: Slot=%u Src=%u Dst=%s%u", i + 1, slotNo, srcId, flco == FLCO::GROUP ? "TG" : "", dstId);

					// Rewrite the slot and/or TG or neither
					bool rewritten = false;
					for (CRewrite* rewrite: m_dmrNetRewrites[i]) {
						PROCESS_RESULT ret = rewrite->process(data, trace);
						if (ret == PROCESS_RESULT::MATCHED) {
							rewritten = true;
							break;
						}
					}

					if (rewritten) {
						// Check that the rewritten slot is free to use.
						slotNo = data.getSlotNo();
						if (m_extStatus[slotNo].m_status == DMRGW_STATUS::NONE || (
								m_extStatus[slotNo].m_status == DMRGW_STATUS::DMRNETWORK &&
								m_extStatus[slotNo].m_dmrNetwork == i)
						) {
							for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
								(*it)->stopVoice(slotNo);
							m_repeater->write(data);
							m_extStatus[slotNo].m_status = DMRGW_STATUS::DMRNETWORK;
							m_extStatus[slotNo].m_dmrNetwork = i;
							timer[slotNo]->setTimeout(netTimeout);
							timer[slotNo]->start();
						}
					}

					if (!rewritten && trace)
						LogDebug("Rule Trace,\tnot matched so rejected");
				}

				ret = m_dmrNetworks[i]->wantsBeacon();
				if (ret)
					m_repeater->writeBeacon();
			}
		}

		processRadioPosition();

		processTalkerAlias();

		if (m_networkXlxEnabled && (m_xlxVoice != nullptr)) {
			ret = m_xlxVoice->read(data);
			if (ret) {
				m_repeater->write(data);
				m_extStatus[m_xlxSlot].m_status = DMRGW_STATUS::XLXREFLECTOR;
				timer[m_xlxSlot]->setTimeout(netTimeout);
				timer[m_xlxSlot]->start();
			}
		}

		for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it) {
			ret = (*it)->read(data);
			if (ret)
				m_repeater->write(data);
		}

		if (m_socket != nullptr)
			processDynamicTGControl();

		remoteControl();

		unsigned int ms = stopWatch.elapsed();
		stopWatch.start();

		m_repeater->clock(ms);

		m_xlxRelink.clock(ms);

		for (unsigned int i = 0; i < m_dmrNetworkCount; i++)
			if (m_dmrNetworks[i] != nullptr)
				m_dmrNetworks[i]->clock(ms);

		if (m_xlxNetwork != nullptr)
			m_xlxNetwork->clock(ms);

		if (m_xlxReflectors != nullptr)
			m_xlxReflectors->clock(ms);

		if (m_xlxVoice != nullptr)
			m_xlxVoice->clock(ms);

#if defined(USE_GPSD)
		if (m_gpsd != nullptr)
			m_gpsd->clock(ms);
#endif

		if (m_writer != nullptr)
			m_writer->clock(ms);

		for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it)
			(*it)->clock(ms);

		// Check timer for both slots & free if expired
		for (unsigned int i = 1U; i < 3U; i++) {
			timer[i]->clock(ms);
			if (timer[i]->isRunning() && timer[i]->hasExpired()) {
				m_extStatus[i].m_status = DMRGW_STATUS::NONE;
				timer[i]->stop();
			}
		}

		if (ms < 10U)
			CThread::sleep(10U);
	}

	delete m_xlxVoice;

	m_repeater->close();
	delete m_repeater;

	if (m_remoteControl != nullptr) {
		m_remoteControl->close();
		delete m_remoteControl;
	}

#if defined(USE_GPSD)
	if (m_gpsd != nullptr) {
		m_gpsd->close();
		delete m_gpsd;
	}
#endif

	if (m_writer != nullptr) {
		m_writer->close();
		delete m_writer;
	}

	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		if (m_dmrNetworks[i] != nullptr) {
			m_dmrNetworks[i]->close(true);
			delete m_dmrNetworks[i];
		}
	}

	if (m_xlxNetwork != nullptr) {
		m_xlxNetwork->close(true);
		delete m_xlxNetwork;
	}

	if (m_socket != nullptr) {
		m_socket->close();
		delete m_socket;
	}

	delete timer[1U];
	delete timer[2U];

	delete m_xlxReflectors;

	return 0;
}

bool CDMRGateway::createMMDVM()
{
	std::string rptAddress   = m_conf.getRptAddress();
	unsigned short rptPort   = m_conf.getRptPort();
	std::string localAddress = m_conf.getLocalAddress();
	unsigned short localPort = m_conf.getLocalPort();
	bool debug               = m_conf.getDebug();

	LogInfo("MMDVM Network Parameters");
	LogInfo("    Rpt Address: %s", rptAddress.c_str());
	LogInfo("    Rpt Port: %hu", rptPort);
	LogInfo("    Local Address: %s", localAddress.c_str());
	LogInfo("    Local Port: %hu", localPort);

	m_repeater = new CMMDVMNetwork(rptAddress, rptPort, localAddress, localPort, debug);

	bool ret = m_repeater->open();
	if (!ret) {
		delete m_repeater;
		m_repeater = nullptr;
		return false;
	}

	return true;
}

bool CDMRGateway::createDMRNetwork(unsigned int index)
{
	std::string address  = m_conf.getDMRNetworkAddress(index);
	unsigned short port  = m_conf.getDMRNetworkPort(index);
	unsigned short local = m_conf.getDMRNetworkLocal(index);
	unsigned int id      = m_conf.getDMRNetworkId(index);
	std::string password = m_conf.getDMRNetworkPassword(index);
	bool location        = m_conf.getDMRNetworkLocation(index);
	bool debug           = m_conf.getDMRNetworkDebug(index);
	m_dmrName[index]     = m_conf.getDMRNetworkName(index);

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network %i Parameters", index + 1);
	LogInfo("    Name: %s", m_dmrName[index].c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetworks[index] = new CDMRNetwork(address, port, local, id, password, m_dmrName[index], location, debug);

	std::string options = m_conf.getDMRNetworkOptions(index);

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetworks[index]->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmrName[index], config);
	m_dmrNetworks[index]->setConfig(config, len);

	bool ret = m_dmrNetworks[index]->open();
	if (!ret) {
		delete m_dmrNetworks[index];
		m_dmrNetworks[index] = nullptr;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != nullptr))
		m_gpsd->addNetwork(m_dmrNetworks[index]);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetworkTGRewrites(index);
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite  = new CRewriteTG(m_dmrName[index], (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmrName[index], (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmrRFRewrites[index].push_back(rfRewrite);
		m_dmrNetRewrites[index].push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetworkPCRewrites(index);
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmrName[index], (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmrRFRewrites[index].push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetworkTypeRewrites(index);
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmrName[index], (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmrRFRewrites[index].push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetworkSrcRewrites(index);
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmrName[index], (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmrNetRewrites[index].push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetworkTGDynRewrites(index);
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = nullptr;
		if (m_conf.getVoiceEnabled()) {
			std::string language  = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = nullptr;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmrName[index], (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmrName[index], (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmrRFRewrites[index].push_back(rfRewriteDynTG);
		m_dmrNetRewrites[index].push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetworkIdRewrites(index);
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmrName[index], (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmrName[index], (*it).m_netId, (*it).m_rfId);

		m_dmrSrcRewrites[index].push_back(rewriteSrcId);
		m_dmrNetRewrites[index].push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetworkPassAllTG(index);
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG  = new CPassAllTG(m_dmrName[index], *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmrName[index], *it);

		m_dmrPassalls[index].push_back(rfPassAllTG);
		m_dmrNetRewrites[index].push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetworkPassAllPC(index);
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC  = new CPassAllPC(m_dmrName[index], *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmrName[index], *it);

		m_dmrPassalls[index].push_back(rfPassAllPC);
		m_dmrNetRewrites[index].push_back(netPassAllPC);
	}

	m_dmrNetworks[index]->enable(true);

	return true;
}

bool CDMRGateway::createXLXNetwork()
{
	std::string fileName    = m_conf.getXLXNetworkFile();
	unsigned int reloadTime = m_conf.getXLXNetworkReloadTime();

	m_xlxReflectors = new CReflectors(fileName, reloadTime);

	bool ret = m_xlxReflectors->load();
	if (!ret) {
		delete m_xlxReflectors;
		return false;
	}

	m_xlxLocal         = m_conf.getXLXNetworkLocal();
	m_xlxPort          = m_conf.getXLXNetworkPort();
	m_xlxPassword      = m_conf.getXLXNetworkPassword();
	m_xlxId            = m_conf.getXLXNetworkId();
	m_xlxDebug         = m_conf.getXLXNetworkDebug();
	m_xlxUserControl   = m_conf.getXLXNetworkUserControl();

	if (m_xlxId == 0U)
		m_xlxId = m_repeater->getId();

	m_xlxSlot    = m_conf.getXLXNetworkSlot();
	m_xlxTG      = m_conf.getXLXNetworkTG();
	m_xlxBase    = m_conf.getXLXNetworkBase();
	m_xlxStartup = m_conf.getXLXNetworkStartup();
	m_xlxModule  = m_conf.getXLXNetworkModule();

	unsigned int xlxRelink  = m_conf.getXLXNetworkRelink();

	LogInfo("XLX Network Parameters");
	LogInfo("    Id: %u", m_xlxId);
	LogInfo("    Hosts file: %s", fileName.c_str());
	LogInfo("    Reload time: %u minutes", reloadTime);
	if (m_xlxLocal > 0U)
		LogInfo("    Local: %hu", m_xlxLocal);
	else
		LogInfo("    Local: random");
	LogInfo("    Port: %hu", m_xlxPort);
	LogInfo("    Slot: %u", m_xlxSlot);
	LogInfo("    TG: %u", m_xlxTG);
	LogInfo("    Base: %u", m_xlxBase);

	if (m_xlxStartup != "4000")
		LogInfo("    Startup: XLX%s", m_xlxStartup.c_str());

	if (xlxRelink > 0U) {
		m_xlxRelink.setTimeout(xlxRelink * 60U);
		LogInfo("    Relink: %u minutes", xlxRelink);
	} else {
		LogInfo("    Relink: disabled");
	}

	if (m_xlxUserControl)
		LogInfo("    User Control: enabled");
	else
		LogInfo("    User Control: disabled");

	if (m_xlxModule != 0U)
		LogInfo("    Module: %c", m_xlxModule);

	if (m_xlxStartup != "4000")
		linkXLX(m_xlxStartup);

	m_rptRewrite = new CRewriteTG("XLX", XLX_SLOT, XLX_TG, m_xlxSlot, m_xlxTG, 1U);
	m_xlxRewrite = new CRewriteTG("XLX", m_xlxSlot, m_xlxTG, XLX_SLOT, XLX_TG, 1U);

	return true;
}

bool CDMRGateway::createDynamicTGControl()
{
	unsigned short port = m_conf.getDynamicTGControlPort();

	m_socket = new CUDPSocket(port);

	bool ret = m_socket->open();
	if (!ret) {
		delete m_socket;
		m_socket = nullptr;
		return false;
	}

	return true;
}

bool CDMRGateway::linkXLX(const std::string &number)
{
	CReflector* reflector = m_xlxReflectors->find(number);
	if (reflector == nullptr)
		return false;

	if (m_xlxNetwork != nullptr) {
		LogMessage("XLX, Disconnecting from XLX%s", m_xlxNumber.c_str());
		m_xlxNetwork->close(true);
		delete m_xlxNetwork;
	}

	m_xlxConnected = false;
	m_xlxRelink.stop();

	m_xlxNetwork = new CDMRNetwork(reflector->m_address, m_xlxPort, m_xlxLocal, m_xlxId, m_xlxPassword, "XLX", false, m_xlxDebug);

	unsigned char config[400U];
	unsigned int len = getConfig("XLX", config);
	m_xlxNetwork->setConfig(config, len);

	bool ret = m_xlxNetwork->open();
	if (!ret) {
		delete m_xlxNetwork;
		m_xlxNetwork = nullptr;
		return false;
	}

	m_xlxNumber = number;
	if (m_xlxModule != 0U)
		m_xlxRoom  = ((int(m_xlxModule) - 64U) + 4000U);
	else
		m_xlxRoom  = reflector->m_startup;
	m_xlxReflector = 4000U;

	LogMessage("XLX, Connecting to XLX%s", m_xlxNumber.c_str());

	m_xlxNetwork->enable(m_networkXlxEnabled);

	return true;
}

void CDMRGateway::unlinkXLX()
{
	if (m_xlxNetwork != nullptr) {
		m_xlxNetwork->close(true);
		delete m_xlxNetwork;
		m_xlxNetwork = nullptr;
	}

	m_xlxConnected = false;
	m_xlxRelink.stop();
}

void CDMRGateway::writeXLXLink(unsigned int srcId, unsigned int dstId, CDMRNetwork* network)
{
	assert(network != nullptr);

	unsigned int streamId = ::rand() + 1U;

	CDMRData data;

	data.setSlotNo(XLX_SLOT);
	data.setFLCO(FLCO::USER_USER);
	data.setSrcId(srcId);
	data.setDstId(dstId);
	data.setDataType(DT_VOICE_LC_HEADER);
	data.setN(0U);
	data.setStreamId(streamId);

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];

	CDMRLC lc;
	lc.setSrcId(srcId);
	lc.setDstId(dstId);
	lc.setFLCO(FLCO::USER_USER);

	CDMRFullLC fullLC;
	fullLC.encode(lc, buffer, DT_VOICE_LC_HEADER);

	CDMRSlotType slotType;
	slotType.setColorCode(COLOR_CODE);
	slotType.setDataType(DT_VOICE_LC_HEADER);
	slotType.getData(buffer);

	CSync::addDMRDataSync(buffer, true);

	data.setData(buffer);

	for (unsigned int i = 0U; i < 3U; i++) {
		data.setSeqNo(i);
		network->write(data);
	}

	data.setDataType(DT_TERMINATOR_WITH_LC);

	fullLC.encode(lc, buffer, DT_TERMINATOR_WITH_LC);

	slotType.setDataType(DT_TERMINATOR_WITH_LC);
	slotType.getData(buffer);

	data.setData(buffer);

	for (unsigned int i = 0U; i < 2U; i++) {
		data.setSeqNo(i + 3U);
		network->write(data);
	}
}

bool CDMRGateway::rewrite(std::vector<CRewrite*>& rewrites, CDMRData & data, bool trace)
{
	for (std::vector<CRewrite*>::iterator it = rewrites.begin(); it != rewrites.end(); ++it) {
		PROCESS_RESULT ret = (*it)->process(data, trace);
		if (ret == PROCESS_RESULT::MATCHED)
			return true;
	}

	return false;
}

unsigned int CDMRGateway::getConfig(const std::string& name, unsigned char* buffer)
{
	assert(buffer != nullptr);

	float lat = m_conf.getInfoLatitude();
	if ((lat > 90) || (lat < -90))
		lat = 0;

	float lon = m_conf.getInfoLongitude();
	if ((lon > 180) || (lon < -180))
		lon = 0;

	int height = m_conf.getInfoHeight();
	if (height > 999)
		height = 999;
	else if (height < 0)
		height = 0;

	std::string location    = m_conf.getInfoLocation();
	std::string description = m_conf.getInfoDescription();
	std::string url         = m_conf.getInfoURL();

	::sprintf((char*)buffer, "%8.8s%9.9s%9.9s%2.2s%2.2s%+08.4f%+09.4f%03d%-20.20s%-19.19s%c%-124.124s%40.40s%40.40s",
		m_config + 0U, m_config + 8U, m_config + 17U, m_config + 26U, m_config + 28U,
		lat, lon, height, location.c_str(),
		description.c_str(), m_config[30U], url.c_str(), m_config + 31U, m_config + 71U);

	m_callsign = std::string((char*)m_config + 0U, 8U);
	size_t n = m_callsign.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
	if (n != std::string::npos)
		m_callsign.erase(n);

	char frequency[10U];
	::memset(frequency, 0x00U, 10U);
	::memcpy(frequency, m_config + 8U, 9U);
	m_rxFrequency = (unsigned int)::atoi(frequency);

	::memset(frequency, 0x00U, 10U);
	::memcpy(frequency, m_config + 17U, 9U);
	m_txFrequency = (unsigned int)::atoi(frequency);

	LogInfo("%s: configuration message: %s", name.c_str(), buffer);

	return (unsigned int)::strlen((char*)buffer);
}

void CDMRGateway::processRadioPosition()
{
	unsigned char buffer[50U];
	unsigned int length;
	bool ret = m_repeater->readRadioPosition(buffer, length);
	if (!ret)
		return;

	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		if (m_networkEnabled[i] &&
			m_dmrNetworks[i] != nullptr && 
			(
				(m_extStatus[1U].m_status == DMRGW_STATUS::DMRNETWORK && m_extStatus[1U].m_dmrNetwork == i) ||
				(m_extStatus[2U].m_status == DMRGW_STATUS::DMRNETWORK && m_extStatus[2U].m_dmrNetwork == i)
			)
		) {
			m_dmrNetworks[i]->writeRadioPosition(buffer, length);
		}
	}
}

void CDMRGateway::processTalkerAlias()
{
	unsigned char buffer[50U];
	unsigned int length;
	bool ret = m_repeater->readTalkerAlias(buffer, length);
	if (!ret)
		return;

	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		if (m_networkEnabled[i] &&
			m_dmrNetworks[i] != nullptr && 
			(
				(m_extStatus[1U].m_status == DMRGW_STATUS::DMRNETWORK && m_extStatus[1U].m_dmrNetwork == i) ||
				(m_extStatus[2U].m_status == DMRGW_STATUS::DMRNETWORK && m_extStatus[2U].m_dmrNetwork == i)
			)
		) {
			m_dmrNetworks[i]->writeTalkerAlias(buffer, length);
		}
	}
}

void CDMRGateway::createAPRS()
{
	if (!m_conf.getAPRSEnabled())
		return;

	std::string address = m_conf.getAPRSAddress();
	unsigned short port = m_conf.getAPRSPort();
	std::string suffix  = m_conf.getAPRSSuffix();
	bool debug          = m_conf.getDebug();

	m_writer = new CAPRSWriter(m_callsign, suffix, address, port, debug);

	std::string desc    = m_conf.getAPRSDescription();
	std::string symbol  = m_conf.getAPRSSymbol();

	m_writer->setInfo(m_txFrequency, m_rxFrequency, desc, symbol);

	float latitude  = m_conf.getInfoLatitude();
	float longitude = m_conf.getInfoLongitude();
	int height      = m_conf.getInfoHeight();

	m_writer->setLocation(latitude, longitude, height);

	bool ret = m_writer->open();
	if (!ret) {
		delete m_writer;
		m_writer = nullptr;
		return;
	}

#if defined(USE_GPSD)
	if (m_gpsd != nullptr)
		m_gpsd->setAPRS(m_writer);
#endif
}

void CDMRGateway::processDynamicTGControl()
{
	unsigned char buffer[100U];
	sockaddr_storage address;
	unsigned int addrlen;
	int len = m_socket->read(buffer, 100U, address, addrlen);
	if (len <= 0)
		return;

	buffer[len] = '\0';

	if (::memcmp(buffer + 0U, "DynTG", 5U) == 0) {
		char* pSlot = ::strtok((char*)(buffer + 5U), ", \r\n");
		char* pTG   = ::strtok(nullptr, ", \r\n");

		if (pSlot == nullptr || pTG == nullptr) {
			LogWarning("Malformed dynamic TG control message");
			return;
		}

		unsigned int slot = (unsigned int)::atoi(pSlot);
		unsigned int tg   = (unsigned int)::atoi(pTG);

		for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
			(*it)->tgChange(slot, tg);
	} else {
		LogWarning("Unknown dynamic TG control message: %s", buffer);
	}
}

void CDMRGateway::remoteControl()
{
	if (m_remoteControl == nullptr)
		return;

	REMOTE_COMMAND command = m_remoteControl->getCommand();
	switch (command) {
		//!!TODO: make command with argument to remove networks limit
		case REMOTE_COMMAND::ENABLE_NETWORK1:
			if (m_dmrNetworkCount < 1) break;
			processEnableCommand(m_dmrNetworks[0], "DMR Network 1", m_networkEnabled[0], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK2:
			if (m_dmrNetworkCount < 2) break;
			processEnableCommand(m_dmrNetworks[1], "DMR Network 2", m_networkEnabled[1], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK3:
			if (m_dmrNetworkCount < 3) break;
			processEnableCommand(m_dmrNetworks[2], "DMR Network 3", m_networkEnabled[2], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK4:
			if (m_dmrNetworkCount < 4) break;
			processEnableCommand(m_dmrNetworks[3], "DMR Network 4", m_networkEnabled[3], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK5:
			if (m_dmrNetworkCount < 5) break;
			processEnableCommand(m_dmrNetworks[4], "DMR Network 5", m_networkEnabled[4], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK6:
			if (m_dmrNetworkCount < 6) break;
			processEnableCommand(m_dmrNetworks[5], "DMR Network 6", m_networkEnabled[5], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK7:
			if (m_dmrNetworkCount < 7) break;
			processEnableCommand(m_dmrNetworks[6], "DMR Network 7", m_networkEnabled[6], true);
			break;
		case REMOTE_COMMAND::ENABLE_NETWORK8:
			if (m_dmrNetworkCount < 8) break;
			processEnableCommand(m_dmrNetworks[7], "DMR Network 8", m_networkEnabled[7], true);
			break;
		case REMOTE_COMMAND::ENABLE_XLX:
			if (m_xlxVoice != nullptr) {
				m_xlxVoice->reset();
			}
			processEnableCommand(m_xlxNetwork, "XLX Network", m_networkXlxEnabled, true);
			break;
		//!!TODO: make command with argument to remove networks limit
		case REMOTE_COMMAND::DISABLE_NETWORK1:
			if (m_dmrNetworkCount < 1) break;
			processEnableCommand(m_dmrNetworks[0], "DMR Network 1", m_networkEnabled[0], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK2:
			if (m_dmrNetworkCount < 2) break;
			processEnableCommand(m_dmrNetworks[1], "DMR Network 2", m_networkEnabled[1], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK3:
			if (m_dmrNetworkCount < 3) break;
			processEnableCommand(m_dmrNetworks[2], "DMR Network 3", m_networkEnabled[2], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK4:
			if (m_dmrNetworkCount < 4) break;
			processEnableCommand(m_dmrNetworks[3], "DMR Network 4", m_networkEnabled[3], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK5:
			if (m_dmrNetworkCount < 5) break;
			processEnableCommand(m_dmrNetworks[4], "DMR Network 5", m_networkEnabled[4], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK6:
			if (m_dmrNetworkCount < 6) break;
			processEnableCommand(m_dmrNetworks[5], "DMR Network 6", m_networkEnabled[5], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK7:
			if (m_dmrNetworkCount < 7) break;
			processEnableCommand(m_dmrNetworks[6], "DMR Network 7", m_networkEnabled[6], false);
			break;
		case REMOTE_COMMAND::DISABLE_NETWORK8:
			if (m_dmrNetworkCount < 8) break;
			processEnableCommand(m_dmrNetworks[7], "DMR Network 8", m_networkEnabled[7], false);
			break;
		case REMOTE_COMMAND::DISABLE_XLX:
			processEnableCommand(m_xlxNetwork, "XLX Network", m_networkXlxEnabled, false);
			break;
		default:
			break;
	}
}

void CDMRGateway::processEnableCommand(CDMRNetwork* network, const std::string& name, bool& mode, bool enabled)
{
	LogDebug("Setting '%s' mode current=%s new=%s", name.c_str(), mode ? "true" : "false", enabled ? "true" : "false");

	if (network != nullptr) {
		mode = enabled;
		network->enable(enabled);
	}
}

void CDMRGateway::buildNetworkStatusString(std::string &str)
{
	str = "";
	buildNetworkStatusNetworkString(str, "xlx", m_xlxNetwork, m_networkXlxEnabled);
	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		str += " ";
		std::string netName = "net" + std::to_string(i + 1);
		buildNetworkStatusNetworkString(str, netName, m_dmrNetworks[i], m_networkEnabled[i]);
	}
}

void CDMRGateway::buildNetworkStatusNetworkString(std::string &str, const std::string& name, CDMRNetwork* network, bool enabled)
{
	str += name + ":"+ (((network == nullptr) || (enabled == false)) ? "n/a" : (network->isConnected() ? "conn" : "disc"));
}

void CDMRGateway::buildNetworkHostsString(std::string &str)
{
	str = "";
	buildNetworkHostNetworkString(str, "xlx", m_xlxNetwork);
	for (unsigned int i = 0; i < m_dmrNetworkCount; i++) {
		str += " ";
		std::string netName = "net" + std::to_string(i + 1);
		buildNetworkHostNetworkString(str, netName, m_dmrNetworks[i]);
	}
}

void CDMRGateway::buildNetworkHostNetworkString(std::string &str, const std::string& name, CDMRNetwork* network)
{
	if (network && (network == m_xlxNetwork)) {
		std::string module = ((m_xlxReflector >= 4001U && m_xlxReflector <= 4026U) ? ("_" + std::string(1, (('A' + (m_xlxReflector % 100U)) - 1U))) : "");
		str += name + ":\"XLX" + m_xlxNumber + module + "\"";
	} else {
		std::string host = ((network == nullptr) ? "NONE" : network->getName());
		str += name + ":\""+ ((network == nullptr) ? "NONE" : ((host.length() > 0) ? host : "NONE")) + "\"";
	}
}
