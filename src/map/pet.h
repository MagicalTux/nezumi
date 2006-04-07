// $Id$
#ifndef _PET_H_
#define _PET_H_

#define MAX_PET_DB 300
#define PETLOOT_SIZE 20 // [Valaris]

struct pet_db {
	int class;
	char name[25], jname[25]; // 24 + NULL
	int itemID;
	int EggID;
	int AcceID;
	int FoodID;
	int fullness;
	int hungry_delay;
	int r_hungry;
	int r_full;
	int intimate;
	int die;
	int capture;
	int speed;
	char s_perfor;
	int talk_convert_class;
	int attack_rate;
	int defence_attack_rate;
	int change_target_rate;
	unsigned char *script;
};
extern struct pet_db pet_db[MAX_PET_DB];

enum { PET_CLASS, PET_CATCH, PET_EGG, PET_EQUIP, PET_FOOD };

int pet_hungry_val(struct map_session_data *sd);
int pet_target_check(struct map_session_data *sd, struct block_list *bl, int type);
void pet_stopattack(struct pet_data *pd);
int pet_changestate(struct pet_data *pd, int state, int type);
int pet_walktoxy(struct pet_data *pd, int x, int y);
int pet_stop_walking(struct pet_data *pd, int type);
int search_petDB_index(int key, int type);
void pet_remove_map(struct map_session_data *sd);
int pet_data_init(struct map_session_data *sd);
int pet_birth_process(struct map_session_data *sd);
int pet_recv_petdata(int account_id, struct s_pet *p, int flag);
void pet_select_egg(struct map_session_data *sd, short egg_index);
int pet_catch_process1(struct map_session_data *sd, int target_class);
void pet_catch_process2(struct map_session_data *sd, int target_id);
int pet_get_egg(int account_id, int pet_id, int flag);
void pet_menu(struct map_session_data *sd, unsigned char menunum);
void pet_change_name(struct map_session_data *sd, char *name);
int pet_equipitem(struct map_session_data *sd, int idx);
void pet_unequipitem(struct map_session_data *sd);
void pet_food(struct map_session_data *sd);
int pet_lootitem_drop(struct pet_data *pd, struct map_session_data *sd);
TIMER_FUNC(pet_delay_item_drop2);
int pet_ai_sub_hard_lootsearch(struct block_list *bl, va_list ap);
TIMER_FUNC(pet_skill_bonus_timer); // [Valaris]
TIMER_FUNC(pet_recovery_timer); // [Valaris]
TIMER_FUNC(pet_mag_timer); // [Valaris]
TIMER_FUNC(pet_heal_timer); // [Valaris]
TIMER_FUNC(pet_skillattack_timer); // [Valaris]

int do_init_pet(void);
int do_final_pet(void);

#endif // _PET_H_

