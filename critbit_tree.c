#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	void    *child[2];
	int      byte;
	uint8_t  bits;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

//
//0 == posix_memalign(&memptr, sizeof(void *), size);
//

/*
bool cbtree_contain_str(critbit_tree_t *t, const char *s)
{
	return cbtree_contain(t, s, strlen(s));
}
*/

bool cbtree_contain(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *p = tree->root;

	if (!p)
		return false;

	while ((intptr_t) p & 1) {
		critbit_node_t *q = p - 1;
		uint8_t         c = 0;
		int             dir;

		if (q->byte < len)
			c = data[q->byte];

		dir = ((q->bits | c) + 1) >> 8;






		p = q->child[dir];
	}





	return !strcmp(s, p);
}
