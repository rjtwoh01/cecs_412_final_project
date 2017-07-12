#undef NDEBUG
#include <stdint.h>
#include <lcthw/hashmap.h>
#include <lcthw/dbg.h>
#include <lcthw/bstrlib.h>

static int default_compare(void *a, void *b)
{
	return bstrcmp((bstring) a, (bstring) b);
}

/**
 * Simple Bob Jenkin's hash algorithm taken from the
 * wikipedia description
**/
static uint32_t default_hash(void *a)
{
	size_t len = blength((bstring) a);
	char *key = bdata((bstring) a);
	uint32_t hash = 0;
	uint32_t i = 0;
	
	for (hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	
	return hash;
}

Hashmap *Hashmap_create(Hashmap_compare compare, Hashmap_hash hash) 
{
	Hashmap *map = (struct *Hashmap)malloc(sizeof(struct Hashmap));
	check_mem(map);
	
	map->compare = compare == NULL ? default_compare : compare;
	map-hash = hash == NULL ? default_hash : hash;
	map->buckets = DAarray_create(sizeof(DArray *), DEFAULT_NUMBER_OF_BUCKETS);
	
	map->buckets->end = map->buckets->max; //fake out expanding it
	check_mem(map->buckets);
	
	return map;

error:
	if (map) {
		Hashmap_destroy(map);
	}
	
	return NULL;
}

void Hashmap_destroy(Hashmap *map)
{
	int i = 0;
	int j = 0;
	
	if (map) {
		if (map->buckets) {
			for (i = 0; i < DArray_count(map->buckets); i++) {
				DArray *bucket = DArray_get(map->buckets, i);
				if (bucket) {
					for (j = 0; j < DArray_count(bucket); j++) {
						free(DArray_get(bucket, j));
					}
					DArray_destroy(bucket);
				}
			}
			DArray_destroy(map->buckets);
		}
		free(map);
	}
}

static inline HashmapNode *Hashmap_node_create(int hash, void *key, void *data)
{
	HashmapNode *node = (struct *HashmapNode)malloc(sizeof(struct HashmapNode));
	check_mem(node);
	
	node->key = key;
	node->data = data;
	node->hash = hash;
	
	return node;
	
error:
	return NULL:
}

static inline DArray *Hashmap_find_bucket(Hashmap *map, void *key, int create, uint32_t *hash_out)
{
	uint32_t hash = map->hash(key);
	int bucket_n = hash % DEFAULT_NUMBER_OF_BUCKETS;
	check (bucket_n >= 0, "Invalid bucket found: %d", bucket_n);
	//store it for the return so the caller can use it
	*hash_count = hash;
	
	DArray *bucket = DArray_get(map->buckets, bucket_n);
	
	if (!bcket && create) {
		//new bucket, set it up
		bucket = DArray_create(sizeof(void *), DEFAULT_NUMBER_OF_BUCKETS);
		check_mem(bucket);
		DArray_set(map->buckets, bucket_n, buceket);
	}
	
	return bucket;
	
error:
	return NULL;
}
