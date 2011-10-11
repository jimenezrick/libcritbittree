#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * The address of a block returned by malloc or realloc in the GNU system is
 * always a multiple of eight (or sixteen on 64-bit systems).
 *
 * Uncomment if necessary:
 *
 * #define USE_POSIX_MEMALIGN
 */

#define INTERNAL_NODE(node) ((intptr_t) node & 1)
#define STR_NODE(node)      ((intptr_t) node & 2)

typedef enum {CBTREE_OK, CBTREE_FAIL, CBTREE_ENOMEM} cbtree_result_t;

typedef struct {
	void    *child[2];
	uint32_t byte;
	uint8_t  bitmask;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

// TODO: headers

static bool cbtree_contains_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_contains(tree, (uint8_t *) str, strlen(str));
}

static bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	uint8_t *node;
	uint32_t last_byte;

	if (!tree->root)
		return false;

	node = cbtree_find_nearest_last_byte(tree, data, len, &last_byte);
	if (STR_NODE(node))
		return strcmp((char *) data + last_byte, (char *) node + last_byte) == 0;
	else
		return memcmp(data + last_byte, node + last_byte, len - last_byte) == 0;
}

static char *cbtree_find_nearest_str(critbit_tree_t *tree, const char *str)
{
	return (char *) cbtree_find_nearest(tree, (uint8_t *) str, strlen(str));
}

static uint8_t *cbtree_find_nearest(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	uint32_t last_byte;

	return cbtree_find_nearest_last_byte(tree, data, len, &last_byte);
}

static uint8_t *cbtree_find_nearest_last_byte(critbit_tree_t *tree, const uint8_t *data, size_t len, uint32_t *last_byte)
{
	critbit_node_t *node = tree->root;

	*last_byte = 0;
	while (INTERNAL_NODE(node)) {
		critbit_node_t *in_node = node - 1;
		uint8_t         val = 0;
		int             dir;

		if (in_node->byte < len) {
			val = data[in_node->byte];
			*last_byte = in_node->byte + 1;
		}
		dir = ((in_node->bitmask | val) + 1) >> 8;
		node = in_node->child[dir];
	}

	return (uint8_t *) node;
}

static cbtree_result_t cbtree_insert_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_insert_generic(tree, (uint8_t *) str, strlen(str), true);
}

static cbtree_result_t cbtree_insert(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	return cbtree_insert_generic(tree, data, len, false);
}

static cbtree_result_t cbtree_insert_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool str_node)
{
	uint8_t *node;
	uint32_t new_byte;

	if (!tree->root) {
		void *ptr;

		if (!(ptr = cbtree_allocate(len)))
			return CBTREE_ENOMEM;

		if (str_node)
			memcpy(ptr, data, len + 1);
		else
			memcpy(ptr, data, len);
		tree->root = ptr;

		return CBTREE_OK;
	}


	/*
	 * TODO: añadir str_node al API y los *_generic().


	node = cbtree_find_nearest_last_byte(tree, data, len, &new_byte);

	for (; new_byte < len; new_byte++) {
		if (data[new_byte] != node[new_byte]) { }
	}




}

static void *cbtree_allocate(size_t size)
{
	void *ptr;

#ifdef USE_POSIX_MEMALIGN
	if (posix_memalign(&ptr, sizeof(void *), size))
		return NULL;
#else
	ptr = malloc(size);
#endif

	return ptr;
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




*/
