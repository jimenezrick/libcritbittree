#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INTERNAL_NODE(node) ((intptr_t) node & 1)
#define VAR_LEN_NODE(node)  ((intptr_t) node & 2)

typedef enum {CBTREE_OK, CBTREE_FAIL, CBTREE_ENOMEM} cbtree_result_t;

typedef struct {
	void    *child[2];
	uint32_t byte;
	uint8_t  bitmask;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

static bool cbtree_contains_str(critbit_tree_t *tree, const char *str);
static bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len);
static critbit_node_t *cbtree_find_nearest(critbit_tree_t *tree, const uint8_t *data, size_t len);

static bool cbtree_contains_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_contains(tree, (uint8_t *) str, strlen(str));
}

static bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *node;

	if ((node = cbtree_find_nearest(tree, data, len)) == NULL)
		return false;

	if (VAR_LEN_NODE(node))
		return strcmp((char *) data, (char *) node) == 0;
	else
		return memcmp(data, node, len) == 0;
}

static critbit_node_t *cbtree_find_nearest(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	critbit_node_t *node = tree->root;

	if (!tree->root)
		return NULL;

	while (INTERNAL_NODE(node)) {
		critbit_node_t *in_node = node - 1;
		uint8_t         b = 0;
		int             dir;

		if (in_node->byte < len)
			b = data[in_node->byte];
		dir = ((in_node->bitmask | b) + 1) >> 8;
		node = in_node->child[dir];
	}

	return node;
}






/*



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
*/
