/*
*   Copyright (C) 2017 by Jonathan Naylor G4KLX
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

#if !defined(PASSALLTG_H)
#define	PASSALLTG_H

#include "Rewrite.h"
#include "DMRData.h"

class CPassAllTG : public IRewrite {
public:
	CPassAllTG(const char* name, unsigned int slot);
	virtual ~CPassAllTG();

	virtual bool process(CDMRData& data, bool trace);

private:
	const char*  m_name;
	unsigned int m_slot;
};


#endif
