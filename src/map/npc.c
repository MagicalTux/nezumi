// $Id$

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> // isspace
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#include "../common/db.h"
#include "../common/timer.h"
#include "nullpo.h"
#include "../common/malloc.h"
#include "map.h"
#include "npc.h"
#include "clif.h"
#include "intif.h"
#include "pc.h"
#include "itemdb.h"
#include "script.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "skill.h"
#include "grfio.h"
#include "status.h"
#include "atcommand.h"
#include "chrif.h"
#include "log.h"
#ifdef __PHP_ENGINE
#include "npc_php.h"
#endif

#ifdef MEMWATCH
#include "memwatch.h"
#endif

struct npc_src_list {
	struct npc_src_list * next;
	char *name;
};

static struct npc_src_list *npc_src_first = NULL;
static struct npc_src_list *npc_src_last = NULL;
static int npc_id = START_NPC_NUM;
static int npc_warp = 0;
static int npc_shop = 0;
static int npc_script = 0;
static int npc_mob = 0;

char *current_file = NULL;
int npc_get_new_npc_id(void) { return npc_id++; } // first with 110000000 (START_NPC_NUM)

static struct dbt *ev_db;
static struct dbt *npcname_db;

struct event_data {
	struct npc_data *nd;
	intptr_t pos;
};
static struct tm ev_tm_b;	// ���v�C�x���g�p

static TIMER_FUNC(npc_walktimer); // [Valaris]
static int npc_walktoxy_sub(struct npc_data *nd); // [Valaris]


/*==========================================
 * NPC�̖�����/�L����
 * npc_enable
 * npc_enable_sub �L������OnTouch�C�x���g�����s
 *------------------------------------------
 */
int npc_enable_sub(struct block_list *bl, va_list ap) {
	struct map_session_data *sd;
	struct npc_data *nd;
	char *name;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd = va_arg(ap, struct npc_data *));

	if (bl->type == BL_PC && (sd = (struct map_session_data *)bl)) {

		if (nd->flag & 1) // ����������Ă���
			return 1;

		if (sd->areanpc_id == nd->bl.id)
			return 1;
		CALLOC(name, char, 50);
		strncpy(name, nd->name, 50);
		sd->areanpc_id = nd->bl.id;
		npc_event(sd, strcat(name, "::OnTouch"), 0);
		FREE(name);
	}

	return 0;
}

int npc_enable(const char *name, int flag)
{
	struct npc_data *nd = strdb_search(npcname_db, name);

	if (nd == NULL)
		return 0;

	if (flag & 1) { // �L����
		nd->flag &= ~1;
		clif_spawnnpc(nd);
	} else if (flag & 2) {
		nd->flag &= ~1;
		nd->option = 0x0000;
		clif_changeoption(&nd->bl);
	} else if (flag & 4) {
		nd->flag |= 1;
		nd->option = 0x0002;
		clif_changeoption(&nd->bl);
	} else { // ������
		nd->flag |= 1;
		clif_clearchar(&nd->bl, 0);
	}
	if (flag & 3 && (nd->u.scr.xs > 0 || nd->u.scr.ys > 0))
		map_foreachinarea(npc_enable_sub, nd->bl.m, nd->bl.x - nd->u.scr.xs, nd->bl.y - nd->u.scr.ys, nd->bl.x + nd->u.scr.xs, nd->bl.y + nd->u.scr.ys, BL_PC, nd);

	return 0;
}

/*==========================================
 * NPC�𖼑O�ŒT��
 *------------------------------------------
 */
struct npc_data* npc_name2id(const char *name)
{
	return strdb_search(npcname_db, name);
}

/*==========================================
 * �C�x���g�L���[�̃C�x���g����
 *------------------------------------------
 */
int npc_event_dequeue(struct map_session_data *sd) {
	char *name;
	int i;

	nullpo_retr(0, sd);

	sd->npc_id = 0;
	if (sd->eventqueue[0][0]) { // �L���[�̃C�x���g����
		size_t ev;
		// find an empty place in eventtimer list
		for(ev = 0; ev < MAX_EVENTTIMER; ev++)
			if (sd->eventtimer[ev] == -1)
				break;
		if (ev < MAX_EVENTTIMER) { // generate and insert the timer
			// copy the first event name
			CALLOC(name, char, 50); // 49 + NULL
			strncpy(name, sd->eventqueue[0], 49);
			// shift queued events down by one
			for(i = 1; i < MAX_EVENTQUEUE; i++) {
				memset(&sd->eventqueue[i-1], 0, sizeof(sd->eventqueue[i-1]));
				strncpy(sd->eventqueue[i-1], sd->eventqueue[i], 49); // 49 + NULL
			}
			// clear the last event
			memset(&sd->eventqueue[MAX_EVENTQUEUE-1], 0, sizeof(sd->eventqueue[MAX_EVENTQUEUE-1]));
			// add the timer
			sd->eventtimer[ev] = add_timer(gettick_cache + 100, pc_eventtimer, sd->bl.id, (intptr_t)name);
		} else
			printf("npc_event_dequeue: event timer is full !\n");
	}

	return 0;
}

int npc_delete(struct npc_data *nd)
{
	nullpo_retr(1, nd);

	if(nd->bl.prev == NULL)
		return 1;

	clif_clearchar_area(&nd->bl,1);
	map_delblock(&nd->bl);

	return 0;
}

/*==========================================
 * �C�x���g�̒x�����s
 *------------------------------------------
 */
TIMER_FUNC(npc_event_timer) {
/*	struct map_session_data *sd = map_id2sd(id);

	if (sd==NULL)
		return 0;

	npc_event(sd,(const char *)data, 0);
	free((void*)data);*/
	char *eventname = (char *)data;
	struct event_data *ev = (struct event_data *)strdb_search(ev_db, eventname);
	struct npc_data *nd;
	struct map_session_data *sd = map_id2sd(id);
	size_t i;

	if ((ev == NULL || (nd = ev->nd) == NULL)) {
		if (battle_config.error_log)
			printf("npc_event: event not found [%s]\n", eventname);
	} else {
		for(i = 0; i < MAX_EVENTTIMER; i++) {
			if (nd->eventtimer[i] == tid) {
				nd->eventtimer[i] = -1;
				if (sd != NULL)
					npc_event(sd, eventname, 0);
				break;
			}
		}
		if (i == MAX_EVENTTIMER && battle_config.error_log)
			printf("npc_event_timer: event timer not found [%s]!\n", eventname);
	}

	FREE(eventname);

	return 0;
}

int npc_timer_event(const char *eventname) // Added by RoVeRT
{
	struct event_data *ev = strdb_search(ev_db, eventname);
	struct npc_data *nd;
//	int xs,ys;

	if ((ev == NULL || (nd=ev->nd) == NULL)) {
		printf("npc_event: event not found [%s]\n", eventname);
		return 0;
	}

	run_script(nd->u.scr.script, ev->pos, nd->bl.id, nd->bl.id);

	return 0;
}

/*
int npc_timer_sub_sub(void *key, void *data, va_list ap) { // Added by RoVeRT
	char *p = (char *)key;
	struct event_data *ev=(struct event_data *)data;
	int *c = va_arg(ap,int *);
	int tick = 0;
	char temp[10];
	char event[100];

	if (ev->nd->bl.id == (int)*c && (p=strchr(p, ':')) && p && strncasecmp("::OnTimer", p, 8) == 0) {
		sscanf(&p[9], "%s", temp);
		tick = atoi(temp);

		strcpy(event, ev->nd->name);
		strcat(event, p);

		if (gettick_cache >= ev->nd->lastaction && gettick_cache - ev->nd->timer >= tick) {
			npc_timer_event(event);
			ev->nd->lastaction = gettick_cache;
		}
	}

	return 0;
}

int npc_timer_sub(void *key,void *data,va_list ap)	// Added by RoVeRT
{
	struct npc_data *nd=(struct npc_data*)data;

	if(nd->timer == -1)
		return 0;

	strdb_foreach(ev_db,npc_timer_sub_sub,&nd->bl.id);

	return 0;
}

int npc_timer(int tid,unsigned int tick,int id,int data)	// Added by RoVeRT
{
	strdb_foreach(npcname_db,npc_timer_sub);

	free((void*)data);

	return 0;
}*/

/*==========================================
 * �C�x���g�p���x���̃G�N�X�|�[�g
 * npc_parse_script->strdb_foreach����Ă΂��
 *------------------------------------------
 */
int npc_event_export(void *key, void *data, va_list ap) {
	char *lname = (char *)key;
	struct npc_data *nd = va_arg(ap,struct npc_data *);

	if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
		struct event_data *ev;
		char *buf;
		char *p = strchr(lname, ':');
		// �G�N�X�|�[�g�����
		CALLOC(ev, struct event_data, 1);
		CALLOC(buf, char, 50);
		if (p == NULL || (p - lname) > 24) {
			printf("npc_event_export: label name error !\n");
			exit(1);
		}else{
			ev->nd = nd;
			ev->pos = (intptr_t)data;
			*p = '\0';
			sprintf(buf, "%s::%s", nd->exname, lname);
			*p = ':';
			strdb_insert(ev_db, buf, ev);
//			if (battle_config.etc_log)
//				printf("npc_event_export: export [%s]\n", buf);
		}
	}

	return 0;
}

/*==========================================
 * �S�Ă�NPC��On*�C�x���g���s
 *------------------------------------------
 */
int npc_event_doall_sub(void *key,void *data,va_list ap)
{
	char *p=(char *)key;
	struct event_data *ev;
	int *c;
	const char *name;

	nullpo_retr(0, ev=(struct event_data *)data);
	nullpo_retr(0, ap);
	nullpo_retr(0, c=va_arg(ap,int *));

	name=va_arg(ap,const char *);

	if ((p = strchr(p, ':')) && p && strcasecmp(name, p) == 0) {
		run_script(ev->nd->u.scr.script, ev->pos, 0, ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}

int npc_event_doall(const char *name)
{
	int c = 0;
	char buf[64] = "::";

	strncpy(buf + 2, name, 62);
	strdb_foreach(ev_db, npc_event_doall_sub, &c, buf);

	return c;
}

int npc_event_do_sub(void *key,void *data,va_list ap)
{
	char *p=(char *)key;
	struct event_data *ev;
	int *c;
	const char *name;

	nullpo_retr(0, ev=(struct event_data *)data);
	nullpo_retr(0, ap);
	nullpo_retr(0, c=va_arg(ap,int *));

	name=va_arg(ap,const char *);

	if (p && strcasecmp(name, p) == 0) {
		run_script(ev->nd->u.scr.script,ev->pos,0,ev->nd->bl.id);
		(*c)++;
	}

	return 0;
}

int npc_event_do(const char *name) {
	int c = 0;

	if (*name==':' && name[1] == ':') {
		return npc_event_doall(name + 2);
	}

	strdb_foreach(ev_db, npc_event_do_sub, &c, name);

	return c;
}

/*==========================================
 * ���v�C�x���g���s
 *------------------------------------------
 */
TIMER_FUNC(npc_event_do_clock) {
	time_t timer;
	struct tm *t;
	char buf[64];
	char *day = "";
	int c = 0;

	time(&timer);
	t = localtime(&timer);

	//t->tm_wday : weekday 0 ~ 6  ( 0 = sunday )
	switch (t->tm_wday) {
	case 0: day = "Sun"; break;
	case 1: day = "Mon"; break;
	case 2: day = "Tue"; break;
	case 3: day = "Wed"; break;
	case 4: day = "Thu"; break;
	case 5: day = "Fri"; break;
	case 6: day = "Sat"; break;
	}

	if (t->tm_min != ev_tm_b.tm_min) {
		sprintf(buf, "OnMinute%02d", t->tm_min);
		c += npc_event_doall(buf);
		sprintf(buf, "OnClock%02d%02d", t->tm_hour, t->tm_min);
		c += npc_event_doall(buf);
		sprintf(buf, "On%s%02d%02d", day, t->tm_hour, t->tm_min);
		c += npc_event_doall(buf);
	}
	if (t->tm_hour != ev_tm_b.tm_hour) {
		sprintf(buf, "OnHour%02d", t->tm_hour);
		c += npc_event_doall(buf);
	}
	if (t->tm_mday!= ev_tm_b.tm_mday) {
		sprintf(buf,"OnDay%02d%02d", t->tm_mon + 1, t->tm_mday);
		c+=npc_event_doall(buf);
	}
	memcpy(&ev_tm_b, t, sizeof(ev_tm_b));

	return c;
}

/*==========================================
 * OnInit�C�x���g���s(&���v�C�x���g�J�n)
 *------------------------------------------
 */
int npc_event_do_oninit(void)
{
	printf("npc: Event '" CL_WHITE "OnInit" CL_RESET "' executed with '" CL_WHITE "%d" CL_RESET "' NPCs.\n", npc_event_doall("OnInit"));

	add_timer_interval(gettick_cache + 100, npc_event_do_clock, 0, 0, 15000); // "ontimer" are coded for every min. so, check only every 15 sec (not every sec)

	return 0;
}

/*==========================================
 * OnTimer NPC event - by RoVeRT
 *------------------------------------------
 */
int npc_addeventtimer(struct npc_data *nd, int tick, const char *name) {
	int i;
	char *evname;

	for(i = 0; i < MAX_EVENTTIMER; i++)
		if (nd->eventtimer[i] == -1)
			break;
	if (i < MAX_EVENTTIMER) {
		CALLOC(evname, char, 25); // 24 + NULL
		strncpy(evname, name, 24);
		nd->eventtimer[i] = add_timer(gettick_cache + tick, npc_event_timer, nd->bl.id, (intptr_t)evname);
	} else
		printf("npc_addtimer: event timer is full !\n");

	return 0;
}

int npc_deleventtimer(struct npc_data *nd, const char *name) {
	int i;
	char *evname;

	for(i = 0; i < MAX_EVENTTIMER; i++)
		if (nd->eventtimer[i] != -1 && strcmp((evname = (char *)(get_timer(nd->eventtimer[i])->data)), name) == 0) {
			delete_timer(nd->eventtimer[i], npc_event_timer);
			FREE(evname);
			nd->eventtimer[i] = -1;
			break;
		}

	return 0;
}

int npc_cleareventtimer(struct npc_data *nd) {
	int i;
	char *evname;

	for(i = 0; i < MAX_EVENTTIMER; i++)
		if (nd->eventtimer[i] != -1) {
			evname = (char *)(get_timer(nd->eventtimer[i])->data);
			delete_timer(nd->eventtimer[i], npc_event_timer);
			FREE(evname);
			nd->eventtimer[i] = -1;
		}

	return 0;
}

int npc_do_ontimer_sub(void *key, void *data, va_list ap) {
	char *p = (char *)key;
	struct event_data *ev = (struct event_data *)data;
	int *c = va_arg(ap, int *);
//	struct map_session_data *sd=va_arg(ap,struct map_session_data *);
	int option=va_arg(ap,int);
	int tick=0;
	char temp[10];
	char event[50];

	if (ev->nd->bl.id == (int)*c && (p=strchr(p,':')) && p && strncasecmp("::OnTimer", p, 8) == 0) {
		sscanf(&p[9], "%s", temp);
		tick = atoi(temp);

		strcpy(event, ev->nd->name);
		strcat(event, p);

		if (option != 0) {
			npc_addeventtimer(ev->nd, tick, event);
		} else {
			npc_deleventtimer(ev->nd, event);
		}
	}

	return 0;
}

int npc_do_ontimer(int npc_idx, struct map_session_data *sd, int option) {
	strdb_foreach(ev_db, npc_do_ontimer_sub, &npc_idx, sd, option);

	return 0;
}

/*==========================================
 * �^�C�}�[�C�x���g�p���x���̎�荞��
 * npc_parse_script->strdb_foreach����Ă΂��
 *------------------------------------------
 */
int npc_timerevent_import(void *key, void *data, va_list ap)
{
	char *lname = (char *)key;
	struct npc_data *nd = va_arg(ap, struct npc_data *);
	int t = 0, n = 0;

	if (sscanf(lname, "OnTimer%d%n", &t, &n) == 1 && lname[n] == ':') {
		// �^�C�}�[�C�x���g
		struct npc_timerevent_list *te = nd->u.scr.timer_event;
		int j, i = nd->u.scr.timeramount;
		if (te == NULL) {
			CALLOC(te, struct npc_timerevent_list, 1);
		} else {
			REALLOC(te, struct npc_timerevent_list, i + 1);
			memset(te + i, 0, sizeof(struct npc_timerevent_list));
		}
		for(j = 0; j < i; j++) {
			if (te[j].timer > t) {
				memmove(te + j + 1, te + j, sizeof(struct npc_timerevent_list) * (i - j));
				break;
			}
		}
		te[j].timer = t;
		te[j].pos = (intptr_t)data;
		nd->u.scr.timer_event = te;
		nd->u.scr.timeramount = i + 1;
	}

	return 0;
}

/*==========================================
 * �^�C�}�[�C�x���g���s
 *------------------------------------------
 */
TIMER_FUNC(npc_timerevent) {
	int next,t;
	struct npc_data* nd = (struct npc_data *)map_id2bl(id);
	struct npc_timerevent_list *te;

	if (nd == NULL || nd->u.scr.nexttimer < 0) {
		printf("npc_timerevent: ??\n");
		return 0;
	}

	nd->u.scr.timertick = tick;
	te = nd->u.scr.timer_event + nd->u.scr.nexttimer;
	nd->u.scr.timerid = -1;

	t = nd->u.scr.timer += data;
	nd->u.scr.nexttimer++;
	if (nd->u.scr.timeramount>nd->u.scr.nexttimer) {
		next= nd->u.scr.timer_event[nd->u.scr.nexttimer].timer - t;
		nd->u.scr.timerid = add_timer(tick + next, npc_timerevent, id, next);
	}

	run_script(nd->u.scr.script, te->pos, nd->u.scr.rid, nd->bl.id);

	return 0;
}

/*==========================================
 * �^�C�}�[�C�x���g�J�n
 *------------------------------------------
 */
int npc_timerevent_start(struct npc_data *nd, int rid)
{
	int j, n, next;

	nullpo_retr(0, nd);

	n = nd->u.scr.timeramount;
	if (nd->u.scr.nexttimer >= 0 || n == 0)
		return 0;

	for(j = 0; j < n; j++) {
		if (nd->u.scr.timer_event[j].timer > nd->u.scr.timer)
			break;
	}
	if (j >= n) // check if there is a timer to use !!BEFORE!! you write stuff to the structures [Shinomori]
		return 0;

	nd->u.scr.nexttimer = j;
	nd->u.scr.timertick = gettick_cache;
	if (rid >= 0) // if rid is less than 0 leave it unchanged [celest]
		nd->u.scr.rid = rid; // changed to: attaching to given rid by default [Shinomori]

	next = nd->u.scr.timer_event[j].timer - nd->u.scr.timer;
	nd->u.scr.timerid = add_timer(gettick_cache + next, npc_timerevent, nd->bl.id, next);

	return 0;
}

/*==========================================
 * �^�C�}�[�C�x���g�I��
 *------------------------------------------
 */
int npc_timerevent_stop(struct npc_data *nd)
{
	nullpo_retr(0, nd);

	if (nd->u.scr.nexttimer >= 0) {
		nd->u.scr.nexttimer = -1;
		nd->u.scr.timer += (int)(gettick_cache - nd->u.scr.timertick);
		if (nd->u.scr.timerid != -1) {
			delete_timer(nd->u.scr.timerid, npc_timerevent);
			nd->u.scr.timerid = -1;
		}
		nd->u.scr.rid = 0;
	}

	return 0;
}

/*==========================================
 * �^�C�}�[�l�̏���
 *------------------------------------------
 */
int npc_gettimerevent_tick(struct npc_data *nd)
{
	int tick;

	nullpo_retr(0, nd);

	tick = nd->u.scr.timer;

	if (nd->u.scr.nexttimer >= 0)
		tick += (int)(gettick_cache - nd->u.scr.timertick);

	return tick;
}

/*==========================================
 * �^�C�}�[�l�̐ݒ�
 *------------------------------------------
 */
int npc_settimerevent_tick(struct npc_data *nd, int newtimer)
{
	int flag;

	nullpo_retr(0, nd);

	flag = nd->u.scr.nexttimer;

	npc_timerevent_stop(nd);
	nd->u.scr.timer = newtimer;
	if (flag >= 0)
		npc_timerevent_start(nd, -1);

	return 0;
}

/*==========================================
 * �C�x���g�^��NPC����
 *------------------------------------------
 */
int npc_event(struct map_session_data *sd, const char *eventname, int mob_kill)
{
	struct event_data *ev = strdb_search(ev_db, eventname);
	struct npc_data *nd;
	int xs, ys;
	char mobevent[100];

	if (sd == NULL) {
		printf("npc_event nullpo?\n");
	}

	if (ev == NULL && eventname && strcmp(((eventname) + strlen(eventname) - 9), "::OnTouch") == 0)
		return 1;

	if (ev == NULL || (nd = ev->nd) == NULL) {
//		if (mob_kill && (ev == NULL || (nd = ev->nd) == NULL)) {
		if (mob_kill) {
			strcpy(mobevent, eventname);
			strcat(mobevent, "::OnMyMobDead");
			ev = strdb_search(ev_db, mobevent);
			if (ev == NULL || (nd = ev->nd) == NULL) {
				if (strncasecmp(eventname, "GM_MONSTER", 10) != 0)
					printf("npc_event: event not found [%s]\n", mobevent);
				return 0;
			}
		}
		else {
			if (battle_config.error_log)
				printf("npc_event: event not found [%s]\n", eventname);
			return 0;
		}
	}

	xs=nd->u.scr.xs;
	ys=nd->u.scr.ys;
	if (xs>=0 && ys>=0 ) {
		if (nd->bl.m != sd->bl.m )
			return 1;
		if ( xs>0 && (sd->bl.x<nd->bl.x-xs/2 || nd->bl.x+xs/2<sd->bl.x) )
			return 1;
		if ( ys>0 && (sd->bl.y<nd->bl.y-ys/2 || nd->bl.y+ys/2<sd->bl.y) )
			return 1;
	}

	if (sd->npc_id != 0) {
//		if (battle_config.error_log)
//			printf("npc_event: npc_id != 0\n");
		int i;
		for(i = 0; i < MAX_EVENTQUEUE; i++)
			if (!sd->eventqueue[i][0])
				break;
		if (i == MAX_EVENTQUEUE) {
			if (battle_config.error_log)
				printf("npc_event: event queue is full !\n");
		} else {
//			if (battle_config.etc_log)
//				printf("npc_event: enqueue\n");
			memset(&sd->eventqueue[i], 0, sizeof(sd->eventqueue[i]));
			strncpy(sd->eventqueue[i], eventname, 49); // 49 + NULL
		}
		return 1;
	}
	if (nd->flag & 1) {	// ����������Ă���
		npc_event_dequeue(sd);
		return 0;
	}

	sd->npc_id=nd->bl.id;
	sd->npc_pos=run_script(nd->u.scr.script,ev->pos,sd->bl.id,nd->bl.id);

	return 0;
}

int npc_command_sub(void *key, void *data, va_list ap)
{
	char *p = (char *)key;
	struct event_data *ev = (struct event_data *)data;
	char *npcname = va_arg(ap,char *);
	char *command = va_arg(ap,char *);
	char temp[100];

	if (strcmp(ev->nd->name, npcname) == 0 && (p = strchr(p,':')) && p && strncasecmp("::OnCommand", p, 10) == 0) {
		sscanf(&p[11], "%s", temp);

		if (strcmp(command,temp) == 0)
			run_script(ev->nd->u.scr.script, ev->pos, 0, ev->nd->bl.id);
	}

	return 0;
}

int npc_command(struct map_session_data *sd, char *npcname, char *command)
{
	strdb_foreach(ev_db, npc_command_sub, npcname, command);

	return 0;
}

/*==========================================
 * �ڐG�^��NPC����
 *------------------------------------------
 */
int npc_touch_areanpc(struct map_session_data *sd,int m,int x,int y)
{
	int i, f = 1;
	int xs, ys;

	nullpo_retr(1, sd);

	if (sd->npc_id != 0)
		return 1;

	for(i = 0; i < map[m].npc_num; i++) {
		if (map[m].npc[i]->flag & 1) { // ����������Ă���
			f = 0;
			continue;
		}

		switch(map[m].npc[i]->bl.subtype) {
		case WARP:
			xs = map[m].npc[i]->u.warp.xs;
			ys = map[m].npc[i]->u.warp.ys;
			break;
		case SCRIPT:
			xs = map[m].npc[i]->u.scr.xs;
			ys = map[m].npc[i]->u.scr.ys;
			break;
		default:
			continue;
		}
		if (x >= map[m].npc[i]->bl.x - xs / 2 && x < map[m].npc[i]->bl.x - xs / 2 + xs &&
		    y >= map[m].npc[i]->bl.y - ys / 2 && y < map[m].npc[i]->bl.y - ys / 2 + ys)
			break;
	}
	if (i == map[m].npc_num) {
		if (f) {
			if (battle_config.error_log)
				printf("npc_touch_areanpc : some bug \n");
		}
		return 1;
	}

	switch(map[m].npc[i]->bl.subtype) {
	case WARP:
		skill_stop_dancing(&sd->bl, 0);
		pc_setpos(sd, map[m].npc[i]->u.warp.name, map[m].npc[i]->u.warp.x, map[m].npc[i]->u.warp.y, 0);
		break;
	case SCRIPT:
	  {
		char *name;
		if (sd->areanpc_id == map[m].npc[i]->bl.id)
			return 1;
		CALLOC(name, char, 50);
		strncpy(name, map[m].npc[i]->name, 50);
		sd->areanpc_id = map[m].npc[i]->bl.id;
		if (npc_event(sd, strcat(name, "::OnTouch"), 0) > 0)
			npc_click(sd, map[m].npc[i]->bl.id);
		FREE(name);
		break;
	  }
	}

	return 0;
}

/*==========================================
 * �߂����ǂ����̔���
 *------------------------------------------
 */
int npc_checknear(struct map_session_data *sd, struct npc_data *nd) {
//	nullpo_retr(0, sd); // checked before to call function

	if (nd == NULL || nd->bl.type != BL_NPC)
		return 1;

	if (nd->class < 0) // �C�x���g�n�͏��OK
		return 0;

	// �G���A����
	if (nd->bl.m != sd->bl.m ||
	    nd->bl.x < sd->bl.x - AREA_SIZE - 1 || nd->bl.x > sd->bl.x + AREA_SIZE + 1 ||
	    nd->bl.y < sd->bl.y - AREA_SIZE - 1 || nd->bl.y > sd->bl.y + AREA_SIZE + 1)
		return 1;

	return 0;
}

/*==========================================
 * NPC�̃I�[�v���`���b�g����
 *------------------------------------------
 */
int npc_globalmessage(const char *name, char *mes) {
	struct npc_data *nd = (struct npc_data *)strdb_search(npcname_db, name);
	char *temp;

	if (!nd)
		return 0;

	CALLOC(temp, char, strlen(name) + 3 + strlen(mes) + 1); // + NULL
	sprintf(temp, "%s : %s", name, mes);
	clif_GlobalMessage(&nd->bl, temp);
	FREE(temp);

	return 0;
}

// === CHECK IF PLAYER CAN USE NPC ===
// ===================================
void npc_click(struct map_session_data *sd, int id)
{
	struct npc_data *nd;

	// nullpo_retr(1, sd);

	if(sd->trade_partner != 0)		// player can't use npc:s while trading
		return;

	// todo: maybe add chat check here ? [Harbin]

	if(sd->sc_data[SC_FREEZE].timer != -1 || sd->sc_data[SC_STAN].timer != -1 || sd->sc_data[SC_SLEEP].timer != -1 || (sd->sc_data[SC_STONE].timer != -1 && sd->sc_data[SC_STONE].val2 == 0))
		return;

	nd = (struct npc_data *)map_id2bl(id);

	if(npc_checknear(sd, nd))
		return;

	if(nd->flag & 1)
		return;

	sd->npc_id = id;

	switch(nd->bl.subtype)
	{
		case SHOP:
			clif_npcbuysell(sd, id);
			npc_event_dequeue(sd);
			break;
		case SCRIPT:
			sd->npc_pos = run_script(nd->u.scr.script, 0, sd->bl.id, id);
			break;
	}
	return;
}

/*==========================================
 *
 *------------------------------------------
 */
void npc_scriptcont(struct map_session_data *sd, int id) {
	struct npc_data *nd;

	nullpo_retv(sd);

	if (id != sd->npc_id)
		return;

	nd = (struct npc_data *)map_id2bl(id);
	if (npc_checknear(sd, nd)) // check if npc exists too and is a NPC
		return;

	sd->npc_pos = run_script(nd->u.scr.script, sd->npc_pos, sd->bl.id, id);

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
void npc_buysellsel(struct map_session_data *sd, int id, int type) {
	struct npc_data *nd;

//	nullpo_retr(1, sd); // checked before to call function

	nd = (struct npc_data *)map_id2bl(id);
	if (npc_checknear(sd, nd)) // check if npc exists too and is a NPC
		return;

	if (nd->bl.subtype != SHOP) {
		sd->npc_id = 0;
		return;
	}
	if (nd->flag & 1) // ����������Ă���
		return;

	sd->npc_shopid = id;
	if (type == 0) {
		clif_buylist(sd, nd);
	} else {
		clif_selllist(sd);
	}

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_buylist(struct map_session_data *sd, int n, unsigned short *item_list) {
	struct npc_data *nd;
	double z;
	int i, j, w, skill, new = 0;
	struct item_data *item_data;

//	nullpo_retr(3, sd); // checked before to call function
//	nullpo_retr(3, item_list); // checked before to call function

	nd = (struct npc_data*)map_id2bl(sd->npc_shopid);
	if (npc_checknear(sd, nd)) // check if npc exists too and is a NPC
		return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

	if (nd->bl.subtype != SHOP)
		return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

	w = 0;
	z = 0.;
	for(i = 0; i < n; i++) {
		int nameid, amount;
		amount = item_list[i * 2];
		if (amount <= 0)
			return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

		nameid = item_list[i * 2 + 1];
		if (nameid <= 0 || (item_data = itemdb_search(nameid)) == NULL)
			return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.
		for(j = 0; nd->u.shop_item[j].nameid; j++) {
			if (nd->u.shop_item[j].nameid == nameid)
				break;
		}
		if (nd->u.shop_item[j].nameid == 0)
			return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

		if (itemdb_isequip3(nd->u.shop_item[j].nameid) && item_list[i*2] > 1) {
			item_list[i*2] = 1;
			char message_to_gm[strlen(msg_txt(538)) + strlen(msg_txt(539)) + strlen(msg_txt(540)) + strlen(msg_txt(507)) + strlen(msg_txt(508))];
			sprintf(message_to_gm, "Player %s (%d:%d) sent a hexed packet trying to buy %d of nonstackable item %d!", sd->status.name, (int)sd->status.account_id, (int)sd->status.char_id, item_list[i*2], nd->u.shop_item[j].nameid); 
			intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message_to_gm);
			chrif_char_ask_name(-1, sd->status.name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
			clif_setwaitclose(sd->fd); // forced to disconnect because of the hack
			// message about the ban
			sprintf(message_to_gm, msg_txt(540)); //  This player has been definitivly blocked.
			intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message_to_gm);
			return 3;
		}

		if (item_data->flag.value_notdc)
			z += ((double)nd->u.shop_item[j].value * (double)amount);
		else
			z += ((double)pc_modifybuyvalue(sd, nd->u.shop_item[j].value) * (double)amount);
		if (z < 0. || z > (double)sd->status.zeny)
			return 1; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

		switch(pc_checkadditem(sd, nameid, amount)) {
		case ADDITEM_EXIST:
			break;
		case ADDITEM_NEW:
			new++;
			break;
		case ADDITEM_OVERAMOUNT:
			return 2; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.
		}

		w += item_data->weight * amount;
		if (w + sd->weight > sd->max_weight)
			return 2; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.
	}
	if (pc_inventoryblank(sd) < new)
		return 3; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.

	pc_payzeny(sd, (int)z);
	for(i = 0;i < n; i++) {
		struct item item_tmp;

		memset(&item_tmp, 0, sizeof(item_tmp));
		item_tmp.nameid = item_list[i * 2 + 1];
		item_tmp.identify = 1; // npc�̔��A�C�e���͊Ӓ�ς�

		pc_additem(sd, &item_tmp, item_list[i * 2]);
	}

	//���l�o���l
	if (battle_config.shop_exp > 0 && z > 0. && (skill = pc_checkskill(sd, MC_DISCOUNT)) > 0) {
		if (sd->status.skill[MC_DISCOUNT].flag >= 2 && sd->status.skill[MC_DISCOUNT].flag != 13) // flag: 0 (normal), 1 (only card), 2-12 (card and skill (skill level +2)), 13 (cloneskill)
			if (skill < sd->status.skill[MC_DISCOUNT].flag - 2)
				skill = sd->status.skill[MC_DISCOUNT].flag - 2;
		if (skill > 0) {
			z = (log(z * (double)skill) * (double)battle_config.shop_exp / 100.);
			if (z < 1.)
				z = 1.;
			pc_gainexp(sd, 0, (int)z);
		}
	}

	return 0; // 0: The deal has successfully completed., 1: You dont have enough zeny., 2: you are overcharged!, 3: You are over your weight limit.
}

/*==========================================
 *
 *------------------------------------------
 */
int npc_selllist(struct map_session_data *sd, int n, unsigned short *item_list) {
	struct npc_data *nd;
	double z;
	int i, skill, idx;
	struct item_data *item_data;
	struct item inventory[MAX_INVENTORY]; // too fix cumulativ selling (hack)

//	nullpo_retr(1, sd); // checked before to call function
//	nullpo_retr(1, item_list); // checked before to call function

	nd = (struct npc_data*)map_id2bl(sd->npc_shopid);
	if (npc_checknear(sd, nd)) // check if npc exists too and is a NPC
		return 1;

	if (nd->bl.subtype != SHOP) // don't sell to a non-shop NPC!
		return 1;

	// get inventory of player
	memcpy(&inventory, &sd->status.inventory, sizeof(struct item) * MAX_INVENTORY);

	z = 0.;
	for(i = 0; i < n; i++) {
		int nameid, amount;
		idx = item_list[i * 2] - 2;
		if (idx < 0 || idx >= MAX_INVENTORY)
			return 1;
		amount = item_list[i * 2 + 1];
		if (amount <= 0 || inventory[idx].amount < amount)
			return 1;
		inventory[idx].amount = inventory[idx].amount - amount;
		nameid = inventory[idx].nameid;
		if (nameid <= 0 || (item_data = itemdb_search(nameid)) == NULL)
			return 1;
		if (item_data->flag.value_notoc)
			z += ((double)item_data->value_sell * (double)amount);
		else
			z += ((double)pc_modifysellvalue(sd, item_data->value_sell) * (double)amount);
		if (z < 0.) // fix overflow
			return 1;
		if (z > (double)MAX_ZENY - (double)sd->status.zeny) { // fix max zeny
			clif_displaymessage(sd->fd, msg_txt(649)); // You have too much zenys. You can not get more zenys.
			return 1;
		}
	}

	pc_getzeny(sd, (int)z);
	log_npcsell(sd, n, item_list, (int) z);
	for(i = 0; i < n; i++) {
		idx = item_list[i * 2] - 2;
		if (sd->inventory_data[idx] != NULL &&
		    sd->inventory_data[idx]->type == 7 &&
		    sd->status.inventory[idx].card[0] == (short)0xff00)
				if (search_petDB_index(sd->status.inventory[idx].nameid, PET_EGG) >= 0)
					intif_delete_petdata((*(long *)(&sd->status.inventory[idx].card[1])));
		pc_delitem(sd, idx, item_list[i * 2 + 1], 0);
	}

	//���l�o���l
	if (battle_config.shop_exp > 0 && z > 0. && (skill = pc_checkskill(sd, MC_OVERCHARGE)) > 0) {
		if (sd->status.skill[MC_OVERCHARGE].flag >= 2 && sd->status.skill[MC_OVERCHARGE].flag != 13) // flag: 0 (normal), 1 (only card), 2-12 (card and skill (skill level +2)), 13 (cloneskill)
			if (skill < sd->status.skill[MC_OVERCHARGE].flag - 2)
				skill = sd->status.skill[MC_OVERCHARGE].flag - 2;
		if (skill > 0) {
			z = (log(z * (double)skill) * (double)battle_config.shop_exp / 100.);
			if (z < 1.)
				z = 1.;
			pc_gainexp(sd, 0, (int)z);
		}
	}

	return 0;
}

// [Valaris] NPC Walking

/*==========================================
 * Time calculation concerning one step next to npc
 *------------------------------------------
 */
static int calc_next_walk_step(struct npc_data *nd) {
	nullpo_retr(0, nd);

	if (nd->walkpath.path_pos >= nd->walkpath.path_len)
		return -1;
	if (nd->walkpath.path[nd->walkpath.path_pos] & 1)
		return status_get_speed(&nd->bl) * 14 / 10;

	return status_get_speed(&nd->bl);
}


/*==========================================
 * npc Walk processing
 *------------------------------------------
 */
static int npc_walk(struct npc_data *nd, unsigned int tick, int data)
{
	int moveblock;
	int i;
	static int dirx[8] = {0, -1, -1, -1,  0,  1, 1, 1};
	static int diry[8] = {1,  1,  0, -1, -1, -1, 0, 1};
	int x, y, dx, dy;

	nullpo_retr(0, nd);

	nd->state.state = MS_IDLE;
	if (nd->walkpath.path_pos >= nd->walkpath.path_len || nd->walkpath.path_pos != data)
		return 0;

	nd->walkpath.path_half ^= 1;
	if (nd->walkpath.path_half == 0) {
		nd->walkpath.path_pos++;
		if (nd->state.change_walk_target) {
			npc_walktoxy_sub(nd);
			return 0;
		}
	}
	else {
		if (nd->walkpath.path[nd->walkpath.path_pos] >= 8)
			return 1;

		x = nd->bl.x;
		y = nd->bl.y;
		if (map_getcell(nd->bl.m, x, y, CELL_CHKNOPASS)) {
			npc_stop_walking(nd, 1);
			return 0;
		}
		nd->dir = nd->walkpath.path[nd->walkpath.path_pos];
		dx = dirx[nd->dir];
		dy = diry[nd->dir];
		if (map_getcell(nd->bl.m, x + dx, y + dy, CELL_CHKNOPASS)) {
			npc_walktoxy_sub(nd);
			return 0;
		}

		moveblock = (x/BLOCK_SIZE != (x+dx)/BLOCK_SIZE || y/BLOCK_SIZE != (y+dy)/BLOCK_SIZE);

		nd->state.state=MS_WALK;
		map_foreachinmovearea(clif_npcoutsight,nd->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,dx,dy,BL_PC,nd);

		x += dx;
		y += dy;

		if(moveblock) map_delblock(&nd->bl);
		nd->bl.x = x;
		nd->bl.y = y;
		if(moveblock) map_addblock(&nd->bl);

		map_foreachinmovearea(clif_npcinsight,nd->bl.m,x-AREA_SIZE,y-AREA_SIZE,x+AREA_SIZE,y+AREA_SIZE,-dx,-dy,BL_PC,nd);
		nd->state.state = MS_IDLE;
	}
	if ((i = calc_next_walk_step(nd)) > 0) {
		i = i >> 1;
		if(i < 1 && nd->walkpath.path_half == 0)
			i = 1;
		nd->walktimer = add_timer(tick + i, npc_walktimer, nd->bl.id, nd->walkpath.path_pos);
		nd->state.state = MS_WALK;

		if(nd->walkpath.path_pos>=nd->walkpath.path_len)
			clif_fixnpcpos(nd);	// When npc stops, retransmission current of a position.
	}

	return 0;
}

int npc_changestate(struct npc_data *nd, int state, int type)
{
	int i;

	nullpo_retr(0, nd);

	if (nd->walktimer != -1)
		delete_timer(nd->walktimer, npc_walktimer);

	nd->walktimer = -1;
	nd->state.state = state;

	switch(state) {
	case MS_WALK:
		if ((i = calc_next_walk_step(nd)) > 0) {
			i = i >> 2;
			nd->walktimer = add_timer(gettick_cache + i, npc_walktimer, nd->bl.id, 0);
		}
		else
			nd->state.state = MS_IDLE;
		break;
	case MS_DELAY:
		nd->walktimer = add_timer(gettick_cache + type, npc_walktimer, nd->bl.id, 0);
		break;

	}

	return 0;
}

static TIMER_FUNC(npc_walktimer) {
	struct npc_data *nd;

	nd = (struct npc_data*)map_id2bl(id);
	if (nd == NULL || nd->bl.type != BL_NPC)
		return 1;

	if (nd->walktimer != tid) {
		return 0;
	}

	nd->walktimer = -1;

	if (nd->bl.prev == NULL)
		return 1;

	switch(nd->state.state) {
		case MS_WALK:
			npc_walk(nd, tick, data);
			break;
		case MS_DELAY:
			npc_changestate(nd, MS_IDLE, 0);
			break;
		default:
			break;
	}

	return 0;
}

static int npc_walktoxy_sub(struct npc_data *nd)
{
	struct walkpath_data wpd;

	nullpo_retr(0, nd);

	if (path_search(&wpd, nd->bl.m, nd->bl.x, nd->bl.y, nd->to_x, nd->to_y, nd->state.walk_easy))
		return 1;
	memcpy(&nd->walkpath, &wpd, sizeof(wpd));

	nd->state.change_walk_target = 0;
	npc_changestate(nd, MS_WALK, 0);

	clif_movenpc(nd);

	return 0;
}

int npc_walktoxy(struct npc_data *nd,int x,int y,int easy)
{
	struct walkpath_data wpd;

	nullpo_retr(0, nd);

	if(nd->state.state == MS_WALK && path_search(&wpd,nd->bl.m,nd->bl.x,nd->bl.y,x,y,0) )
		return 1;

	nd->state.walk_easy = easy;
	nd->to_x=x;
	nd->to_y=y;
	if(nd->state.state == MS_WALK) {
		nd->state.change_walk_target=1;
	} else {
		return npc_walktoxy_sub(nd);
	}

	return 0;
}

int npc_stop_walking(struct npc_data *nd, int type)
{
	nullpo_retr(0, nd);

	if (nd->state.state == MS_WALK || nd->state.state == MS_IDLE) {
		int dx = 0, dy = 0;

		nd->walkpath.path_len = 0;
		if (type & 4) {
			dx = nd->to_x - nd->bl.x;
			if (dx < 0)
				dx = -1;
			else if (dx > 0)
				dx = 1;
			dy = nd->to_y - nd->bl.y;
			if (dy < 0)
				dy = -1;
			else if (dy > 0)
				dy = 1;
		}
		nd->to_x = nd->bl.x + dx;
		nd->to_y = nd->bl.y + dy;
		if (dx != 0 || dy != 0) {
			npc_walktoxy_sub(nd);
			return 0;
		}
		npc_changestate(nd, MS_IDLE, 0);
	}
	if (type & 0x01)
		clif_fixnpcpos(nd);
	if (type & 0x02) {
		int delay = status_get_dmotion(&nd->bl);
		if (nd->canmove_tick < gettick_cache)
			nd->canmove_tick = gettick_cache + delay;
	}

	return 0;
}

//
// �������֌W
//

/*==========================================
 * �ǂݍ���npc�t�@�C���̃N���A
 *------------------------------------------
 */
void npc_clearsrcfile() {
	struct npc_src_list *p = npc_src_first;

	while(p) {
		struct npc_src_list *p2 = p;
		p = p->next;
		FREE(p2->name);
		FREE(p2);
	}
	npc_src_first = NULL;
	npc_src_last = NULL;
}

/*==========================================
 * �ǂݍ���npc�t�@�C���̒ǉ�
 *------------------------------------------
 */
void npc_addsrcfile(char *name)
{
	struct npc_src_list *new;
	char *ptr_to_ext;

	if (strcasecmp(name, "clear") == 0) {
		npc_clearsrcfile();
		return;
	}
	ptr_to_ext = name + (strlen(name) - 4);
	if (strcasecmp(ptr_to_ext, ".php") == 0) {
#ifdef __PHP_ENGINE
		npc_php_addsrcfile(name);
#endif
		return;
	}

	// prevent multiple insert of source files
  {
	struct npc_src_list *p = npc_src_first;
	while (p) { // found the file, no need to insert it again
		if (strcmp(name, p->name) == 0)
			return;
		p = p->next;
	}
  }

	CALLOC(new, struct npc_src_list, 1);
	CALLOC(new->name, char, strlen(name) + 1); // + NULL
	new->next = NULL;
	strcpy(new->name, name);
	if (npc_src_first == NULL)
		npc_src_first = new;
	if (npc_src_last)
		npc_src_last->next = new;

	npc_src_last = new;
}

/*==========================================
 * �ǂݍ���npc�t�@�C���̍폜
 *------------------------------------------
 */
void npc_delsrcfile(char *name)
{
	struct npc_src_list *p=npc_src_first,*pp=NULL,**lp=&npc_src_first;

	if (strcasecmp(name, "all") == 0) {
		npc_clearsrcfile();
		return;
	}

	for( ; p; lp=&p->next,pp=p,p=p->next) {
		if ( strcmp(p->name,name)==0 ) {
			*lp=p->next;
			if ( npc_src_last==p )
				npc_src_last=pp;
			FREE(p->name);
			FREE(p);
			break;
		}
	}
}

/*==========================================
 * warp�s���
 *------------------------------------------
 */
int npc_parse_warp(char *w1, char *w3, char *w4, int lines) {
	int x, y, xs, ys, to_x, to_y, m;
	int i, j, ii, jj, count;
	char mapname[17], to_mapname[17]; // 16 + NULL
	struct npc_data *nd;

	// �����̌��`�F�b�N
	if (sscanf(w1, "%[^,],%d,%d", mapname, &x, &y) != 3 ||
	    sscanf(w4, "%d,%d,%[^,],%d,%d", &xs, &ys, to_mapname, &to_x, &to_y) != 5) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad warp line" CL_RESET ": %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET ".\n", w3, current_file, lines);
		return 1;
	}

	// check map names (need to check both map name, a GM command can call this function directly)
	if (strstr(mapname, ".gat") == NULL || mapname[0] == '\0' || strlen(mapname) > 16)
	{
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad source map name" CL_RESET " in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", w3, current_file, lines);
		return 1;
	}
	if(strstr(to_mapname, ".gat") == NULL || to_mapname[0] == '\0' || strlen(to_mapname) > 16)
	{
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad destination map name" CL_RESET " in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", w3, current_file, lines);
		return 1;
	}

	// check destination warp coordinates (if possible)
	if ((m = map_mapname2mapid(to_mapname)) >= 0) { // map id on this server (m == -1 if not in actual map-server)
		// out of range destination
		if (to_x < 0 || to_x >= map[m].xs || to_y < 0 || to_y >= map[m].ys) {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_RED "Bad destination coordinates" CL_RESET " (out of range: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", to_mapname, to_x, to_y, w3, current_file, lines);
			return 1;
		} else if (map_getcell(m, to_x, to_y, CELL_CHKNOPASS)) {
			if (current_file != NULL) // if not a GM command, but a script
#ifdef __DEBUG
				printf(CL_RED "Bad destination coordinates" CL_RESET " (can not walk on this position: %s,%d,%d, cell type: %d) in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", to_mapname, to_x, to_y, map_getcell(m, to_x, to_y, CELL_GETTYPE), w3, current_file, lines);
#else
				printf(CL_RED "Bad destination coordinates" CL_RESET " (can not walk on this position: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", to_mapname, to_x, to_y, w3, current_file, lines);
#endif
			return 1;
		} else if (map_getcell(m, to_x, to_y, CELL_CHKNPC)) {
			if (current_file != NULL) { // if not a GM command, but a script
				int k, xs1, ys1;
				// search npc name
				for(k = 0; k < map[m].npc_num; k++) {
					switch(map[m].npc[k]->bl.subtype) {
					case WARP:
						xs1 = map[m].npc[k]->u.warp.xs;
						ys1 = map[m].npc[k]->u.warp.ys;
						break;
					case SCRIPT:
						xs1 = map[m].npc[k]->u.scr.xs;
						ys1 = map[m].npc[k]->u.scr.ys;
						break;
					default:
						continue;
					}
					if (to_x >= map[m].npc[k]->bl.x - xs1 / 2 && to_x <= map[m].npc[k]->bl.x - xs1 / 2 + xs1 &&
					    to_y >= map[m].npc[k]->bl.y - ys1 / 2 && to_y <= map[m].npc[k]->bl.y - ys1 / 2 + ys1)
						break;
				}
				if (k != map[m].npc_num)
					printf(CL_YELLOW "WARNING: Bad destination coordinates" CL_RESET " (same of another npc/warp (%s): %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_YELLOW "loaded" CL_RESET ".\n", map[m].npc[k]->name, to_mapname, to_x, to_y, w3, current_file, lines);
				else
					printf(CL_YELLOW "WARNING: Bad destination coordinates" CL_RESET " (same of another npc/warp: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_YELLOW "loaded" CL_RESET ".\n", to_mapname, to_x, to_y, w3, current_file, lines);
			}
		}
	}

	m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)
	// don't accept warp if not in this map-server
	if (m < 0)
		return 1;

	// check source warp coordinates
	if (x < 0 || x >= map[m].xs || y < 0 || y >= map[m].ys) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad source coordinates" CL_RESET " (out of range: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED " not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	} else if (map_getcell(m, x, y, CELL_CHKNPC)) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad source coordinates" CL_RESET " (same of another npc/warp: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED " not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	}
	// check all position of source warp coordinates
	count = 0;
	for(i = 0; i < ys + 2; i++) {
		ii = y - (ys + 2) / 2 + i;
		for(j = 0; j < xs + 2; j++) {
			jj = x - (xs + 2) / 2 + j;
			if (map_getcell(m, jj, ii, CELL_CHKNOPASS))
				continue;
			if (map_getcell(m, jj, ii, CELL_CHKNPC)) // don't set 2 npc on same place
				continue;
			count++;
		}
	}
	if (count == 0) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "No available square" CL_RESET " (warp can not be used, position: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	} else if (count == 1) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Only 1 available square" CL_RESET " (it's difficult to use warp, position: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_RED "not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	}
	if (map_getcell(m, x, y, CELL_CHKNOPASS)) {
		if (current_file != NULL) { // if not a GM command, but a script
#ifdef __DEBUG
			printf(CL_YELLOW "WARNING: Bad source coordinates" CL_RESET " (can not walk on this position: %s,%d,%d, cell type: %d) in warp: %s (file:%s:%d), ", mapname, x, y, map_getcell(m, x, y, CELL_GETTYPE), w3, current_file, lines);
#else
			printf(CL_YELLOW "WARNING: Bad source coordinates" CL_RESET " (can not walk on this position: %s,%d,%d) in warp: %s (file:%s:%d), ", mapname, x, y, w3, current_file, lines);
#endif
			if (count == 2)
				printf(CL_YELLOW "and only 2 available squares" CL_RESET " -> " CL_YELLOW "loaded" CL_RESET "!\n");
			else
				printf("but this warp has %d available squares -> " CL_YELLOW "can be used" CL_RESET ".\n", count);
		}
	} else if (count == 2) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_YELLOW "WARNING: Only 2 available squares" CL_RESET " (it's difficult to use warp, position: %s,%d,%d) in warp: %s (file:%s:%d) -> " CL_YELLOW "loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
	} // with 3 squares, warp can be used by players without problem

	// warning on warp name
	if (strlen(w3) > 24) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_YELLOW "WARNING: Invalid warp name" CL_RESET" (warp: %s) - Name too long (> 24 char.) -> only 24 first characters are used (file:%s:%d).\n", w3, current_file, lines);
	}

	CALLOC(nd, struct npc_data, 1);
	nd->bl.id = npc_get_new_npc_id();
	nd->n = map_addnpc(m, nd);

	if (nd->n == -1) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Too many NPCs (%d) in map %s. Warp %s not loaded" CL_RESET " (file:%s:%d).\n", MAX_NPC_PER_MAP, mapname, w3, current_file, lines);
		FREE(nd);
		return 1;
	}

//	nd->bl.prev = NULL;
//	nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
//	nd->dir = 0;
//	nd->flag = 0;
	strncpy(nd->name, w3, 24);
	strncpy(nd->exname, w3, 24);

//	nd->chat_id = 0;
	if (!battle_config.warp_point_debug)
		nd->class = WARP_CLASS;
	else
		nd->class = WARP_DEBUG_CLASS;
	nd->speed = 200;
//	nd->option = 0;
//	nd->opt1 = 0;
//	nd->opt2 = 0;
//	nd->opt3 = 0;
	strncpy(nd->u.warp.name, to_mapname, 16); // 17 - NULL
	xs += 2;
	ys += 2;
	nd->u.warp.x = to_x;
	nd->u.warp.y = to_y;
	nd->u.warp.xs = xs;
	nd->u.warp.ys = ys;

	for(i = 0; i < ys; i++) {
		ii = y - ys / 2 + i;
		for(j = 0; j < xs; j++) {
			jj = x - xs / 2 + j;
			if (map_getcell(m, jj, ii, CELL_CHKNOPASS))
				continue;
			if (map_getcell(m, jj, ii, CELL_CHKNPC)) // don't set 2 npc on same place
				continue;
			map_setcell(m, jj, ii, CELL_SETNPC);
		}
	}

//	printf("warp npc %s %d read done\n", mapname, nd->bl.id);
	npc_warp++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = WARP;
	map_addblock(&nd->bl);
	clif_spawnnpc(nd);
	strdb_insert(npcname_db, nd->name, nd);

	return 0;
}

/*==========================================
 * shop�s���
 *------------------------------------------
 */
static int npc_parse_shop(char *w1, char *w3, char *w4, int lines) {
#define MAX_SHOPITEMS 100
	char *p;
	int x, y, dir, m;
	short pos;
	char mapname[17]; // 16 + NULL
	struct npc_data *nd;

	// �����̌��`�F�b�N
	if (sscanf(w1, "%[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
	    strchr(w4, ',') == NULL) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad shop line" CL_RESET ": %s (file:%s:%d)\n", w3, current_file, lines);
		return 1;
	}
	// check map name (note: mapname already tested before to call parse function)
	m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)

	// check position coordinates
	if (x < 0 || x >= map[m].xs || y < 0 || y >= map[m].ys) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad position coordinates" CL_RESET " (out of range: %s,%d,%d) in shop: %s (file:%s:%d) -> " CL_RED " not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	} else if (map_getcell(m, x, y, CELL_CHKNPC)) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad position coordinates" CL_RESET " (same of another npc/warp: %s,%d,%d) in shop: %s (file:%s:%d) -> " CL_RED " not loaded" CL_RESET "!\n", mapname, x, y, w3, current_file, lines);
		return 1;
	}

	// warning on warp name
	if (strlen(w3) > 24) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_YELLOW "WARNING: Invalid shop name" CL_RESET" (shop: %s) - Name too long (> 24 char.) -> only 24 first characters are used (file:%s:%d).\n", w3, current_file, lines);
	}

	CALLOC(nd, struct npc_data, 1);
	nd->bl.id = npc_get_new_npc_id();
	nd->n = map_addnpc(m, nd);

	if (nd->n == -1) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Too many NPCs (%d) in map %s. Shop %s not loaded" CL_RESET " (file:%s:%d) -> " CL_RED " not loaded" CL_RESET ".\n", MAX_NPC_PER_MAP, mapname, w3, current_file, lines);
		FREE(nd);
		return 1;
	}

	CALLOC(nd->u.shop_item, struct npc_item_list, MAX_SHOPITEMS + 1);

	p = strchr(w4, ',');
	pos = 0;
	while(p && pos < MAX_SHOPITEMS) {
		int nameid, value;
		struct item_data *id;
		p++;
		if (sscanf(p, "%d:%d", &nameid, &value) != 2)
			break;
		if ((id = itemdb_search(nameid)) == NULL) {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "Unknown item in a shop" CL_RESET " (id: %d): %s (file:%s:%d) -> " CL_YELLOW "item not added in the shop" CL_RESET ".\n", nameid, w3, current_file, lines);
		} else {
			nd->u.shop_item[pos].nameid = nameid;
			if (value < 0)
				value = id->value_buy;
			nd->u.shop_item[pos].value = value;
			// check for bad prices that can possibly cause exploits
			if ((double)value * 75. < (double)id->value_sell * 124.) { // ((double)value * 75.) / 100. < ((double)id->value_sell * 124.) / 100.
				printf("[NPC file %s], possible exploit OC/DC (" CL_YELLOW "WARNING" CL_RESET "):\n", current_file);
				printf("Item %s [%d] buying:%d < selling:%d.\n", id->name, id->nameid, value * 75 / 100, id->value_sell * 124 / 100);
				nd->u.shop_item[pos].value = ((double)id->value_sell * 124.) / 75. + 1.;
				printf("->set to: buy %d, sell %d (change shop NPC please).\n", nd->u.shop_item[pos].value, id->value_sell);
			}
			pos++;
		}
		p = strchr(p, ',');
	}
	if (pos == 0) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad shop line" CL_RESET " (no item in the shop): %s (file:%s:%d) -> " CL_RED " not loaded" CL_RESET ".\n", w3, current_file, lines);
		FREE(nd->u.shop_item);
		FREE(nd);
		return 1;
	}
	nd->u.shop_item[pos++].nameid = 0;
	REALLOC(nd->u.shop_item, struct npc_item_list, pos);

//	nd->bl.prev = NULL;
//	nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->dir = dir;
//	nd->flag = 0;
	strncpy(nd->name, w3, 24);
	nd->class = atoi(w4);
	nd->speed = 200;
//	nd->chat_id = 0;
//	nd->option = 0;
//	nd->opt1 = 0;
//	nd->opt2 = 0;
//	nd->opt3 = 0;

	//printf("shop npc %s %d read done\n", mapname, nd->bl.id);
	npc_shop++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = SHOP;
	map_addblock(&nd->bl);
	clif_spawnnpc(nd);
	strdb_insert(npcname_db, nd->name, nd);

	return 0;
}

/*==========================================
 * NPC�̃��x���f�[�^�R���o�[�g
 *------------------------------------------
 */
int npc_convertlabel_db(void *key, void *data, va_list ap) {
	char *lname = (char *)key;
	struct npc_data *nd;
	struct npc_label_list *lst;
	int num;
	char *p = strchr(lname, ':');

	nullpo_retr(0, ap);
	nullpo_retr(0, nd = va_arg(ap, struct npc_data *));

	lst = nd->u.scr.label_list;
	num = nd->u.scr.label_list_num;
	if (!lst) {
		CALLOC(lst, struct npc_label_list, 1);
		num = 0;
	}else {
		REALLOC(lst, struct npc_label_list, num + 1);
		memset(lst + num, 0, sizeof(struct npc_label_list));
	}

	*p = '\0';

	// here we check if the label fit into the buffer
	if (strlen(lname) > 23) {
		printf("npc_parse_script: " CL_RED "label name longer than 23 chars!" CL_RESET " '" CL_RED "%s" CL_RESET "' (in file '" CL_WHITE "%s" CL_RESET "')\n", lname, current_file);
		exit(1);
	}

	strncpy(lst[num].name, lname, 24);
	*p = ':';
	lst[num].pos = (intptr_t)data;
	nd->u.scr.label_list = lst;
	nd->u.scr.label_list_num = num + 1;

	return 0;
}

/*==========================================
 * script�s���
 *------------------------------------------
 */
static int npc_parse_script(char *w1, char *w2, char *w3, char *w4, char *first_line, FILE *fp, int *lines)
{
	int x, y, dir, m, xs = 0, ys = 0, class = 0; // [Valaris] thanks to fov
	char mapname[17]; // 16 + NULL
	char *srcbuf = NULL;
	unsigned char *script;
	int srcsize = 65536;
	int startline = 0;
	char line[1024];
	int i;
	struct npc_data *nd;
	int evflag = 0;
	struct dbt *label_db;
	char *p;
	struct npc_label_list *label_dup = NULL;
	int label_dupnum = 0;
	int src_id = 0;

	if (strcmp(w1, "-") == 0) {
		x = 0; y = 0; m = -1;
	} else {
		// �����̌��`�F�b�N
		if (sscanf(w1, "%[^,],%d,%d,%d", mapname, &x, &y, &dir) != 4 ||
		    (strcmp(w2, "script") == 0 && strchr(w4, ',') == NULL)) {
			if (current_file)
				printf("Bad script line: '" CL_RED "%s" CL_RESET "' (file: '" CL_WHITE "%s" CL_RESET "').\n", w3, current_file);
			else
				printf(CL_RED "Bad script line" CL_RESET ": %s.\n", w3);
			return 1;
		}
		m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)
	}

	if (strcmp(w2, "script") == 0) {
		// �X�N���v�g�̉��
		CALLOC(srcbuf, char, srcsize);
		if (strchr(first_line, '{')) {
			strcpy(srcbuf, strchr(first_line, '{'));
			startline = *lines;
		} else
			srcbuf[0] = 0;
		while(1) {
			for(i = strlen(srcbuf) - 1; i >= 0 && isspace(srcbuf[i]); i--)
				;
			if (i >= 0 && srcbuf[i] == '}')
				break;
			if (fgets(line, sizeof(line), fp) == NULL) // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
				break;
			(*lines)++;
			if (strlen(srcbuf) + strlen(line) + 1 >= srcsize) {
				srcsize += 65536;
				REALLOC(srcbuf, char, srcsize);
				memset(srcbuf + (srcsize - 65536), 0, 65536);
			}
			if (srcbuf[0] != '{') {
				if (strchr(line, '{')) {
					strcpy(srcbuf, strchr(line, '{'));
					startline = *lines;
				}
			} else
				strcat(srcbuf, line);
		}
		script = parse_script((unsigned char *)srcbuf, startline);
		if (script == NULL) {
			// script parse error?
			FREE(srcbuf);
			return 1;
		}

	} else {
		// duplicate����
		char srcname[128];
		struct npc_data *nd2;
		if (sscanf(w2, "duplicate(%[^)])", srcname) != 1) {
			printf("bad duplicate name (in %s)! : %s", current_file, w2);
			return 0;
		}
		if ((nd2 = npc_name2id(srcname)) == NULL) {
			printf("bad duplicate name (in %s)! (not exist) : %s\n", current_file, srcname);
			return 0;
		}
		script = nd2->u.scr.script;
		label_dup = nd2->u.scr.label_list;
		label_dupnum = nd2->u.scr.label_list_num;
		src_id = nd2->bl.id;

	}// end of �X�N���v�g���

	CALLOC(nd, struct npc_data, 1);

	if (m == -1) {
		// �X�N���v�g�R�s�[�p�̃_�~�[NPC

	} else if (sscanf(w4, "%d,%d,%d", &class, &xs, &ys) == 3) {
		// �ڐG�^NPC
		int j;

		if (xs >= 0) xs = xs * 2 + 1;
		if (ys >= 0) ys = ys * 2 + 1;

		if (class >= 0) {

			for(i = 0; i < ys; i++) {
				for(j = 0; j < xs; j++) {
					if (map_getcell(m, x - xs / 2 + j, y - ys / 2 + i, CELL_CHKNOPASS))
						continue;
					map_setcell(m, x - xs / 2 + j, y - ys / 2 + i, CELL_SETNPC);
				}
			}
		}

		nd->u.scr.xs = xs;
		nd->u.scr.ys = ys;
	} else { // �N���b�N�^NPC
		class = atoi(w4);
		nd->u.scr.xs = 0;
		nd->u.scr.ys = 0;
	}

	if (class < 0 && m >= 0) { // �C�x���g�^NPC
		evflag = 1;
	}

	while((p = strchr(w3, ':'))) {
		if (p[1] == ':') break;
	}
	if (p) {
		*p = 0;
		strncpy(nd->name, w3, 24);
		strncpy(nd->exname, p + 2, 24);
	} else {
		strncpy(nd->name, w3, 24);
		strncpy(nd->exname, w3, 24);
	}

	nd->bl.prev = nd->bl.next = NULL;
	nd->bl.m = m;
	nd->bl.x = x;
	nd->bl.y = y;
	nd->bl.id = npc_get_new_npc_id();
	nd->dir = dir;
	nd->flag = 0;
	nd->class = class;
	nd->speed = 200;
	nd->u.scr.script = script;
	nd->u.scr.src_id = src_id;
	nd->chat_id = 0;
	nd->option = 0;
	nd->opt1 = 0;
	nd->opt2 = 0;
	nd->opt3 = 0;
	nd->walktimer = -1;
	for(i = 0; i < MAX_EVENTTIMER; i++)
		nd->eventtimer[i] = -1;

	//printf("script npc %s %d %d read done\n", mapname, nd->bl.id, nd->class);
	npc_script++;
	nd->bl.type = BL_NPC;
	nd->bl.subtype = SCRIPT;
	if (m >= 0) {
		nd->n = map_addnpc(m,nd);
		map_addblock(&nd->bl);

		if (evflag) { // �C�x���g�^
			struct event_data *ev;
			CALLOC(ev, struct event_data, 1);
			ev->nd=nd;
			ev->pos=0;
			strdb_insert(ev_db, nd->exname, ev);
		} else
			clif_spawnnpc(nd);
	}
	strdb_insert(npcname_db, nd->exname, nd);

	//-----------------------------------------
	// ���x���f�[�^�̏���
	if (srcbuf) {
		// script�{�̂�����ꍇ�̏���

		// ���x���f�[�^�̃R���o�[�g
		label_db = script_get_label_db();
		strdb_foreach(label_db, npc_convertlabel_db, nd);

		// �����g��Ȃ��̂Ńo�b�t�@���
		FREE(srcbuf);

	} else {
		// duplicate

//		CALLOC(nd->u.scr.label_list, struct npc_label_list, label_dupnum);
//		memcpy(nd->u.scr.label_list, label_dup, sizeof(struct npc_label_list) * label_dupnum);

		nd->u.scr.label_list = label_dup; // ���x���f�[�^���L
		nd->u.scr.label_list_num = label_dupnum;
	}

	//-----------------------------------------
	// �C�x���g�p���x���f�[�^�̃G�N�X�|�[�g
	for(i = 0; i < nd->u.scr.label_list_num; i++) {
		char *lname = nd->u.scr.label_list[i].name;
		int pos = nd->u.scr.label_list[i].pos;

		if ((lname[0] == 'O' || lname[0] == 'o') && (lname[1] == 'N' || lname[1] == 'n')) {
			struct event_data *ev;
			char *buf;
			// �G�N�X�|�[�g�����
			CALLOC(ev, struct event_data, 1);
			CALLOC(buf, char, 50);
			if (strlen(lname) > 24) {
				printf("npc_parse_script: label name error: too long name (%s) !\n", current_file);
				exit(1);
			} else {
				ev->nd = nd;
				ev->pos = pos;
				sprintf(buf, "%s::%s", nd->exname, lname);
				strdb_insert(ev_db, buf, ev);
			}
		}
	}

	//-----------------------------------------
	// ���x���f�[�^����^�C�}�[�C�x���g��荞��
	for(i = 0; i < nd->u.scr.label_list_num; i++) {
		int t = 0, n = 0;
		char *lname = nd->u.scr.label_list[i].name;
		int pos = nd->u.scr.label_list[i].pos;
		if (sscanf(lname, "OnTimer%d%n", &t, &n) == 1 && lname[n] == '\0') {
			// �^�C�}�[�C�x���g
			struct npc_timerevent_list *te = nd->u.scr.timer_event;
			int j, k = nd->u.scr.timeramount;
			if(te == NULL) {
				CALLOC(te, struct npc_timerevent_list, 1);
			} else {
				REALLOC(te, struct npc_timerevent_list, k + 1);
				memset(te + k, 0, sizeof(struct npc_timerevent_list));
			}
			for(j = 0; j < k; j++) {
				if (te[j].timer > t) {
					memmove(te + j + 1, te + j, sizeof(struct npc_timerevent_list) * (k - j));
					break;
				}
			}
			te[j].timer = t;
			te[j].pos = pos;
			nd->u.scr.timer_event = te;
			nd->u.scr.timeramount = k + 1;
		}
	}
	nd->u.scr.nexttimer = -1;
	nd->u.scr.timerid = -1;

	return 0;
}

/*==========================================
 * function�s���
 *------------------------------------------
 */
static int npc_parse_function(char *w1, char *w3, char *w4, char *first_line, FILE *fp, int *lines)
{
	char *srcbuf = NULL;
	unsigned char *script;
	int srcsize = 65536;
	int startline = 0;
	char line[1024];
	int i;
//	struct dbt *label_db;
	char *p;

	// �X�N���v�g�̉��
	CALLOC(srcbuf, char, srcsize);
	if (strchr(first_line, '{')) {
		strcpy(srcbuf, strchr(first_line, '{'));
		startline = *lines;
	} else
		srcbuf[0] = 0;
	while(1) {
		for(i = strlen(srcbuf) - 1; i >= 0 && isspace(srcbuf[i]); i--)
			;
		if (i >= 0 && srcbuf[i] == '}')
			break;
		if (fgets(line, sizeof(line), fp) == NULL) // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
			break;
		(*lines)++;
		if (strlen(srcbuf) + strlen(line) + 1 >= srcsize) {
			srcsize += 65536;
			REALLOC(srcbuf, char, srcsize);
			memset(srcbuf + (srcsize - 65536), 0, 65536);
		}
		if (srcbuf[0] != '{') {
			if (strchr(line, '{')) {
				strcpy(srcbuf, strchr(line, '{'));
				startline = *lines;
			}
		} else
			strcat(srcbuf, line);
	}
	script = parse_script((unsigned char *)srcbuf, startline);
	if (script == NULL) {
		// script parse error?
		FREE(srcbuf);
		return 1;
	}

	CALLOC(p, char, 50);

	strncpy(p, w3, 50);
	strdb_insert(script_get_userfunc_db(), p, script);

//	label_db = script_get_label_db();

	// �����g��Ȃ��̂Ńo�b�t�@���
	FREE(srcbuf);

//	printf("function %s => %p\n", p, script);

	return 0;
}


/*==========================================
 * mob�s���
 *------------------------------------------
 */
int npc_parse_mob(char *w1, char *w3, char *w4) {
	int m, x, y, xs, ys, class, num, delay1, delay2, level, size;
	int i;
	char mapname[17]; // 16 + NULL
	char mobname[25]; // 24 + NULL
	char eventname[25] = ""; // 24 + NULL
	struct mob_data *md;

	xs = ys = 0;
	delay1 = delay2 = 0;
	// �����̌��`�F�b�N
	if (sscanf(w1, "%[^,],%d,%d,%d,%d", mapname, &x, &y, &xs, &ys) < 3 ||
	    sscanf(w4, "%d,%d,%d,%d,%s", &class, &num, &delay1, &delay2, eventname) < 2) {
		printf("bad monster line: %s.\n", w3);
		return 1;
	}

	m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)

//	printf("%d->", num);
	if (num > 1 && battle_config.mob_count_rate != 100) {
		if ((num = num * battle_config.mob_count_rate / 100) < 1)
			num = 1;
	} else if (num < 0) {
		printf("bad monster line: %s (negativ number of mob: %d).\n", w3, num);
		return 1;
	}
//	printf("%d (%d%%)\n", num, battle_config.mob_count_rate);

	size = 0;
	if (class >= (MAX_MOB_DB * 2)) {
		size = 2;
		class -= (MAX_MOB_DB * 2); // big
	} else if (class >= MAX_MOB_DB) {
		size = 1;
		class -= MAX_MOB_DB; // small
	}

	for(i = 0; i < num; i++) {
		CALLOC(md, struct mob_data, 1);

		md->bl.prev = NULL;
		md->bl.next = NULL;
		md->bl.m = m;
		md->bl.x = x;
		md->bl.y = y;

		md->size = size;

		level = 0;
		if (sscanf(w3, "%[^,],%d", mobname, &level) > 1)
			md->level = level;
		if(strcmp(mobname, "--en--") == 0)
			strncpy(md->name, mob_db[class].name, 24);
		else if(strcmp(mobname, "--ja--") == 0)
			strncpy(md->name, mob_db[class].jname, 24);
		else
			strncpy(md->name, w3, 24);

		md->n = i;
		md->base_class = md->class = class;
		md->bl.id = npc_get_new_npc_id();
		md->m = m;
		md->x0 = x;
		md->y0 = y;
		md->xs = xs;
		md->ys = ys;
		md->spawndelay1 = delay1;
		md->spawndelay2 = delay2;

		memset(&md->state, 0, sizeof(md->state));
		md->timer = -1;
		md->target_id = 0;
		md->attacked_id = 0;
		md->speed = mob_db[class].speed;

		if (mob_db[class].mode & 0x02) {
			CALLOC(md->lootitem, struct item, LOOTITEM_SIZE);
		} else
			md->lootitem = NULL;

		memset(md->npc_event, 0, sizeof(md->npc_event));
		if (strlen(eventname) >= 4) {
			strncpy(md->npc_event, eventname, 24);
		}

		md->bl.type = BL_MOB;
		map_addiddb(&md->bl);
		mob_spawn(md->bl.id);

		npc_mob++;
	}
	//printf("warp npc %s %d read done\n", mapname, nd->bl.id);

	return 0;
}

/*==========================================
 * �}�b�v�t���O�s�̉��
 *------------------------------------------
 */
int npc_parse_mapflag(char *w1, char *w3, char *w4, int lines) {
	int m;
	char mapname[17], savemap[17];
	int savex, savey;
	char drop_arg1[16], drop_arg2[16];
	int drop_id = 0;
	char drop_type = 0;
	int drop_per = 0;

	// �����̌��`�F�b�N
	memset(mapname, 0, sizeof(mapname));
	if (sscanf(w1, "%[^,]", mapname) != 1) {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Bad map flag line" CL_RESET ": %s (file:%s:%d)!\n", w3, current_file, lines);
		return 1;
	}

	// check map name (a GM command can call this function directly)
	if(strstr(mapname, ".gat") == NULL || mapname[0] == '\0' || strlen(mapname) > 16)
	{
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_RED "Invalid map name" CL_RESET " in map flag line: %s (file:%s:%d)!\n", w3, current_file, lines);
		return 1;
	}

	m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)
	// don't accept flag if not in this map-server
	if (m < 0)
		return 1;

//�}�b�v�t���O
	if (strcasecmp(w3, "nomemo") == 0) { // 0
		map[m].flag.nomemo = 1;
	} else if (strcasecmp(w3, "noteleport") == 0) { // 1
		map[m].flag.noteleport = 1;
	} else if (strcasecmp(w3, "nosave") == 0) { // 2
		map[m].flag.nosave = 1;
		if (strcmp(w4, "SavePoint") == 0) {
			memset(map[m].save.map, 0, sizeof(map[m].save.map));
			strncpy(map[m].save.map, "SavePoint", 16); // 17 - NULL
			map[m].save.x = -1;
			map[m].save.y = -1;
		} else if (sscanf(w4, "%[^,],%d,%d", savemap, &savex, &savey) == 3) {
			memset(map[m].save.map, 0, sizeof(map[m].save.map));
			strncpy(map[m].save.map, savemap, 16); // 17 - NULL
			map[m].save.x = savex;
			map[m].save.y = savey;
		} else {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_RED "Invalid save point" CL_RESET " in map flag line: %s (file:%s:%d)!\n", w3, current_file, lines);
			return 1;
		}
	} else if (strcasecmp(w3, "nobranch") == 0) { // 3
		map[m].flag.nobranch = 1; // forbid usage of Dead Branch (604), Bloody Branch (12103) and Poring Box (12109)
	} else if (strcasecmp(w3, "nopenalty") == 0) { // 4
		map[m].flag.nopenalty = 1;
	} else if (strcasecmp(w3, "nozenypenalty") == 0) { // 5
		map[m].flag.nozenypenalty = 1;
	} else if (strcasecmp(w3, "pvp") == 0) { // 6
		map[m].flag.pvp = 1;
	} else if (strcasecmp(w3, "pvp_noparty") == 0) { // 7
		map[m].flag.pvp_noparty = 1;
	} else if (strcasecmp(w3, "pvp_noguild") == 0) { // 8
		map[m].flag.pvp_noguild = 1;
	} else if (strcasecmp(w3, "gvg") == 0) { // 9
		map[m].flag.gvg = 1;
	} else if (strcasecmp(w3, "gvg_noparty") == 0) { // 10
		map[m].flag.gvg_noparty = 1;
	} else if (strcasecmp(w3, "notrade") == 0) { // 11
		map[m].flag.notrade = 1;
	} else if (strcasecmp(w3, "noskill") == 0) { // 12
		map[m].flag.noskill = 1;
	} else if (strcasecmp(w3, "nowarp") == 0) { // 13
		map[m].flag.nowarp = 1;
	} else if (strcasecmp(w3, "nopvp") == 0) { // nopvp for pk mode [Valaris] // 14
		map[m].flag.pvp = 0;
	} else if (strcasecmp(w3, "noicewall") == 0) { // noicewall [Valaris] // 15
		map[m].flag.noicewall = 1;
	} else if (strcasecmp(w3, "snow") == 0) { // snow [Valaris] // 16
		map[m].flag.snow = 1;
	} else if (strcasecmp(w3, "fog") == 0) { // fog [Valaris] // 17
		map[m].flag.fog = 1;
	} else if (strcasecmp(w3, "sakura") == 0) { // sakura [Valaris] // 18
		map[m].flag.sakura = 1;
	} else if (strcasecmp(w3, "leaves") == 0) { // leaves [Valaris] // 19
		map[m].flag.leaves = 1;
	} else if (strcasecmp(w3, "rain") == 0) { // rain [Valaris] // 20
		map[m].flag.rain = 1;
	} else if (strcasecmp(w3, "indoors") == 0) { // celest // 21
		map[m].flag.indoors = 1;
	} else if (strcasecmp(w3, "nogo") == 0) { // celest // 22
		map[m].flag.nogo = 1;
	} else if (strcasecmp(w3, "nospell") == 0) { // 23
		map[m].flag.nospell = 1;
	} else if (strcasecmp(w3, "nowarpto") == 0) { // 24
		map[m].flag.nowarpto = 1;
	} else if (strcasecmp(w3, "noreturn") == 0) { // 25
		map[m].flag.noreturn = 1;
	} else if (strcasecmp(w3, "monster_noteleport") == 0) { // 26
		map[m].flag.monster_noteleport = 1;
	} else if (strcasecmp(w3, "pvp_nocalcrank") == 0) { // 27
		map[m].flag.pvp_nocalcrank = 1;
	} else if (strcasecmp(w3, "noexp") == 0) { // 28
		map[m].flag.nobaseexp = 1;
		map[m].flag.nojobexp = 1;
	} else if (strcasecmp(w3, "nobaseexp") == 0) { // 29
		map[m].flag.nobaseexp = 1;
	} else if (strcasecmp(w3, "nojobexp") == 0) { // 30
		map[m].flag.nojobexp = 1;
	} else if (strcasecmp(w3, "nodrop") == 0 || // 31
	           strcasecmp(w3, "noloot") == 0) { // for compatibility with eAthena
		map[m].flag.nomobdrop = 1;
		map[m].flag.nomvpdrop = 1;
		if (strcasecmp(w3, "noloot") == 0)
			printf(CL_YELLOW "WARNING: Unknown map flag" CL_RESET ": noloot (file:%s:%d)! Nezumi uses 'nodrop' mapflag.\n", current_file, lines);
	} else if (strcasecmp(w3, "nomobdrop") == 0 || // 32
	           strcasecmp(w3, "nomobloot") == 0) { // for compatibility with eAthena
		map[m].flag.nomobdrop = 1;
		if (strcasecmp(w3, "nomobloot") == 0) {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "WARNING: Unknown map flag" CL_RESET ": nomobloot (file:%s:%d)! Nezumi uses 'nomobdrop' mapflag.\n", current_file, lines);
		}
	} else if (strcasecmp(w3, "nomvpdrop") == 0 || // 33
	           strcasecmp(w3, "nomvploot") == 0) { // for compatibility with eAthena
		map[m].flag.nomvpdrop = 1;
		if (strcasecmp(w3, "nomvploot") == 0) {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "WARNING: Unknown map flag" CL_RESET ": nomvploot (file:%s:%d)! Nezumi uses 'nomvpdrop' mapflag.\n", current_file, lines);
		}
	} else if (strcasecmp(w3, "pvp_nightmaredrop") == 0) { // 34
		if (sscanf(w4, "%[^,],%[^,],%d", drop_arg1, drop_arg2, &drop_per) == 3) {
			if (strcmp(drop_arg1, "random") == 0)
				drop_id = -1;
			else if (itemdb_exists((drop_id = atoi(drop_arg1))) == NULL)
				drop_id = 0;
			if (drop_id) {
				if (strcmp(drop_arg2, "inventory") == 0)
					drop_type = 1;
				else if (strcmp(drop_arg2, "equip") == 0)
					drop_type = 2;
				else if (strcmp(drop_arg2, "all") == 0)
					drop_type = 3;
				else {
					if (current_file != NULL) // if not a GM command, but a script
						printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": pvp_nightmaredrop (file:%s:%d)! Argument 2 must be: 'inventory' or 'equip' or 'all'.\n", current_file, lines);
					return 1;
				}
				// add the drop list
				if (map[m].drop_list_num == 0) { // MAX_DROP_PER_MAP -> now, dynamic
					MALLOC(map[m].drop_list, struct drop_list, 1);
				} else {
					REALLOC(map[m].drop_list, struct drop_list, map[m].drop_list_num + 1);
				}
				map[m].drop_list[map[m].drop_list_num].drop_id = drop_id;
				map[m].drop_list[map[m].drop_list_num].drop_type = drop_type;
				map[m].drop_list[map[m].drop_list_num].drop_per = drop_per;
				map[m].drop_list_num++;
				map[m].flag.pvp_nightmaredrop = 1;
			} else {
				if (current_file != NULL) // if not a GM command, but a script
					printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": pvp_nightmaredrop (file:%s:%d)! Argument 1 must be: 'random' or a valid item_id.\n", current_file, lines);
				return 1;
			}
		} else {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": pvp_nightmaredrop (file:%s:%d)! Number of parameters is incorrect.\n", current_file, lines);
			return 1;
		}
	} else if (strcasecmp(w3, "nogmcmd") == 0) { // 35
		// when you use this mapflag (nogmcmd, you forbid GM commands upper than the mentionned level, included @ban, @kick, etc...)
		int gmlvl;
		if (sscanf(w4, "%d", &gmlvl) == 1) {
			if (gmlvl >=0 && gmlvl <= 100)
				map[m].flag.nogmcmd = gmlvl;
			else {
				if (current_file != NULL) // if not a GM command, but a script
					printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": nogmcmd (file:%s:%d)! Value must be between 0 to 100 (not: %d).\n", current_file, lines, gmlvl);
				return 1;
			}
		} else {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": nogmcmd (file:%s:%d)! Please, specify a minimum level.\n", current_file, lines);
			return 1;
		}
	} else if (strcasecmp(w3, "mingmlvl") == 0) { // 36
		int gmlvl;
		if (sscanf(w4, "%d", &gmlvl) == 1) {
			if (gmlvl >=0 && gmlvl <= 100)
				map[m].flag.mingmlvl = gmlvl;
			else {
				if (current_file != NULL) // if not a GM command, but a script
					printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": mingmlvl (file:%s:%d)! Value must be between 0 to 100 (not: %d).\n", current_file, lines, gmlvl);
				return 1;
			}
		} else {
			if (current_file != NULL) // if not a GM command, but a script
				printf(CL_YELLOW "WARNING: Invalid map flag" CL_RESET ": mingmlvl (file:%s:%d)! Please, specify a minimum level.\n", current_file, lines);
			return 1;
		}
	} else if (strcasecmp(w3, "guild_dungeon") == 0) {
		map[m].flag.guild_dungeon = 1;
	} else if (strcasecmp(w3, "water") == 0) {
		// not used in code
	} else {
		if (current_file != NULL) // if not a GM command, but a script
			printf(CL_YELLOW "WARNING: Unknown map flag" CL_RESET ": %s (file:%s:%d)!\n", w3, current_file, lines);
		return 1;
	}

	return 0;
}

static int npc_read_indoors(void) {
	char *buf, *p;
	int s, m;

	buf = (char *)grfio_reads("data\\indoorrswtable.txt", &s);

	if (buf == NULL)
		return -1;

	buf[s] = 0;
	p = buf;
	while(p - buf < s) {
		char buf2[64];

		memset(buf2, 0, sizeof(buf2));
		if (sscanf(p, "%[^#]#", buf2) == 1) {
			char mapname[17]; // 16 + NULL
			if (strlen(buf2) > 0 && strlen(buf2) < sizeof(mapname)) {
				memset(mapname, 0, sizeof(mapname));
				strncpy(mapname, buf2, strlen(buf2) - 4);
				strcat(mapname, ".gat");
				if ((m = map_mapname2mapid(mapname)) >= 0) // map id on this server (m == -1 if not in actual map-server)
					map[m].flag.indoors = 1;
			}
		}

		p = strchr(p, 10);
		if (!p)
			break;
		p++;
	}
	FREE(buf);

	printf("File '" CL_WHITE "data\\indoorrswtable.txt" CL_RESET "' readed.\n");

	return 0;
}

static int ev_db_final(void *key, void *data, va_list ap) {
	FREE(data);
	if (strstr(key, "::") != NULL) {
		FREE(key);
	}

	return 0;
}

static int npcname_db_final(void *key, void *data, va_list ap) {
	return 0;
}

/*==========================================
 * �I��
 *------------------------------------------
 */
int do_final_npc(void) {
	int i, count;
	int count_warp, count_shop, count_script, count_mob;
	struct block_list *bl;
	struct npc_data *nd;
	struct mob_data *md;
	struct chat_data *cd;
	struct pet_data *pd;

	if (ev_db)
		strdb_final(ev_db, ev_db_final);
	if (npcname_db)
		strdb_final(npcname_db, npcname_db_final);

	count = 0;
	count_warp = 0;
	count_shop = 0;
	count_script = 0;
	count_mob = 0;
	for(i = START_NPC_NUM; i < npc_id; i++) {
		if ((bl = map_id2bl(i))) {
			if (bl->type == BL_NPC && (nd = (struct npc_data *)bl)) {
				if (nd->chat_id && (cd = (struct chat_data*)map_id2bl(nd->chat_id))) {
					FREE(cd);
				}
				if (nd->bl.subtype == SCRIPT) {
					FREE(nd->u.scr.timer_event);
					if (nd->u.scr.src_id == 0) {
						FREE(nd->u.scr.script);
						FREE(nd->u.scr.label_list);
					}
					count_script++;
				} else if (nd->bl.subtype == SHOP) {
					FREE(nd->u.shop_item);
					count_shop++;
				} else if (nd->bl.subtype == WARP) {
					// nothing to do
					count_warp++;
				}
				FREE(nd);
			} else if (bl->type == BL_MOB && (md = (struct mob_data *)bl)) {
				FREE(md->lootitem);
				FREE(md);
				count_mob++;
			} else if (bl->type == BL_PET && (pd = (struct pet_data *)bl)) {
				FREE(pd->lootitem);
				FREE(pd);
			}
			count++;
		}
	}

	printf("Successfully removed and freed from memory: '" CL_WHITE "%d" CL_RESET "' NPCs of which are:\n", count);
	printf("\t-" CL_WHITE "%d" CL_RESET " warps\n", count_warp);
	printf("\t-" CL_WHITE "%d" CL_RESET " shops\n", count_shop);
	printf("\t-" CL_WHITE "%d" CL_RESET " scripts\n", count_script);
	printf("\t-" CL_WHITE "%d" CL_RESET " mobs\n", count_mob);

	return 0;
}

void ev_release(struct dbn *db, int which) {
	if (which & 0x1) {
		FREE(db->key);
	}
	if (which & 0x2) {
		FREE(db->data);
	}
}

/*==========================================
 * npc������
 *------------------------------------------
 */
int do_init_npc(void) {
	struct npc_src_list *nsl;
	FILE *fp;
	char line[1024];
	int m, lines;

	// indoorrswtable.txt and etcinfo.txt [Celest]
	if (battle_config.indoors_override_grffile)
		npc_read_indoors();
//	npc_read_weather();

	printf("Loading NPCs...\r");

	ev_db = strdb_init(24);
	npcname_db = strdb_init(24);

	ev_db->release = ev_release;

	memset(&ev_tm_b, -1, sizeof(ev_tm_b));

	for(nsl = npc_src_first; nsl; nsl = nsl->next) {
		current_file = nsl->name; // to display in error
		if ((fp = fopen(nsl->name, "r")) == NULL) {
			printf("File not found: '%s'.\n", nsl->name);
			exit(1);
		}
		if ((npc_id - START_NPC_NUM) % 20 == 1) {
			printf("Loading NPCs [%d]: %-57s\r", npc_id - START_NPC_NUM, nsl->name);
			fflush(stdout);
		}
		lines = 0;
		// sizeof(line) - 1 because after, we can add cariage return!
		while(fgets(line, sizeof(line) - 1, fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
			char w1[1024], w2[1024], w3[1024], w4[1024], mapname[1024];
			int i, j, w4pos, count;
			lines++;

			if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
				continue;
			if ((line[(i = strlen(line) - 1)] != '\n' && line[i] != '\r')) { // fixed last line of a script file
				line[i+1] = '\n';
				line[i+2] = '\0';
			}
			// �s�v�ȃX�y�[�X��^�u�̘A���͋l�߂�
			for(i = j = 0; line[i]; i++) {
				if (line[i] == ' ') {
					if (!((line[i + 1] && (isspace(line[i + 1]) || line[i + 1] == ',')) ||
					                     (j && line[j-1] == ',')))
						line[j++] = ' ';
				} else if (line[i] == '\t') {
					if (!(j && line[j-1] == '\t'))
						line[j++] = '\t';
				} else
					line[j++] = line[i];
			}
			// �ŏ��̓^�u��؂�Ń`�F�b�N���Ă݂āA�_���Ȃ�X�y�[�X��؂�Ŋm�F
			memset(w1, 0, sizeof(w1));
			memset(w2, 0, sizeof(w2));
			memset(w3, 0, sizeof(w3));
			memset(w4, 0, sizeof(w4));
			if ((count = sscanf(line, "%[^\t]\t%[^\t]\t%[^\t\r\n]\t%n%[^\t\r\n]", w1, w2, w3, &w4pos, w4)) < 3 &&
			    (count = sscanf(line, "%s%s%s%n%s", w1, w2, w3, &w4pos, w4)) < 3) {
				continue;
			}
			// �}�b�v�̑��݊m�F
			if (strcasecmp(w2, "script") != 0 || (strcmp(w1,"-") != 0 && strcasecmp(w1, "function") != 0)) {
				sscanf(w1, "%[^,]", mapname);
				// check map name
				if(strstr(mapname, ".gat") == NULL || mapname[0] == '\0' || strlen(mapname) > 16)
				{
					printf(CL_RED "Bad map name" CL_RESET " at start of a script (file:%s, line %d)!\n", current_file, lines);
					continue;
				}
				m = map_mapname2mapid(mapname); // map id on this server (m == -1 if not in actual map-server)
				if (m < 0) {
					// "mapname" is not assigned to this server (it's not an error -> no display)
					char *p;
					if (strcasecmp(w2, "script") == 0) { // check to not display error on each line if map-name doesn't in this server
						int counter = 0; // to count number of { and }
						if ((p = strchr(line, '{'))) { // first '{' can be on same line of "script" or on a following line
							counter++;
							if (strchr(p, '}')) { // if script finished on same line...
								counter--;
								if (counter == 0)
									continue;
							}
						}
						while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
							lines++;
							if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
								continue;
							if (strchr(line, '{'))
								counter++;
							if (strchr(line, '}')) {
								counter--;
								if (counter == 0)
									break;
							}
						}
						// if break because end of file
						if (counter != 0)
							printf(CL_RED "Bad script ending" CL_RESET ": incorrect number of '{' and '}' (%d more '{') (file:%s)!\n", counter, current_file);
					}
					continue;
				}
			}
			if (strcasecmp(w2, "warp") == 0 && count > 3) {
				npc_parse_warp(w1, w3, w4, lines);
			} else if (strcasecmp(w2, "shop") == 0 && count > 3) {
				npc_parse_shop(w1, w3, w4, lines);
			} else if (strcasecmp(w2, "script") == 0 && count > 3) {
				if (strcasecmp(w1, "function") == 0) {
					npc_parse_function(w1, w3, w4, line + w4pos, fp, &lines);
				} else {
					if (npc_parse_script(w1, w2, w3, w4, line + w4pos, fp, &lines)) // on error
						printf("\n");
				}
			} else if ((i = 0, sscanf(w2, "duplicate%n", &i), (i > 0 && w2[i] == '(')) && count > 3) {
				npc_parse_script(w1, w2, w3, w4, line + w4pos, fp, &lines);
			} else if (strcasecmp(w2, "monster") == 0 && count > 3) {
				npc_parse_mob(w1, w3, w4);
			} else if (strcasecmp(w2, "mapflag") == 0 && count >= 3) {
				npc_parse_mapflag(w1, w3, w4, lines);
			}
		}
		fclose(fp);
		current_file = NULL;
	}

#ifdef __DEBUG
	// check warning on destination warp position
	// this check is necessary because a warp can exist before than an other npc is set on same place
  {
	int i;
	for(i = START_NPC_NUM; i < npc_id; i++) {
		struct npc_data *nd = (struct npc_data *)map_id2bl(i);
		if (nd && nd->bl.type == BL_NPC && nd->bl.subtype == WARP) {
			// check destination warp coordinates (if possible)
			if ((m = map_mapname2mapid(nd->u.warp.name)) >= 0) { // map id on this server (m == -1 if not in actual map-server)
				if (map_getcell(m, nd->u.warp.x, nd->u.warp.y, CELL_CHKNPC)) {
					int j, xs, ys;
					// search npc name
					for(j = 0; j < map[m].npc_num; j++) {
						switch(map[m].npc[j]->bl.subtype) {
						case WARP:
							xs = map[m].npc[j]->u.warp.xs;
							ys = map[m].npc[j]->u.warp.ys;
							break;
						case SCRIPT:
							xs = map[m].npc[j]->u.scr.xs;
							ys = map[m].npc[j]->u.scr.ys;
							break;
						default:
							continue;
						}
						if (nd->u.warp.x >= map[m].npc[j]->bl.x - xs / 2 && nd->u.warp.x <= map[m].npc[j]->bl.x - xs / 2 + xs &&
						    nd->u.warp.y >= map[m].npc[j]->bl.y - ys / 2 && nd->u.warp.y <= map[m].npc[j]->bl.y - ys / 2 + ys)
							break;
					}
					if (j != map[m].npc_num)
						printf(CL_YELLOW "WARNING: Bad destination coordinates" CL_RESET " (same of another npc/warp (%s): %s,%d,%d) in warp: %s.\n", map[m].npc[j]->name, nd->u.warp.name, nd->u.warp.x, nd->u.warp.y, nd->name);
					else
						printf(CL_YELLOW "WARNING: Bad destination coordinates" CL_RESET " (same of another npc/warp: %s,%d,%d) in warp: %s.\n", nd->u.warp.name, nd->u.warp.x, nd->u.warp.y, nd->name);
				}
			}
		}
	}
  }
#endif // __DEBUG

	printf(CL_WHITE "%d" CL_RESET " NPCs Loaded: %50s\n", npc_id - START_NPC_NUM, "");
	printf("\t-" CL_WHITE "%d" CL_RESET " warps\n", npc_warp);
	printf("\t-" CL_WHITE "%d" CL_RESET " shops\n", npc_shop);
	printf("\t-" CL_WHITE "%d" CL_RESET " scripts\n", npc_script);
	printf("\t-" CL_WHITE "%d" CL_RESET " mobs\n", npc_mob);

	add_timer_func_list(npc_walktimer, "npc_walktimer"); // [Valaris]
	add_timer_func_list(npc_event_timer, "npc_event_timer");
	add_timer_func_list(npc_event_do_clock, "npc_event_do_clock");
	add_timer_func_list(npc_timerevent, "npc_timerevent");

	return 0;
}
