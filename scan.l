%{
/*
 * Copyright (c) 2013-2015 Masao Uebayashi <uebayasi@tombiinc.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "gram.h"
%}

kw_prefix	^(PREFIX|prefix)
kw_endian	^(ENDIAN|endian)
kw_size		^(SIZE|size)
kw_comment	^(COMMENT|comment)
kw_offset	^(OFFSET|offset)
id		[A-Za-z][A-Za-z0-9_]*
xdigit		[0-9a-fA-F]
xdigits		{xdigit}{xdigit}*
number		(0x)?{xdigits}
char		.
space		[ \t]*
blank		[ \t][ \t]*
text		[^\n][^\n]*
newline		[\n]
ignorelines	^[ \t\n]*{newline}
ignore		#.*{newline}

%s		_COMMENT

%%

{kw_comment}{space} {
	BEGIN _COMMENT;
}
<_COMMENT>{newline} {
	BEGIN INITIAL;
}
<_COMMENT>{text} {
	return COMMENT;
}
{kw_prefix} {
	return KW_PREFIX;
}
{kw_size} {
	return KW_SIZE;
}
{kw_endian} {
	return KW_ENDIAN;
}
{kw_offset} {
	return KW_OFFSET;
}
{id} {
	return ID;
}
{number} {
	return NUMBER;
}
{ignorelines} {
	return SEP;
}
{ignore} {
}
{newline} {
	return NEWLINE;
}
{blank} ;
. ;

%%
