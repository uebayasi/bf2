%{
#include "gram.h"
%}

kw_prefix	(PREFIX|prefix)
kw_endian	(ENDIAN|endian)
kw_size		(SIZE|size)
kw_comment	(COMMENT|comment)
id		[A-Za-z][A-Za-z0-9_]*
number		[0-9][0-9]*
endian		[BbLl][A-Za-z]*
char		.
space		[ \t]*
blank		[ \t][ \t]*
text		[^\n][^\n]*
newline		[\n]
ignoreline	^{space}#?.*{newline}
ignore		#.*{newline}

%s		_COMMENT

%%

{kw_comment} {
	BEGIN _COMMENT;
}
<_COMMENT>{blank} {
}
<_COMMENT>{newline} {
	BEGIN INITIAL;
}
<_COMMENT>{text} {
	return COMMENT;
}
{kw_prefix}	return PREFIX;
{kw_size}	return SIZE;
{id}		return ID;
{number}	return NUMBER;

{ignoreline}	;
{ignore}	;
{newline}	return NEWLINE;
{blank}		;
.		;

%%
