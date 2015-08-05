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

#include <assert.h>

#include "gram_local.h"

#ifndef MAX
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif

/*
 * Print bit-field accessors.
 */

struct num {
	int v;
};

/* Value represented as byte vector. */
struct vec {
	uint8_t vs[MAXASIZ / NBBY];
};

/* Value with shift. */
struct sv {
	int shift;
	union {
		/* Real value is (mask << shift). */
		struct mask mask;
		/* Real value is (num.v << shift). */
		struct num num;
		/* Real value is ((vec[0] | vec[(1] << 8) | ...) << shift). */
		struct vec vec;
	} u;
};

/* Value with access-size and index. */
struct aiv {
	/* Real value is, (v << shift + (asiz * aidx)). */
	int asiz;
	int aidx;
	struct sv sv;
};

/* Access. */
struct accs {
	int align;
	int minasiz;
	struct aiv aivs[4];
};

/* 8/16/32/64 -> 0/1/2/3 */
#define asiz2idx(a) \
    (((((a) >> 3) & 0xc) ? 0x2 : 0) | ((((a) >> 3) & 0x6) ? 0x1 : 0))

static void analyze_accs(const struct mask *, struct accs *);
static int calc_access_align(const struct mask *);
static int calc_access_size(const int, const struct mask *);
static int calc_access_shift(const int, const int, const struct mask *);
static void print_accs(const char *, struct accs *, const struct enumerlist *);
static void print_mask(const char *, const struct aiv *, const int);
static void print_enumer(const char *, const struct aiv *,
    const struct enumer *);
static void print_vec(char *, const size_t, const struct aiv *,
    void (*)(const struct aiv *, struct aiv *));
static void mask2vec(const struct aiv *, struct aiv *);
static void num2vec(const struct aiv *, struct aiv *);

struct global global_storage, *global = &global_storage;

void
field_print(const struct field *field)
{
	struct accs accs_storage, *accs = &accs_storage;
	analyze_accs(&field->mask, accs);

	const char *fmt = "/* field: name=%s offset=%d width=%d */\n";
	static int once = 0;
	if (field->name == NULL) {
		/* Unused bits (pads). */
		if (global->verbose) {
			if (once++ != 0)
				putchar('\n');
			printf(fmt, "(unused)", field->mask.S, field->width);
		}
	} else {
#if 0
		if (once++ != 0)
			putchar('\n');
#endif
		if (global->verbose) {
			printf(fmt, field->name, field->mask.S, field->width);
		}
		const char * const name = field->name;
		char prefixstr[256];
		snprintf(prefixstr, sizeof(prefixstr), "%s%s%s",
		    global->cur_reg->prefix ? global->cur_reg->prefix : "",
		    global->cur_reg->prefix ? "_" : "", name);
		print_accs(prefixstr, accs, field->enumerlist);
	}
}

static void
analyze_accs(const struct mask *mask, struct accs *accs)
{
	/* Check alignment.  Aligned access is simpler (only byte-swaps). */
	accs->align = calc_access_align(mask);

	/* Minimal register access width == alignment size. */
	accs->minasiz = calc_access_size(accs->align, mask);

	int asiz;
	for (asiz = accs->minasiz; asiz <= MAXASIZ; asiz <<= 1) {
		struct aiv *aiv = &accs->aivs[asiz2idx(asiz)];
		aiv->asiz = asiz;
		aiv->aidx = mask->S / asiz;
		aiv->sv.shift = calc_access_shift(accs->align, asiz, mask);
		aiv->sv.u.mask = *mask;
	}
}

/* Right most bit set. */
#define _BfAlignRMB(o)	((o) & -(o))
#define _BfAlign(o)	((o) == 0 ? (1 << 30)/*INT_MAX*/ : _BfAlignRMB(o))

static int
calc_access_align(const struct mask *mask)
{
	/*
	 * Calc alignment by checking the right most bit.
	 */
	const int align = MIN(_BfAlign(mask->S), _BfAlign(mask->E));

	/*
	 * Ignore alignment more than 64-bit.
	 */
	return MIN(align, MAXASIZ);
}

static int
calc_access_size(const int align, const struct mask *mask)
{
	if (align > 0) {
		return align;
	} else {
		/*
		 * Determine the (minimal) access size.
		 *
		 * If a 2-bit maak crosses no 8-bit boundary (0x18 for
		 * example), it can be accessed by 8-bit.  If it crosses
		 * 8-bit boundary (0x0180 for example), 16-bit access is
		 * needed.  Find the biggest boundary (8/16/32/64 in
		 * order), then it is the minimal access size!
		 */

		/* 8-bit indexes. */
		int i = mask->S / MINASIZ;
		int j = (mask->E - 1) / MINASIZ;

		int asiz = MINASIZ;

		while (i > 0 && j > 0) {
			/*
			 * (S, E) have the same index
			 * -> not crossing boundary.
			 */
			if (i == j)
				break;
			/* Larger alignment next. */
			asiz <<= 1;
			i >>= 1;
			j >>= 1;
		}

		/* Max is 64-bit access. */
		return MIN(asiz, MAXASIZ);
	}
}

static int
calc_access_shift(const int align, const int asiz, const struct mask *mask)
{
	/* Shift in register. */
	return
	    /* Aligned access - no shift needed. */
	    (asiz == align) ? 0 :
	    /* Unaligned access. */
	    (global->target == ENDIAN_BIG) ?
		/*
		 * Big endian .bf definition is in real memory, not view.
		 * Calc shift from right.
		 */
		((mask->E % asiz) == 0 ? 0 : (asiz - (mask->E % asiz))) :
		/*
		 * Little endian .bf definition is in little endian view.
		 * Simply calc shift from left.
		 */
		(mask->S % asiz);
}

static void
print_accs(const char *prefixstr, struct accs *accs,
    const struct enumerlist *enumerlist)
{
	/* Provide all access widths >minasiz. */
	int asiz;
	for (asiz = accs->minasiz; asiz <= MAXASIZ; asiz <<= 1) {
		/* Follow explicit access sizes when specified. */
		if (global->asiz != 0 && (global->asiz & asiz) == 0)
			continue;
		struct aiv *aiv = &accs->aivs[asiz2idx(asiz)];
		print_mask(prefixstr, aiv, accs->align);

		struct enumer *enumer;
		if (enumerlist != NULL) {
			TAILQ_FOREACH(enumer, &enumerlist->head, entry) {
				print_enumer(prefixstr, aiv, enumer);
			}
		}

	}
}

/* Print accessors. */
static void
print_mask(const char *prefixstr, const struct aiv *aiv, const int align)
{
#define P prefixstr
#define A aiv->asiz
	printf("#define %s_INDEX_%d %d\n", P, A, aiv->aidx);

	if (global->full) {
		printf("#define %s_OCV_%d(p) "
		    "(*((const uint%d_t *)(p) + %s_INDEX_%d))\n",
		    P, A, A, P, A);
		printf("#define %s_OV_%d(p) "
		    "(*((uint%d_t *)(p) + %s_INDEX_%d))\n",
		    P, A, A, P, A);
	}

	char swapstr[16];
	snprintf(swapstr, sizeof(swapstr), "_BfSwap%s%d",
	    (global->target == ENDIAN_LITTLE) ? "Le" : "Be", A >> 3);
	printf("#define %s_SWAP_%d %s\n", P, A, swapstr);

	if (global->full) {
		printf("#define %s_CV_%d(p) (%s_SWAP_%d(%s_OCV_%d(p)))\n",
		    P, A, P, A, P, A);
		printf("#define %s_V_%d(p, v) do "
		    "{ %s_OV_%d(p) = %s_SWAP_%d(v); } while (0)\n",
		    P, A, P, A, P, A);
	}

	char maskstr[32];
	print_vec(maskstr, sizeof(maskstr), aiv, mask2vec);

	const struct mask *mask = &aiv->sv.u.mask;
	printf("#define %s_SHIFT_%d %d\n",
	    P, A, aiv->sv.shift);
	printf("#define %s_WIDTH_%d %d\n",
	    P, A, mask->E - mask->S);
	printf("#define %s_MASK_%d UINT%d_C(0x%s)\n",
	    P, A, A, maskstr);

	if (global->full) {
		static char readstr[256];
		static char issetstr[256];
		static char clearstr[256];
		static char setstr[256];
		static char writestr[256];
		if (align == aiv->asiz) {
			snprintf(readstr, sizeof(readstr),
			    "%s_CV_%d(p)", P, A);
			snprintf(issetstr, sizeof(readstr),
			    "(%s_CV_%d(p) != 0)", P, A);
			snprintf(clearstr, sizeof(clearstr),
			    "do { %s_V_%d(p, 0); } while (0)", P, A);
			snprintf(setstr, sizeof(setstr),
			    "do { "
			        "%s_V_%d(p, (%s_CV_%d | ~%s_CV_%d(p))); "
			    "} while (0)",
			    P, A, P, A, P, A);
			snprintf(writestr, sizeof(writestr),
			    "do { %s_V_%d(p, v); } while (0)",
			    P, A);
		} else {
			snprintf(readstr, sizeof(readstr),
			    "((%s_CV_%d(p) & %s_MASK_%d) >> %s_SHIFT_%d)",
			    P, A, P, A, P, A);
			snprintf(issetstr, sizeof(issetstr),
			    "((%s_CV_%d(p) & %s_MASK_%d) != 0)",
			    P, A, P, A);
			snprintf(clearstr, sizeof(clearstr),
			    "do { "
			        "%s_V_%d(p, (%s_CV_%d(p) & ~%s_MASK_%d)); "
			    "} while (0)",
			    P, A, P, A, P, A);
			snprintf(setstr, sizeof(setstr),
			    "do { "
			        "%s_V_%d(p, (%s_CV_%d(p) | "
				"((~%s_CV_%d(p)) & %s_MASK_%d))); "
			    "} while (0)",
			    P, A, P, A, P, A, P, A);
			snprintf(writestr, sizeof(writestr),
			    "do { "
				"%s_V_%d(p, "
			            "(%s_CV_%d(p) & ~%s_MASK_%d) | "
				    "(((v) << %s_SHIFT_%d) & %s_MASK_%d)); "
			    "} while (0)",
			    P, A, P, A, P, A, P, A, P, A);
		}
		printf("#define %s_READ_%d(p) %s\n", P, A, readstr);
		printf("#define %s_ISSET_%d(p) %s\n", P, A, issetstr);
		printf("#define %s_CLEAR_%d(p) %s\n", P, A, clearstr);
		printf("#define %s_SET_%d(p) %s\n", P, A, setstr);
		printf("#define %s_WRITE_%d(p, v) %s\n", P, A, writestr);
	}
}

/* Print enum constants. */
static void
print_enumer(const char *prefixstr, const struct aiv *aiv,
    const struct enumer *enumer)
{
	struct aiv v_storage, *v = &v_storage;
	*v = *aiv;
	v->sv.u.num.v = enumer->num;

	char enumerstr[32];
	print_vec(enumerstr, sizeof(enumerstr), v, num2vec);

	printf("#define %s_ENUM_%d_%s UINT%d_C(0x%s)\n",
	    P, A, enumer->name, A, enumerstr);
#undef	A
#undef	P
}

static void
print_vec(char *str, const size_t len, const struct aiv *aiv,
    void (*x2vec)(const struct aiv *, struct aiv *))
{
	struct aiv vec_storage, *vec = &vec_storage;
	(*x2vec)(aiv, vec);

	/* Print mask as hex string by 8-bit from MSB. */
	int n = 0;
	int asiz;
	for (asiz = aiv->asiz - NBBY; asiz >= 0; asiz -= NBBY) {
		assert(n < len);
		n += snprintf(str + n, len - n, "%02x",
		    vec->sv.u.vec.vs[asiz / NBBY]);
	}
}

/* 8-bit range mask. */
#define	_BfMask0(s)		((1 << (s)) - 1)
#define	_BfMask(s)		(((s) >= NBBY) ? 0xffU : _BfMask0((s)))
#define _BfMaskR(s, e)		(_BfMask((e)) & ~_BfMask((s)))

static void
mask2vec(const struct aiv *aiv, struct aiv *vec)
{
	const struct mask *mask = &aiv->sv.u.mask;
	int s = aiv->sv.shift;
	int e = s + (mask->E - mask->S);
	int i;
	for (i = 0; i < aiv->asiz; i += NBBY, s -= NBBY, e -= NBBY) {
		uint8_t b;
		if (s >= NBBY || e <= 0)
			b = 0;
		else {
			const int p = MAX(0, s);
			const int q = MAX(0, e);
			b = _BfMaskR(p, q);
		}
		vec->sv.u.vec.vs[i / NBBY] = b;
	}
}

static void
num2vec(const struct aiv *aiv, struct aiv *vec)
{
	const int v = aiv->sv.u.num.v;
	int i;
	for (i = 0; i < aiv->asiz; i += NBBY) {
		const int shift = aiv->sv.shift - i;
		uint8_t b;
		if (shift > NBBY || shift < -31)
			b = 0;
		else
			b = (shift > 0) ?  (v << shift) : (v >> -shift);
		vec->sv.u.vec.vs[i / NBBY] = b;
	}
}

/*
 * Dump info.
 */

struct layout {
	int size;
	char *buf;
};

static struct layout *dump_layout_alloc(void);
static void dump_layout_free(struct layout *);
static void dump_fieldlist_addresses(struct layout *, int);
static void dump_fieldlist_bits(struct layout *, int);
static void dump_fieldlist_legend(void);
static int bits_index(const int, const int, int *);

static void
dump(struct reg *prefix)
{
	struct layout *layout = dump_layout_alloc();
	const char *name = global->cur_reg->prefix;

	putchar('\n');
	printf("/*\n");
	printf(" * %s%s(prefix=%s size=%d target=%d)\n",
	    name ? name : "", name ? " " : "",
	    name, global->cur_reg->size, global->target);
	printf(" *\n");
	printf(" * real memory layout:\n");
	dump_fieldlist_addresses(layout, 0);
	dump_fieldlist_bits(layout, 0);
	printf(" *\n");

	int i;
	for (i = MINASIZ; i <= MAXASIZ; i <<= 1) {
		printf(" * %s endian memory layout view (%d-bit access):\n",
		    (i == MINASIZ) ? "little" : "big", i);
		dump_fieldlist_addresses(layout, i);
		dump_fieldlist_bits(layout, i);
		printf(" *\n");
	}

	dump_fieldlist_legend();
	printf(" */\n");

	dump_layout_free(layout);
}

static struct layout *
dump_layout_alloc(void)
{
	struct layout *layout;

	layout = calloc(sizeof(*layout), 1);
	layout->size = (global->cur_reg->size + MAXASIZ) & ~(MAXASIZ - 1);
	layout->buf = malloc(layout->size);
	memset(layout->buf, '.', layout->size);

	struct field *field;
	char C = 'a';
	int i = 0;
	TAILQ_FOREACH(field, &global->cur_reg->fieldlist->head, entry) {
		int j;
		for (j = 0; j < field->width; j++) {
			const int x = global->target == ENDIAN_BIG ?
			    /* Big endian layout index - simple. */
			    i :
			    /*
			     * Little endian layout index - insane.
			     *
			     * Little endian HWs put bit
			     * 0..7 8..15 16..23 24..31
			     * as
			     * 7..0 15..8 23..16 31..24
			     * in real memory.
			     *
			     * Little endian bf(1) bitfield definition 
			     * is written in little endian "view".  Thus
			     * real memory layout has to be 8-bit swapped.
			     */
			    (i - (i % NBBY) + (NBBY - (i % NBBY) - 1));
			layout->buf[x] = C;
			i++;
		}
		C++;
	}

	return layout;
}

static void
dump_layout_free(struct layout *layout)
{
	free(layout->buf);
	free(layout);
}

static void
dump_fieldlist_addresses(struct layout *layout, int swap)
{
	int i;

	printf(" * ");
	for (i = 0; i < global->cur_reg->size; i += NBBY) {
		if (i > 0 && i % NBBY == 0)
			putchar(' ');
		if (swap == 0) {
			printf("%-"NBBYSTR"d", i);
		} else {
			const int mask = swap - 1;
			const int offset = i & mask;
			const int n = (i - offset) + (swap - offset) - NBBY;
			printf("%"NBBYSTR"d", n);
		}
	}
	putchar('\n');
}

static void
dump_fieldlist_bits(struct layout *layout, int swap)
{
	int i;

	printf(" * ");
	for (i = 0; i < global->cur_reg->size; i += NBBY) {
		int index, incr;
		index = bits_index(i, swap, &incr);
		const char *p = layout->buf + index;
		int j;
		for (j = 0; j < NBBY; j++) {
			putchar(*p);
			p += incr;
		}
		putchar(' ');
	}
	putchar('\n');
}

static void
dump_fieldlist_legend(void)
{
	struct field *field;
	char C = 'a';

	printf(" * ");
	TAILQ_FOREACH(field, &global->cur_reg->fieldlist->head, entry) {
		const char * const name =
		    (field->name != NULL) ? field->name : "(unused)";
		printf("%s%c:%d:%s",
		    (C == 'a') ? "" : " ", C, field->width, name);
		C++;
	}
	putchar('\n');
}

static int
bits_index(const int index, const int swap_size, int *rincr)
{
	if (swap_size == 0) {
		*rincr = 1;
		return index;
	} else {
		const int mask = (swap_size - 1);
		*rincr = -1;
		return (index & ~mask) + (swap_size - 1) - (index & mask);
	}
}
