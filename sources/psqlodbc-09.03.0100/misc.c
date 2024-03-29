/*-------
 * Module:			misc.c
 *
 * Description:		This module contains miscellaneous routines
 *					such as for debugging/logging and string functions.
 *
 * Classes:			n/a
 *
 * API functions:	none
 *
 * Comments:		See "readme.txt" for copyright and license information.
 *-------
 */

#include "psqlodbc.h"
#include "misc.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef WIN32
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#else
#include <process.h>			/* Byron: is this where Windows keeps def.
								 * of getpid ? */
#endif
#include "connection.h"
#include "multibyte.h"

/*
 *	returns STRCPY_FAIL, STRCPY_TRUNCATED, or #bytes copied
 *	(not including null term)
 */
ssize_t
my_strcpy(char *dst, ssize_t dst_len, const char *src, ssize_t src_len)
{
	if (dst_len <= 0)
		return STRCPY_FAIL;

	if (src_len == SQL_NULL_DATA)
	{
		dst[0] = '\0';
		return STRCPY_NULL;
	}
	else if (src_len == SQL_NTS)
		src_len = strlen(src);

	if (src_len <= 0)
		return STRCPY_FAIL;
	else
	{
		if (src_len < dst_len)
		{
			memcpy(dst, src, src_len);
			dst[src_len] = '\0';
		}
		else
		{
			memcpy(dst, src, dst_len - 1);
			dst[dst_len - 1] = '\0';	/* truncated */
			return STRCPY_TRUNCATED;
		}
	}

	return strlen(dst);
}


/*
 * strncpy copies up to len characters, and doesn't terminate
 * the destination string if src has len characters or more.
 * instead, I want it to copy up to len-1 characters and always
 * terminate the destination string.
 */
char *
strncpy_null(char *dst, const char *src, ssize_t len)
{
	int			i;

	if (NULL != dst)
	{
		/* Just in case, check for special lengths */
		if (len == SQL_NULL_DATA)
		{
			dst[0] = '\0';
			return NULL;
		}
		else if (len == SQL_NTS)
			len = strlen(src) + 1;

		for (i = 0; src[i] && i < len - 1; i++)
			dst[i] = src[i];

		if (len > 0)
			dst[i] = '\0';
	}
	return dst;
}


/*------
 *	Create a null terminated string (handling the SQL_NTS thing):
 *		1. If buf is supplied, place the string in there
 *		   (assumes enough space) and return buf.
 *		2. If buf is not supplied, malloc space and return this string
 *------
 */
char *
make_string(const char *s, ssize_t len, char *buf, size_t bufsize)
{
	size_t		length;
	char	   *str;

	if (!s || SQL_NULL_DATA == len)
		return NULL;
	if (len >= 0)
		length =len;
	else if (SQL_NTS == len)
		length = strlen(s);
	else
	{
		mylog("make_string invalid length=%d\n", len);
		return NULL;
	}
	if (buf)
	{
		strncpy_null(buf, s, bufsize > length ? length + 1 : bufsize);
		return buf;
	}

inolog("malloc size=%d\n", length);
	str = malloc(length + 1);
inolog("str=%p\n", str);
	if (!str)
		return NULL;

	strncpy_null(str, s, length + 1);
	return str;
}

/*------
 *	Create a null terminated lower-case string if the
 *	original string contains upper-case characters.
 *	The SQL_NTS length is considered.
 *------
 */
SQLCHAR *
make_lstring_ifneeded(ConnectionClass *conn, const SQLCHAR *s, ssize_t len, BOOL ifallupper)
{
	ssize_t	length = len;
	char	   *str = NULL;
	const char *ccs = (const char *) s;

	if (s && (len > 0 || (len == SQL_NTS && (length = strlen(ccs)) > 0)))
	{
		int	i;
		const UCHAR *ptr;
		encoded_str encstr;

		make_encoded_str(&encstr, conn, ccs);
		for (i = 0, ptr = ccs; i < length; i++, ptr++)
		{
			encoded_nextchar(&encstr);
			if (ENCODE_STATUS(encstr) != 0)
				continue;
			if (ifallupper && islower(*ptr))
			{
				if (str)
				{
					free(str);
					str = NULL;
				}
				break;
			} 
			if (tolower(*ptr) != *ptr)
			{
				if (!str)
				{
					str = malloc(length + 1);
					memcpy(str, s, length);
					str[length] = '\0';
				}
				str[i] = tolower(*ptr);
			}
		}
	}

	return (SQLCHAR *) str;
}


/*
 *	Concatenate a single formatted argument to a given buffer handling the SQL_NTS thing.
 *	"fmt" must contain somewhere in it the single form '%.*s'.
 *	This is heavily used in creating queries for info routines (SQLTables, SQLColumns).
 *	This routine could be modified to use vsprintf() to handle multiple arguments.
 */
static char *
my_strcat(char *buf, const char *fmt, const char *s, ssize_t len)
{
	if (s && (len > 0 || (len == SQL_NTS && strlen(s) > 0)))
	{
		size_t			length = (len > 0) ? len : strlen(s);

		size_t			pos = strlen(buf);

		sprintf(&buf[pos], fmt, length, s);
		return buf;
	}
	return NULL;
}

char *
schema_strcat(char *buf, const char *fmt, const char *s, ssize_t len, const char *tbname, int tbnmlen, ConnectionClass *conn)
{
	if (!s || 0 == len)
	{
		/*
		 * Note that this driver assumes the implicit schema is
		 * the CURRENT_SCHEMA() though it doesn't worth the
		 * naming.
		 */
		if (conn->schema_support && tbname && (tbnmlen > 0 || tbnmlen == SQL_NTS))
			return my_strcat(buf, fmt, CC_get_current_schema(conn), SQL_NTS);
		return NULL;
	}
	return my_strcat(buf, fmt, s, len);
}


void
remove_newlines(char *string)
{
	size_t i, len = strlen(string);

	for (i = 0; i < len; i++)
	{
		if ((PG_LINEFEED == string[i]) ||
			(PG_CARRIAGE_RETURN == string[i]))
			string[i] = ' ';
	}
}


char *
my_trim(char *s)
{
	char *last;

	for (last = s + strlen(s) - 1; last >= s; last--)
	{
		if (*last == ' ')
			*last = '\0';
		else
			break;
	}

	return s;
}

/*
 *	my_strcat1 is a extension of my_strcat.
 *	It can have 1 more parameter than my_strcat.
 */
static char *
my_strcat1(char *buf, const char *fmt, const char *s1, const char *s, ssize_t len)
{
	ssize_t	length = len;

	if (s && (len > 0 || (len == SQL_NTS && (length = strlen(s)) > 0)))
	{
		size_t	pos = strlen(buf);

		if (s1)
			sprintf(&buf[pos], fmt, s1, length, s);
		else
			sprintf(&buf[pos], fmt, length, s);
		return buf;
	}
	return NULL;
}

char *
schema_strcat1(char *buf, const char *fmt, const char *s1, const char *s, ssize_t len, const char *tbname, int tbnmlen, ConnectionClass *conn)
{
	if (!s || 0 == len)
	{
		if (conn->schema_support && tbname && (tbnmlen > 0 || tbnmlen == SQL_NTS))
			return my_strcat1(buf, fmt, s1, CC_get_current_schema(conn), SQL_NTS);
		return NULL;
	}
	return my_strcat1(buf, fmt, s1, s, len);
}

/*
 * snprintf_add is a extension to snprintf
 * It add format to buf at given pos
 */

int
snprintf_add(char *buf, size_t size, const char *format, ...)
{
	int len;
	size_t pos = strlen(buf);
	va_list arglist;
	
	va_start(arglist, format);
	len = vsnprintf(buf + pos, size - pos, format, arglist);
	va_end(arglist);
	return len;
}

/*
 * snprintf_addlen is a extension to snprintf
 * It returns strlen of buf every time (not -1 when truncated)
 */

size_t
snprintf_len(char *buf, size_t size, const char *format, ...)
{
	ssize_t len;
	va_list arglist;
	
	va_start(arglist, format);
	if ((len = vsnprintf(buf, size, format, arglist)) < 0)
		len = size;
	va_end(arglist);
	return len;
}

#ifndef	HAVE_STRLCAT
size_t
strlcat(char *dst, const char *src, size_t size)
{
	size_t ttllen;
	char	*pd = dst;
	const char *ps= src;
	
	for (ttllen = 0; ttllen < size; ttllen++, pd++)
	{
		if (0 == *pd)
			break; 
	}
	if (ttllen >= size - 1)
		return ttllen + strlen(src);
	for (; ttllen < size - 1; ttllen++, pd++, ps++)
	{
		if (0 == (*pd = *ps))
			return ttllen;
	}
	*pd = 0;
	for (; *ps; ttllen++, ps++)
		;
	return ttllen;
}
#endif /* HAVE_STRLCAT */


/*
 * Proprly quote and escape a possibly schema-qualified table name.
 * Returns a statically allocated buffer.
 */
char *
quote_table(const pgNAME schema, pgNAME table)
{
	static char buf[200];
	const char *ptr;
	int			i;

	i = 0;

	if (NAME_IS_VALID(schema))
	{
		buf[i++] = '"';
		for (ptr = SAFE_NAME(schema); *ptr != '\0' && i < sizeof(buf) - 6; ptr++)
		{
			buf[i++] = *ptr;
			if (*ptr == '"')
				buf[i++] = '"'; 		/* escape quotes by doubling them */
		}
		buf[i++] = '"';
		buf[i++] = '.';
	}

	buf[i++] = '"';
	for (ptr = SAFE_NAME(table); *ptr != '\0' && i < sizeof(buf) - 3; ptr++)
	{
		buf[i++] = *ptr;
		if (*ptr == '"')
			buf[i++] = '"'; 		/* escape quotes by doubling them */
	}
	buf[i++] = '"';
	buf[i] = '\0';

	return buf;
}
