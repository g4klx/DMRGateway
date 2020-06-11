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

#include "RewriteSrcId.h"

#include "DMRDefines.h"
#include "Log.h"

#include <cstdio>

CRewriteSrcId::CRewriteSrcId(const std::string& name, unsigned int fromId, unsigned int toId) :
CRewrite(),
m_name(name),
m_fromId(fromId),
m_toId(toId)
{
}

CRewriteSrcId::~CRewriteSrcId()
{
}

PROCESS_RESULT CRewriteSrcId::process(CDMRData& data, bool trace)
{
	unsigned int srcId = data.getSrcId();

	if (srcId != m_fromId) {
		if (trace)
			LogDebug("Rule Trace,\tRewriteSrcId from %s Src=%u: not matched", m_name.c_str(), m_fromId);

		return RESULT_UNMATCHED;
	}

	data.setSrcId(m_toId);

	processMessage(data);

	if (trace) {
		LogDebug("Rule Trace,\tRewriteSrcId from %s Src=%u: matched", m_name.c_str(), m_fromId);
		LogDebug("Rule Trace,\tRewriteSrcId to %s Src=%u", m_name.c_str(), m_toId);
	}

	return RESULT_MATCHED;
}
