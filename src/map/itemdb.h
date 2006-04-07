// $Id$
#ifndef _ITEMDB_H_
#define _ITEMDB_H_

#include "map.h"

struct item_data {
	int nameid;
	char name[25], jname[25]; // 24 + NULL
	char cardillustname[64];
	int value_buy; // 0-600000
	int value_sell; // 0-1000 (but can be more)
	char type; // 0-11
	int class; // 0-16777215
	char sex; // 0-3
	int equip; // equip_location 0-32768
	int weight; // 0-8000
	int atk; // 0-250
	int def; // 0-11
	char range; // 0-11
	char slot; // 0-5
	short look; // 0-207
	short elv; // equip_level 0-95
	char wlv; // weapon_level 0-4
	unsigned char *use_script;	// �񕜂Ƃ����S�����̒��ł�낤���Ȃ�
	unsigned char *equip_script;	// �U��,�h��̑����ݒ�����̒��ŉ\����?
	struct {
		unsigned available : 1;
		unsigned value_notdc : 1;
		unsigned value_notoc : 1;
		unsigned no_equip : 3;
		unsigned no_drop : 1;
		unsigned no_use : 1;
		unsigned no_refine : 1; // [celest]
	} flag;
	int view_id;
};

struct random_item_data {
	int nameid;
	int per;
};

struct item_data* itemdb_searchname(const char *name);
struct item_data* itemdb_search(intptr_t nameid);
struct item_data* itemdb_exists(intptr_t nameid);
#define itemdb_type(n) itemdb_search(n)->type
#define itemdb_atk(n) itemdb_search(n)->atk
#define itemdb_def(n) itemdb_search(n)->def
#define itemdb_look(n) itemdb_search(n)->look
#define itemdb_weight(n) itemdb_search(n)->weight
#define itemdb_equip(n) itemdb_search(n)->equip
#define itemdb_usescript(n) itemdb_search(n)->use_script
#define itemdb_equipscript(n) itemdb_search(n)->equip_script
#define itemdb_wlv(n) itemdb_search(n)->wlv
#define itemdb_range(n) itemdb_search(n)->range
#define itemdb_slot(n) itemdb_search(n)->slot
#define	itemdb_available(n) (itemdb_exists(n) && itemdb_search(n)->flag.available)
#define	itemdb_viewid(n) (itemdb_search(n)->view_id)

int itemdb_searchrandomid(int flags);

int itemdb_isequip(int);
int itemdb_isequip2(struct item_data *);
int itemdb_isequip3(int);
int itemdb_isdropable(int nameid);

// itemdb_equip�}�N����itemdb_equippoint�Ƃ̈Ⴂ��
// �O�҂��I��db�Œ�`���ꂽ�l���̂��̂�Ԃ��̂ɑ΂�
// ��҂�sessiondata���l�������Ƒ��ł̑����\�ꏊ
// ���ׂĂ̑g�ݍ��킹��Ԃ�

void itemdb_reload(void);

void do_final_itemdb(void);
int do_init_itemdb(void);

#endif // _ITEMDB_H_
