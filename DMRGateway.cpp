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

#include "DMRGateway.h"
#include "Version.h"
#include "StopWatch.h"
#include "Thread.h"
#include "Log.h"

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

static bool m_killed = false;
static int  m_signal = 0;

#if !defined(_WIN32) && !defined(_WIN64)
static void sigHandler(int signum)
{
  m_killed = true;
  m_signal = signum;
}
#endif

enum DMRGW_STATUS {
	DMRGWS_NONE,
	DMRGWS_NETWORK,
	DMRGWS_REFLECTOR
};

const char* HEADER1 = "This software is for use on amateur radio networks only,";
const char* HEADER2 = "it is to be used for educational purposes only. Its use on";
const char* HEADER3 = "commercial networks is strictly prohibited.";
const char* HEADER4 = "Copyright(C) 2015-2017 by Jonathan Naylor, G4KLX and others";

int main(int argc, char** argv)
{
        const char* iniFile = DEFAULT_INI_FILE;
        if (argc > 1) {
                for (int currentArg = 1; currentArg < argc; ++currentArg) {
                        std::string arg = argv[currentArg];
                        if ((arg == "-v") || (arg == "--version")) {
                                ::fprintf(stdout, "DMRGateway version %s\n", VERSION);
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
  ::signal(SIGTERM, sigHandler);
  ::signal(SIGHUP,  sigHandler);
#endif

  int ret = 0;

  do {
	  m_signal = 0;

	  CDMRGateway* host = new CDMRGateway(std::string(iniFile));
	  ret = host->run();

	  delete host;

	  if (m_signal == 15)
		  ::LogInfo("Caught SIGTERM, exiting");

	  if (m_signal == 1)
		  ::LogInfo("Caught SIGHUP, restarting");
  } while (m_signal == 1);

  ::LogFinalise();

  return ret;
}

CDMRGateway::CDMRGateway(const std::string& confFile) :
m_conf(confFile),
m_mmdvm(NULL),
m_dmrNetwork(NULL),
m_xlxNetwork(NULL),
m_reflector(0U)
{
}

CDMRGateway::~CDMRGateway()
{
}

int CDMRGateway::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "DMRGateway: cannot read the .ini file\n");
		return 1;
	}

	ret = ::LogInitialise(m_conf.getLogFilePath(), m_conf.getLogFileRoot(), m_conf.getLogFileLevel(), m_conf.getLogDisplayLevel());
	if (!ret) {
		::fprintf(stderr, "DMRGateway: unable to open the log file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	bool m_daemon = m_conf.getDaemon();
	if (m_daemon) {
		// Create new process
		pid_t pid = ::fork();
		if (pid == -1) {
			    ::LogWarning("Couldn't fork() , exiting");
			    return -1;
		    }
		else if (pid != 0)
			exit(EXIT_SUCCESS);

		// Create new session and process group
		if (::setsid() == -1){
			    ::LogWarning("Couldn't setsid(), exiting");
			    return -1;
		    }

		// Set the working directory to the root directory
		if (::chdir("/") == -1){
			    ::LogWarning("Couldn't cd /, exiting");
			    return -1;
		    }

		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
		::close(STDERR_FILENO);
#if !defined(HD44780) && !defined(OLED)
		//If we are currently root...
		if (getuid() == 0) {
			struct passwd* user = ::getpwnam("mmdvm");
			if (user == NULL) {
				::LogError("Could not get the mmdvm user, exiting");
				return -1;
			}
			
			uid_t mmdvm_uid = user->pw_uid;
		    gid_t mmdvm_gid = user->pw_gid;

		    //Set user and group ID's to mmdvm:mmdvm
		    if (setgid(mmdvm_gid) != 0) {
			    ::LogWarning("Could not set mmdvm GID, exiting");
			    return -1;
		    }

			if (setuid(mmdvm_uid) != 0) {
			    ::LogWarning("Could not set mmdvm UID, exiting");
			    return -1;
		    }
		    
		    //Double check it worked (AKA Paranoia) 
		    if (setuid(0) != -1){
			    ::LogWarning("It's possible to regain root - something is wrong!, exiting");
			    return -1;
		    }
		
		}
	}
#else
	::LogWarning("Dropping root permissions in daemon mode is disabled with HD44780 display");
	}
#endif
#endif

	LogInfo(HEADER1);
	LogInfo(HEADER2);
	LogInfo(HEADER3);
	LogInfo(HEADER4);

	LogMessage("DMRGateway-%s is starting", VERSION);

	ret = createMMDVM();
	if (!ret)
		return 1;

	LogMessage("Waiting for MMDVM to connect.....");

	for (;;) {
		CThread::sleep(100U);

		unsigned char config[400U];
		unsigned int len = m_mmdvm->getConfig(config);
		if (len > 0U)
			break;
	}

	LogMessage("MMDVM has connected");

	unsigned int xlxSlot        = m_conf.getXLXSlot();
	unsigned int xlxNetworkSlot = m_conf.getXLXNetworkSlot();
	unsigned int xlxNetworkTG   = m_conf.getXLXNetworkTG();
	unsigned int timeout        = m_conf.getTimeout();

	LogInfo("Id: %u", m_mmdvm->getId());
	LogInfo("XLX Local Slot: %u", xlxSlot);
	LogInfo("XLX Reflector Slot: %u", xlxNetworkSlot);
	LogInfo("XLX TG: %u", xlxNetworkTG);
	LogInfo("Timeout: %us", timeout);


	ret = createDMRNetwork();
	if (!ret)
		return 1;

	ret = createXLXNetwork();
	if (!ret)
		return 1;

	DMRGW_STATUS status = DMRGWS_NONE;

	CTimer timer(1000U, timeout);

	CStopWatch stopWatch;
	stopWatch.start();

	LogMessage("DMRGateway-%s is running", VERSION);

	while (!m_killed) {
		CDMRData data;

		bool ret = m_mmdvm->read(data);
		if (ret) {
			unsigned int slotNo = data.getSlotNo();
			if (slotNo == xlxSlot) {
				FLCO flco = data.getFLCO();
				unsigned int id = data.getDstId();

				if (flco == FLCO_GROUP && id == xlxNetworkTG) {
					data.setSlotNo(xlxNetworkSlot);
					m_xlxNetwork->write(data);
					status = DMRGWS_REFLECTOR;
					timer.start();
				} else if (flco == FLCO_USER_USER) {
					unsigned int reflector = data.getDstId();
					if (reflector != m_reflector) {
						LogMessage("Switching to reflector %u", reflector);
						m_reflector = reflector;
					}

					data.setSlotNo(xlxNetworkSlot);
					m_xlxNetwork->write(data);
					status = DMRGWS_REFLECTOR;
					timer.start();
				} else {
					m_dmrNetwork->write(data);
					status = DMRGWS_NETWORK;
					timer.start();
				}
			} else {
				m_dmrNetwork->write(data);
			}
		}

		ret = m_xlxNetwork->read(data);
		if (ret) {
			if (status == DMRGWS_NONE || status == DMRGWS_REFLECTOR) {
				unsigned int slotNo = data.getSlotNo();
				if (slotNo == xlxNetworkSlot) {
					data.setSlotNo(xlxSlot);
					m_mmdvm->write(data);
					status = DMRGWS_REFLECTOR;
					timer.start();
				}
			}
		}

		ret = m_dmrNetwork->read(data);
		if (ret) {
			unsigned int slotNo = data.getSlotNo();
			if (slotNo == xlxSlot) {
				// Stop BM from using the same TG as XLX
				unsigned int dstId = data.getDstId();
				FLCO flco = data.getFLCO();
				if (flco != FLCO_GROUP || dstId != xlxNetworkTG) {
					if (status == DMRGWS_NONE || status == DMRGWS_NETWORK) {
						m_mmdvm->write(data);
						status = DMRGWS_NETWORK;
						timer.start();
					}
				}
			} else {
				m_mmdvm->write(data);
			}
		}

		unsigned int ms = stopWatch.elapsed();
		stopWatch.start();

		m_mmdvm->clock(ms);
		m_dmrNetwork->clock(ms);
		m_xlxNetwork->clock(ms);

		timer.clock(ms);
		if (timer.isRunning() && timer.hasExpired()) {
			status = DMRGWS_NONE;
			timer.stop();
		}

		if (ms < 10U)
			CThread::sleep(10U);
	}

	LogMessage("DMRGateway-%s is exiting on receipt of SIGHUP1", VERSION);

	m_mmdvm->close();
	delete m_mmdvm;

	m_dmrNetwork->close();
	delete m_dmrNetwork;

	m_xlxNetwork->close();
	delete m_xlxNetwork;

	return 0;
}

bool CDMRGateway::createMMDVM()
{
	std::string address = m_conf.getMMDVMAddress();
	unsigned int port   = m_conf.getMMDVMPort();
	unsigned int local  = m_conf.getMMDVMLocal();
	bool debug          = m_conf.getMMDVMDebug();

	LogInfo("MMDVM Network Parameters");
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %u", port);
	if (local > 0U)
		LogInfo("    Local: %u", local);
	else
		LogInfo("    Local: random");

	m_mmdvm = new CMMDVMNetwork(address, port, local, debug);

	bool ret = m_mmdvm->open();
	if (!ret) {
		delete m_mmdvm;
		m_mmdvm = NULL;
		return false;
	}

	return true;
}

bool CDMRGateway::createDMRNetwork()
{
	std::string address  = m_conf.getDMRNetworkAddress();
	unsigned int port    = m_conf.getDMRNetworkPort();
	unsigned int local   = m_conf.getDMRNetworkLocal();
	unsigned int id      = m_mmdvm->getId();
	std::string password = m_conf.getDMRNetworkPassword();
	bool debug           = m_conf.getDMRNetworkDebug();

	LogInfo("DMR Network Parameters");
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %u", port);
	if (local > 0U)
		LogInfo("    Local: %u", local);
	else
		LogInfo("    Local: random");

	m_dmrNetwork = new CDMRNetwork(address, port, local, id, password, debug);

	std::string options = m_mmdvm->getOptions();
	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_dmrNetwork->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = m_mmdvm->getConfig(config);

	m_dmrNetwork->setConfig(config, len);

	bool ret = m_dmrNetwork->open();
	if (!ret) {
		delete m_dmrNetwork;
		m_dmrNetwork = NULL;
		return false;
	}

	return true;
}

bool CDMRGateway::createXLXNetwork()
{
	std::string address  = m_conf.getXLXNetworkAddress();
	unsigned int port    = m_conf.getXLXNetworkPort();
	unsigned int local   = m_conf.getXLXNetworkLocal();
	unsigned int id      = m_mmdvm->getId();
	std::string password = m_conf.getXLXNetworkPassword();
	std::string options  = m_conf.getXLXNetworkOptions();
	bool debug           = m_conf.getXLXNetworkDebug();

	LogInfo("XLX Network Parameters");
	LogInfo("    Address: %s", address.c_str());
	LogInfo("    Port: %u", port);
	if (local > 0U)
		LogInfo("    Local: %u", local);
	else
		LogInfo("    Local: random");

	m_xlxNetwork = new CDMRNetwork(address, port, local, id, password, debug);

	if (!options.empty()) {
		LogInfo("    Options: %s", options.c_str());
		m_xlxNetwork->setOptions(options);
	}

	unsigned char config[400U];
	unsigned int len = m_mmdvm->getConfig(config);

	m_xlxNetwork->setConfig(config, len);

	bool ret = m_xlxNetwork->open();
	if (!ret) {
		delete m_xlxNetwork;
		m_xlxNetwork = NULL;
		return false;
	}

	return true;
}
