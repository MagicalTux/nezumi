// $Id: script.h 239 2006-01-20 21:56:08Z Zug $
#ifndef _LOGS_H_
#define _LOGS_H_

//int log_pick(struct map_session_data *sd, char *type, int mob_id, int nameid, int amount, struct item *item);
//int log_drop(struct map_session_data *sd, int monster_id, int *log_drop);
//int log_zeny(struct map_session_data *sd, char *type, struct map_session_data *src_sd, int amount);
//int log_drop(struct map_session_data *sd, int monster_id, int *log_drop);
//int log_present(struct map_session_data *sd, int source_type, int nameid);
//int log_produce(struct map_session_data *sd, int nameid, int slot1, int slot2, int slot3, int success);

int log_refine (struct map_session_data *sd, int equipid, unsigned short state);
int log_npcsell(struct map_session_data *sd, int n, unsigned short *item_list, int zeny);
int log_pcdrop (struct map_session_data *sd, int idx, int amount);
int log_pcpick (struct map_session_data *sd, struct item *item, int amount);
//int log_mvpdrop(struct map_session_data *sd, int monster_id, int *log_mvp);

//int log_trade(struct map_session_data *sd,struct map_session_data *target_sd,int n,int amount);
//int log_tostorage(struct map_session_data *sd,int n, int guild);
//int log_fromstorage(struct map_session_data *sd,int n, int guild);

//int log_vend(struct map_session_data *sd,struct map_session_data *vsd,int n,int amount,int zeny);
//int log_atcommand(struct map_session_data *sd, const char *message);
//int log_npc(struct map_session_data *sd, const char *message);
//int log_chat(char *type, int type_id, int src_charid, int src_accid, char *map, int x, int y, char *dst_charname, char *message);
//int log_config_read(char *cfgName);
//int should_log_item(int filter, int nameid, int amount); //log filter check
//int log_strcpy_itemname( char * name, struct item *item );

char * log_itemgetname (struct item *item);

extern short log_refine_level;
extern short log_npcsell_level;
extern short log_pcdrop_level;
extern short log_pcpick_level;

struct Log_Config {
    //int enable_logs;
    //int sql_logs;
    //int rare_items_log;
	unsigned short refine_level;
	unsigned short produce_level;
	unsigned short npcsell_level;
	unsigned short pcdrop_level;
	/*
    int branch, pick, drop, steal, mvpdrop, present, produce, refine, trade, vend, zeny, gm, npc, storage, chat;
    char log_branch[32], log_pick[32], log_zeny[32], log_drop[32], log_mvpdrop[32], log_present[32], log_produce[32], log_refine[32], log_trade[32], log_vend[32], log_gm[32], log_npc[32], log_storage[32], log_chat[32];
    char log_branch_db[32], log_pick_db[32], log_zeny_db[32], log_drop_db[32], log_mvpdrop_db[32], log_present_db[32], log_produce_db[32], log_refine_db[32], log_trade_db[32], log_vend_db[32], log_gm_db[32], log_npc_db[32], log_chat_db[32];
    int uptime;
    char log_uptime[32];
	*/
} log_config;

#endif			//_LOGS_H_
