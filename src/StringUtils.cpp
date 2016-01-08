// Copyright © 2016 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: MIT, see the LICENSE file.

#include "stdafx.h"

#include "StringUtils.h"

using namespace CellVision;

std::istream& StringUtils::safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	while (true)
	{
		int c = sb->sbumpc();

		switch (c)
		{
			case '\n':
				return is;
			case '\r':
				if (sb->sgetc() == '\n')
					sb->sbumpc();
				return is;
			case EOF:
				if (t.empty())
					is.setstate(std::ios::eofbit);
				return is;
			default:
				t += char(c);
		}
	}
}
