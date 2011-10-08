#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	void    *child[2];
	int      byte;
	uint8_t  bitmask;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

bool cbtree_contain_str(critbit_tree_t *tree, const char *str);
bool cbtree_contain(critbit_tree_t *tree, const uint8_t *data, size_t len);

bool cbtree_contain_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_contain(tree, (uint8_t *) str, 0);
}

bool cbtree_contain(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *p = tree->root;

	if (!p)
		return false;
	if (!len)
		len = strlen((char *) data);

	while ((intptr_t) p & 1) {
		critbit_node_t *q = p - 1;
		uint8_t         c = 0;
		int             dir;

		if (q->byte < len)
			c = data[q->byte];
		dir = ((q->bitmask | c) + 1) >> 8;
		p = q->child[dir];
	}

	if (len)
		return memcmp(data, p, len);
	else
		return strcmp((char *) data, (char *) p) == 0;
}
