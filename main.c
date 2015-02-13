#include <stdio.h>

#include "gram_local.h"

int yyparse(void);

int
main(int ac, char *av[])
{
	yyparse();
	struct list *l;
	TAILQ_FOREACH(l, &alllistlist->head, entry) {
		if (l->reg != NULL) {
			printf("list:reg\n");
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
