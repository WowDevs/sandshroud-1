/*
 * Sandshroud Hearthstone
 * Copyright (C) 2010 - 2011 Sandshroud <http://www.sandshroud.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "MD5.h"

MD5Hash::MD5Hash()
{
	MD5_Init(&mC);
}

MD5Hash::~MD5Hash()
{

}

void MD5Hash::UpdateData(const std::string &str)
{
	UpdateData((const uint8*)str.data(), (int)str.length());
}

void MD5Hash::UpdateData(const uint8 *dta, int len)
{
	MD5_Update(&mC, dta, len);
}

void MD5Hash::Finalize()
{
	MD5_Final(mDigest, &mC);
}

void MD5Hash::Initialize()
{
	MD5_Init(&mC);
}

