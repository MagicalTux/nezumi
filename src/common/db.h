// $Id$
#ifndef _DB_H_
#define _DB_H_

#include <stdarg.h>
#include <config.h>

#define HASH_SIZE (256 + 27)

#define RED 0
#define BLACK 1

struct dbn {
	struct dbn *parent, *left, *right;
	int color;
	void *key;
	void *data;
	int deleted; // �폜�ς݃t���O(db_foreach)
};

struct dbt {
	int (*cmp)(struct dbt*, void*, void*);
	uintptr_t (*hash)(struct dbt*, void*);
	// which 1 - key,   2 - data,  3 - both
	void (*release)(struct dbn*, int which);
	int maxlen;
	struct dbn *ht[HASH_SIZE];
	int item_count; // vf?
	const char* alloc_file; // DB?t@C
	int alloc_line; // DB?s
	// db_foreach ������db_erase �����΍�Ƃ��āA
	// db_foreach ���I���܂Ń��b�N���邱�Ƃɂ���
	struct db_free {
		struct dbn *z;
		struct dbn **root;
	} *free_list;
	int free_count;
	int free_max;
	int free_lock;
};

#define strdb_search(t,k)   db_search((t), (void*)(k))
#define strdb_insert(t,k,d) db_insert((t), (void*)(k), (void*)(d))
#define strdb_erase(t,k)    db_erase((t), (void*)(k))
#define strdb_foreach       db_foreach
#define strdb_final         db_final
#define numdb_search(t,k)   db_search((t), (void*)(k))
#define numdb_insert(t,k,d) db_insert((t), (void*)(k), (void*)(d))
#define numdb_erase(t,k)    db_erase((t), (void*)(k))
#define numdb_foreach       db_foreach
#define numdb_final         db_final

#define strdb_init(a)       strdb_init_(a,__FILE__, __LINE__)
#define numdb_init()        numdb_init_(__FILE__, __LINE__)

struct dbt* strdb_init_(int maxlen, const char *file, int line);
struct dbt* numdb_init_(const char *file, int line);

void* db_search(struct dbt *table, void* key);
struct dbn* db_insert(struct dbt *table, void* key, void* data);
void* db_erase(struct dbt *table, void* key);
void db_foreach(struct dbt*, int(*)(void*, void*, va_list), ...);
void db_final(struct dbt*, int(*)(void*, void*, va_list), ...);

#endif // _DB_H_
