/*
*   Copyright (C) 2017,2020 by Jonathan Naylor G4KLX
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

#include "RewriteDstId.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>

CRewriteDstId::CRewriteDstId(const std::string& name, unsigned int fromId, unsigned int toId) :
CRewrite(),
m_name(name),
m_fromId(fromId),
m_toId(toId)
{
}

CRewriteDstId::~CRewriteDstId()
{
}

PROCESS_RESULT CRewriteDstId::process(CDMRData& data, bool trace)
{
	FLCO flco = data.getFLCO();
	unsigned int dstId = data.getDstId();

	if (flco != FLCO_USER_USER || dstId != m_fromId) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteDstId from %s Src=%u: not matched", m_name.c_str(), m_fromId);

		return RESULT_UNMATCHED;
	}

	data.setDstId(m_toId);

	processMessage(data);

	if (trace) {
		LogDebug("Rule Trace,\tRewriteDstId from %s Src=%u: matched", m_name.c_str(), m_fromId);
		LogDebug("Rule Trace,\tRewriteDstId to %s Src=%u", m_name.c_str(), m_toId);
	}

	return RESULT_MATCHED;
}
