/******************************************************************************/

/*
 * Helper macros for yacc(1)
 */

/******************************************************************************/

/*
 * YYDECL1
 * YYDECL2
 * YYDECL3
 * YYDECL4
 */

#define	YYDECL1(x, ta, a)						\
	YYDECLLIST(x);							\
	YYDECLSTR1(x, ta, a);						\
	YYDECLMK1(x, ta, a)
#define	YYDECL2(x, ta, a, tb, b)					\
	YYDECLLIST(x);							\
	YYDECLSTR2(x, ta, a, tb, b);					\
	YYDECLMK2(x, ta, a, tb, b)
#define	YYDECL3(x, ta, a, tb, b, tc, c)					\
	YYDECLLIST(x);							\
	YYDECLSTR3(x, ta, a, tb, b, tc, c);				\
	YYDECLMK3(x, ta, a, tb, b, tc, c)
#define	YYDECL4(x, ta, a, tb, b, tc, c)					\
	YYDECLLIST(x);							\
	YYDECLSTR4(x, ta, a, tb, b, tc, c, td, d);			\
	YYDECLMK3(x, ta, a, tb, b, tc, c)

#define	YYDECLLIST(x)							\
struct x##list;								\
struct x##list {							\
	struct x##list *next;						\
	struct x *x;							\
};									\
struct x##list *mk##x##list(struct x##list *, struct x *);		\
void print##x##list(struct x##list *);					\
void print##x##listcb(struct x *, void *);				\
void iter##x##list(struct x##list *,					\
    void (*)(struct x *, void *), void *)

#define	YYDECLSTR1(x, ta, a)						\
struct x {								\
	ta a;								\
}
#define	YYDECLSTR2(x, ta, a, tb, b)					\
struct x {								\
	ta a;								\
	tb b;								\
}
#define	YYDECLSTR3(x, ta, a, tb, b, tc, c)				\
struct x {								\
	ta a;								\
	tb b;								\
	tc c;								\
}
#define	YYDECLSTR4(x, ta, a, tb, b, tc, c, td, d)			\
struct x {								\
	ta a;								\
	tb b;								\
	tc c;								\
	td d;								\
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
	YYDEFLIST(x, ta, a)						\
	YYDEFMK1(x, ta, a)
#define	YYDEF2(x, ta, a, tb, b)						\
	YYDEFLIST(x, ta, a)						\
	YYDEFMK2(x, ta, a, tb, b)
#define	YYDEF3(x, ta, a, tb, b, tc, c)					\
	YYDEFLIST(x, ta, a)						\
	YYDEFMK3(x, ta, a, tb, b, tc, c)
#define	YYDEF4(x, ta, a, tb, b, tc, c, td, d)				\
	YYDEFLIST(x, ta, a)						\
	YYDEFMK4(x, ta, a, tb, b, tc, c, td, d)

#define	YYDEFLIST(x, ta, a)						\
	YYDEFMK2(x##list, struct x##list *, next, struct x *, x)	\
	YYDEFPRINT(x)							\
	YYDEFITER(x)

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
	struct x##list *next = x##list->next;				\
	if (next != NULL) {						\
		iter##x##list(next, proc, v);				\
	}								\
	(*proc)(x##list->x, v);						\
}

/******************************************************************************/
