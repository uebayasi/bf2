#include <assert.h>
#include <stdio.h>

#include "gram_local.h"

int yyparse(void);
void field_print(const struct field *);

int
main(int ac, char *av[])
{
	yyparse();

	global->asiz = 64;	// XXX

	struct list *l;
	TAILQ_FOREACH(l, &alllistlist->head, entry) {
		if (l->reg != NULL) {
			global->cur_reg = l->reg;

			struct field *f;
			if (l->reg->fieldlist != NULL) {
				TAILQ_FOREACH(f, &l->reg->fieldlist->head, entry) {
					printf(" %s\n", f->name);
					field_print(f);
				}
			}
		}
		if (l->commentlist != NULL) {
			printf("list:comment\n");
			struct comment *c;
			TAILQ_FOREACH(c, &l->commentlist->head, entry) {
				printf(" %s\n", c->text);
			}
		}
	}
	return 0;
}
