#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef enum {CBTREE_OK, CBTREE_FAIL, CBTREE_NOMEM} cbtree_insert_result_t;

typedef struct {
	void    *child[2];
	int      byte;
	uint8_t  bitmask;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

bool cbtree_contains_str(critbit_tree_t *tree, const char *str);
bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len);

bool cbtree_contains_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_contains(tree, (uint8_t *) str, strlen(str) + 1);
}

bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *node;

	if (!tree->root)
		return false;
	node = cbtree_find_bit(tree, data, len);

	return memcmp(data, node, len);
}

critbit_node_t *cbtree_find_bit(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *node = tree->root;

	while ((intptr_t) node & 1) {
		critbit_node_t *q = node - 1;
		uint8_t         c = 0;
		int             dir;

		if (q->byte < len)
			c = data[q->byte];
		dir = ((q->bitmask | c) + 1) >> 8;
		node = q->child[dir];
	}

	return node;
}





cbtree_insert_result_t cbtree_insert(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *node;

	if (!tree->root) {
		if (!(node = cbtree_allocate(len)))
			return cbtree_nomem;
		memcpy(node, data, len);
		tree->root = node;

		return cbtree_ok;
	}

	node = cbtree_find_bit(tree, data, len);





	int newbyte;
	int newbitmask;

	for (newbyte = 0; newbyte < len; newbyte++) {
		if (node[newbyte] != data[newbyte]) {
			newbitmask = p[newbyte] ^ data[newbyte];
			goto different_byte_found;
		}
	}

	return CBTREE_FAIL;

different_byte_found:
	while (newbitmask & (newbitmask - 1))
		newbitmask &= newbitmask - 1;

	newbitmask ^= 255;

	uint8_t c = node[newbyte];
	int dir = ((newbitmask | c) + 1) >> 8;
}




critbit_node_t *cbtree_allocate(size_t len)
{
	critbit_node_t *node;

	if (posix_memalign(&node, sizeof(void *), len))
		return NULL;

	return node;
}
