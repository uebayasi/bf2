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

////////////////////////////////////////////////////////////////////////////////
// XXX Generate this using DSL
////////////////////////////////////////////////////////////////////////////////

#include "yacc.h"

#include "scan.h"

#define MINASIZ (1 * NBBY)
#define MAXASIZ (8 * NBBY)

#ifndef NBBY
#define NBBY		8
#endif
#ifndef NBBYSTR
#define NBBYSTR		"8"
#endif

enum param_type {
	PARAM_SIZE = 1,
	PARAM_ENDIAN = 2,
	PARAM_OFFSET = 3,
};

union param_value {
	int size;
	int endian;
};

enum endian {
	ENDIAN_LITTLE = 1,
	ENDIAN_BIG = 2,
};

struct mask { int S; int E; };

YYDECL3(list,
	struct commentlist *, commentlist,
	struct def *, def,
	struct reg *, reg,
);
YYDECL0(sep,
);
YYDECL1(comment,
	const char *, text,
);
YYDECL3(def,
	long long, base,
	int, size,
	enum endian, endian,
);
#define	reg_ext \
	int type; \
	int size; \
	enum endian endian; \
	int offset;
YYDECL3(reg,
	const char *, prefix,
	struct paramlist *, paramlist,
	struct fieldlist *, fieldlist,
	reg_ext
);
#define	param_ext \
	int offset;
YYDECL3(param,
	int, type,
	int, size,
	enum endian, endian,
	param_ext
);
#define	field_ext \
	struct mask mask;
YYDECL3(field,
	int, width,
	const char *, name,
	struct enumerlist *, enumerlist,
	field_ext
);
YYDECL2(enumer,
	int, num,
	const char *, name,
);

////////////////////////////////////////////////////////////////////////////////

struct global {
	int asiz;
	int verbose;
	int full;
	struct reg *cur_reg, *prev_reg;
	struct field *cur_field, *prev_field;
	struct enumer *cur_enumer, *prev_enumer;
	enum endian target;
	int maxsize;
	struct {
		long long base;
		int size;
		enum endian endian;
	} def;
};

////////////////////////////////////////////////////////////////////////////////

extern struct listlist *alllistlist;
extern struct listlistlist all;
extern struct global *global;

////////////////////////////////////////////////////////////////////////////////

void field_print(const struct field *);
