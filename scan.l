%{
#include "gram.h"
%}

ignore		[#]
kw_prefix	(PREFIX|prefix)
kw_endian	(ENDIAN|endian)
kw_size		(SIZE|size)
kw_comment	(COMMENT|comment)
id		[A-Za-z][A-Za-z0-9_]*
number		[0-9][0-9]*
endian		[BbLl][A-Za-z]*
char		.
blank		[ \t][ \t]*
newline		[\n]

%%

{kw_prefix}	return PREFIX;
{kw_size}	return SIZE;
{id}		return ID;
{number}	return NUMBER;
{newline}	return NEWLINE;
.		;

%%
