/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "Misc.h"

using namespace std;


namespace Misc
{


/*
 * String functions
 */

void str_printf(char **pbuf, int *psize, const char *fmt, ...)
{
	va_list va;
	int len;

	if (*psize <= 0)
		return;
	if (*psize == 1)
	{
		**pbuf = '\0';
		return;
	}
	va_start(va, fmt);
	len = vsnprintf(*pbuf, *psize, fmt, va);
	if (len >= *psize)
		len = *psize - 1;
	*psize -= len;
	*pbuf += len;
}


void StringTrimLeft(string& s, string set)
{
	size_t startpos = s.find_first_not_of(set);
	if (string::npos != startpos)
		s = s.substr(startpos);
}


void StringTrimRight(string& s, string set)
{
	size_t endpos = s.find_last_not_of(set);
	if (string::npos != endpos )
		s = s.substr(0, endpos + 1);
}


void StringTrim(string& s, string set)
{
	StringTrimLeft(s, set);
	StringTrimRight(s, set);
}


void StringSingleSpaces(string& s, string set)
{
	int src;
	int dest;

	int is_space;
	int was_space;
	int started;

	src = 0;
	dest = 0;
	is_space = 0;
	was_space = 0;
	started = 0;
	for (unsigned i = 0; i < s.length(); i++)
	{
		is_space = CharInSet(s[i], set);
		if (is_space)
		{
			src++;
			if (!was_space && started)
				s[dest++] = ' ';
		}
		else
		{
			s[dest++] = s[src++];
			started = 1;
		}
		was_space = is_space;
	}

	/* Get rid of possible extra space at the end */
	if (dest && is_space)
		dest--;

	/* Erase trailing characters */
	s.erase(dest);
}


void StringToLower(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
}


void StringToUpper(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
}


void StringTokenize(vector<std::string>& tokens, string s, string set)
{
	string token;

	int token_start;
	int is_end;

	/* Extract tokens */
	token_start = -1;
	token = "";
	for (unsigned i = 0; i <= s.length(); i++)
	{
		/* End of string */
		is_end = i == s.length();

		/* Start a token */
		if (!is_end && !CharInSet(s[i], set)
				&& token_start == -1)
			token_start = i;

		/* End a token */
		if (token_start > -1 && (is_end || CharInSet(s[i], set)))
		{
			token = s.substr(token_start, i - token_start);
			tokens.push_back(token);
			token_start = -1;
		}
	}
}




/*
 * String maps
 */

static const char *string_map_unknown = "<unknown>";

const char *StringMapValue(StringMap map, int value)
{
	int error;
	return StringMapValue(map, value, error);
}


const char *StringMapValue(StringMap map, int value, int& error)
{
	int index;

	/* Find value */
	error = 0;
	for (index = 0; map[index].text; index++)
		if (map[index].value == value)
			return map[index].text;
	
	/* Not found */
	error = 1;
	return string_map_unknown;
}


int StringMapString(StringMap map, const char *text)
{
	int error;
	return StringMapString(map, text, error);
}


int StringMapString(StringMap map, const char *text, int& error)
{
	int index;

	/* Find value */
	error = 0;
	for (index = 0; map[index].text; index++)
		if (!strcmp(map[index].text, text))
			return map[index].value;

	/* Not found */
	error = 1;
	return 0;
}


int StringMapStringCase(StringMap map, const char *text)
{
	int error;
	return StringMapStringCase(map, text, error);
}


int StringMapStringCase(StringMap map, const char *text, int& error)
{
	int index;

	/* Find value */
	error = 0;
	for (index = 0; map[index].text; index++)
		if (!strcasecmp(map[index].text, text))
			return map[index].value;

	/* Not found */
	error = 1;
	return 0;
}


string StringMapFlags(StringMap map, unsigned int flags)
{
	int i;
	int err;

	stringstream s;
	string comma = "";
	s << '{';
	for (i = 0; i < 32; i++)
	{
		if (flags & (1 << i))
		{
			s << comma;
			const char *text = StringMapValue(map, 1 << i, err);
			if (err)
				s << (1 << i);
			else
				s << text;
			comma = "|";
		}
	}
	s << '}';

	/* Return created text */
	return s.str();
}


string StringMapGetValues(StringMap map)
{
	stringstream s;
	string comma;
	int index;

	index = 0;
	comma = "";
	s << '{';
	while (map[index].text)
	{
		s << comma << map[index].text;
		index++;
		comma = ",";
	}
	s << '}';
	return s.str();
}





/*
 * Output messages
 */

void fatal(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "fatal: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}


void panic(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "panic: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	abort();
}


void warning(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}


}  /* namespace Misc */
