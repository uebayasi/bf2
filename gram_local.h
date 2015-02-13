#include "yacc.h"

#include "scan.h"

enum param_type {
	PARAM_SIZE = 1,
	PARAM_ENDIAN = 2,
};

union param_value {
	int size;
	int endian;
};

enum endian {
	ENDIAN_LITTLE = 1,
	ENDIAN_BIG = 2,
};

YYDECL2(list,
	struct commentlist *, commentlist,
	struct reg *, reg,
);
YYDECL0(sep,
);
YYDECL1(comment,
	const char *, text,
);
YYDECL3(reg,
	const char *, prefix,
	struct paramlist *, paramlist,
	struct fieldlist *, fieldlist,
);
YYDECL3(param,
	int, type,
	int, size,
	enum endian, endian,
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

extern struct listlist *alllistlist;
extern struct listlistlist all;
