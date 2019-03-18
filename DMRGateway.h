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

#if !defined(DMRGateway_H)
#define	DMRGateway_H

#include "RepeaterProtocol.h"
#include "MMDVMNetwork.h"
#include "DMRNetwork.h"
#include "Reflectors.h"
#include "RewriteTG.h"
#include "Rewrite.h"
#include "Timer.h"
#include "Conf.h"

#include <string>

class CDMRGateway
{
public:
	CDMRGateway(const std::string& confFile);
	~CDMRGateway();

	int run();

private:
	CConf              m_conf;
	IRepeaterProtocol* m_repeater;
	unsigned char*     m_config;
	unsigned int       m_configLen;
	CDMRNetwork*       m_dmrNetwork1;
	std::string        m_dmr1Name;
	CDMRNetwork*       m_dmrNetwork2;
	std::string        m_dmr2Name;
	CDMRNetwork*       m_dmrNetwork3;
	std::string        m_dmr3Name;
	CDMRNetwork*       m_dmrNetwork4;
	std::string        m_dmr4Name;
	CReflectors*       m_xlxReflectors;
	CDMRNetwork*       m_xlxNetwork;
	unsigned int       m_xlxId;
	unsigned int       m_xlxNumber;
	unsigned int       m_xlxReflector;
	unsigned int       m_xlxSlot;
	unsigned int       m_xlxTG;
	unsigned int       m_xlxBase;
	unsigned int       m_xlxLocal;
    unsigned int       m_xlxPort;
    std::string        m_xlxPassword;
	unsigned int       m_xlxStartup;
	unsigned int       m_xlxRoom;
	CTimer 	           m_xlxRelink;
	bool               m_xlxConnected;
	bool               m_xlxDebug;
    bool               m_xlxUserControl;
    char               m_xlxModule;
	CRewriteTG*        m_rptRewrite;
	CRewriteTG*        m_xlxRewrite;
	std::vector<CRewrite*> m_dmr1NetRewrites;
	std::vector<CRewrite*> m_dmr1RFRewrites;
	std::vector<CRewrite*> m_dmr2NetRewrites;
	std::vector<CRewrite*> m_dmr2RFRewrites;
	std::vector<CRewrite*> m_dmr3NetRewrites;
	std::vector<CRewrite*> m_dmr3RFRewrites;
	std::vector<CRewrite*> m_dmr4NetRewrites;
	std::vector<CRewrite*> m_dmr4RFRewrites;
	std::vector<CRewrite*> m_dmr1Passalls;
	std::vector<CRewrite*> m_dmr2Passalls;
	std::vector<CRewrite*> m_dmr3Passalls;
	std::vector<CRewrite*> m_dmr4Passalls;

	bool createMMDVM();
	bool createDMRNetwork1();
	bool createDMRNetwork2();
	bool createDMRNetwork3();
	bool createDMRNetwork4();
	bool createXLXNetwork();

	bool linkXLX(unsigned int number);
	void unlinkXLX();
	void writeXLXLink(unsigned int srcId, unsigned int dstId, CDMRNetwork* network);

	unsigned int getConfig(const std::string& name, unsigned char* buffer);
};

#endif
