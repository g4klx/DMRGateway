/*
 *   Copyright (C) 2015-2021,2024 by Jonathan Naylor G4KLX
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
m_status(NULL),
m_repeater(NULL),
m_config(NULL),
m_configLen(0U),
m_dmrNetwork1(NULL),
m_dmr1Name(),
m_dmrNetwork2(NULL),
m_dmr2Name(),
m_dmrNetwork3(NULL),
m_dmr3Name(),
m_dmrNetwork4(NULL),
m_dmr4Name(),
m_dmrNetwork5(NULL),
m_dmr5Name(),
m_xlxReflectors(NULL),
m_xlxNetwork(NULL),
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
m_rptRewrite(NULL),
m_xlxRewrite(NULL),
m_xlxVoice(NULL),
m_dmr1NetRewrites(),
m_dmr1RFRewrites(),
m_dmr1SrcRewrites(),
m_dmr2NetRewrites(),
m_dmr2RFRewrites(),
m_dmr2SrcRewrites(),
m_dmr3NetRewrites(),
m_dmr3RFRewrites(),
m_dmr3SrcRewrites(),
m_dmr4NetRewrites(),
m_dmr4RFRewrites(),
m_dmr4SrcRewrites(),
m_dmr5NetRewrites(),
m_dmr5RFRewrites(),
m_dmr5SrcRewrites(),
m_dmr1Passalls(),
m_dmr2Passalls(),
m_dmr3Passalls(),
m_dmr4Passalls(),
m_dmr5Passalls(),
m_dynVoices(),
m_dynRF(),
m_socket(NULL),
m_writer(NULL),
m_callsign(),
m_txFrequency(0U),
m_rxFrequency(0U),
#if defined(USE_GPSD)
m_gpsd(NULL),
#endif
m_network1Enabled(false),
m_network2Enabled(false),
m_network3Enabled(false),
m_network4Enabled(false),
m_network5Enabled(false),
m_networkXlxEnabled(false),
m_remoteControl(NULL)
{
	CUDPSocket::startup();

	m_status = new DMRGW_STATUS[3U];
	m_status[1U] = DMRGWS_NONE;
	m_status[2U] = DMRGWS_NONE;

	m_config = new unsigned char[400U];
}

CDMRGateway::~CDMRGateway()
{
	for (std::vector<CRewrite*>::iterator it = m_dmr1NetRewrites.begin(); it != m_dmr1NetRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr1RFRewrites.begin(); it != m_dmr1RFRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr1SrcRewrites.begin(); it != m_dmr1SrcRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr2NetRewrites.begin(); it != m_dmr2NetRewrites.end(); ++it)
		delete *it;
	
	for (std::vector<CRewrite*>::iterator it = m_dmr2RFRewrites.begin(); it != m_dmr2RFRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr2SrcRewrites.begin(); it != m_dmr2SrcRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr3NetRewrites.begin(); it != m_dmr3NetRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr3RFRewrites.begin(); it != m_dmr3RFRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr3SrcRewrites.begin(); it != m_dmr3SrcRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr4NetRewrites.begin(); it != m_dmr4NetRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr4RFRewrites.begin(); it != m_dmr4RFRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr4SrcRewrites.begin(); it != m_dmr4SrcRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr5NetRewrites.begin(); it != m_dmr5NetRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr5RFRewrites.begin(); it != m_dmr5RFRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr5SrcRewrites.begin(); it != m_dmr5SrcRewrites.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr1Passalls.begin(); it != m_dmr1Passalls.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr2Passalls.begin(); it != m_dmr2Passalls.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr3Passalls.begin(); it != m_dmr3Passalls.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr4Passalls.begin(); it != m_dmr4Passalls.end(); ++it)
		delete *it;

	for (std::vector<CRewrite*>::iterator it = m_dmr5Passalls.begin(); it != m_dmr5Passalls.end(); ++it)
		delete *it;

	for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it)
		delete* it;

	delete m_rptRewrite;
	delete m_xlxRewrite;

	delete[] m_status;
	delete[] m_config;

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
			if (user == NULL) {
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

	m_network1Enabled = m_conf.getDMRNetwork1Enabled();
	m_network2Enabled = m_conf.getDMRNetwork2Enabled();
	m_network3Enabled = m_conf.getDMRNetwork3Enabled();
	m_network4Enabled = m_conf.getDMRNetwork4Enabled();
	m_network5Enabled = m_conf.getDMRNetwork5Enabled();
	m_networkXlxEnabled = m_conf.getXLXNetworkEnabled();

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
			m_gpsd = NULL;
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

		if (m_xlxNetwork != NULL) {
			m_xlxVoice = new CXLXVoice(directory, language, m_repeater->getId(), m_xlxSlot, m_xlxTG);
			bool ret = m_xlxVoice->open();
			if (!ret) {
				delete m_xlxVoice;
				m_xlxVoice = NULL;
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
			m_remoteControl = NULL;
		}
	}

	if (m_network1Enabled && m_conf.getDMRNetwork1Enabled()) {
		ret = createDMRNetwork1();
		if (!ret)
			return 1;
	}

	if (m_network2Enabled && m_conf.getDMRNetwork2Enabled()) {
		ret = createDMRNetwork2();
		if (!ret)
			return 1;
	}

	if (m_network3Enabled && m_conf.getDMRNetwork3Enabled()) {
		ret = createDMRNetwork3();
		if (!ret)
			return 1;
	}

	if (m_network4Enabled && m_conf.getDMRNetwork4Enabled()) {
		ret = createDMRNetwork4();
		if (!ret)
			return 1;
	}

	if (m_network5Enabled && m_conf.getDMRNetwork5Enabled()) {
		ret = createDMRNetwork5();
		if (!ret)
			return 1;
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

	unsigned int dmr1SrcId[3U];
	unsigned int dmr1DstId[3U];
	dmr1SrcId[1U] = dmr1SrcId[2U] = dmr1DstId[1U] = dmr1DstId[2U] = 0U;

	unsigned int dmr2SrcId[3U];
	unsigned int dmr2DstId[3U];
	dmr2SrcId[1U] = dmr2SrcId[2U] = dmr2DstId[1U] = dmr2DstId[2U] = 0U;

	unsigned int dmr3SrcId[3U];
	unsigned int dmr3DstId[3U];
	dmr3SrcId[1U] = dmr3SrcId[2U] = dmr3DstId[1U] = dmr3DstId[2U] = 0U;

	unsigned int dmr4SrcId[3U];
	unsigned int dmr4DstId[3U];
	dmr4SrcId[1U] = dmr4SrcId[2U] = dmr4DstId[1U] = dmr4DstId[2U] = 0U;

	unsigned int dmr5SrcId[3U];
	unsigned int dmr5DstId[3U];
	dmr5SrcId[1U] = dmr5SrcId[2U] = dmr5DstId[1U] = dmr5DstId[2U] = 0U;

	CStopWatch stopWatch;
	stopWatch.start();

	LogMessage("DMRGateway-%s is running", VERSION);

	while (!m_killed) {
		if (m_networkXlxEnabled && (m_xlxNetwork != NULL)) {
			bool connected = m_xlxNetwork->isConnected();
			if (connected && !m_xlxConnected) {
				if (m_xlxReflector >= 4001U && m_xlxReflector <= 4026U) {
					writeXLXLink(m_xlxId, m_xlxReflector, m_xlxNetwork);
					char c = ('A' + (m_xlxReflector % 100U)) - 1U;
					LogMessage("XLX, Linking to reflector XLX%s %c", m_xlxNumber.c_str(), c);
					if (m_xlxVoice != NULL)
						m_xlxVoice->linkedTo(m_xlxNumber, m_xlxReflector);
				} else if (m_xlxRoom >= 4001U && m_xlxRoom <= 4026U) {
					writeXLXLink(m_xlxId, m_xlxRoom, m_xlxNetwork);
					char c = ('A' + (m_xlxRoom % 100U)) - 1U;
					LogMessage("XLX, Linking to reflector XLX%s %c", m_xlxNumber.c_str(), c);
					if (m_xlxVoice != NULL)
						m_xlxVoice->linkedTo(m_xlxNumber, m_xlxRoom);
					m_xlxReflector = m_xlxRoom;
				} else {
					if (m_xlxVoice != NULL)
						m_xlxVoice->linkedTo(m_xlxNumber, 0U);
				}

				m_xlxConnected = true;

				if (m_xlxNumber == m_xlxStartup && m_xlxRoom == m_xlxReflector)
					m_xlxRelink.stop();
				else
					m_xlxRelink.start();
			} else if (!connected && m_xlxConnected) {
				LogMessage("XLX, Unlinking from XLX%s due to loss of connection", m_xlxNumber.c_str());

				if (m_xlxVoice != NULL)
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
					if (m_xlxVoice != NULL) {
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

			if (flco == FLCO_GROUP && slotNo == m_xlxSlot && dstId == m_xlxTG) {
				if (m_xlxReflector != m_xlxRoom || m_xlxNumber != m_xlxStartup)
					m_xlxRelink.start();

				m_xlxRewrite->process(data, false);
				if (m_networkXlxEnabled) {
					m_xlxNetwork->write(data);
				}
				m_status[slotNo] = DMRGWS_XLXREFLECTOR;
				timer[slotNo]->setTimeout(rfTimeout);
				timer[slotNo]->start();
			} else if ((dstId <= (m_xlxBase + 26U) || dstId == (m_xlxBase + 1000U)) && flco == FLCO_USER_USER && slotNo == m_xlxSlot && dstId >= m_xlxBase && m_xlxUserControl) {
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

				m_status[slotNo] = DMRGWS_XLXREFLECTOR;
				timer[slotNo]->setTimeout(rfTimeout);
				timer[slotNo]->start();

				if (m_xlxVoice != NULL) {
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
			} else if (dstId >= (m_xlxBase + 4000U) && dstId < (m_xlxBase + 5000U) && flco == FLCO_USER_USER && slotNo == m_xlxSlot && m_xlxUserControl) {
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
					LogDebug("Rule Trace, RF transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				PROCESS_RESULT result = RESULT_UNMATCHED;

				if (m_network1Enabled && (m_dmrNetwork1 != NULL)) {
					// Rewrite the slot and/or TG or neither
					for (std::vector<CRewrite*>::iterator it = m_dmr1RFRewrites.begin(); it != m_dmr1RFRewrites.end(); ++it) {
						PROCESS_RESULT res = (*it)->process(data, trace);
						if (res != RESULT_UNMATCHED) {
							result = res;
							break;
						}
					}

					if (result == RESULT_MATCHED) {
						if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK1) {
							rewrite(m_dmr1SrcRewrites, data, trace);
							m_dmrNetwork1->write(data);
							m_status[slotNo] = DMRGWS_DMRNETWORK1;
							timer[slotNo]->setTimeout(rfTimeout);
							timer[slotNo]->start();
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network2Enabled && (m_dmrNetwork2 != NULL)) {
						// Rewrite the slot and/or TG or neither
						for (std::vector<CRewrite*>::iterator it = m_dmr2RFRewrites.begin(); it != m_dmr2RFRewrites.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK2) {
								rewrite(m_dmr2SrcRewrites, data, trace);
								m_dmrNetwork2->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK2;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network3Enabled && (m_dmrNetwork3 != NULL)) {
						// Rewrite the slot and/or TG or neither
						for (std::vector<CRewrite*>::iterator it = m_dmr3RFRewrites.begin(); it != m_dmr3RFRewrites.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK3) {
								rewrite(m_dmr3SrcRewrites, data, trace);
								m_dmrNetwork3->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK3;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network4Enabled && (m_dmrNetwork4 != NULL)) {
						// Rewrite the slot and/or TG or neither
						for (std::vector<CRewrite*>::iterator it = m_dmr4RFRewrites.begin(); it != m_dmr4RFRewrites.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK4) {
								rewrite(m_dmr4SrcRewrites, data, trace);
								m_dmrNetwork4->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK4;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network5Enabled && (m_dmrNetwork5 != NULL)) {
						// Rewrite the slot and/or TG or neither
						for (std::vector<CRewrite*>::iterator it = m_dmr5RFRewrites.begin(); it != m_dmr5RFRewrites.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK5) {
								rewrite(m_dmr5SrcRewrites, data, trace);
								m_dmrNetwork5->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK5;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network1Enabled && (m_dmrNetwork1 != NULL)) {
						for (std::vector<CRewrite*>::iterator it = m_dmr1Passalls.begin(); it != m_dmr1Passalls.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK1) {
								rewrite(m_dmr1SrcRewrites, data, trace);
								m_dmrNetwork1->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK1;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network2Enabled && (m_dmrNetwork2 != NULL)) {
						for (std::vector<CRewrite*>::iterator it = m_dmr2Passalls.begin(); it != m_dmr2Passalls.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK2) {
								rewrite(m_dmr2SrcRewrites, data, trace);
								m_dmrNetwork2->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK2;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network3Enabled && (m_dmrNetwork3 != NULL)) {
						for (std::vector<CRewrite*>::iterator it = m_dmr3Passalls.begin(); it != m_dmr3Passalls.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK3) {
								rewrite(m_dmr3SrcRewrites, data, trace);
								m_dmrNetwork3->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK3;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network4Enabled && (m_dmrNetwork4 != NULL)) {
						for (std::vector<CRewrite*>::iterator it = m_dmr4Passalls.begin(); it != m_dmr4Passalls.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK4) {
								rewrite(m_dmr4SrcRewrites, data, trace);
								m_dmrNetwork4->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK4;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED) {
					if (m_network5Enabled && (m_dmrNetwork5 != NULL)) {
						for (std::vector<CRewrite*>::iterator it = m_dmr5Passalls.begin(); it != m_dmr5Passalls.end(); ++it) {
							PROCESS_RESULT res = (*it)->process(data, trace);
							if (res != RESULT_UNMATCHED) {
								result = res;
								break;
							}
						}

						if (result == RESULT_MATCHED) {
							if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK5) {
								rewrite(m_dmr5SrcRewrites, data, trace);
								m_dmrNetwork5->write(data);
								m_status[slotNo] = DMRGWS_DMRNETWORK5;
								timer[slotNo]->setTimeout(rfTimeout);
								timer[slotNo]->start();
							}
						}
					}
				}

				if (result == RESULT_UNMATCHED && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}
		}

		if (m_networkXlxEnabled && (m_xlxNetwork != NULL)) {
			ret = m_xlxNetwork->read(data);
			if (ret) {
				if (m_status[m_xlxSlot] == DMRGWS_NONE || m_status[m_xlxSlot] == DMRGWS_XLXREFLECTOR) {
					bool ret = m_rptRewrite->process(data, false);
					if (ret) {
						m_repeater->write(data);
						m_status[m_xlxSlot] = DMRGWS_XLXREFLECTOR;
						timer[m_xlxSlot]->setTimeout(netTimeout);
						timer[m_xlxSlot]->start();
					} else {
						unsigned int slotNo = data.getSlotNo();
						unsigned int dstId  = data.getDstId();
						FLCO flco           = data.getFLCO();
						LogWarning("XLX%s, Unexpected data from slot %u %s%u", m_xlxNumber.c_str(), slotNo, flco == FLCO_GROUP ? "TG" : "", dstId);
					}
				}
			}
		}

		if (m_network1Enabled && (m_dmrNetwork1 != NULL)) {
			ret = m_dmrNetwork1->read(data);
			if (ret) {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId  = data.getSrcId();
				unsigned int dstId  = data.getDstId();
				FLCO flco           = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != dmr1SrcId[slotNo] || dstId != dmr1DstId[slotNo])) {
					dmr1SrcId[slotNo] = srcId;
					dmr1DstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, network 1 transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				// Rewrite the slot and/or TG or neither
				bool rewritten = false;
				for (std::vector<CRewrite*>::iterator it = m_dmr1NetRewrites.begin(); it != m_dmr1NetRewrites.end(); ++it) {
					bool ret = (*it)->process(data, trace);
					if (ret) {
						rewritten = true;
						break;
					}
				}

				if (rewritten) {
					// Check that the rewritten slot is free to use.
					slotNo = data.getSlotNo();
					if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK1) {
						for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
							(*it)->stopVoice(slotNo);
						m_repeater->write(data);
						m_status[slotNo] = DMRGWS_DMRNETWORK1;
						timer[slotNo]->setTimeout(netTimeout);
						timer[slotNo]->start();
					}
				}

				if (!rewritten && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}

			ret = m_dmrNetwork1->wantsBeacon();
			if (ret)
				m_repeater->writeBeacon();
		}

		if (m_network2Enabled && (m_dmrNetwork2 != NULL)) {
			ret = m_dmrNetwork2->read(data);
			if (ret) {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId  = data.getSrcId();
				unsigned int dstId  = data.getDstId();
				FLCO flco           = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != dmr2SrcId[slotNo] || dstId != dmr2DstId[slotNo])) {
					dmr2SrcId[slotNo] = srcId;
					dmr2DstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, network 2 transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				// Rewrite the slot and/or TG or neither
				bool rewritten = false;
				for (std::vector<CRewrite*>::iterator it = m_dmr2NetRewrites.begin(); it != m_dmr2NetRewrites.end(); ++it) {
					bool ret = (*it)->process(data, trace);
					if (ret) {
						rewritten = true;
						break;
					}
				}

				if (rewritten) {
					// Check that the rewritten slot is free to use.
					slotNo = data.getSlotNo();
					if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK2) {
						for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
							(*it)->stopVoice(slotNo);
						m_repeater->write(data);
						m_status[slotNo] = DMRGWS_DMRNETWORK2;
						timer[slotNo]->setTimeout(netTimeout);
						timer[slotNo]->start();
					}
				}

				if (!rewritten && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}

			ret = m_dmrNetwork2->wantsBeacon();
			if (ret)
				m_repeater->writeBeacon();
		}

		if (m_network3Enabled && (m_dmrNetwork3 != NULL)) {
			ret = m_dmrNetwork3->read(data);
			if (ret) {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId = data.getSrcId();
				unsigned int dstId = data.getDstId();
				FLCO flco = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != dmr3SrcId[slotNo] || dstId != dmr3DstId[slotNo])) {
					dmr3SrcId[slotNo] = srcId;
					dmr3DstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, network 3 transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				// Rewrite the slot and/or TG or neither
				bool rewritten = false;
				for (std::vector<CRewrite*>::iterator it = m_dmr3NetRewrites.begin(); it != m_dmr3NetRewrites.end(); ++it) {
					bool ret = (*it)->process(data, trace);
					if (ret) {
						rewritten = true;
						break;
					}
				}

				if (rewritten) {
					// Check that the rewritten slot is free to use.
					slotNo = data.getSlotNo();
					if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK3) {
						for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
							(*it)->stopVoice(slotNo);
						m_repeater->write(data);
						m_status[slotNo] = DMRGWS_DMRNETWORK3;
						timer[slotNo]->setTimeout(netTimeout);
						timer[slotNo]->start();
					}
				}

				if (!rewritten && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}

			ret = m_dmrNetwork3->wantsBeacon();
			if (ret)
				m_repeater->writeBeacon();
		}

		if (m_network4Enabled && (m_dmrNetwork4 != NULL)) {
			ret = m_dmrNetwork4->read(data);
			if (ret) {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId = data.getSrcId();
				unsigned int dstId = data.getDstId();
				FLCO flco = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != dmr4SrcId[slotNo] || dstId != dmr4DstId[slotNo])) {
					dmr4SrcId[slotNo] = srcId;
					dmr4DstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, network 4 transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				// Rewrite the slot and/or TG or neither
				bool rewritten = false;
				for (std::vector<CRewrite*>::iterator it = m_dmr4NetRewrites.begin(); it != m_dmr4NetRewrites.end(); ++it) {
					bool ret = (*it)->process(data, trace);
					if (ret) {
						rewritten = true;
						break;
					}
				}

				if (rewritten) {
					// Check that the rewritten slot is free to use.
					slotNo = data.getSlotNo();
					if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK4) {
						for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
							(*it)->stopVoice(slotNo);
						m_repeater->write(data);
						m_status[slotNo] = DMRGWS_DMRNETWORK4;
						timer[slotNo]->setTimeout(netTimeout);
						timer[slotNo]->start();
					}
				}

				if (!rewritten && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}

			ret = m_dmrNetwork4->wantsBeacon();
			if (ret)
				m_repeater->writeBeacon();
		}

		if (m_network5Enabled && (m_dmrNetwork5 != NULL)) {
			ret = m_dmrNetwork5->read(data);
			if (ret) {
				unsigned int slotNo = data.getSlotNo();
				unsigned int srcId = data.getSrcId();
				unsigned int dstId = data.getDstId();
				FLCO flco = data.getFLCO();

				bool trace = false;
				if (ruleTrace && (srcId != dmr5SrcId[slotNo] || dstId != dmr5DstId[slotNo])) {
					dmr5SrcId[slotNo] = srcId;
					dmr5DstId[slotNo] = dstId;
					trace = true;
				}

				if (trace)
					LogDebug("Rule Trace, network 5 transmission: Slot=%u Src=%u Dst=%s%u", slotNo, srcId, flco == FLCO_GROUP ? "TG" : "", dstId);

				// Rewrite the slot and/or TG or neither
				bool rewritten = false;
				for (std::vector<CRewrite*>::iterator it = m_dmr5NetRewrites.begin(); it != m_dmr5NetRewrites.end(); ++it) {
					bool ret = (*it)->process(data, trace);
					if (ret) {
						rewritten = true;
						break;
					}
				}

				if (rewritten) {
					// Check that the rewritten slot is free to use.
					slotNo = data.getSlotNo();
					if (m_status[slotNo] == DMRGWS_NONE || m_status[slotNo] == DMRGWS_DMRNETWORK5) {
						for (std::vector<CRewriteDynTGRF*>::iterator it = m_dynRF.begin(); it != m_dynRF.end(); ++it)
							(*it)->stopVoice(slotNo);
						m_repeater->write(data);
						m_status[slotNo] = DMRGWS_DMRNETWORK5;
						timer[slotNo]->setTimeout(netTimeout);
						timer[slotNo]->start();
					}
				}

				if (!rewritten && trace)
					LogDebug("Rule Trace,\tnot matched so rejected");
			}

			ret = m_dmrNetwork5->wantsBeacon();
			if (ret)
				m_repeater->writeBeacon();
		}

		processRadioPosition();

		processTalkerAlias();

		if (m_networkXlxEnabled && (m_xlxVoice != NULL)) {
			ret = m_xlxVoice->read(data);
			if (ret) {
				m_repeater->write(data);
				m_status[m_xlxSlot] = DMRGWS_XLXREFLECTOR;
				timer[m_xlxSlot]->setTimeout(netTimeout);
				timer[m_xlxSlot]->start();
			}
		}

		for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it) {
			ret = (*it)->read(data);
			if (ret)
				m_repeater->write(data);
		}

		if (m_socket != NULL)
			processDynamicTGControl();

		remoteControl();

		unsigned int ms = stopWatch.elapsed();
		stopWatch.start();

		m_repeater->clock(ms);

		m_xlxRelink.clock(ms);

		if (m_dmrNetwork1 != NULL)
			m_dmrNetwork1->clock(ms);

		if (m_dmrNetwork2 != NULL)
			m_dmrNetwork2->clock(ms);

		if (m_dmrNetwork3 != NULL)
			m_dmrNetwork3->clock(ms);

		if (m_dmrNetwork4 != NULL)
			m_dmrNetwork4->clock(ms);

		if (m_dmrNetwork5 != NULL)
			m_dmrNetwork5->clock(ms);

		if (m_xlxNetwork != NULL)
			m_xlxNetwork->clock(ms);

		if (m_xlxReflectors != NULL)
			m_xlxReflectors->clock(ms);

		if (m_xlxVoice != NULL)
			m_xlxVoice->clock(ms);

#if defined(USE_GPSD)
		if (m_gpsd != NULL)
			m_gpsd->clock(ms);
#endif

		if (m_writer != NULL)
			m_writer->clock(ms);

		for (std::vector<CDynVoice*>::iterator it = m_dynVoices.begin(); it != m_dynVoices.end(); ++it)
			(*it)->clock(ms);

		for (unsigned int i = 1U; i < 3U; i++) {
			timer[i]->clock(ms);
			if (timer[i]->isRunning() && timer[i]->hasExpired()) {
				m_status[i] = DMRGWS_NONE;
				timer[i]->stop();
			}
		}

		if (ms < 10U)
			CThread::sleep(10U);
	}

	delete m_xlxVoice;

	m_repeater->close();
	delete m_repeater;

	if (m_remoteControl != NULL) {
		m_remoteControl->close();
		delete m_remoteControl;
	}

#if defined(USE_GPSD)
	if (m_gpsd != NULL) {
		m_gpsd->close();
		delete m_gpsd;
	}
#endif

	if (m_writer != NULL) {
		m_writer->close();
		delete m_writer;
	}

	if (m_dmrNetwork1 != NULL) {
		m_dmrNetwork1->close(true);
		delete m_dmrNetwork1;
	}

	if (m_dmrNetwork2 != NULL) {
		m_dmrNetwork2->close(true);
		delete m_dmrNetwork2;
	}

	if (m_dmrNetwork3 != NULL) {
		m_dmrNetwork3->close(true);
		delete m_dmrNetwork3;
	}

	if (m_dmrNetwork4 != NULL) {
		m_dmrNetwork4->close(true);
		delete m_dmrNetwork4;
	}

	if (m_dmrNetwork5 != NULL) {
		m_dmrNetwork5->close(true);
		delete m_dmrNetwork5;
	}

	if (m_xlxNetwork != NULL) {
		m_xlxNetwork->close(true);
		delete m_xlxNetwork;
	}

	if (m_socket != NULL) {
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
		m_repeater = NULL;
		return false;
	}

	return true;
}

bool CDMRGateway::createDMRNetwork1()
{
	std::string address  = m_conf.getDMRNetwork1Address();
	unsigned short port  = m_conf.getDMRNetwork1Port();
	unsigned short local = m_conf.getDMRNetwork1Local();
	unsigned int id      = m_conf.getDMRNetwork1Id();
	std::string password = m_conf.getDMRNetwork1Password();
	bool location        = m_conf.getDMRNetwork1Location();
	bool debug           = m_conf.getDMRNetwork1Debug();
	m_dmr1Name           = m_conf.getDMRNetwork1Name();

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network 1 Parameters");
	LogInfo("    Name: %s", m_dmr1Name.c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetwork1 = new CDMRNetwork(address, port, local, id, password, m_dmr1Name, location, debug);

	std::string options = m_conf.getDMRNetwork1Options();

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork1->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmr1Name, config);
	m_dmrNetwork1->setConfig(config, len);

	bool ret = m_dmrNetwork1->open();
	if (!ret) {
		delete m_dmrNetwork1;
		m_dmrNetwork1 = NULL;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != NULL))
		m_gpsd->addNetwork(m_dmrNetwork1);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetwork1TGRewrites();
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite  = new CRewriteTG(m_dmr1Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmr1Name, (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmr1RFRewrites.push_back(rfRewrite);
		m_dmr1NetRewrites.push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetwork1PCRewrites();
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmr1Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr1RFRewrites.push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetwork1TypeRewrites();
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmr1Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr1RFRewrites.push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetwork1SrcRewrites();
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmr1Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmr1NetRewrites.push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetwork1TGDynRewrites();
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = NULL;
		if (m_conf.getVoiceEnabled()) {
			std::string language  = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = NULL;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmr1Name, (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmr1Name, (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmr1RFRewrites.push_back(rfRewriteDynTG);
		m_dmr1NetRewrites.push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetwork1IdRewrites();
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmr1Name, (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmr1Name, (*it).m_netId, (*it).m_rfId);

		m_dmr1SrcRewrites.push_back(rewriteSrcId);
		m_dmr1NetRewrites.push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetwork1PassAllTG();
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG  = new CPassAllTG(m_dmr1Name, *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmr1Name, *it);

		m_dmr1Passalls.push_back(rfPassAllTG);
		m_dmr1NetRewrites.push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetwork1PassAllPC();
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC  = new CPassAllPC(m_dmr1Name, *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmr1Name, *it);

		m_dmr1Passalls.push_back(rfPassAllPC);
		m_dmr1NetRewrites.push_back(netPassAllPC);
	}

	m_dmrNetwork1->enable(true);

	return true;
}

bool CDMRGateway::createDMRNetwork2()
{
	std::string address  = m_conf.getDMRNetwork2Address();
	unsigned short port  = m_conf.getDMRNetwork2Port();
	unsigned short local = m_conf.getDMRNetwork2Local();
	unsigned int id      = m_conf.getDMRNetwork2Id();
	std::string password = m_conf.getDMRNetwork2Password();
	bool location        = m_conf.getDMRNetwork2Location();
	bool debug           = m_conf.getDMRNetwork2Debug();
	m_dmr2Name           = m_conf.getDMRNetwork2Name();

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network 2 Parameters");
	LogInfo("    Name: %s", m_dmr2Name.c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetwork2 = new CDMRNetwork(address, port, local, id, password, m_dmr2Name, location, debug);

	std::string options = m_conf.getDMRNetwork2Options();

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork2->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmr2Name, config);
	m_dmrNetwork2->setConfig(config, len);

	bool ret = m_dmrNetwork2->open();
	if (!ret) {
		delete m_dmrNetwork2;
		m_dmrNetwork2 = NULL;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != NULL))
		m_gpsd->addNetwork(m_dmrNetwork2);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetwork2TGRewrites();
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite  = new CRewriteTG(m_dmr2Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmr2Name, (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmr2RFRewrites.push_back(rfRewrite);
		m_dmr2NetRewrites.push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetwork2PCRewrites();
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmr2Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr2RFRewrites.push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetwork2TypeRewrites();
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmr2Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr2RFRewrites.push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetwork2SrcRewrites();
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmr2Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmr2NetRewrites.push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetwork2TGDynRewrites();
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = NULL;
		if (m_conf.getVoiceEnabled()) {
			std::string language = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = NULL;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmr2Name, (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmr2Name, (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmr2RFRewrites.push_back(rfRewriteDynTG);
		m_dmr2NetRewrites.push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetwork2IdRewrites();
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmr2Name, (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmr2Name, (*it).m_netId, (*it).m_rfId);

		m_dmr2SrcRewrites.push_back(rewriteSrcId);
		m_dmr2NetRewrites.push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetwork2PassAllTG();
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG  = new CPassAllTG(m_dmr2Name, *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmr2Name, *it);

		m_dmr2Passalls.push_back(rfPassAllTG);
		m_dmr2NetRewrites.push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetwork2PassAllPC();
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC  = new CPassAllPC(m_dmr2Name, *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmr2Name, *it);

		m_dmr2Passalls.push_back(rfPassAllPC);
		m_dmr2NetRewrites.push_back(netPassAllPC);
	}

	m_dmrNetwork2->enable(true);

	return true;
}

bool CDMRGateway::createDMRNetwork3()
{
	std::string address  = m_conf.getDMRNetwork3Address();
	unsigned short port  = m_conf.getDMRNetwork3Port();
	unsigned short local = m_conf.getDMRNetwork3Local();
	unsigned int id      = m_conf.getDMRNetwork3Id();
	std::string password = m_conf.getDMRNetwork3Password();
	bool location        = m_conf.getDMRNetwork3Location();
	bool debug           = m_conf.getDMRNetwork3Debug();
	m_dmr3Name           = m_conf.getDMRNetwork3Name();

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network 3 Parameters");
	LogInfo("    Name: %s", m_dmr3Name.c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetwork3 = new CDMRNetwork(address, port, local, id, password, m_dmr3Name, location, debug);

	std::string options = m_conf.getDMRNetwork3Options();

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork3->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmr3Name, config);
	m_dmrNetwork3->setConfig(config, len);

	bool ret = m_dmrNetwork3->open();
	if (!ret) {
		delete m_dmrNetwork3;
		m_dmrNetwork3 = NULL;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != NULL))
		m_gpsd->addNetwork(m_dmrNetwork3);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetwork3TGRewrites();
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite = new CRewriteTG(m_dmr3Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmr3Name, (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmr3RFRewrites.push_back(rfRewrite);
		m_dmr3NetRewrites.push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetwork3PCRewrites();
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmr3Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr3RFRewrites.push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetwork3TypeRewrites();
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmr3Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr3RFRewrites.push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetwork3SrcRewrites();
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmr3Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmr3NetRewrites.push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetwork3TGDynRewrites();
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = NULL;
		if (m_conf.getVoiceEnabled()) {
			std::string language = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = NULL;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmr3Name, (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmr3Name, (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmr3RFRewrites.push_back(rfRewriteDynTG);
		m_dmr3NetRewrites.push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetwork3IdRewrites();
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmr3Name, (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmr3Name, (*it).m_netId, (*it).m_rfId);

		m_dmr3SrcRewrites.push_back(rewriteSrcId);
		m_dmr3NetRewrites.push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetwork3PassAllTG();
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG = new CPassAllTG(m_dmr3Name, *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmr3Name, *it);

		m_dmr3Passalls.push_back(rfPassAllTG);
		m_dmr3NetRewrites.push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetwork3PassAllPC();
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC = new CPassAllPC(m_dmr3Name, *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmr3Name, *it);

		m_dmr3Passalls.push_back(rfPassAllPC);
		m_dmr3NetRewrites.push_back(netPassAllPC);
	}

	m_dmrNetwork3->enable(true);

	return true;
}

bool CDMRGateway::createDMRNetwork4()
{
	std::string address  = m_conf.getDMRNetwork4Address();
	unsigned short port  = m_conf.getDMRNetwork4Port();
	unsigned short local = m_conf.getDMRNetwork4Local();
	unsigned int id      = m_conf.getDMRNetwork4Id();
	std::string password = m_conf.getDMRNetwork4Password();
	bool location        = m_conf.getDMRNetwork4Location();
	bool debug           = m_conf.getDMRNetwork4Debug();
	m_dmr4Name           = m_conf.getDMRNetwork4Name();

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network 4 Parameters");
	LogInfo("    Name: %s", m_dmr4Name.c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetwork4 = new CDMRNetwork(address, port, local, id, password, m_dmr4Name, location, debug);

	std::string options = m_conf.getDMRNetwork4Options();

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork4->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmr4Name, config);
	m_dmrNetwork4->setConfig(config, len);

	bool ret = m_dmrNetwork4->open();
	if (!ret) {
		delete m_dmrNetwork4;
		m_dmrNetwork4 = NULL;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != NULL))
		m_gpsd->addNetwork(m_dmrNetwork4);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetwork4TGRewrites();
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite = new CRewriteTG(m_dmr4Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmr4Name, (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmr4RFRewrites.push_back(rfRewrite);
		m_dmr4NetRewrites.push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetwork4PCRewrites();
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmr4Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr4RFRewrites.push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetwork4TypeRewrites();
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmr4Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr4RFRewrites.push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetwork4SrcRewrites();
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmr4Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmr4NetRewrites.push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetwork4TGDynRewrites();
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = NULL;
		if (m_conf.getVoiceEnabled()) {
			std::string language = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = NULL;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmr4Name, (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmr4Name, (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmr4RFRewrites.push_back(rfRewriteDynTG);
		m_dmr4NetRewrites.push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetwork4IdRewrites();
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmr4Name, (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmr4Name, (*it).m_netId, (*it).m_rfId);

		m_dmr4SrcRewrites.push_back(rewriteSrcId);
		m_dmr4NetRewrites.push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetwork4PassAllTG();
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG = new CPassAllTG(m_dmr4Name, *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmr4Name, *it);

		m_dmr4Passalls.push_back(rfPassAllTG);
		m_dmr4NetRewrites.push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetwork4PassAllPC();
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC = new CPassAllPC(m_dmr4Name, *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmr4Name, *it);

		m_dmr4Passalls.push_back(rfPassAllPC);
		m_dmr4NetRewrites.push_back(netPassAllPC);
	}

	m_dmrNetwork4->enable(true);

	return true;
}

bool CDMRGateway::createDMRNetwork5()
{
	std::string address  = m_conf.getDMRNetwork5Address();
	unsigned short port  = m_conf.getDMRNetwork5Port();
	unsigned short local = m_conf.getDMRNetwork5Local();
	unsigned int id      = m_conf.getDMRNetwork5Id();
	std::string password = m_conf.getDMRNetwork5Password();
	bool location        = m_conf.getDMRNetwork5Location();
	bool debug           = m_conf.getDMRNetwork5Debug();
	m_dmr5Name           = m_conf.getDMRNetwork5Name();

	if (id == 0U)
		id = m_repeater->getId();

	LogInfo("DMR Network 5 Parameters");
	LogInfo("    Name: %s", m_dmr5Name.c_str());
	LogInfo("    Id: %u", id);
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %hu", port);
	if (local > 0U)
		LogInfo("    Local: %hu", local);
	else
		LogInfo("    Local: random");
	LogInfo("    Location Data: %s", location ? "yes" : "no");

	m_dmrNetwork5 = new CDMRNetwork(address, port, local, id, password, m_dmr5Name, location, debug);

	std::string options = m_conf.getDMRNetwork5Options();

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork5->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = getConfig(m_dmr5Name, config);
	m_dmrNetwork5->setConfig(config, len);

	bool ret = m_dmrNetwork5->open();
	if (!ret) {
		delete m_dmrNetwork5;
		m_dmrNetwork5 = NULL;
		return false;
	}

#if defined(USE_GPSD)
	if (location && (m_gpsd != NULL))
		m_gpsd->addNetwork(m_dmrNetwork5);
#endif

	std::vector<CTGRewriteStruct> tgRewrites = m_conf.getDMRNetwork5TGRewrites();
	for (std::vector<CTGRewriteStruct>::const_iterator it = tgRewrites.begin(); it != tgRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite RF: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U);
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:TG%u -> %u:TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG);
		else
			LogInfo("    Rewrite Net: %u:TG%u-TG%u -> %u:TG%u-TG%u", (*it).m_toSlot, (*it).m_toTG, (*it).m_toTG + (*it).m_range - 1U, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U);

		CRewriteTG* rfRewrite = new CRewriteTG(m_dmr5Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);
		CRewriteTG* netRewrite = new CRewriteTG(m_dmr5Name, (*it).m_toSlot, (*it).m_toTG, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_range);

		m_dmr5RFRewrites.push_back(rfRewrite);
		m_dmr5NetRewrites.push_back(netRewrite);
	}

	std::vector<CPCRewriteStruct> pcRewrites = m_conf.getDMRNetwork5PCRewrites();
	for (std::vector<CPCRewriteStruct>::const_iterator it = pcRewrites.begin(); it != pcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewritePC* rewrite = new CRewritePC(m_dmr5Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr5RFRewrites.push_back(rewrite);
	}

	std::vector<CTypeRewriteStruct> typeRewrites = m_conf.getDMRNetwork5TypeRewrites();
	for (std::vector<CTypeRewriteStruct>::const_iterator it = typeRewrites.begin(); it != typeRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite RF: %u:TG%u -> %u:%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId);
		else
			LogInfo("    Rewrite RF: %u:TG%u-%u -> %u:%u-%u", (*it).m_fromSlot, (*it).m_fromTG, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toId, (*it).m_toId + (*it).m_range - 1U);

		CRewriteType* rewrite = new CRewriteType(m_dmr5Name, (*it).m_fromSlot, (*it).m_fromTG, (*it).m_toSlot, (*it).m_toId, (*it).m_range);

		m_dmr5RFRewrites.push_back(rewrite);
	}

	std::vector<CSrcRewriteStruct> srcRewrites = m_conf.getDMRNetwork5SrcRewrites();
	for (std::vector<CSrcRewriteStruct>::const_iterator it = srcRewrites.begin(); it != srcRewrites.end(); ++it) {
		if ((*it).m_range == 1)
			LogInfo("    Rewrite Net: %u:%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG);
		else
			LogInfo("    Rewrite Net: %u:%u-%u -> %u:TG%u", (*it).m_fromSlot, (*it).m_fromId, (*it).m_fromId + (*it).m_range - 1U, (*it).m_toSlot, (*it).m_toTG);

		CRewriteSrc* rewrite = new CRewriteSrc(m_dmr5Name, (*it).m_fromSlot, (*it).m_fromId, (*it).m_toSlot, (*it).m_toTG, (*it).m_range);

		m_dmr5NetRewrites.push_back(rewrite);
	}

	std::vector<CTGDynRewriteStruct> dynRewrites = m_conf.getDMRNetwork5TGDynRewrites();
	for (std::vector<CTGDynRewriteStruct>::const_iterator it = dynRewrites.begin(); it != dynRewrites.end(); ++it) {
		LogInfo("    Dyn Rewrite: %u:TG%u-%u:TG%u <-> %u:TG%u (disc %u:%u) (status %u:%u) (%u exclusions)", (*it).m_slot, (*it).m_fromTG, (*it).m_slot, (*it).m_fromTG + (*it).m_range - 1U, (*it).m_slot, (*it).m_toTG, (*it).m_slot, (*it).m_discPC, (*it).m_slot, (*it).m_statusPC, (*it).m_exclTGs.size());

		CDynVoice* voice = NULL;
		if (m_conf.getVoiceEnabled()) {
			std::string language = m_conf.getVoiceLanguage();
			std::string directory = m_conf.getVoiceDirectory();

			voice = new CDynVoice(directory, language, m_repeater->getId(), (*it).m_slot, (*it).m_toTG);
			bool ret = voice->open();
			if (!ret) {
				delete voice;
				voice = NULL;
			} else {
				m_dynVoices.push_back(voice);
			}
		}

		CRewriteDynTGNet* netRewriteDynTG = new CRewriteDynTGNet(m_dmr5Name, (*it).m_slot, (*it).m_toTG);
		CRewriteDynTGRF* rfRewriteDynTG = new CRewriteDynTGRF(m_dmr5Name, (*it).m_slot, (*it).m_fromTG, (*it).m_toTG, (*it).m_discPC, (*it).m_statusPC, (*it).m_range, (*it).m_exclTGs, netRewriteDynTG, voice);

		m_dmr5RFRewrites.push_back(rfRewriteDynTG);
		m_dmr5NetRewrites.push_back(netRewriteDynTG);
		m_dynRF.push_back(rfRewriteDynTG);
	}

	std::vector<CIdRewriteStruct> idRewrites = m_conf.getDMRNetwork5IdRewrites();
	for (std::vector<CIdRewriteStruct>::const_iterator it = idRewrites.begin(); it != idRewrites.end(); ++it) {
		LogInfo("    Rewrite Id: %u <-> %u", (*it).m_rfId, (*it).m_netId);

		CRewriteSrcId* rewriteSrcId = new CRewriteSrcId(m_dmr5Name, (*it).m_rfId, (*it).m_netId);
		CRewriteDstId* rewriteDstId = new CRewriteDstId(m_dmr5Name, (*it).m_netId, (*it).m_rfId);

		m_dmr5SrcRewrites.push_back(rewriteSrcId);
		m_dmr5NetRewrites.push_back(rewriteDstId);
	}

	std::vector<unsigned int> tgPassAll = m_conf.getDMRNetwork5PassAllTG();
	for (std::vector<unsigned int>::const_iterator it = tgPassAll.begin(); it != tgPassAll.end(); ++it) {
		LogInfo("    Pass All TG: %u", *it);

		CPassAllTG* rfPassAllTG = new CPassAllTG(m_dmr5Name, *it);
		CPassAllTG* netPassAllTG = new CPassAllTG(m_dmr5Name, *it);

		m_dmr5Passalls.push_back(rfPassAllTG);
		m_dmr5NetRewrites.push_back(netPassAllTG);
	}

	std::vector<unsigned int> pcPassAll = m_conf.getDMRNetwork5PassAllPC();
	for (std::vector<unsigned int>::const_iterator it = pcPassAll.begin(); it != pcPassAll.end(); ++it) {
		LogInfo("    Pass All PC: %u", *it);

		CPassAllPC* rfPassAllPC = new CPassAllPC(m_dmr5Name, *it);
		CPassAllPC* netPassAllPC = new CPassAllPC(m_dmr5Name, *it);

		m_dmr5Passalls.push_back(rfPassAllPC);
		m_dmr5NetRewrites.push_back(netPassAllPC);
	}

	m_dmrNetwork5->enable(true);

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
		m_socket = NULL;
		return false;
	}

	return true;
}

bool CDMRGateway::linkXLX(const std::string &number)
{
	CReflector* reflector = m_xlxReflectors->find(number);
	if (reflector == NULL)
		return false;

	if (m_xlxNetwork != NULL) {
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
		m_xlxNetwork = NULL;
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
	if (m_xlxNetwork != NULL) {
		m_xlxNetwork->close(true);
		delete m_xlxNetwork;
		m_xlxNetwork = NULL;
	}

	m_xlxConnected = false;
	m_xlxRelink.stop();
}

void CDMRGateway::writeXLXLink(unsigned int srcId, unsigned int dstId, CDMRNetwork* network)
{
	assert(network != NULL);

	unsigned int streamId = ::rand() + 1U;

	CDMRData data;

	data.setSlotNo(XLX_SLOT);
	data.setFLCO(FLCO_USER_USER);
	data.setSrcId(srcId);
	data.setDstId(dstId);
	data.setDataType(DT_VOICE_LC_HEADER);
	data.setN(0U);
	data.setStreamId(streamId);

	unsigned char buffer[DMR_FRAME_LENGTH_BYTES];

	CDMRLC lc;
	lc.setSrcId(srcId);
	lc.setDstId(dstId);
	lc.setFLCO(FLCO_USER_USER);

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
	for (std::vector<CRewrite*>::iterator it = rewrites.begin(); it != rewrites.end(); ++it)
		if ((*it)->process(data, trace))
			return true;
	return false;
}

unsigned int CDMRGateway::getConfig(const std::string& name, unsigned char* buffer)
{
	assert(buffer != NULL);

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

	if (m_network1Enabled && (m_dmrNetwork1 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK1 || m_status[2U] == DMRGWS_DMRNETWORK1))
		m_dmrNetwork1->writeRadioPosition(buffer, length);

	if (m_network2Enabled && (m_dmrNetwork2 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK2 || m_status[2U] == DMRGWS_DMRNETWORK2))
		m_dmrNetwork2->writeRadioPosition(buffer, length);

	if (m_network3Enabled && (m_dmrNetwork3 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK3 || m_status[2U] == DMRGWS_DMRNETWORK3))
		m_dmrNetwork3->writeRadioPosition(buffer, length);

	if (m_network4Enabled && (m_dmrNetwork4 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK4 || m_status[2U] == DMRGWS_DMRNETWORK4))
		m_dmrNetwork4->writeRadioPosition(buffer, length);

	if (m_network5Enabled && (m_dmrNetwork5 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK5 || m_status[2U] == DMRGWS_DMRNETWORK5))
		m_dmrNetwork5->writeRadioPosition(buffer, length);
}

void CDMRGateway::processTalkerAlias()
{
	unsigned char buffer[50U];
	unsigned int length;
	bool ret = m_repeater->readTalkerAlias(buffer, length);
	if (!ret)
		return;

	if (m_network1Enabled && (m_dmrNetwork1 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK1 || m_status[2U] == DMRGWS_DMRNETWORK1))
		m_dmrNetwork1->writeTalkerAlias(buffer, length);

	if (m_network2Enabled && (m_dmrNetwork2 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK2 || m_status[2U] == DMRGWS_DMRNETWORK2))
		m_dmrNetwork2->writeTalkerAlias(buffer, length);

	if (m_network3Enabled && (m_dmrNetwork3 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK3 || m_status[2U] == DMRGWS_DMRNETWORK3))
		m_dmrNetwork3->writeTalkerAlias(buffer, length);

	if (m_network4Enabled && (m_dmrNetwork4 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK4 || m_status[2U] == DMRGWS_DMRNETWORK4))
		m_dmrNetwork4->writeTalkerAlias(buffer, length);

	if (m_network5Enabled && (m_dmrNetwork5 != NULL) && (m_status[1U] == DMRGWS_DMRNETWORK5 || m_status[2U] == DMRGWS_DMRNETWORK5))
		m_dmrNetwork5->writeTalkerAlias(buffer, length);
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
		m_writer = NULL;
		return;
	}

#if defined(USE_GPSD)
	if (m_gpsd != NULL)
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
		char* pTG   = ::strtok(NULL, ", \r\n");

		if (pSlot == NULL || pTG == NULL) {
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
	if (m_remoteControl == NULL)
		return;

	REMOTE_COMMAND command = m_remoteControl->getCommand();
	switch (command) {
		case RCD_ENABLE_NETWORK1:
			processEnableCommand(m_dmrNetwork1, "DMR Network 1", m_network1Enabled, true);
			break;
		case RCD_ENABLE_NETWORK2:
			processEnableCommand(m_dmrNetwork2, "DMR Network 2", m_network2Enabled, true);
			break;
		case RCD_ENABLE_NETWORK3:
			processEnableCommand(m_dmrNetwork3, "DMR Network 3", m_network3Enabled, true);
			break;
		case RCD_ENABLE_NETWORK4:
			processEnableCommand(m_dmrNetwork4, "DMR Network 4", m_network4Enabled, true);
			break;
		case RCD_ENABLE_NETWORK5:
			processEnableCommand(m_dmrNetwork5, "DMR Network 5", m_network5Enabled, true);
			break;
		case RCD_ENABLE_XLX:
			if (m_xlxVoice != NULL) {
				m_xlxVoice->reset();
			}
			processEnableCommand(m_xlxNetwork, "XLX Network", m_networkXlxEnabled, true);
			break;
		case RCD_DISABLE_NETWORK1:
			processEnableCommand(m_dmrNetwork1, "DMR Network 1", m_network1Enabled, false);
			break;
		case RCD_DISABLE_NETWORK2:
			processEnableCommand(m_dmrNetwork2, "DMR Network 2", m_network2Enabled, false);
			break;
		case RCD_DISABLE_NETWORK3:
			processEnableCommand(m_dmrNetwork3, "DMR Network 3", m_network3Enabled, false);
			break;
		case RCD_DISABLE_NETWORK4:
			processEnableCommand(m_dmrNetwork4, "DMR Network 4", m_network4Enabled, false);
			break;
		case RCD_DISABLE_NETWORK5:
			processEnableCommand(m_dmrNetwork5, "DMR Network 5", m_network5Enabled, false);
			break;
		case RCD_DISABLE_XLX:
			processEnableCommand(m_xlxNetwork, "XLX Network", m_networkXlxEnabled, false);
			break;
		default:
			break;
	}
}

void CDMRGateway::processEnableCommand(CDMRNetwork* network, const std::string& name, bool& mode, bool enabled)
{
	LogDebug("Setting '%s' mode current=%s new=%s", name.c_str(), mode ? "true" : "false", enabled ? "true" : "false");

	if (network != NULL) {
		mode = enabled;
		network->enable(enabled);
	}
}

void CDMRGateway::buildNetworkStatusString(std::string &str)
{
	str = "";
	buildNetworkStatusNetworkString(str, "xlx", m_xlxNetwork, m_networkXlxEnabled);
	str += " ";
	buildNetworkStatusNetworkString(str, "net1", m_dmrNetwork1, m_network1Enabled);
	str += " ";
	buildNetworkStatusNetworkString(str, "net2", m_dmrNetwork2, m_network2Enabled);
	str += " ";
	buildNetworkStatusNetworkString(str, "net3", m_dmrNetwork3, m_network3Enabled);
	str += " ";
	buildNetworkStatusNetworkString(str, "net4", m_dmrNetwork4, m_network4Enabled);
	str += " ";
	buildNetworkStatusNetworkString(str, "net5", m_dmrNetwork5, m_network5Enabled);
}

void CDMRGateway::buildNetworkStatusNetworkString(std::string &str, const std::string& name, CDMRNetwork* network, bool enabled)
{
	str += name + ":"+ (((network == NULL) || (enabled == false)) ? "n/a" : (network->isConnected() ? "conn" : "disc"));
}

void CDMRGateway::buildNetworkHostsString(std::string &str)
{
	str = "";
	buildNetworkHostNetworkString(str, "xlx", m_xlxNetwork);
	str += " ";
	buildNetworkHostNetworkString(str, "net1", m_dmrNetwork1);
	str += " ";
	buildNetworkHostNetworkString(str, "net2", m_dmrNetwork2);
	str += " ";
	buildNetworkHostNetworkString(str, "net3", m_dmrNetwork3);
	str += " ";
	buildNetworkHostNetworkString(str, "net4", m_dmrNetwork4);
	str += " ";
	buildNetworkHostNetworkString(str, "net5", m_dmrNetwork5);
}

void CDMRGateway::buildNetworkHostNetworkString(std::string &str, const std::string& name, CDMRNetwork* network)
{
	if (network && (network == m_xlxNetwork)) {
		std::string module = ((m_xlxReflector >= 4001U && m_xlxReflector <= 4026U) ? ("_" + std::string(1, (('A' + (m_xlxReflector % 100U)) - 1U))) : "");
		str += name + ":\"XLX" + m_xlxNumber + module + "\"";
	} else {
		std::string host = ((network == NULL) ? "NONE" : network->getName());
		str += name + ":\""+ ((network == NULL) ? "NONE" : ((host.length() > 0) ? host : "NONE")) + "\"";
	}
}
