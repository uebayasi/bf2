#include <stdio.h>

#include "gram_local.h"

int yyparse(void);
void field_print(const struct field *);

int
main(int ac, char *av[])
{
	yyparse();
	struct list *l;
	TAILQ_FOREACH(l, &alllistlist->head, entry) {
		if (l->reg != NULL && l->reg->fieldlist) {
			printf("list:reg\n");
			struct field *f;
			TAILQ_FOREACH(f, &l->reg->fieldlist->head, entry) {
				printf(" %s\n", f->name);
				field_print(f);
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
