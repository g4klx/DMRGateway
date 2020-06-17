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

#if !defined(REWRITEDYNTGRF_H)
#define	REWRITEDYNTGRF_H

#include "DynVoice.h"
#include "Rewrite.h"
#include "DMRData.h"

#include "RewriteDynTGNet.h"

#include <string>
#include <vector>

class CRewriteDynTGRF : public CRewrite {
public:
	CRewriteDynTGRF(const std::string& name, unsigned int slot, unsigned int fromTG, unsigned int toTG, unsigned int discPC, unsigned int statusPC, unsigned int range, const std::vector<unsigned int>& exclTGs, CRewriteDynTGNet* rewriteNet, CDynVoice* voice);
	virtual ~CRewriteDynTGRF();

	virtual PROCESS_RESULT process(CDMRData& data, bool trace);

	void stopVoice(unsigned int slot);

	void tgChange(unsigned int slot, unsigned int tg);

private:
	std::string       m_name;
	unsigned int      m_slot;
	unsigned int      m_fromTGStart;
	unsigned int      m_fromTGEnd;
	unsigned int      m_toTG;
	unsigned int      m_discPC;
	unsigned int      m_statusPC;
	std::vector<unsigned int> m_exclTGs;
	CRewriteDynTGNet* m_rewriteNet;
	CDynVoice*        m_voice;
	unsigned int      m_currentTG;
};


#endif
