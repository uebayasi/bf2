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
		if (l->reg != NULL) {
			global->cur_reg = l->reg;

			struct field *f;
			if (l->reg->fieldlist != NULL) {
				TAILQ_FOREACH(f, &l->reg->fieldlist->head, entry) {
					field_print(f);
				}
			}
		}
		if (l->commentlist != NULL) {
			struct comment *c;
			TAILQ_FOREACH(c, &l->commentlist->head, entry) {
				printf("// %s\n", c->text);
			}
		}
	}
	return 0;
}
