#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * The address of a block returned by malloc or realloc in the GNU system is
 * always a multiple of eight (or sixteen on 64-bit systems). But if you need
 * to use posix_memalign() instead, then uncomment:
 *
 * #define USE_POSIX_MEMALIGN
 */

#define IS_INTERNAL(node)          ((uintptr_t) node & 1)
#define GENERATE_DIR(bitmask, val) (((bitmask | val) + 1) >> 8)

typedef enum {CBTREE_OK, CBTREE_FAIL, CBTREE_ENOMEM} cbtree_result_t;

typedef struct {
	void    *child[2];
	uint32_t byte;
	uint8_t  bitmask;
} critbit_node_t;

typedef struct {
	void *root;
} critbit_tree_t;

//
// TODO: permitir busqueda por prefijo, y mas operaciones
//

static inline bool cbtree_contains_str(critbit_tree_t *tree, const char *str);
static inline bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len);
static bool cbtree_contains_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str);
static inline char *cbtree_find_nearest_str(critbit_tree_t *tree, const char *str);
static inline uint8_t *cbtree_find_nearest(critbit_tree_t *tree, const uint8_t *data, size_t len);
static uint8_t *cbtree_find_nearest_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str, uint32_t *next_byte);
static inline cbtree_result_t cbtree_insert_str(critbit_tree_t *tree, const char *str);
static inline cbtree_result_t cbtree_insert(critbit_tree_t *tree, const uint8_t *data, size_t len);
static cbtree_result_t cbtree_insert_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str);
static cbtree_result_t cbtree_insert_empty_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str);
static inline void *cbtree_allocate(size_t size);
static bool cbtree_cmp_bytes(const uint8_t *data1, const uint8_t *data2, size_t len, uint32_t *diff_byte);

static inline bool cbtree_contains_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_contains_generic(tree, (uint8_t *) str, strlen(str), true);
}

static inline bool cbtree_contains(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	return cbtree_contains_generic(tree, data, len, false);
}

static bool cbtree_contains_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str)
{
	uint8_t *node;
	uint32_t next_byte;

	if (!tree->root)
		return false;

	node = cbtree_find_nearest_generic(tree, data, len, is_str, &next_byte);
	if (is_str)
		return strcmp((char *) data + next_byte, (char *) node + next_byte) == 0;
	else
		return memcmp(data + next_byte, node + next_byte, len - next_byte) == 0;
}

static inline char *cbtree_find_nearest_str(critbit_tree_t *tree, const char *str)
{
	return (char *) cbtree_find_nearest_generic(tree, (uint8_t *) str, strlen(str), true, (uint32_t []) {0});
}

static inline uint8_t *cbtree_find_nearest(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	return cbtree_find_nearest_generic(tree, data, len, false, (uint32_t []) {0});
}

static uint8_t *cbtree_find_nearest_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str, uint32_t *next_byte)
{
	critbit_node_t *node = tree->root;

	*next_byte = 0;
	while (IS_INTERNAL(node)) {
		critbit_node_t *int_node = (critbit_node_t *) ((uintptr_t) node - 1);
		uint8_t         val = 0;
		int             dir;

		if (int_node->byte < len) {
			val = data[int_node->byte];
			*next_byte = int_node->byte + 1;
		}
		dir = GENERATE_DIR(int_node->bitmask, val);
		node = int_node->child[dir];
	}

	return (uint8_t *) node;
}

static inline cbtree_result_t cbtree_insert_str(critbit_tree_t *tree, const char *str)
{
	return cbtree_insert_generic(tree, (uint8_t *) str, strlen(str), true);
}

static inline cbtree_result_t cbtree_insert(critbit_tree_t *tree, const uint8_t *data, size_t len)
{
	return cbtree_insert_generic(tree, data, len, false);
}

static cbtree_result_t cbtree_insert_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str)
{
	uint32_t new_byte, diff_byte;
	uint8_t *ext_node, new_bitmask, val;
	int      dir;

	if (!tree->root)
		return cbtree_insert_empty_generic(tree, data, len, is_str);

	ext_node = cbtree_find_nearest_generic(tree, data, len, is_str, &new_byte);
	if (cbtree_cmp_bytes(data + new_byte, ext_node + new_byte, len - new_byte, is_str, &diff_byte)) {
		if (is_str && ext_node[new_byte] != '\0')
			new_bitmask = ext_node[new_byte];
		else
			return CBTREE_FAIL;
	} else {
		new_byte += diff_byte;
		new_bitmask = data[new_byte] ^ ext_node[new_byte];
	}

	new_bitmask |= new_bitmask >> 1;
	new_bitmask |= new_bitmask >> 2;
	new_bitmask |= new_bitmask >> 4;
	new_bitmask = (new_bitmask & ~(new_bitmask >> 1)) ^ UINT8_MAX;

	val = ext_node[new_byte];
	dir = GENERATE_DIR(new_bitmask, val);

	// XXX: ponerlo junto al resto de declaraciones de arriba
	critbit_node_t *new_node;
	uint8_t        *new_data;
	//

	if (!(new_node = cbtree_allocate(sizeof(critbit_node_t))))
		return CBTREE_ENOMEM;
	if (is_str && !(new_data = cbtree_allocate(len + 1)))
		return CBTREE_ENOMEM;
	else if (!(new_data = cbtree_allocate(len)))
		return CBTREE_ENOMEM;

	if (is_str)
		memcpy(new_data, data, len + 1);
	else
		memcpy(new_data, data, len);

	new_node->byte           = new_byte;
	new_node->bitmask        = new_bitmask;
	new_node->child[1 - dir] = new_data;







	// XXX: Meter este codigo en una funcion auxiliar
	void **wherep = &tree->root;

	for (;;) {
		uint8_t *p = *wherep;

		if (!IS_INTERNAL(p))
			break;

		critbit_node_t *q = p - 1;

		if (q->byte > new_byte)
			break;
		if (q->byte == new_byte && q->bitmask > new_bitmask)
			break;

		uint8_t val = 0;
		if (q->byte < len)
			val = data[q->byte];
		const int dir = GENERATE_DIR(q->bitmask, val);
		wherep = q->child + dir;
	}

	new_node->child[new_dir] = *wherep;
	*wherep = new_node + 1;
	// XXX
}

static cbtree_result_t cbtree_insert_empty_generic(critbit_tree_t *tree, const uint8_t *data, size_t len, bool is_str)
{
	void *ptr;

	if (!(ptr = cbtree_allocate(len)))
		return CBTREE_ENOMEM;

	if (is_str)
		memcpy(ptr, data, len + 1);
	else
		memcpy(ptr, data, len);
	tree->root = ptr;

	return CBTREE_OK;
}

static inline void *cbtree_allocate(size_t size)
{
#ifdef USE_POSIX_MEMALIGN
	void *ptr;

	if (posix_memalign(&ptr, sizeof(void *), size))
		return NULL;

	return ptr;
#else
	return malloc(size);
#endif
}

static bool cbtree_cmp_bytes(const uint8_t *restrict data1, const uint8_t *restrict data2, size_t len, uint32_t *diff_byte)
{
	*diff_byte = 0;
	for (; data1 & ~(sizeof(long) - 1) && *diff_byte < len; (*diff_byte)++) {
		if (data1[*diff_byte] != data2[*diff_byte])
			return false;
	}

	for (; *diff_byte < len && len - *diff_byte >= sizeof(long); *diff_byte += sizeof(long)) {
		if (*((long *) (data1 + *diff_byte)) != *((long *) (data2 + *diff_byte)))
			break;
	}

	for (; *diff_byte < len; (*diff_byte)++) {
		if (data1[*diff_byte] != data2[*diff_byte])
			return false;
	}

	return true;
}
