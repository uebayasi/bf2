%{
#include <stdio.h>
#include <stdlib.h>

#include "yacc.h"

#include "scan.h"

#include "gram_local.h"

void dolist(struct list *);

void proccommentlist(struct commentlist *, void *);
void proccomment(struct comment *, void *);
void procreg(struct reg *, void *);
void procfield(struct field *, void *);

void yyerror(const char *msg);
int yywrap(void);
%}

%union {
	struct listlist *listlist;
	struct list *list;
	struct commentlist *commentlist;
	struct comment *comment;
	struct sep *sep;
	struct reg *reg;
	struct paramlist *paramlist;
	struct param *param;
	struct fieldlist *fieldlist;
	struct field *field;
	struct enumerlist *enumerlist;
	struct enumer *enumer;
	const char *str;
	int num;
}

%token	COMMENT
%token	KW_PREFIX
%token	KW_SIZE
%token	KW_ENDIAN
%token	ENDIAN
%token	ID
%token	NUMBER
%token	SEP
%token	NEWLINE

%type	<listlist>	listlist
%type	<list>		list
%type	<commentlist>	commentlist
%type	<comment>	comment
%type	<sep>		sep
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

listlist: /* empty */ {
		$$ = NULL;
	} | listlist list {
		$$ = mklistlist($1, $2);
		alllistlist = $$;
	}
list: commentlist {
		$$ = mklist($1, NULL);
	} | sep {
		$$ = mklist(NULL, NULL);
	} | reg {
		$$ = mklist(NULL, $1);
	}
sep: SEP {
		$$ = mksep();
	}
commentlist: /* empty */ {
		$$ = NULL;
	} | commentlist comment {
		$$ = mkcommentlist($1, $2);
	}
comment: COMMENT {
		$$ = mkcomment(strdup(yytext));
	}
reg: prefix paramlist fieldlist {
		$$ = mkreg($1, $2, $3);
	}
prefix: kw_prefix id newline {
		$$ = strdup($2);
	}
paramlist: /* empty */ {
		$$ = NULL;
	} | paramlist param {
		$$ = mkparamlist($1, $2);
	}
param: kw_size number newline {
		$$ = mkparam(PARAM_SIZE, $2, 0);
	} | kw_endian id newline {
		enum endian endian;
		if ($2[0] == 'L' || $2[0] == 'l')
			endian = ENDIAN_LITTLE;
		else
			endian = ENDIAN_BIG;
		$$ = mkparam(PARAM_ENDIAN, 0, endian);
	}
fieldlist: /* empty */ {
		$$ = NULL;
	} | fieldlist field {
		$$ = mkfieldlist($1, $2);
	}
field: number id enumerlist newline {
		$$ = mkfield($1, $2, $3);
	} | number newline {
		$$ = mkfield($1, NULL, NULL);
	}
enumerlist: /* empty */ {
		$$ = NULL;
	} | enumerlist enumer {
		$$ = mkenumerlist($1, $2);
	}
enumer: number id {
		$$ = mkenumer($1, $2);
	}
kw_prefix: KW_PREFIX ;
kw_size: KW_SIZE ;
kw_endian: KW_ENDIAN ;
id: ID {
		$$ = strdup(yytext);
	}
number: NUMBER {
		$$ = atoi(yytext);
	}
newline: NEWLINE ;

%%

struct listlist *alllistlist;

YYDEF2(list,
	struct commentlist *, commentlist,
	struct reg *, reg)
YYDEF1(comment,
	const char *, text)
YYDEF0(sep)
YYDEF3(reg,
	const char *, prefix,
	struct paramlist *, paramlist,
	struct fieldlist *, fieldlist)
YYDEF3(param,
	int, type,
	int, size,
	enum endian, endian)
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
