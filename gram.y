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

#include <stdio.h>
#include <stdlib.h>

#include "yacc.h"

#include "scan.h"

#include "gram_local.h"

void dolist(struct list *);

struct reg *makereg(const char *, struct paramlist *,
    struct fieldlist *);
struct field *makefield(int, const char *, struct enumerlist *);
void proccommentlist(struct commentlist *, void *);
void proccomment(struct comment *, void *);
void procreg(struct reg *, void *);
void procfield(struct field *, void *);

void yyerror(const char *msg);
int yywrap(void);

struct field *prev_field;
%}

%union {
	struct listlist *listlist;
	struct list *list;
	struct commentlist *commentlist;
	struct comment *comment;
	struct sep *sep;
	struct base *base;
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
%token	KW_BASE
%token	KW_PREFIX
%token	KW_SIZE
%token	KW_ENDIAN
%token	KW_OFFSET
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
%type	<base>		base
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

listlist	: /* empty */ {
			$$ = NULL;
		}
		| listlist list {
			$$ = mklistlist($1, $2);
			alllistlist = $$;
		}

list		: commentlist { $$ = mklist($1, NULL, NULL); }
		| sep { $$ = mklist(NULL, NULL, NULL); }
		| base { $$ = mklist(NULL, $1, NULL); }
		| reg { $$ = mklist(NULL, NULL, $1); }

sep		: SEP { $$ = mksep(); }

commentlist	: /* empty */ { $$ = NULL; }
		| commentlist comment { $$ = mkcommentlist($1, $2); }

comment		: COMMENT { $$ = mkcomment(strdup(yytext)); }

base		: kw_base number newline {
			$$ = mkbase($2);
		}

reg		: prefix paramlist fieldlist {
			prev_field = NULL;
			$$ = makereg($1, $2, $3);
		}

prefix		: kw_prefix id newline { $$ = strdup($2); }

paramlist	: /* empty */ { $$ = NULL; }
		| paramlist param { $$ = mkparamlist($1, $2); }

param		: kw_size number newline { $$ = mkparam(PARAM_SIZE, $2, 0); }
		| kw_endian id newline {
			enum endian endian;
			if ($2[0] == 'L' || $2[0] == 'l')
				endian = ENDIAN_LITTLE;
			else
				endian = ENDIAN_BIG;
			$$ = mkparam(PARAM_ENDIAN, 0, endian);
		}
		| kw_offset number newline {
			$$ = mkparam(PARAM_OFFSET, 0, 0); // XXX
			$$->offset = (1 << 31) | $2; // XXX
		}

fieldlist	: /* empty */ { $$ = NULL; }
		| fieldlist field { $$ = mkfieldlist($1, $2); }

field		: number id enumerlist newline { $$ = makefield($1, $2, $3); }
		| number newline { $$ = makefield($1, NULL, NULL); }

enumerlist	: /* empty */ { $$ = NULL; }
		| enumerlist enumer { $$ = mkenumerlist($1, $2); }

enumer		: number id { $$ = mkenumer($1, $2); }

kw_base		: KW_BASE ;

kw_prefix	: KW_PREFIX ;

kw_size		: KW_SIZE ;

kw_endian	: KW_ENDIAN ;

kw_offset	: KW_OFFSET ;

id		: ID { $$ = strdup(yytext); }

number		: NUMBER { $$ = (int)strtol(yytext, NULL, 0); }

newline		: NEWLINE ;

%%

struct listlist *alllistlist;

YYDEF3(list,
	struct commentlist *, commentlist,
	struct base *, base,
	struct reg *, reg)
YYDEF1(comment,
	const char *, text)
YYDEF0(sep)
YYDEF1(base,
	long long, addr)
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

struct reg *
makereg(const char *prefix, struct paramlist *paramlist,
    struct fieldlist *fieldlist)
{
	struct reg *reg;
	struct param *param;

	reg = mkreg(prefix, paramlist, fieldlist);
	TAILQ_FOREACH(param, &reg->paramlist->head, entry) {
		switch (param->type) {
		case PARAM_SIZE:
			reg->size = param->size;
			break;
		case PARAM_ENDIAN:
			reg->endian = param->endian;
			break;
		case PARAM_OFFSET:
			reg->offset = param->offset;
			break;
		}
	}
	return reg;
}

struct field *
makefield(int width, const char *name, struct enumerlist *enumer)
{
	struct field *field;
	int offset;

	if (prev_field == NULL)
		offset = 0;
	else
		offset = prev_field->mask.E + prev_field->width;
	field = mkfield(width, name, enumer);
	field->mask.S = offset;
	field->mask.E = field->mask.S + width;
	prev_field = field;

	return field;
}

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
