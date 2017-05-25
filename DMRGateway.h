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
#include "RewriteTG.h"
#include "Rewrite.h"
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
	CDMRNetwork*       m_dmrNetwork1;
	CDMRNetwork*       m_dmrNetwork2;
	CDMRNetwork*       m_xlxNetwork;
	unsigned int       m_reflector;
	unsigned int       m_xlxSlot;
	unsigned int       m_xlxTG;
	unsigned int       m_xlxBase;
	CRewriteTG*        m_rptRewrite;
	CRewriteTG*        m_xlxRewrite;
	std::vector<IRewrite*> m_dmr1NetRewrites;
	std::vector<IRewrite*> m_dmr1RFRewrites;
	std::vector<IRewrite*> m_dmr2NetRewrites;
	std::vector<IRewrite*> m_dmr2RFRewrites;

	bool createMMDVM();
	bool createDMRNetwork1();
	bool createDMRNetwork2();
	bool createXLXNetwork();
};

#endif
