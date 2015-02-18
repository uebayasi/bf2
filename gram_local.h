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

YYDECL2(list,
	struct commentlist *, commentlist,
	struct reg *, reg,
);
YYDECL0(sep,
);
YYDECL1(comment,
	const char *, text,
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
};

////////////////////////////////////////////////////////////////////////////////

extern struct listlist *alllistlist;
extern struct listlistlist all;
extern struct global *global;

////////////////////////////////////////////////////////////////////////////////

void field_print(const struct field *);
