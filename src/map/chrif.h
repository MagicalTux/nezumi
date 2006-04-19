// $Id$
#ifndef _CHRIF_H_
#define _CHRIF_H_

void chrif_setuserid(char*);
void chrif_setpasswd(char*);
void chrif_setip(char*);
void chrif_setport(int);

int chrif_isconnect(void);

int chrif_authreq(struct map_session_data *);
void chrif_save(struct map_session_data*);
int chrif_charselectreq(struct map_session_data *);

int chrif_changemapserver(struct map_session_data *sd, char *name, int x, int y, int ip, short port);

int chrif_searchcharid(int char_id);
int chrif_changegm(int id, const char *pass, int len);
int chrif_changeemail(int id, const char *actual_email, const char *new_email);
int chrif_char_ask_name(int id, char * character_name, short operation_type, int year_gmlvl, int month, int day, int hour, int minute, int second);
void chrif_saveaccountreg2(struct map_session_data *sd);
void chrif_saveglobalreg(struct map_session_data *sd);
int chrif_reloadGMdb(void);

void chrif_send_friends(int friend_char_id1, int friend_char_id2); // 0x2b24 <friend_char_id1>.L <friend_char_id2>.L
void chrif_friend_delete(int friend_list_char_id, int deleted_char_id, unsigned char flag); // owner of friend list, deleted friend (flag: 0: done on map-server, 1: not done for friend)

int chrif_save_scdata(struct map_session_data *sd);

int do_final_chrif(void);
int do_init_chrif(void);

#endif // _CHRIF_H_
