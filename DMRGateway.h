/*
 *   Copyright (C) 2015,2016,2017,2019,2020,2025 by Jonathan Naylor G4KLX
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

#if !defined(DMRGateway_H)
#define	DMRGateway_H

#include "RemoteControl.h"
#include "RewriteDynTGNet.h"
#include "RewriteDynTGRF.h"
#include "MMDVMNetwork.h"
#include "DMRNetwork.h"
#include "APRSWriter.h"
#include "Reflectors.h"
#include "XLXVoice.h"
#include "UDPSocket.h"
#include "RewriteTG.h"
#include "DynVoice.h"
#include "Rewrite.h"
#include "Timer.h"
#include "Conf.h"
#include "GPSD.h"

#include <string>

enum class DMRGW_STATUS {
	NONE,
	DMRNETWORK,
	XLXREFLECTOR
};

struct CDMRGWExtStatus {
	DMRGW_STATUS m_status;
	unsigned int m_dmrNetwork;
};

class CDMRGateway
{
public:
	CDMRGateway(const std::string& confFile);
	~CDMRGateway();

	int run();

	void buildNetworkStatusString(std::string &str);
	void buildNetworkHostsString(std::string &str);

private:
	CConf              m_conf;
	CDMRGWExtStatus*   m_extStatus;
	CMMDVMNetwork*     m_repeater;
	unsigned char*     m_config;
	unsigned int       m_configLen;
	unsigned int       m_dmrNetworkCount;
	std::vector<CDMRNetwork*> m_dmrNetworks;
	std::vector<std::string> m_dmrName;
	CReflectors*       m_xlxReflectors;
	CDMRNetwork*       m_xlxNetwork;
	unsigned int       m_xlxId;
	std::string        m_xlxNumber;
	unsigned int       m_xlxReflector;
	unsigned int       m_xlxSlot;
	unsigned int       m_xlxTG;
	unsigned int       m_xlxBase;
	unsigned short     m_xlxLocal;
	unsigned short     m_xlxPort;
	std::string        m_xlxPassword;
	std::string        m_xlxStartup;
	unsigned int       m_xlxRoom;
	CTimer 	           m_xlxRelink;
	bool               m_xlxConnected;
	bool               m_xlxDebug;
	bool               m_xlxUserControl;
	char               m_xlxModule;
	CRewriteTG*        m_rptRewrite;
	CRewriteTG*        m_xlxRewrite;
	CXLXVoice*         m_xlxVoice;
	std::vector<std::vector<CRewrite*>> m_dmrNetRewrites;
	std::vector<std::vector<CRewrite*>> m_dmrRFRewrites;
	std::vector<std::vector<CRewrite*>> m_dmrSrcRewrites;
	std::vector<std::vector<CRewrite*>> m_dmrPassalls;
	std::vector<CDynVoice*> m_dynVoices;
	std::vector<CRewriteDynTGRF*> m_dynRF;
	CUDPSocket*            m_socket;
	CAPRSWriter*           m_writer;
	std::string            m_callsign;
	unsigned int           m_txFrequency;
	unsigned int           m_rxFrequency;
#if defined(USE_GPSD)
	CGPSD*                 m_gpsd;
#endif
	bool*                  m_networkEnabled;
	bool                   m_networkXlxEnabled;
	CRemoteControl*        m_remoteControl;

	bool createMMDVM();
	bool createDMRNetwork(unsigned int index);
	bool createXLXNetwork();
	bool createDynamicTGControl();

	bool linkXLX(const std::string &number);
	void unlinkXLX();
	void writeXLXLink(unsigned int srcId, unsigned int dstId, CDMRNetwork* network);

	bool rewrite(std::vector<CRewrite*>& rewrites, CDMRData& data, bool trace);

	unsigned int getConfig(const std::string& name, unsigned char* buffer);

	void processRadioPosition();
	void processTalkerAlias();
	void createAPRS();
	void processDynamicTGControl();
	void remoteControl();
	void processEnableCommand(CDMRNetwork* network, const std::string& name, bool& mode, bool enabled);
	void buildNetworkStatusNetworkString(std::string &str, const std::string& name, CDMRNetwork* network, bool enabled);
	void buildNetworkHostNetworkString(std::string &str, const std::string& name, CDMRNetwork* network);
};

#endif
