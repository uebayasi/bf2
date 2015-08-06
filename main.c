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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gram_local.h"

int yyparse(void);
void field_print(const struct field *);

long long strtonum(const char *, long long, long long, const char **);

int
main(int argc, char *argv[])
{
	int c;
	extern char *optarg;

	while ((c = getopt(argc, argv, "a:BfLv")) != -1) {
		int asiz;
		switch (c) {
		case 'a':
			asiz = strtonum(optarg, MINASIZ, MAXASIZ, NULL);
			switch (asiz) {
			case MINASIZ:
			case 2*MINASIZ:
			case 4*MINASIZ:
			case MAXASIZ:
				global->asiz |= asiz;
				break;
			default:
				fprintf(stderr, "invalid access size (%d)\n",
				    asiz);
				exit(1);
				/* NOTREACHED */
			}
			break;
		case 'B':
			global->target = ENDIAN_BIG;
			break;
		case 'f':
			global->full = 1;
			break;
		case 'L':
			global->target = ENDIAN_LITTLE;
			break;
		case 'v':
			global->verbose = 1;
			break;
		}
	}

	yyparse();

	struct list *l;
	TAILQ_FOREACH(l, &alllistlist->head, entry) {
		// Register
		if (l->reg != NULL) {
			global->cur_reg = l->reg;

			if (l->reg->offset != 0) {
				printf("#define %s 0x%x\n", l->reg->prefix,
				    l->reg->offset & ~(1 << 31));
			}
			if (l->reg->fieldlist != NULL) {
				struct field *f;
				TAILQ_FOREACH(f, &l->reg->fieldlist->head, entry) {
					field_print(f);
				}
			}
		}
		// Comment
		if (l->commentlist != NULL) {
			printf("/*\n");
			struct comment *c;
			TAILQ_FOREACH(c, &l->commentlist->head, entry) {
				printf(" * %s\n", c->text);
			}
			printf(" */\n");
		}
		// Separator
		if (l->reg == NULL && l->base == NULL && l->commentlist == NULL) {
			putchar('\n');
		}
	}
	return 0;
}
