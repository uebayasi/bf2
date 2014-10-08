%{
#include <stdio.h>
#include <stdlib.h>

#include "yacc.h"

#include "scan.h"

enum {
	PARAM_SIZE = 1,
	PARAM_ENDIAN = 2,
};

YYDECL2(list,
	struct comment *, comment,
	struct reg *, reg,
);
YYDECL1(comment,
	const char *, text,
);
YYDECL3(reg,
	const char *, prefix,
	struct paramlist *, paramlist,
	struct fieldlist *, fieldlist,
);
YYDECL2(param,
	int, type,
	int, size,
);
YYDECL3(field,
	int, width,
	const char *, name,
	struct enumerlist *, enumerlist,
);
YYDECL2(enumer,
	int, num,
	const char *, name,
);

void proccommentlist(struct commentlist *, void *);
void proccomment(struct comment *, void *);
void procreg(struct reg *, void *);
void procfield(struct field *, void *);

void yyerror(const char *msg);
int yywrap(void);
%}

%union {
	int num;
	const char *str;
	struct enumer *enumer;
	struct enumerlist *enumerlist;
	struct field *field;
	struct fieldlist *fieldlist;
	struct param *param;
	struct paramlist *paramlist;
	struct reg *reg;
	struct comment *comment;
	struct list *list;
	struct listlist *listlist;
}

%token	COMMENT
%token	PREFIX
%token	SIZE
%token	ID
%token	NUMBER
%token	NEWLINE

%type	<listlist>	listlist
%type	<list>		list
%type	<comment>	comment
%type	<reg>		reg
%type	<paramlist>	paramlist
%type	<param>		param
%type	<fieldlist>	fieldlist
%type	<field>		field
%type	<enumerlist>	enumerlist
%type	<enumer>	enumer
%type	<str>		prefix
%type	<str>		id
%type	<num>		number

%%

listlist:
	{
		$$ = NULL;
	}
	|
	listlist list
	{
		$$ = mklistlist($1, $2);
	}
list:
   	comment
	{
		$$ = mklist($1, NULL);
	}
	|
	reg
	{
		$$ = mklist(NULL, $1);
	}
comment:
	COMMENT
	{
		$$ = mkcomment(strdup(yytext));
	}
reg:
	prefix paramlist fieldlist
	{
		$$ = mkreg($1, $2, $3);
	}
prefix:
	kw_prefix id newline
	{
		$$ = strdup($2);
	}
paramlist:
	{
		$$ = NULL;
	}
	|
	paramlist param
	{
		$$ = mkparamlist($1, $2);
	}
param:
	kw_size number newline
	{
		$$ = mkparam(PARAM_SIZE, $2);
	}
fieldlist:
	{
		$$ = NULL;
	}
	|
	fieldlist field
	{
		$$ = mkfieldlist($1, $2);
	}
field:
	number id enumerlist newline
	{
		$$ = mkfield($1, $2, $3);
	}
enumerlist:
	{
		$$ = NULL;
	}
	|
	enumerlist enumer
	{
		$$ = mkenumerlist($1, $2);
	}
enumer:
	number id
	{
		$$ = mkenumer($1, $2);
	}
kw_prefix:
	PREFIX
	;
kw_size:
	SIZE
	;
id:
	ID
	{
		$$ = strdup(yytext);
	}
number:
	NUMBER
	{
		$$ = atoi(yytext);
	}
newline:
       NEWLINE
       ;

%%

YYDEF2(list,
	struct comment *, comment,
	struct reg *, reg)
YYDEF1(comment,
	const char *, text)
YYDEF3(reg,
	const char *, prefix,
	struct paramlist *, paramlist,
	struct fieldlist *, fieldlist)
YYDEF2(param,
	int, type,
	int, size)
YYDEF3(field,
	int, width,
	const char *, name,
	struct enumerlist *, enumerlist)
YYDEF2(enumer,
	int, num,
	const char *, name)

void
proccomment(struct comment *comment, void *v)
{
	printf(" * %s\n", comment->text);
}

void
procreg(struct reg *reg, void *v)
{
	printf("reg: %s\n", reg->prefix);
	iterfieldlist(reg->fieldlist, procfield, NULL);
}

void
procfield(struct field *field, void *v)
{
	printf("field: %s\n", field->name);
}

void
yyerror(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

int
yywrap(void)
{
	return 1;
}

int
main(int ac, char *av[])
{
	yyparse();
	return 0;
}
