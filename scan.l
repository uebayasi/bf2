%{
#include "gram.h"
%}

kw_prefix	^(PREFIX|prefix)
kw_endian	^(ENDIAN|endian)
kw_size		^(SIZE|size)
kw_comment	^(COMMENT|comment)
id		[A-Za-z][A-Za-z0-9_]*
number		[0-9][0-9]*
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
