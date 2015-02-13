/******************************************************************************/

/*
 * Helper macros for yacc(1)
 */

/******************************************************************************/

#include "queue.h"

/*
 * YYDECL1
 * YYDECL2
 * YYDECL3
 * YYDECL4
 */

#define	YYDECL1(x, ta, a, ext)						\
	YYDECLLIST(x);							\
	YYDECLSTR1(x, ta, a, ext);					\
	YYDECLMK1(x, ta, a)
#define	YYDECL2(x, ta, a, tb, b, ext)					\
	YYDECLLIST(x);							\
	YYDECLSTR2(x, ta, a, tb, b, ext);				\
	YYDECLMK2(x, ta, a, tb, b)
#define	YYDECL3(x, ta, a, tb, b, tc, c, ext)				\
	YYDECLLIST(x);							\
	YYDECLSTR3(x, ta, a, tb, b, tc, c, ext);			\
	YYDECLMK3(x, ta, a, tb, b, tc, c)
#define	YYDECL4(x, ta, a, tb, b, tc, c, td, d, ext)			\
	YYDECLLIST(x);							\
	YYDECLSTR4(x, ta, a, tb, b, tc, c, td, d, ext);			\
	YYDECLMK3(x, ta, a, tb, b, tc, c, td, d)

#define	YYDECLLIST(x)							\
struct x##list;								\
struct x##list {							\
	TAILQ_HEAD(x##list##head, x) head;				\
	struct x *x;							\
};									\
struct x##list *mk##x##list(struct x##list *, struct x *);		\
void print##x##list(struct x##list *);					\
void print##x##listcb(struct x *, void *);				\
void iter##x##list(struct x##list *,					\
    void (*)(struct x *, void *), void *)

#define	YYDECLSTR1(x, ta, a, ext)					\
struct x;								\
struct x {								\
	TAILQ_ENTRY(x) entry;						\
	ta a;								\
	ext								\
}
#define	YYDECLSTR2(x, ta, a, tb, b, ext)				\
struct x;								\
struct x {								\
	TAILQ_ENTRY(x) entry;						\
	ta a;								\
	tb b;								\
	ext								\
}
#define	YYDECLSTR3(x, ta, a, tb, b, tc, c, ext)				\
struct x;								\
struct x {								\
	TAILQ_ENTRY(x) entry;						\
	ta a;								\
	tb b;								\
	tc c;								\
	ext								\
}
#define	YYDECLSTR4(x, ta, a, tb, b, tc, c, td, d, ext)			\
struct x;								\
struct x {								\
	TAILQ_ENTRY(x) entry;						\
	ta a;								\
	tb b;								\
	tc c;								\
	td d;								\
	ext								\
}

#define	YYDECLMK1(x, ta, a)						\
struct x *mk##x(ta)
#define	YYDECLMK2(x, ta, a, tb, b)					\
struct x *mk##x(ta, tb)
#define	YYDECLMK3(x, ta, a, tb, b, tc, c)				\
struct x *mk##x(ta, tb, tc)
#define	YYDECLMK4(x, ta, a, tb, b, tc, c, td, d)			\
struct x *mk##x(ta, tb, tc, td)

/******************************************************************************/

/*
 * YYDEF1
 * YYDEF2
 * YYDEF3
 * YYDEF4
 */

#define	YYDEF1(x, ta, a)						\
	YYDEFLIST(x, a)							\
	YYDEFMK1(x, ta, a)
#define	YYDEF2(x, ta, a, tb, b)						\
	YYDEFLIST(x, a)							\
	YYDEFMK2(x, ta, a, tb, b)
#define	YYDEF3(x, ta, a, tb, b, tc, c)					\
	YYDEFLIST(x, a)							\
	YYDEFMK3(x, ta, a, tb, b, tc, c)
#define	YYDEF4(x, ta, a, tb, b, tc, c, td, d)				\
	YYDEFLIST(x, a)							\
	YYDEFMK4(x, ta, a, tb, b, tc, c, td, d)

#define	YYDEFLIST(x, a)							\
	YYDEFMKLIST(x, a)						\
	YYDEFPRINT(x)							\
	YYDEFITER(x)

#define TAILQ_MOVE(head, oldhead, field) do {				\
	(head)->tqh_first = (oldhead)->tqh_first;			\
	(head)->tqh_last = (oldhead)->tqh_last;				\
	(head)->tqh_first->field.tqe_prev = &(head)->tqh_first;		\
	*(head)->tqh_last = TAILQ_END(head);				\
} while (/*CONSTCOND*/0)

#define	YYDEFMKLIST(x, a)						\
struct x##list *							\
mk##x##list(struct x##list *ol, struct x *e)				\
{									\
	struct x##list *nl = calloc(sizeof *nl, 1);			\
	nl->x = e;							\
	if (ol == NULL)							\
		TAILQ_INIT(&nl->head);					\
	else								\
		TAILQ_MOVE(&nl->head, &ol->head, entry);		\
	TAILQ_INSERT_TAIL(&nl->head, e, entry);				\
	return nl;							\
}
#define	YYDEFMK1(x, ta, a)						\
struct x *								\
mk##x(ta a)								\
{									\
	struct x *x = calloc(sizeof *x, 1);				\
	x->a = a;							\
	return x;							\
}
#define	YYDEFMK2(x, ta, a, tb, b)					\
struct x *								\
mk##x(ta a, tb b)							\
{									\
	struct x *x = calloc(sizeof *x, 1);				\
	x->a = a;							\
	x->b = b;							\
	return x;							\
}
#define	YYDEFMK3(x, ta, a, tb, b, tc, c)				\
struct x *								\
mk##x(ta a, tb b, tc c)							\
{									\
	struct x *x = calloc(sizeof *x, 1);				\
	x->a = a;							\
	x->b = b;							\
	x->c = c;							\
	return x;							\
}
#define	YYDEFMK4(x, ta, a, tb, b, tc, c, td, d)				\
struct x *								\
mk##x(ta a, tb b, tc c, td d)						\
{									\
	struct x *x = calloc(sizeof *x, 1);				\
	x->a = a;							\
	x->b = b;							\
	x->c = c;							\
	x->d = d;							\
	return x;							\
}

#define	YYDEFPRINT(x)							\
void									\
print##x##list(struct x##list *x##list)					\
{									\
	if (x##list != NULL) {						\
		iter##x##list(x##list, print##x##listcb, NULL);		\
	}								\
}									\
void									\
print##x##listcb(struct x *x, void *v)					\
{									\
	/*print##y(y);*/						\
}

#define	YYDEFITER(x)							\
void									\
iter##x##list(struct x##list *x##list,					\
    void (*proc)(struct x *, void *), void *v)				\
{									\
	(*proc)(x##list->x, v);						\
}

/******************************************************************************/
