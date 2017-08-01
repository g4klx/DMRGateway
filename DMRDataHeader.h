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

#ifndef DMRDataHeader_H
#define DMRDataHeader_H

class CDMRDataHeader
{
public:
	CDMRDataHeader();
	~CDMRDataHeader();

	bool put(const unsigned char* bytes);

	void get(unsigned char* bytes) const;

	bool          getGI() const;
	void          setGI(bool group);

	unsigned int  getSrcId() const;
	void          setSrcId(unsigned int id);

	unsigned int  getDstId() const;
	void          setDstId(unsigned int id);

private:
	unsigned char* m_data;
};

#endif

