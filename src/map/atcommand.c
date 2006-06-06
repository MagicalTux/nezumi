// $Id$
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/core.h"
#include "../common/version.h"
#include "nullpo.h"

#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"
#include "grfio.h"
#include "status.h"

#ifdef USE_SQL
#include "mail.h"
#endif /* USE_SQL */

#define STATE_BLIND 0x10

short log_gm_level = 40;

static char command_symbol = '@'; /* first char of the commands */
static char char_command_symbol = '#'; /* first char of the remote commands */
static char main_channel_symbol = '.'; /* first char of the main channel */

time_t last_spawn; /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */

#define MSG_NUMBER 1000
static char *msg_table[MSG_NUMBER]; /* Server messages (0-499 reserved for GM commands, 500-999 reserved for others) */
static int msg_config_read_done = 0; /* for multiple configuration read */

static struct synonym_table { /* table for GM command synonyms */
	char* synonym;
	char* command;
} *synonym_table = NULL;
static int synonym_count = 0; /* number of synonyms */

#define ATCOMMAND_FUNC(x) int atcommand_ ## x (const int fd, struct map_session_data* sd, const char* original_command, const char* command, const char* message)
ATCOMMAND_FUNC(broadcast);
ATCOMMAND_FUNC(localbroadcast);
ATCOMMAND_FUNC(localbroadcast2);
ATCOMMAND_FUNC(rurap);
ATCOMMAND_FUNC(rura);
ATCOMMAND_FUNC(where);
ATCOMMAND_FUNC(jumpto);
ATCOMMAND_FUNC(jump);
ATCOMMAND_FUNC(users);
ATCOMMAND_FUNC(who);
ATCOMMAND_FUNC(who2);
ATCOMMAND_FUNC(who3);
ATCOMMAND_FUNC(whomap);
ATCOMMAND_FUNC(whomap2);
ATCOMMAND_FUNC(whomap3);
ATCOMMAND_FUNC(whogm);
ATCOMMAND_FUNC(whozeny);
ATCOMMAND_FUNC(whozenymap);
ATCOMMAND_FUNC(whohas);
ATCOMMAND_FUNC(whohasmap);
ATCOMMAND_FUNC(happyhappyjoyjoy);
ATCOMMAND_FUNC(happyhappyjoyjoymap);
ATCOMMAND_FUNC(save);
ATCOMMAND_FUNC(load);
ATCOMMAND_FUNC(charload);
ATCOMMAND_FUNC(charloadmap);
ATCOMMAND_FUNC(charloadall);
ATCOMMAND_FUNC(speed);
ATCOMMAND_FUNC(charspeed);
ATCOMMAND_FUNC(charspeedmap);
ATCOMMAND_FUNC(charspeedall);
ATCOMMAND_FUNC(storage);
ATCOMMAND_FUNC(charstorage);
ATCOMMAND_FUNC(guildstorage);
ATCOMMAND_FUNC(charguildstorage);
ATCOMMAND_FUNC(option);
ATCOMMAND_FUNC(optionadd);
ATCOMMAND_FUNC(optionremove);
ATCOMMAND_FUNC(hide);
ATCOMMAND_FUNC(jobchange);
ATCOMMAND_FUNC(die);
ATCOMMAND_FUNC(kill);
ATCOMMAND_FUNC(alive);
ATCOMMAND_FUNC(kami);
ATCOMMAND_FUNC(kamib);
ATCOMMAND_FUNC(kamiGM);
ATCOMMAND_FUNC(heal);
ATCOMMAND_FUNC(item);
ATCOMMAND_FUNC(charitem);
ATCOMMAND_FUNC(charitemall);
ATCOMMAND_FUNC(item2);
ATCOMMAND_FUNC(itemreset);
ATCOMMAND_FUNC(charitemreset);
ATCOMMAND_FUNC(itemcheck);
ATCOMMAND_FUNC(charitemcheck);
ATCOMMAND_FUNC(baselevelup);
ATCOMMAND_FUNC(joblevelup);
ATCOMMAND_FUNC(help);
ATCOMMAND_FUNC(gm);
ATCOMMAND_FUNC(pvpoff);
ATCOMMAND_FUNC(pvpon);
ATCOMMAND_FUNC(gvgoff);
ATCOMMAND_FUNC(gvgon);
ATCOMMAND_FUNC(model);
ATCOMMAND_FUNC(go);
ATCOMMAND_FUNC(spawn);
ATCOMMAND_FUNC(spawnmap);
ATCOMMAND_FUNC(spawnall);
ATCOMMAND_FUNC(summon);
ATCOMMAND_FUNC(deadbranch);
ATCOMMAND_FUNC(chardeadbranch);
ATCOMMAND_FUNC(deadbranchmap);
ATCOMMAND_FUNC(deadbranchall);
ATCOMMAND_FUNC(killmonster);
ATCOMMAND_FUNC(killmonster2);
ATCOMMAND_FUNC(killmonsterarea);
ATCOMMAND_FUNC(killmonster2area);
ATCOMMAND_FUNC(refine);
ATCOMMAND_FUNC(refineall);
ATCOMMAND_FUNC(produce);
ATCOMMAND_FUNC(memo);
ATCOMMAND_FUNC(gat);
ATCOMMAND_FUNC(packet);
ATCOMMAND_FUNC(statuspoint);
ATCOMMAND_FUNC(skillpoint);
ATCOMMAND_FUNC(zeny);
ATCOMMAND_FUNC(param);
ATCOMMAND_FUNC(guildlevelup);
ATCOMMAND_FUNC(charguildlevelup);
ATCOMMAND_FUNC(makeegg);
ATCOMMAND_FUNC(hatch);
ATCOMMAND_FUNC(petfriendly);
ATCOMMAND_FUNC(pethungry);
ATCOMMAND_FUNC(petrename);
ATCOMMAND_FUNC(charpetrename);
ATCOMMAND_FUNC(recall);
ATCOMMAND_FUNC(recallall);
ATCOMMAND_FUNC(character_job);
ATCOMMAND_FUNC(revive);
ATCOMMAND_FUNC(charheal);
ATCOMMAND_FUNC(character_stats);
ATCOMMAND_FUNC(character_stats_all);
ATCOMMAND_FUNC(character_option);
ATCOMMAND_FUNC(character_optionadd);
ATCOMMAND_FUNC(character_optionremove);
ATCOMMAND_FUNC(character_save);
ATCOMMAND_FUNC(night);
ATCOMMAND_FUNC(day);
ATCOMMAND_FUNC(doommap);
ATCOMMAND_FUNC(doom);
ATCOMMAND_FUNC(raisemap);
ATCOMMAND_FUNC(raise);
ATCOMMAND_FUNC(character_baselevel);
ATCOMMAND_FUNC(character_joblevel);
ATCOMMAND_FUNC(change_level);
ATCOMMAND_FUNC(kick);
ATCOMMAND_FUNC(kickmap);
ATCOMMAND_FUNC(kickall);
ATCOMMAND_FUNC(allskill);
ATCOMMAND_FUNC(questskill);
ATCOMMAND_FUNC(charquestskill);
ATCOMMAND_FUNC(lostskill);
ATCOMMAND_FUNC(charlostskill);
ATCOMMAND_FUNC(spiritball);
ATCOMMAND_FUNC(charspiritball);
ATCOMMAND_FUNC(party);
ATCOMMAND_FUNC(guild);
ATCOMMAND_FUNC(resetstate);
ATCOMMAND_FUNC(resetskill);
ATCOMMAND_FUNC(charskreset);
ATCOMMAND_FUNC(charstreset);
ATCOMMAND_FUNC(charreset);
ATCOMMAND_FUNC(charstpoint);
ATCOMMAND_FUNC(charmodel);
ATCOMMAND_FUNC(charskpoint);
ATCOMMAND_FUNC(charzeny);
ATCOMMAND_FUNC(agitstart);
ATCOMMAND_FUNC(agitend);
ATCOMMAND_FUNC(reloaditemdb);
ATCOMMAND_FUNC(reloadmobdb);
ATCOMMAND_FUNC(reloadskilldb);
ATCOMMAND_FUNC(reloadscript);
//ATCOMMAND_FUNC(reloadgmdb); // removed, it's automatic now
ATCOMMAND_FUNC(mapexit);
ATCOMMAND_FUNC(idsearch);
ATCOMMAND_FUNC(whodrops);
ATCOMMAND_FUNC(mapinfo);
ATCOMMAND_FUNC(mobinfo);
ATCOMMAND_FUNC(dye);
ATCOMMAND_FUNC(hair_style);
ATCOMMAND_FUNC(hair_color);
ATCOMMAND_FUNC(stat_all);
ATCOMMAND_FUNC(change_sex);
ATCOMMAND_FUNC(char_change_sex);
ATCOMMAND_FUNC(char_block);
ATCOMMAND_FUNC(char_ban);
ATCOMMAND_FUNC(char_unblock);
ATCOMMAND_FUNC(char_unban);
ATCOMMAND_FUNC(mount_peco);
ATCOMMAND_FUNC(char_mount_peco);
ATCOMMAND_FUNC(falcon);
ATCOMMAND_FUNC(char_falcon);
ATCOMMAND_FUNC(cart);
ATCOMMAND_FUNC(char_cart);
ATCOMMAND_FUNC(remove_cart);
ATCOMMAND_FUNC(char_remove_cart);
ATCOMMAND_FUNC(guildspy);
ATCOMMAND_FUNC(partyspy);
ATCOMMAND_FUNC(repairall);
ATCOMMAND_FUNC(guildrecall);
ATCOMMAND_FUNC(partyrecall);
ATCOMMAND_FUNC(nuke);
ATCOMMAND_FUNC(enablenpc);
ATCOMMAND_FUNC(disablenpc);
ATCOMMAND_FUNC(servertime);
ATCOMMAND_FUNC(chardelitem);
ATCOMMAND_FUNC(jail);
ATCOMMAND_FUNC(unjail);
ATCOMMAND_FUNC(jailtime);
ATCOMMAND_FUNC(charjailtime);
ATCOMMAND_FUNC(disguise);
ATCOMMAND_FUNC(undisguise);
ATCOMMAND_FUNC(chardisguise);
ATCOMMAND_FUNC(charundisguise);
ATCOMMAND_FUNC(chardisguisemap);
ATCOMMAND_FUNC(charundisguisemap);
ATCOMMAND_FUNC(chardisguiseall);
ATCOMMAND_FUNC(charundisguiseall);
ATCOMMAND_FUNC(changelook);
ATCOMMAND_FUNC(charchangelook);
ATCOMMAND_FUNC(ignorelist);
ATCOMMAND_FUNC(charignorelist);
ATCOMMAND_FUNC(inall);
ATCOMMAND_FUNC(exall);
ATCOMMAND_FUNC(email);
ATCOMMAND_FUNC(effect);
ATCOMMAND_FUNC(character_item_list);
ATCOMMAND_FUNC(character_storage_list);
ATCOMMAND_FUNC(character_cart_list);
ATCOMMAND_FUNC(addwarp);
ATCOMMAND_FUNC(follow);
ATCOMMAND_FUNC(unfollow);
ATCOMMAND_FUNC(skillon);
ATCOMMAND_FUNC(skilloff);
ATCOMMAND_FUNC(nospell);
ATCOMMAND_FUNC(killer);
ATCOMMAND_FUNC(charkiller);
ATCOMMAND_FUNC(npcmove);
ATCOMMAND_FUNC(killable);
ATCOMMAND_FUNC(charkillable);
ATCOMMAND_FUNC(chareffect);
ATCOMMAND_FUNC(chardye);
ATCOMMAND_FUNC(charhairstyle);
ATCOMMAND_FUNC(charhaircolor);
ATCOMMAND_FUNC(dropall);
ATCOMMAND_FUNC(chardropall);
ATCOMMAND_FUNC(storeall);
ATCOMMAND_FUNC(charstoreall);
ATCOMMAND_FUNC(skillid);
ATCOMMAND_FUNC(useskill);
ATCOMMAND_FUNC(rain);
ATCOMMAND_FUNC(snow);
ATCOMMAND_FUNC(sakura);
ATCOMMAND_FUNC(fog);
ATCOMMAND_FUNC(leaves);
ATCOMMAND_FUNC(rainbow);
ATCOMMAND_FUNC(clsweather);
ATCOMMAND_FUNC(mobsearch);
ATCOMMAND_FUNC(cleanmap);
ATCOMMAND_FUNC(adjgmlvl);
ATCOMMAND_FUNC(adjgmlvl2);
ATCOMMAND_FUNC(adjcmdlvl);
ATCOMMAND_FUNC(trade);
ATCOMMAND_FUNC(send);
ATCOMMAND_FUNC(setbattleflag);
ATCOMMAND_FUNC(setmapflag);
ATCOMMAND_FUNC(unmute);
ATCOMMAND_FUNC(uptime);
ATCOMMAND_FUNC(clock);
ATCOMMAND_FUNC(mute);
ATCOMMAND_FUNC(refresh);
ATCOMMAND_FUNC(petid);
ATCOMMAND_FUNC(identify);
ATCOMMAND_FUNC(motd);
ATCOMMAND_FUNC(gmotd);
ATCOMMAND_FUNC(misceffect);
ATCOMMAND_FUNC(skilltree);
ATCOMMAND_FUNC(marry);
ATCOMMAND_FUNC(divorce);
ATCOMMAND_FUNC(rings);
ATCOMMAND_FUNC(grind);
ATCOMMAND_FUNC(grind2);
ATCOMMAND_FUNC(sound);

ATCOMMAND_FUNC(npctalk);
ATCOMMAND_FUNC(pettalk);
ATCOMMAND_FUNC(autoloot);
ATCOMMAND_FUNC(autolootloot);
ATCOMMAND_FUNC(displayexp);
ATCOMMAND_FUNC(displaydrop);
ATCOMMAND_FUNC(displaylootdrop);
ATCOMMAND_FUNC(display_player_hp);
ATCOMMAND_FUNC(main);
ATCOMMAND_FUNC(request);
ATCOMMAND_FUNC(version);
ATCOMMAND_FUNC(version2);

// Duelling commands by Daven
ATCOMMAND_FUNC(duel);
ATCOMMAND_FUNC(dueloff);
ATCOMMAND_FUNC(accept);
ATCOMMAND_FUNC(reject);
ATCOMMAND_FUNC(duelinfo);

#ifdef USE_SQL
ATCOMMAND_FUNC(checkmail);
ATCOMMAND_FUNC(listmail);
ATCOMMAND_FUNC(listnewmail);
ATCOMMAND_FUNC(readmail);
ATCOMMAND_FUNC(sendmail);
ATCOMMAND_FUNC(sendprioritymail);
ATCOMMAND_FUNC(deletemail);
#endif /* USE_SQL */

/*==========================================
 *AtCommandInfo atcommand_info[]�\���̂̒�`
 *------------------------------------------
 */
// First char of commands is configured in atcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read atcommand_athena.conf first please.
// Note: Be sure that all commands are in lower case in this structure
static struct AtCommandInfo {
	AtCommandType type;
	const char* command;
	unsigned char level;
	int (*proc)(const int, struct map_session_data*, const char* original_command, const char* command, const char* message);
} atcommand_info[] = {
	{ AtCommand_RuraP,                 "@rura+",                60, atcommand_rurap },
	{ AtCommand_RuraP,                 "@charrura",             60, atcommand_rurap },
	{ AtCommand_RuraP,                 "@charwarp",             60, atcommand_rurap },
	{ AtCommand_RuraP,                 "@charmapmove",          60, atcommand_rurap },
	{ AtCommand_Rura,                  "@rura",                 40, atcommand_rura },
	{ AtCommand_Warp,                  "@warp",                 40, atcommand_rura },
	{ AtCommand_MapMove,               "@mapmove",              40, atcommand_rura }, // /mm /mapmove command
	{ AtCommand_Where,                 "@where",                 3, atcommand_where },
	{ AtCommand_JumpTo,                "@jumpto",               20, atcommand_jumpto }, // + /shift and /remove
	{ AtCommand_JumpTo,                "@warpto",               20, atcommand_jumpto },
	{ AtCommand_JumpTo,                "@shift",                20, atcommand_jumpto },
	{ AtCommand_JumpTo,                "@goto",                 20, atcommand_jumpto },
	{ AtCommand_Jump,                  "@jump",                 40, atcommand_jump },
	{ AtCommand_Users,                 "@users",                10, atcommand_users },
	{ AtCommand_Who,                   "@who",                  20, atcommand_who },
	{ AtCommand_Who,                   "@whois",                20, atcommand_who },
	{ AtCommand_Who2,                  "@who2",                 20, atcommand_who2 },
	{ AtCommand_Who3,                  "@who3",                 20, atcommand_who3 },
	{ AtCommand_WhoMap,                "@whomap",               20, atcommand_whomap },
	{ AtCommand_WhoMap2,               "@whomap2",              20, atcommand_whomap2 },
	{ AtCommand_WhoMap3,               "@whomap3",              20, atcommand_whomap3 },
	{ AtCommand_WhoGM,                 "@whogm",                20, atcommand_whogm },
	{ AtCommand_Save,                  "@save",                 40, atcommand_save },
	{ AtCommand_Load,                  "@return",               40, atcommand_load },
	{ AtCommand_Load,                  "@load",                 40, atcommand_load },
	{ AtCommand_CharLoad,              "@charreturn",           60, atcommand_charload },
	{ AtCommand_CharLoad,              "@charload",             60, atcommand_charload },
	{ AtCommand_CharLoadMap,           "@charreturnmap",        60, atcommand_charloadmap },
	{ AtCommand_CharLoadMap,           "@charloadmap",          60, atcommand_charloadmap },
	{ AtCommand_CharLoadMap,           "@returnmap",            60, atcommand_charloadmap },
	{ AtCommand_CharLoadMap,           "@loadmap",              60, atcommand_charloadmap },
	{ AtCommand_CharLoadAll,           "@charreturnall",        80, atcommand_charloadall },
	{ AtCommand_CharLoadAll,           "@charloadall",          80, atcommand_charloadall },
	{ AtCommand_CharLoadAll,           "@returnall",            80, atcommand_charloadall },
	{ AtCommand_CharLoadAll,           "@loadall",              80, atcommand_charloadall },
	{ AtCommand_Speed,                 "@speed",                40, atcommand_speed },
	{ AtCommand_CharSpeed,             "@charspeed",            60, atcommand_charspeed },
	{ AtCommand_CharSpeedMap,          "@charspeedmap",         60, atcommand_charspeedmap },
	{ AtCommand_CharSpeedMap,          "@speedmap",             60, atcommand_charspeedmap },
	{ AtCommand_CharSpeedAll,          "@charspeedall",         80, atcommand_charspeedall },
	{ AtCommand_CharSpeedAll,          "@speedall",             80, atcommand_charspeedall },
	{ AtCommand_Storage,               "@storage",               3, atcommand_storage },
	{ AtCommand_CharStorage,           "@charstorage",          20, atcommand_charstorage },
	{ AtCommand_GuildStorage,          "@gstorage",             50, atcommand_guildstorage },
	{ AtCommand_GuildStorage,          "@guildstorage",         50, atcommand_guildstorage },
	{ AtCommand_CharGuildStorage,      "@chargstorage",         60, atcommand_charguildstorage },
	{ AtCommand_CharGuildStorage,      "@charguildstorage",     60, atcommand_charguildstorage },
	{ AtCommand_Option,                "@option",               40, atcommand_option },
	{ AtCommand_OptionAdd,             "@optionadd",            40, atcommand_optionadd },
	{ AtCommand_OptionRemove,          "@optionremove",         40, atcommand_optionremove },
	{ AtCommand_Hide,                  "@hide",                 20, atcommand_hide }, // + /hide
	{ AtCommand_JobChange,             "@jobchange",            40, atcommand_jobchange },
	{ AtCommand_JobChange,             "@jchange",              40, atcommand_jobchange },
	{ AtCommand_JobChange,             "@job",                  40, atcommand_jobchange },
	{ AtCommand_Die,                   "@die",                   2, atcommand_die },
	{ AtCommand_Die,                   "@killme",                2, atcommand_die },
	{ AtCommand_Die,                   "@suicide",               2, atcommand_die },
	{ AtCommand_Kill,                  "@kill",                 60, atcommand_kill },
	{ AtCommand_Kill,                  "@charkill",             60, atcommand_kill },
	{ AtCommand_Kill,                  "@chardie",              60, atcommand_kill },
	{ AtCommand_Alive,                 "@alive",                60, atcommand_alive },
	{ AtCommand_Alive,                 "@revive",               60, atcommand_alive },
	{ AtCommand_Alive,                 "@resurrect",            60, atcommand_alive },
	{ AtCommand_Heal,                  "@heal",                 40, atcommand_heal },
	{ AtCommand_Heal,                  "@restore",              40, atcommand_heal },
	{ AtCommand_Kami,                  "@kami",                 40, atcommand_kami },
	{ AtCommand_Kami,                  "@nb",                   40, atcommand_kami }, // /b, /nb and /bb command
	{ AtCommand_KamiB,                 "@kamib",                40, atcommand_kamib },
	{ AtCommand_KamiC,                 "@kamic",                40, atcommand_kami }, // code by [LuzZza]
	{ AtCommand_KamiGM,                "@bgm",                  20, atcommand_kamiGM },
	{ AtCommand_KamiGM,                "@kamigm",               20, atcommand_kamiGM },
	{ AtCommand_Item,                  "@item",                 60, atcommand_item }, // + /item
	{ AtCommand_CharItem,              "@charitem",             60, atcommand_charitem },
	{ AtCommand_CharItem,              "@giveitem",             60, atcommand_charitem },
	{ AtCommand_CharItemAll,           "@itemall",              80, atcommand_charitemall },
	{ AtCommand_CharItemAll,           "@charitemall",          80, atcommand_charitemall },
	{ AtCommand_CharItemAll,           "@giveitemall",          80, atcommand_charitemall },
	{ AtCommand_Item2,                 "@item2",                60, atcommand_item2 },
	{ AtCommand_ItemReset,             "@itemreset",            40, atcommand_itemreset },
	{ AtCommand_ItemReset,             "@inventoryreset",       40, atcommand_itemreset },
	{ AtCommand_CharItemReset,         "@charitemreset",        60, atcommand_charitemreset },
	{ AtCommand_CharItemReset,         "@charinventoryreset",   60, atcommand_charitemreset },
	{ AtCommand_ItemCheck,             "@itemcheck",            60, atcommand_itemcheck },
	{ AtCommand_CharItemCheck,         "@charitemcheck",        60, atcommand_charitemcheck },
	{ AtCommand_BaseLevelUp,           "@lvup",                 60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@lvlup",                60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@levelup",              60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@blvl",                 60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@blevel",               60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@blvlup",               60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@blevelup",             60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@baselvlup",            60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,           "@baselevelup",          60, atcommand_baselevelup },
	{ AtCommand_JobLevelUp,            "@jlvl",                 60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@jlevel",               60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@jlvlup",               60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@jlevelup",             60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@joblvup",              60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@joblvlup",             60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,            "@joblevelup",           60, atcommand_joblevelup },
	{ AtCommand_H,                     "@h",                     2, atcommand_help },
	{ AtCommand_Help,                  "@help",                  2, atcommand_help },
	{ AtCommand_GM,                    "@gm",                  100, atcommand_gm },
	{ AtCommand_PvPOff,                "@pvpoff",               40, atcommand_pvpoff },
	{ AtCommand_PvPOn,                 "@pvpon",                40, atcommand_pvpon },
	{ AtCommand_GvGOff,                "@gvgoff",               40, atcommand_gvgoff },
	{ AtCommand_GvGOff,                "@gpvpoff",              40, atcommand_gvgoff },
	{ AtCommand_GvGOn,                 "@gvgon",                40, atcommand_gvgon },
	{ AtCommand_GvGOn,                 "@gpvpon",               40, atcommand_gvgon },
	{ AtCommand_Model,                 "@model",                20, atcommand_model },
	{ AtCommand_Go,                    "@go",                   10, atcommand_go },

	{ AtCommand_Spawn,                 "@spawn",                50, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monster",              50, atcommand_spawn }, // + /monster
	{ AtCommand_Spawn,                 "@spawnsmall",           50, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monstersmall",         50, atcommand_spawn },
	{ AtCommand_Spawn,                 "@spawnbig",             50, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterbig",           50, atcommand_spawn},
	{ AtCommand_Spawn,                 "@spawnagro",            60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsteragro",          60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@spawnneutral",         60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterneutral",       60, atcommand_spawn },

	{ AtCommand_Spawn,                 "@spawnagrosmall",       60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsteragrosmall",     60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@spawnsmallagro",       60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monstersmallagro",     60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@spawnneutralsmall",    60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterneutralsmall",  60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@spawnsmallneutral",    60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monstersmallneutral",  60, atcommand_spawn },

	{ AtCommand_Spawn,                 "@spawnagrobig",         60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsteragrobig",       60, atcommand_spawn},
	{ AtCommand_Spawn,                 "@spawnbigagro",         60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterbigagro",       60, atcommand_spawn},
	{ AtCommand_Spawn,                 "@spawnneutralbig",      60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterneutralbig",    60, atcommand_spawn},
	{ AtCommand_Spawn,                 "@spawnbigneutral",      60, atcommand_spawn },
	{ AtCommand_Spawn,                 "@monsterbigneutral",    60, atcommand_spawn},

	{ AtCommand_SpawnMap,              "@spawnmap",             50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermap",           50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapsmall",        50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapsmall",      50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnsmallmap",        50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstersmallmap",      50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapbig",          50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapbig",        50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnbigmap",          50, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterbigmap",        50, atcommand_spawnmap },

	{ AtCommand_SpawnMap,              "@spawnagromap",         60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsteragromap",       60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapagro",         60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapagro",       60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnneutralmap",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterneutralmap",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapneutral",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapneutral",    60, atcommand_spawnmap },

	{ AtCommand_SpawnMap,              "@spawnmapsmallagro",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapsmallagro",  60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnsmallmapagro",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstersmallmapagro",  60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapagrosmall",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapagrosmall",  60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnsmallagromap",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstersmallagromap",  60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnagromapsmall",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsteragromapsmall",  60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnagrosmallmap",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsteragrosmallmap",  60, atcommand_spawnmap },

	{ AtCommand_SpawnMap,              "@spawnmapsmallneutral",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapsmallneutral", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnsmallmapneutral",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstersmallmapneutral", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapneutralsmall",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapneutralsmall", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnsmallneutralmap",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstersmallneutralmap", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnneutralmapsmall",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterneutralmapsmall", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnneutralsmallmap",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterneutralsmallmap", 60, atcommand_spawnmap },

	{ AtCommand_SpawnMap,              "@spawnmapbigagro",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapbigagro",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnbigmapagro",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterbigmapagro",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapagrobig",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapagrobig",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnbigagromap",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterbigagromap",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnagromapbig",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsteragromapbig",    60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnagrobigmap",      60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsteragrobigmap",    60, atcommand_spawnmap },

	{ AtCommand_SpawnMap,              "@spawnmapbigneutral",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapbigneutral", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnbigmapneutral",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterbigmapneutral", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnmapneutralbig",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monstermapneutralbig", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnbigneutralmap",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterbigneutralmap", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnneutralmapbig",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterneutralmapbig", 60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@spawnneutralbigmap",   60, atcommand_spawnmap },
	{ AtCommand_SpawnMap,              "@monsterneutralbigmap", 60, atcommand_spawnmap },

	{ AtCommand_SpawnAll,              "@spawnall",             60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterall",           60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallsmall",        60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallsmall",      60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnsmallall",        60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monstersmallall",      60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallbig",          60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallbig",        60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnbigall",          60, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterbigall",        60, atcommand_spawnall },

	{ AtCommand_SpawnAll,              "@spawnagroall",         70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsteragroall",       70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallagro",         70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallagro",       70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnneutralall",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterneutralall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallneutral",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallneutral",    70, atcommand_spawnall },

	{ AtCommand_SpawnAll,              "@spawnallsmallagro",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallsmallagro",  70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnsmallallagro",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monstersmallallagro",  70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallagrosmall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallagrosmall",  70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnsmallagroall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monstersmallagroall",  70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnagroallsmall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsteragroallsmall",  70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnagrosmallall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsteragrosmallall",  70, atcommand_spawnall },

	{ AtCommand_SpawnAll,              "@spawnallsmallneutral",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallsmallneutral", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnsmallallneutral",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monstersmallallneutral", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallneutralsmall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallneutralsmall", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnsmallneutralall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monstersmallneutralall", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnneutralallsmall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterneutralallsmall", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnneutralsmallall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterneutralsmallall", 70, atcommand_spawnall },

	{ AtCommand_SpawnAll,              "@spawnallbigagro",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallbigagro",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnbigallagro",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterbigallagro",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallagrobig",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallagrobig",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnbigagroall",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterbigagroall",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnagroallbig",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsteragroallbig",    70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnagrobigall",      70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsteragrobigall",    70, atcommand_spawnall },

	{ AtCommand_SpawnAll,              "@spawnallbigneutral",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallbigneutral", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnbigallneutral",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterbigallneutral", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnallneutralbig",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterallneutralbig", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnbigneutralall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterbigneutralall", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnneutralallbig",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterneutralallbig", 70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@spawnneutralbigall",   70, atcommand_spawnall },
	{ AtCommand_SpawnAll,              "@monsterneutralbigall", 70, atcommand_spawnall },

	{ AtCommand_Summon,                "@summon",               60, atcommand_summon },
	{ AtCommand_Summon,                "@summonsmall",          60, atcommand_summon },
	{ AtCommand_Summon,                "@summonbig",            60, atcommand_summon },

	{ AtCommand_DeadBranch,            "@deadbranch",           60, atcommand_deadbranch },
	{ AtCommand_DeadBranch,            "@dead_branch",          60, atcommand_deadbranch },
	{ AtCommand_DeadBranch,            "@deadbranchsmall",      60, atcommand_deadbranch },
	{ AtCommand_DeadBranch,            "@dead_branchsmall",     60, atcommand_deadbranch },
	{ AtCommand_DeadBranch,            "@deadbranchbig",        60, atcommand_deadbranch },
	{ AtCommand_DeadBranch,            "@dead_branchbig",       60, atcommand_deadbranch },
	{ AtCommand_CharDeadBranch,        "@chardeadbranch",       60, atcommand_chardeadbranch },
	{ AtCommand_CharDeadBranch,        "@chardead_branch",      60, atcommand_chardeadbranch },
	{ AtCommand_CharDeadBranch,        "@chardeadbranchsmall",  60, atcommand_chardeadbranch },
	{ AtCommand_CharDeadBranch,        "@chardead_branchsmall", 60, atcommand_chardeadbranch },
	{ AtCommand_CharDeadBranch,        "@chardeadbranchbig",    60, atcommand_chardeadbranch },
	{ AtCommand_CharDeadBranch,        "@chardead_branchbig",   60, atcommand_chardeadbranch },
	{ AtCommand_DeadBranchMap,         "@deadbranchmap",        60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@dead_branchmap",       60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@deadbranchmapsmall",   60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@deadbranchsmallmap",   60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@dead_branchmapsmall",  60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@dead_branchsmallmap",  60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@deadbranchmapbig",     60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@deadbranchbigmap",     60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@dead_branchmapbig",    60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchMap,         "@dead_branchbigmap",    60, atcommand_deadbranchmap },
	{ AtCommand_DeadBranchAll,         "@deadbranchall",        70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@dead_branchall",       70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@deadbranchallsmall",   70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@deadbranchsmallall",   70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@dead_branchallsmall",  70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@dead_branchsmallall",  70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@deadbranchallbig",     70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@deadbranchbigall",     70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@dead_branchallbig",    70, atcommand_deadbranchall },
	{ AtCommand_DeadBranchAll,         "@dead_branchbigall",    70, atcommand_deadbranchall },

	{ AtCommand_KillMonster,           "@killmonster",          60, atcommand_killmonster },
	{ AtCommand_KillMonster2,          "@killmonster2",         40, atcommand_killmonster2 },
	{ AtCommand_KillMonsterArea,       "@killmonsterarea",      60, atcommand_killmonsterarea },
	{ AtCommand_KillMonster2Area,      "@killmonster2area",     40, atcommand_killmonster2area },
	{ AtCommand_KillMonster2Area,      "@killmonsterarea2",     40, atcommand_killmonster2area },

	{ AtCommand_Refine,                "@refine",               60, atcommand_refine },
	{ AtCommand_RefineAll,             "@refineall",            60, atcommand_refineall },
	{ AtCommand_Produce,               "@produce",              60, atcommand_produce },
	{ AtCommand_Memo,                  "@memo",                 40, atcommand_memo },
	{ AtCommand_GAT,                   "@gat",                  99, atcommand_gat }, // debug function
	{ AtCommand_Packet,                "@packet",               99, atcommand_packet }, // debug function
	{ AtCommand_StatusPoint,           "@stpoint",              60, atcommand_statuspoint },
	{ AtCommand_SkillPoint,            "@skpoint",              60, atcommand_skillpoint },
	{ AtCommand_Zeny,                  "@zeny",                 60, atcommand_zeny },
	{ AtCommand_Strength,              "@str",                  60, atcommand_param },
	{ AtCommand_Agility,               "@agi",                  60, atcommand_param },
	{ AtCommand_Vitality,              "@vit",                  60, atcommand_param },
	{ AtCommand_Intelligence,          "@int",                  60, atcommand_param },
	{ AtCommand_Dexterity,             "@dex",                  60, atcommand_param },
	{ AtCommand_Luck,                  "@luk",                  60, atcommand_param },
	{ AtCommand_GuildLevelUp,          "@guildlvup",            60, atcommand_guildlevelup },
	{ AtCommand_GuildLevelUp,          "@guildlvlup",           60, atcommand_guildlevelup },
	{ AtCommand_GuildLevelUp,          "@guildlevelup",         60, atcommand_guildlevelup },
	{ AtCommand_CharGuildLevelUp,      "@charguildlvup",        60, atcommand_charguildlevelup },
	{ AtCommand_CharGuildLevelUp,      "@charguildlvlup",       60, atcommand_charguildlevelup },
	{ AtCommand_CharGuildLevelUp,      "@charguildlevelup",     60, atcommand_charguildlevelup },
	{ AtCommand_MakeEgg,               "@makeegg",              60, atcommand_makeegg },
	{ AtCommand_MakeEgg,               "@createegg",            60, atcommand_makeegg },
	{ AtCommand_Hatch,                 "@hatch",                40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@incubate",             40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@incubator",            40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@pet_incubator",        40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@petincubator",         40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@petbirth",             40, atcommand_hatch },
	{ AtCommand_Hatch,                 "@pet_birth",            40, atcommand_hatch },
	{ AtCommand_PetFriendly,           "@petfriendly",          40, atcommand_petfriendly },
	{ AtCommand_PetHungry,             "@pethungry",            40, atcommand_pethungry },
	{ AtCommand_PetRename,             "@petrename",             2, atcommand_petrename },
	{ AtCommand_CharPetRename,         "@charpetrename",        50, atcommand_charpetrename },
	{ AtCommand_Recall,                "@recall",               60, atcommand_recall }, // + /recall and /summon
	{ AtCommand_CharacterJob,          "@charjob",              60, atcommand_character_job },
	{ AtCommand_CharacterJob,          "@charjchange",          60, atcommand_character_job },
	{ AtCommand_CharacterJob,          "@charjobchange",        60, atcommand_character_job },
	{ AtCommand_ChangeLevel,           "@charchangelevel",      60, atcommand_change_level },
	{ AtCommand_ChangeLevel,           "@charchangelvl",        60, atcommand_change_level },
	{ AtCommand_ChangeLevel,           "@setchangelevel",       60, atcommand_change_level },
	{ AtCommand_ChangeLevel,           "@setchangelvl",         60, atcommand_change_level },
	{ AtCommand_ChangeLevel,           "@changelevel",          60, atcommand_change_level },
	{ AtCommand_ChangeLevel,           "@changelvl",            60, atcommand_change_level },
	{ AtCommand_Revive,                "@charalive",            60, atcommand_revive },
	{ AtCommand_Revive,                "@charrevive",           60, atcommand_revive },
	{ AtCommand_Revive,                "@charresurrect",        60, atcommand_revive },
	{ AtCommand_CharacterHeal,         "@charheal",             60, atcommand_charheal },
	{ AtCommand_CharacterHeal,         "@charrestore",          60, atcommand_charheal },
	{ AtCommand_CharacterStats,        "@charstats",            20, atcommand_character_stats },
	{ AtCommand_CharacterStatsAll,     "@charstatsall",         40, atcommand_character_stats_all },
	{ AtCommand_CharacterOption,       "@charoption",           60, atcommand_character_option },
	{ AtCommand_CharacterOptionAdd,    "@charoptionadd",        60, atcommand_character_optionadd },
	{ AtCommand_CharacterOptionRemove, "@charoptionremove",     60, atcommand_character_optionremove },
	{ AtCommand_CharacterSave,         "@charsave",             60, atcommand_character_save },
	{ AtCommand_Night,                 "@night",                80, atcommand_night },
	{ AtCommand_Day,                   "@day",                  80, atcommand_day },
	{ AtCommand_Doom,                  "@doom",                 80, atcommand_doom },
	{ AtCommand_DoomMap,               "@doommap",              70, atcommand_doommap },
	{ AtCommand_Raise,                 "@raise",                80, atcommand_raise },
	{ AtCommand_RaiseMap,              "@raisemap",             70, atcommand_raisemap },
	{ AtCommand_CharacterBaseLevel,    "@charlvup",             60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charlvlup",            60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charlevelup",          60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charblvl",             60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charblevel",           60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charblvlup",           60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charblevelup",         60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charbaselvlup",        60, atcommand_character_baselevel },
	{ AtCommand_CharacterBaseLevel,    "@charbaselevelup",      60, atcommand_character_baselevel },
	{ AtCommand_CharacterJobLevel,     "@charjlvl",             60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjlevel",           60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjlvlup",           60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjlevelup",         60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjoblvup",          60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjoblvlup",         60, atcommand_character_joblevel },
	{ AtCommand_CharacterJobLevel,     "@charjoblevelup",       60, atcommand_character_joblevel },
	{ AtCommand_Kick,                  "@kick",                 20, atcommand_kick }, // + right click menu for GM "(name) force to quit"
	{ AtCommand_KickMap,               "@kickmap",              70, atcommand_kickmap },
	{ AtCommand_KickAll,               "@kickall",              99, atcommand_kickall },
	{ AtCommand_AllSkill,              "@allskill",             60, atcommand_allskill },
	{ AtCommand_AllSkill,              "@allskills",            60, atcommand_allskill },
	{ AtCommand_AllSkill,              "@skillall",             60, atcommand_allskill },
	{ AtCommand_AllSkill,              "@skillsall",            60, atcommand_allskill },
	{ AtCommand_QuestSkill,            "@questskill",           40, atcommand_questskill },
	{ AtCommand_CharQuestSkill,        "@charquestskill",       60, atcommand_charquestskill },
	{ AtCommand_LostSkill,             "@lostskill",            40, atcommand_lostskill },
	{ AtCommand_LostSkill,             "@lostquest",            40, atcommand_lostskill },
	{ AtCommand_CharLostSkill,         "@charlostskill",        60, atcommand_charlostskill },
	{ AtCommand_CharLostSkill,         "@charlostquest",        60, atcommand_charlostskill },
	{ AtCommand_SpiritBall,            "@spiritball",           40, atcommand_spiritball },
	{ AtCommand_CharSpiritBall,        "@charspiritball",       60, atcommand_charspiritball },
	{ AtCommand_Party,                 "@party",                 2, atcommand_party },
	{ AtCommand_Guild,                 "@guild",                50, atcommand_guild },
	{ AtCommand_AgitStart,             "@agitstart",            60, atcommand_agitstart },
	{ AtCommand_AgitEnd,               "@agitend",              60, atcommand_agitend },
	{ AtCommand_MapExit,               "@mapexit",              99, atcommand_mapexit },
	{ AtCommand_IDSearch,              "@idsearch",             60, atcommand_idsearch },
	{ AtCommand_IDSearch,              "@itemsearch",           60, atcommand_idsearch },
	{ AtCommand_IDSearch,              "@searchid",             60, atcommand_idsearch },
	{ AtCommand_IDSearch,              "@searchitem",           60, atcommand_idsearch },
	{ AtCommand_WhoDrops,              "@whodrop",               0, atcommand_whodrops },
	{ AtCommand_WhoDrops,              "@whodrops",              0, atcommand_whodrops },
	{ AtCommand_Broadcast,             "@broadcast",            40, atcommand_broadcast },
	{ AtCommand_LocalBroadcast,        "@localbroadcast",       20, atcommand_localbroadcast },
	{ AtCommand_LocalBroadcast,        "@local_broadcast",      20, atcommand_localbroadcast },
	{ AtCommand_LocalBroadcast,        "@lb",                   20, atcommand_localbroadcast },
	{ AtCommand_LocalBroadcast2,       "@nlb",                  40, atcommand_localbroadcast2 }, // /lb, /nlb and /mb commands
	{ AtCommand_LocalBroadcast2,       "@mb",                   40, atcommand_localbroadcast2 }, // /lb, /nlb and /mb commands
	{ AtCommand_RecallAll,             "@recallall",            80, atcommand_recallall },
	{ AtCommand_ResetState,            "@resetstate",           40, atcommand_resetstate }, // + /resetstate
	{ AtCommand_ResetSkill,            "@resetskill",           40, atcommand_resetskill }, // + /resetskill
	{ AtCommand_CharStReset,           "@charstreset",          60, atcommand_charstreset },
	{ AtCommand_CharStReset,           "@charresetstate",       60, atcommand_charstreset },
	{ AtCommand_CharSkReset,           "@charskreset",          60, atcommand_charskreset },
	{ AtCommand_CharSkReset,           "@charresetskill",       60, atcommand_charskreset },
	{ AtCommand_ReloadItemDB,          "@reloaditemdb",         99, atcommand_reloaditemdb }, // admin command
	{ AtCommand_ReloadMobDB,           "@reloadmobdb",          99, atcommand_reloadmobdb }, // admin command
	{ AtCommand_ReloadSkillDB,         "@reloadskilldb",        99, atcommand_reloadskilldb }, // admin command
	{ AtCommand_ReloadScript,          "@rehash",               99, atcommand_reloadscript }, // admin command
	{ AtCommand_ReloadScript,          "@reloadscript",         99, atcommand_reloadscript }, // admin command
//	{ AtCommand_ReloadGMDB,            "@reloadgmdb",           99, atcommand_reloadgmdb }, // admin command // removed, it's automatic now
	{ AtCommand_CharReset,             "@charreset",            60, atcommand_charreset },
	{ AtCommand_CharModel,             "@charmodel",            50, atcommand_charmodel },
	{ AtCommand_CharSKPoint,           "@charskpoint",          60, atcommand_charskpoint },
	{ AtCommand_CharSTPoint,           "@charstpoint",          60, atcommand_charstpoint },
	{ AtCommand_CharZeny,              "@charzeny",             60, atcommand_charzeny },
	{ AtCommand_MapInfo,               "@mapinfo",              40, atcommand_mapinfo },
	{ AtCommand_MobInfo,               "@mobinfo",              20, atcommand_mobinfo },
	{ AtCommand_MobInfo,               "@monsterinfo",          20, atcommand_mobinfo },
	{ AtCommand_MobInfo,               "@mi",                   20, atcommand_mobinfo },
	{ AtCommand_MobInfo,               "@infomob",              20, atcommand_mobinfo },
	{ AtCommand_MobInfo,               "@infomonster",          20, atcommand_mobinfo },
	{ AtCommand_Dye,                   "@dye",                  20, atcommand_dye },
	{ AtCommand_Dye,                   "@ccolor",               20, atcommand_dye },
	{ AtCommand_Dye,                   "@clothescolor",         20, atcommand_dye },
	{ AtCommand_Hstyle,                "@hairstyle",            20, atcommand_hair_style },
	{ AtCommand_Hstyle,                "@hstyle",               20, atcommand_hair_style },
	{ AtCommand_Hcolor,                "@haircolor",            20, atcommand_hair_color },
	{ AtCommand_Hcolor,                "@hcolor",               20, atcommand_hair_color },
	{ AtCommand_StatAll,               "@statall",              60, atcommand_stat_all },
	{ AtCommand_StatAll,               "@statsall",             60, atcommand_stat_all },
	{ AtCommand_StatAll,               "@allstats",             60, atcommand_stat_all },
	{ AtCommand_StatAll,               "@allstat",              60, atcommand_stat_all },
	{ AtCommand_ChangeSex,             "@changesex",            20, atcommand_change_sex },
	{ AtCommand_CharChangeSex,         "@charchangesex",        60, atcommand_char_change_sex },
	{ AtCommand_CharBlock,             "@block",                60, atcommand_char_block },
	{ AtCommand_CharBlock,             "@charblock",            60, atcommand_char_block },
	{ AtCommand_CharBan,               "@ban",                  60, atcommand_char_ban },
	{ AtCommand_CharBan,               "@banish",               60, atcommand_char_ban },
	{ AtCommand_CharBan,               "@charban",              60, atcommand_char_ban },
	{ AtCommand_CharBan,               "@charbanish",           60, atcommand_char_ban },
	{ AtCommand_CharUnBlock,           "@unblock",              80, atcommand_char_unblock },
	{ AtCommand_CharUnBlock,           "@charunblock",          80, atcommand_char_unblock },
	{ AtCommand_CharUnBan,             "@unban",                80, atcommand_char_unban },
	{ AtCommand_CharUnBan,             "@unbanish",             80, atcommand_char_unban },
	{ AtCommand_CharUnBan,             "@charunban",            80, atcommand_char_unban },
	{ AtCommand_CharUnBan,             "@charunbanish",         80, atcommand_char_unban },
	{ AtCommand_MountPeco,             "@peco",                 20, atcommand_mount_peco },
	{ AtCommand_MountPeco,             "@mountpeco",            20, atcommand_mount_peco },
	{ AtCommand_CharMountPeco,         "@charpeco",             50, atcommand_char_mount_peco },
	{ AtCommand_CharMountPeco,         "@charmountpeco",        50, atcommand_char_mount_peco },
	{ AtCommand_Falcon,                "@falcon",               20, atcommand_falcon },
	{ AtCommand_CharFalcon,            "@charfalcon",           50, atcommand_char_falcon },
	{ AtCommand_Cart,                  "@cart",                 20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart0",                20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart1",                20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart2",                20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart3",                20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart4",                20, atcommand_cart },
	{ AtCommand_Cart,                  "@cart5",                20, atcommand_cart },
	{ AtCommand_RemoveCart,            "@removecart",           20, atcommand_remove_cart },
	{ AtCommand_CharCart,              "@charcart",             50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart0",            50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart1",            50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart2",            50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart3",            50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart4",            50, atcommand_char_cart },
	{ AtCommand_CharCart,              "@charcart5",            50, atcommand_char_cart },
	{ AtCommand_CharRemoveCart,        "@charremovecart",       50, atcommand_char_remove_cart },
	{ AtCommand_GuildSpy,              "@guildspy",             60, atcommand_guildspy },
	{ AtCommand_PartySpy,              "@partyspy",             60, atcommand_partyspy },
	{ AtCommand_RepairAll,             "@repairall",            60, atcommand_repairall },
	{ AtCommand_GuildRecall,           "@guildrecall",          60, atcommand_guildrecall },
	{ AtCommand_PartyRecall,           "@partyrecall",          60, atcommand_partyrecall },
	{ AtCommand_Nuke,                  "@nuke",                 60, atcommand_nuke },
	{ AtCommand_Enablenpc,             "@enablenpc",            80, atcommand_enablenpc },
	{ AtCommand_Enablenpc,             "@npcon",                80, atcommand_enablenpc },
	{ AtCommand_Disablenpc,            "@disablenpc",           80, atcommand_disablenpc },
	{ AtCommand_Disablenpc,            "@npcoff",               80, atcommand_disablenpc },
	{ AtCommand_ServerTime,            "@time",                  0, atcommand_servertime },
	{ AtCommand_ServerTime,            "@date",                  0, atcommand_servertime },
	{ AtCommand_ServerTime,            "@server_date",           0, atcommand_servertime },
	{ AtCommand_ServerTime,            "@serverdate",            0, atcommand_servertime },
	{ AtCommand_ServerTime,            "@server_time",           0, atcommand_servertime },
	{ AtCommand_ServerTime,            "@servertime",            0, atcommand_servertime },
	{ AtCommand_CharDelItem,           "@chardelitem",          60, atcommand_chardelitem },
	{ AtCommand_Jail,                  "@jail",                 60, atcommand_jail },
	{ AtCommand_Jail,                  "@prison",               60, atcommand_jail },
	{ AtCommand_UnJail,                "@unjail",               60, atcommand_unjail },
	{ AtCommand_UnJail,                "@discharge",            60, atcommand_unjail },
	{ AtCommand_JailTime,              "@jailtime",              0, atcommand_jailtime },
	{ AtCommand_JailTime,              "@jail_time",             0, atcommand_jailtime },
	{ AtCommand_JailTime,              "@prisontime",            0, atcommand_jailtime },
	{ AtCommand_JailTime,              "@prison_time",           0, atcommand_jailtime },
	{ AtCommand_CharJailTime,          "@charjailtime",         20, atcommand_charjailtime },
	{ AtCommand_CharJailTime,          "@charjail_time",        20, atcommand_charjailtime },
	{ AtCommand_CharJailTime,          "@charprisontime",       20, atcommand_charjailtime },
	{ AtCommand_CharJailTime,          "@charprison_time",      20, atcommand_charjailtime },
	{ AtCommand_Disguise,              "@disguise",             20, atcommand_disguise },
	{ AtCommand_UnDisguise,            "@undisguise",           20, atcommand_undisguise },
	{ AtCommand_CharDisguise,          "@chardisguise",         60, atcommand_chardisguise },
	{ AtCommand_CharUnDisguise,        "@charundisguise",       60, atcommand_charundisguise },
	{ AtCommand_CharDisguiseMap,       "@disguisemap",          60, atcommand_chardisguisemap },
	{ AtCommand_CharDisguiseMap,       "@chardisguisemap",      60, atcommand_chardisguisemap },
	{ AtCommand_CharUnDisguiseMap,     "@undisguisemap",        60, atcommand_charundisguisemap },
	{ AtCommand_CharUnDisguiseMap,     "@charundisguisemap",    60, atcommand_charundisguisemap },
	{ AtCommand_CharDisguiseAll,       "@disguiseall",          70, atcommand_chardisguiseall },
	{ AtCommand_CharDisguiseAll,       "@chardisguiseall",      70, atcommand_chardisguiseall },
	{ AtCommand_CharUnDisguiseAll,     "@undisguiseall",        60, atcommand_charundisguiseall },
	{ AtCommand_CharUnDisguiseAll,     "@charundisguiseall",    60, atcommand_charundisguiseall },
	{ AtCommand_ChangeLook,            "@changelook",           20, atcommand_changelook },
	{ AtCommand_CharChangeLook,        "@charchangelook",       60, atcommand_charchangelook },
	{ AtCommand_IgnoreList,            "@ignorelist",            0, atcommand_ignorelist },
	{ AtCommand_CharIgnoreList,        "@charignorelist",       20, atcommand_charignorelist },
	{ AtCommand_IgnoreList,            "@inall",                20, atcommand_inall },
	{ AtCommand_ExAll,                 "@exall",                20, atcommand_exall },
	{ AtCommand_EMail,                 "@email",                 0, atcommand_email },
	{ AtCommand_Effect,                "@effect",               40, atcommand_effect },
	{ AtCommand_Char_Item_List,        "@charitemlist",         20, atcommand_character_item_list },
	{ AtCommand_Char_Item_List,        "@charinventorylist",    20, atcommand_character_item_list },
	{ AtCommand_Char_Storage_List,     "@charstoragelist",      20, atcommand_character_storage_list },
	{ AtCommand_Char_Cart_List,        "@charcartlist",         20, atcommand_character_cart_list },
	{ AtCommand_Follow,                "@follow",               20, atcommand_follow },
	{ AtCommand_UnFollow,              "@unfollow",             20, atcommand_unfollow },
	{ AtCommand_AddWarp,               "@addwarp",              60, atcommand_addwarp },
	{ AtCommand_SkillOn,               "@skillon",              80, atcommand_skillon },
	{ AtCommand_SkillOff,              "@skilloff",             80, atcommand_skilloff },
	{ AtCommand_NoSpell,               "@nospell",              80, atcommand_nospell },
	{ AtCommand_Killer,                "@killer",               60, atcommand_killer },
	{ AtCommand_CharKiller,            "@charkiller",           60, atcommand_charkiller },
	{ AtCommand_NpcMove,               "@npcmove",              80, atcommand_npcmove },
	{ AtCommand_NpcMove,               "@movenpc",              80, atcommand_npcmove },
	{ AtCommand_Killable,              "@killable",             40, atcommand_killable },
	{ AtCommand_CharKillable,          "@charkillable",         60, atcommand_charkillable },
	{ AtCommand_Chareffect,            "@chareffect",           40, atcommand_chareffect },
	{ AtCommand_Chardye,               "@chardye",              50, atcommand_chardye },
	{ AtCommand_Chardye,               "@charccolor",           50, atcommand_chardye },
	{ AtCommand_Chardye,               "@charclothescolor",     50, atcommand_chardye },
	{ AtCommand_Charhairstyle,         "@charhairstyle",        50, atcommand_charhairstyle },
	{ AtCommand_Charhairstyle,         "@charhstyle",           50, atcommand_charhairstyle },
	{ AtCommand_Charhaircolor,         "@charhaircolor",        50, atcommand_charhaircolor },
	{ AtCommand_Charhaircolor,         "@charhcolor",           50, atcommand_charhaircolor },
	{ AtCommand_Dropall,               "@dropall",              40, atcommand_dropall },
	{ AtCommand_Chardropall,           "@chardropall",          60, atcommand_chardropall },
	{ AtCommand_Storeall,              "@storeall",             40, atcommand_storeall },
	{ AtCommand_Charstoreall,          "@charstoreall",         60, atcommand_charstoreall },
	{ AtCommand_Skillid,               "@skillid",              40, atcommand_skillid },
	{ AtCommand_Useskill,              "@useskill",             40, atcommand_useskill },
	{ AtCommand_Rain,                  "@rain",                 80, atcommand_rain },
	{ AtCommand_Snow,                  "@snow",                 80, atcommand_snow },
	{ AtCommand_Sakura,                "@sakura",               80, atcommand_sakura },
	{ AtCommand_Fog,                   "@fog",                  80, atcommand_fog },
	{ AtCommand_Leaves,                "@leaves",               80, atcommand_leaves },
	{ AtCommand_Rainbow,               "@rainbow",              80, atcommand_rainbow },
	{ AtCommand_Clsweather,            "@clsweather",           80, atcommand_clsweather },
	{ AtCommand_Clsweather,            "@clearweather",         80, atcommand_clsweather },
	{ AtCommand_MobSearch,             "@mobsearch",            20, atcommand_mobsearch },
	{ AtCommand_MobSearch,             "@monstersearch",        20, atcommand_mobsearch },
	{ AtCommand_MobSearch,             "@searchmob",            20, atcommand_mobsearch },
	{ AtCommand_MobSearch,             "@searchmonster",        20, atcommand_mobsearch },
	{ AtCommand_CleanMap,              "@cleanmap",             40, atcommand_cleanmap },
//	{ AtCommand_Shuffle,               "@shuffle",              99, atcommand_shuffle },
//	{ AtCommand_Maintenance,           "@maintenance",          99, atcommand_maintenance },
	{ AtCommand_MiscEffect,            "@misceffect",           60, atcommand_misceffect },
	{ AtCommand_AdjGmLvl,              "@adjgmlvl",             80, atcommand_adjgmlvl },
	{ AtCommand_AdjGmLvl,              "@setgmlvl",             80, atcommand_adjgmlvl },
	{ AtCommand_AdjGmLvl,              "@adjgmlevel",           80, atcommand_adjgmlvl },
	{ AtCommand_AdjGmLvl,              "@setgmlevel",           80, atcommand_adjgmlvl },
	{ AtCommand_AdjGmLvl2,             "@adjgmlvl2",            99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgmlvl2",            99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgmlevel2",          99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgmlevel2",          99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm0",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm0",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm1",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm1",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm2",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm2",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm3",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm3",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm4",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm4",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm5",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm5",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm6",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm6",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm7",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm7",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm8",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm8",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@adjgm9",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjGmLvl2,             "@setgm9",               99, atcommand_adjgmlvl2 },
	{ AtCommand_AdjCmdLvl,             "@adjcmdlvl",            99, atcommand_adjcmdlvl },
	{ AtCommand_AdjCmdLvl,             "@setcmdlvl",            99, atcommand_adjcmdlvl },
	{ AtCommand_AdjCmdLvl,             "@adjcmdlevel",          99, atcommand_adjcmdlvl },
	{ AtCommand_AdjCmdLvl,             "@setcmdlevel",          99, atcommand_adjcmdlvl },
	{ AtCommand_Trade,                 "@trade",                60, atcommand_trade },
	{ AtCommand_Send,                  "@send",                 60, atcommand_send },
	{ AtCommand_SetBattleFlag,         "@setbattleflag",        99, atcommand_setbattleflag },
	{ AtCommand_SetBattleFlag,         "@adjbattleflag",        99, atcommand_setbattleflag },
	{ AtCommand_SetBattleFlag,         "@setbattleconf",        99, atcommand_setbattleflag },
	{ AtCommand_SetBattleFlag,         "@adjbattleconf",        99, atcommand_setbattleflag },
	{ AtCommand_SetMapFlag,            "@setmapflag",           99, atcommand_setmapflag },
	{ AtCommand_SetMapFlag,            "@adjmapflag",           99, atcommand_setmapflag },
	{ AtCommand_UnMute,                "@unmute",               60, atcommand_unmute },
	{ AtCommand_UpTime,                "@uptime",                0, atcommand_uptime },
	{ AtCommand_Clock,                 "@clock",                 0, atcommand_clock },
	{ AtCommand_Mute,                  "@mute",                 99, atcommand_mute },
	{ AtCommand_Mute,                  "@red",                  99, atcommand_mute },
	{ AtCommand_WhoZeny,               "@whozeny",              20, atcommand_whozeny },
	{ AtCommand_WhoZenyMap,            "@whozenymap",           20, atcommand_whozenymap },
	{ AtCommand_WhoHas,                "@whohas",               20, atcommand_whohas },
	{ AtCommand_WhoHasMap,             "@whohasmap",            20, atcommand_whohasmap },
	{ AtCommand_HappyHappyJoyJoy,      "@happyhappyjoyjoy",     40, atcommand_happyhappyjoyjoy },
	{ AtCommand_HappyHappyJoyJoy,      "@happyhappy",           40, atcommand_happyhappyjoyjoy },
	{ AtCommand_HappyHappyJoyJoy,      "@joyjoy",               40, atcommand_happyhappyjoyjoy },
	{ AtCommand_HappyHappyJoyJoyMap,   "@happyhappyjoyjoymap",  40, atcommand_happyhappyjoyjoymap },
	{ AtCommand_HappyHappyJoyJoyMap,   "@happyhappymap",        40, atcommand_happyhappyjoyjoymap },
	{ AtCommand_HappyHappyJoyJoyMap,   "@joyjoymap",            40, atcommand_happyhappyjoyjoymap },
	{ AtCommand_Refresh,               "@refresh",              40, atcommand_refresh },
	{ AtCommand_PetId,                 "@petid",                40, atcommand_petid },
	{ AtCommand_Identify,              "@identify",             40, atcommand_identify },
	{ AtCommand_Motd,                  "@motd",                  0, atcommand_motd },
	{ AtCommand_Gmotd,                 "@gmotd",                60, atcommand_gmotd },
	{ AtCommand_Gmotd,                 "@globalmotd",           60, atcommand_gmotd },
	{ AtCommand_SkillTree,             "@skilltree",            40, atcommand_skilltree },
	{ AtCommand_Marry,                 "@marry",                40, atcommand_marry },
	{ AtCommand_Divorce,               "@divorce",              40, atcommand_divorce },
	{ AtCommand_Rings,                 "@rings",                40, atcommand_rings },
	{ AtCommand_Grind,                 "@grind",                99, atcommand_grind }, // (on test GM command)
	{ AtCommand_Grind,                 "@grindskill",           99, atcommand_grind }, // (on test GM command)
	{ AtCommand_Grind,                 "@grindskills",          99, atcommand_grind }, // (on test GM command)
	{ AtCommand_Grind2,                "@grind2",               60, atcommand_grind2 },
	{ AtCommand_Grind2,                "@grindmob",             60, atcommand_grind2 },
	{ AtCommand_Grind2,                "@grindmonster",         60, atcommand_grind2 },
	{ AtCommand_Grind2,                "@grindmobs",            60, atcommand_grind2 },
	{ AtCommand_Grind2,                "@grindmonsters",        60, atcommand_grind2 },
	{ AtCommand_Sound,                 "@sound",                40, atcommand_sound },

	{ AtCommand_NpcTalk,               "@npctalk",              40, atcommand_npctalk },
	{ AtCommand_NpcTalk,               "@npcmessage",           40, atcommand_npctalk },
	{ AtCommand_NpcTalk,               "@npcmes",               40, atcommand_npctalk },
	{ AtCommand_PetTalk,               "@pettalk",              10, atcommand_pettalk },
	{ AtCommand_PetTalk,               "@petmessage",           10, atcommand_pettalk },
	{ AtCommand_PetTalk,               "@petmes",               10, atcommand_pettalk },
	{ AtCommand_AutoLoot,              "@autoloot",              0, atcommand_autoloot },
	{ AtCommand_AutoLootLoot,          "@autolootloot",          0, atcommand_autolootloot },
	{ AtCommand_Displayexp,            "@displayexp",            0, atcommand_displayexp },
	{ AtCommand_DisplayDrop,           "@displaydrop",           0, atcommand_displaydrop },
	{ AtCommand_DisplayLootDrop,       "@displaylootdrop",       0, atcommand_displaylootdrop },
	{ AtCommand_Display_Player_Hp,     "@displayplayerhp",      60, atcommand_display_player_hp },
	{ AtCommand_Display_Player_Hp,     "@display_player_hp",    60, atcommand_display_player_hp },
	{ AtCommand_Main,                  "@main",                  1, atcommand_main },
	{ AtCommand_Request,               "@request",               0, atcommand_request },
	{ AtCommand_Request,               "@contactgm",             0, atcommand_request },
	{ AtCommand_Version,               "@version",               0, atcommand_version },
	{ AtCommand_Version2,              "@version2",              0, atcommand_version2 },
	{ AtCommand_Duel,				"@duel",			 0, atcommand_duel }, // By Daven
	{ AtCommand_DuelOff,			"@dueloff",			 0, atcommand_dueloff }, // By Daven
	{ AtCommand_Accept,				"@accept",			 0, atcommand_accept }, // By Daven
	{ AtCommand_Reject,				"@reject",			 0, atcommand_reject }, // By Daven
	{ AtCommand_Duel,				"@d",			 0, atcommand_duel }, // By Daven
	{ AtCommand_DuelOff,			"@do",			 0, atcommand_dueloff }, // By Daven
	{ AtCommand_Accept,				"@a",			 0, atcommand_accept }, // By Daven
	{ AtCommand_Reject,				"@r",			 0, atcommand_reject }, // By Daven
	{ AtCommand_DuelInfo,			"@di",			 0, atcommand_duelinfo }, // By Daven
	{ AtCommand_DuelInfo,			"@duelinfo",			 0, atcommand_duelinfo }, // By Daven

#ifdef USE_SQL
	{ AtCommand_CheckMail,             "@checkmail",             1, atcommand_listmail }, // type: 1:checkmail, 2:listmail, 3:listnewmail
	{ AtCommand_ListMail,              "@listmail",              1, atcommand_listmail }, // type: 1:checkmail, 2:listmail, 3:listnewmail
	{ AtCommand_ListNewMail,           "@listnewmail",           1, atcommand_listmail }, // type: 1:checkmail, 2:listmail, 3:listnewmail
	{ AtCommand_ReadMail,              "@readmail",              1, atcommand_readmail },
	{ AtCommand_DeleteMail,            "@deletemail",            1, atcommand_readmail },
	{ AtCommand_SendMail,              "@sendmail",              1, atcommand_sendmail },
	{ AtCommand_SendPriorityMail,      "@sendprioritymail",     80, atcommand_sendmail },
#else /* USE_SQL -> not USE_SQL*/
	{ AtCommand_None,                  "@checkmail",             1, NULL },
	{ AtCommand_None,                  "@listmail",              1, NULL },
	{ AtCommand_None,                  "@listnewmail",           1, NULL },
	{ AtCommand_None,                  "@readmail",              1, NULL },
	{ AtCommand_None,                  "@deletemail",            1, NULL },
	{ AtCommand_None,                  "@sendmail",              1, NULL },
	{ AtCommand_None,                  "@sendprioritymail",     80, NULL },
#endif /* USE_SQL */
	{ AtCommand_Unknown,               NULL,                     1, NULL }
};

/*=========================================
 * Generic variables
 *-----------------------------------------
 */
char atcmd_output[MAX_MSG_LEN + 512]; // at least maximum of message length + some variables (character' name, remaining time, map, etc...)
char atcmd_name[100];
char atcmd_mapname[100];

/*=========================================
 * This function return the name of the job
 *-----------------------------------------
 */
char * job_name(int class) {
	switch (class) {
	case 0:    return "Novice";
	case 1:    return "Swordsman";
	case 2:    return "Mage";
	case 3:    return "Archer";
	case 4:    return "Acolyte";
	case 5:    return "Merchant";
	case 6:    return "Thief";
	case 7:    return "Knight";
	case 8:    return "Priest";
	case 9:    return "Wizard";
	case 10:   return "Blacksmith";
	case 11:   return "Hunter";
	case 12:   return "Assassin";
	case 13:   return "Peco knight";
	case 14:   return "Crusader";
	case 15:   return "Monk";
	case 16:   return "Sage";
	case 17:   return "Rogue";
	case 18:   return "Alchemist";
	case 19:   return "Bard";
	case 20:   return "Dancer";
	case 21:   return "Peco crusader";
	case 22:   return "Wedding";
	case 23:   return "Super Novice";
	case 4001: return "Novice High";
	case 4002: return "Swordsman High";
	case 4003: return "Mage High";
	case 4004: return "Archer High";
	case 4005: return "Acolyte High";
	case 4006: return "Merchant High";
	case 4007: return "Thief High";
	case 4008: return "Lord Knight";
	case 4009: return "High Priest";
	case 4010: return "High Wizard";
	case 4011: return "Whitesmith";
	case 4012: return "Sniper";
	case 4013: return "Assassin Cross";
	case 4014: return "Peco Knight";
	case 4015: return "Paladin";
	case 4016: return "Champion";
	case 4017: return "Professor";
	case 4018: return "Stalker";
	case 4019: return "Creator";
	case 4020: return "Clown";
	case 4021: return "Gypsy";
	case 4022: return "Peco Paladin";
	case 4023: return "Baby Novice";
	case 4024: return "Baby Swordsman";
	case 4025: return "Baby Mage";
	case 4026: return "Baby Archer";
	case 4027: return "Baby Acolyte";
	case 4028: return "Baby Merchant";
	case 4029: return "Baby Thief";
	case 4030: return "Baby Knight";
	case 4031: return "Baby Priest";
	case 4032: return "Baby Wizard";
	case 4033: return "Baby Blacksmith";
	case 4034: return "Baby Hunter";
	case 4035: return "Baby Assassin";
	case 4036: return "Baby Peco Knight";
	case 4037: return "Baby Crusader";
	case 4038: return "Baby Monk";
	case 4039: return "Baby Sage";
	case 4040: return "Baby Rogue";
	case 4041: return "Baby Alchemist";
	case 4042: return "Baby Bard";
	case 4043: return "Baby Dancer";
	case 4044: return "Baby Peco Crusader";
	case 4045: return "Super Baby";
	}

	return "Unknown Job";
}

/*===============================================
 * This function logs all valid GM commands
 *-----------------------------------------------
 */
void log_atcommand(struct map_session_data *sd, const char *command, const char * param) {
	char tmpstr[200];
	unsigned char *sin_addr;

	sin_addr = (unsigned char *)&session[sd->fd]->client_addr.sin_addr;
	if (!param || !*param)
		sprintf(tmpstr, "%s [%d(lvl:%d)] (ip:%d.%d.%d.%d): %s" RETCODE,
		        sd->status.name, (int)sd->status.account_id, sd->GM_level, sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], command);
	else
		sprintf(tmpstr, "%s [%d(lvl:%d)] (ip:%d.%d.%d.%d): %s %s" RETCODE,
		        sd->status.name, (int)sd->status.account_id, sd->GM_level, sin_addr[0], sin_addr[1], sin_addr[2], sin_addr[3], command, param);

	// send log to inter-server
	intif_send_log(1, tmpstr); // 0x3008 <packet_len>.w <log_type>.B <message>.?B (types: 1 GM commands, 2: Trades, 3: Scripts, 4: Vending)

	return;
}

/*===============================================
 * This function return the GM command symbol
 *-----------------------------------------------
 */
char GM_Symbol() {
	return command_symbol;
}

/*===============================================
 * This function return the level of a GM command
 *-----------------------------------------------
 */
int get_atcommand_level(const AtCommandType type) {
	int i;

	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
		if (atcommand_info[i].type == type)
			return atcommand_info[i].level;

	return 100; /* 100: command can not be used */
}

/*==========================================
 *is_atcommand - check for atcommand presence
 *------------------------------------------
 */
AtCommandType is_atcommand(const int fd, struct map_session_data* sd, const char* message, unsigned char gmlvl) {
	const char* str;
	const char* p;
	int s_flag;
	int i;
	char command[100];
	char received_command[100]; // to display on error

	nullpo_retr(AtCommand_None, sd);

	/* if muted, don't use GM command */
	if (!battle_config.allow_atcommand_when_mute && sd->sc_count && sd->sc_data[SC_NOCHAT].timer != -1)
		return AtCommand_Unknown;

	if (!message || !*message || strncmp(message, sd->status.name, strlen(sd->status.name)) != 0)
		return AtCommand_None;

	/* search start of GM command */
	str = message + strlen(sd->status.name);
	s_flag = 0;
	while (*str && (isspace(*str) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str || s_flag == 0) /* if no message or no ':' found */
		return AtCommand_None;

	/* check first char. */
	if (*str != command_symbol && *str != char_command_symbol && *str != main_channel_symbol)
		return AtCommand_None;

	/* get GM level */
	if (battle_config.atc_gmonly != 0 && !gmlvl)
		return AtCommand_None;

	/* extract gm command */
	p = str;
	while (*p && !isspace(*p))
		p++;
	i = p - str; /* speed up */
	if (i >= sizeof(command) - 5 || (i < 2 && *str != main_channel_symbol)) /* too long (-4 for @char + NULL, remote commands), or too short */
		return AtCommand_Unknown;
	memset(command, 0, sizeof(command));
	if (*str == char_command_symbol) {
		command[0] = command_symbol;
		command[1] = 'c';
		command[2] = 'h';
		command[3] = 'a';
		command[4] = 'r';
		strncpy(command + 5, str + 1, i - 1);
	} else if (*str == main_channel_symbol) {
		command[0] = command_symbol;
		command[1] = 'm';
		command[2] = 'a';
		command[3] = 'i';
		command[4] = 'n';
		p = str + 1;
	} else
		strncpy(command, str, i);
	// conserv received command to display on error
	memset(received_command, 0, sizeof(received_command));
	strncpy(received_command, str, i);
	/* for compare, reduce command in lowercase */
	for (i = 0; command[i]; i++)
		command[i] = tolower((unsigned char)command[i]); // tolower needs unsigned char

	// check for synonym here
	for (i = 0; i < synonym_count; i++) {
		if (strcmp(command + 1, synonym_table[i].synonym) == 0) {
			memset(command + 1, 0, sizeof(command) - 1); // don't change command_symbol (+1)
			strcpy(command + 1, synonym_table[i].command);
			break;
		}
	}

	/* search gm command type and check level*/
	i = 0;
	while (atcommand_info[i].type != AtCommand_Unknown) {
		if (strcmp(command + 1, atcommand_info[i].command + 1) == 0) {
			if (gmlvl < atcommand_info[i].level) {
				/* do loop until end for speed up */
				while (atcommand_info[i].type != AtCommand_Unknown)
					i++;
			}
			break;
		}
		i++;
	}
	/* if not found */
	if (atcommand_info[i].type == AtCommand_Unknown || atcommand_info[i].proc == NULL) {
		/* doesn't return Unknown if player is normal player (display the text, not display: unknown command) */
		if (gmlvl <= battle_config.atcommand_max_player_gm_level)
			return AtCommand_None;
		else {
			sprintf(atcmd_output, msg_txt(153), received_command); // %s is Unknown Command.
			clif_displaymessage(fd, atcmd_output);
			return AtCommand_Unknown;
		}
	}

	// check mingmlvl map flag
	if (gmlvl < (unsigned char)map[sd->bl.m].flag.mingmlvl) {
		switch (battle_config.mingmlvl_message) { // Which message do we send when a GM can use a command, but mingmlvl map flag block it?
		case 0: // Send no message (GM command is displayed like when GM speaks).
			return AtCommand_None;
		case 1: // Send 'unknown command'.
			sprintf(atcmd_output, msg_txt(153), received_command); // %s is Unknown Command.
			clif_displaymessage(fd, atcmd_output);
			return AtCommand_Unknown;
		case 2: // Send a special message (like: you are not authorized...).
		default:
			clif_displaymessage(fd, msg_txt(268)); // You're not authorized to use this command on this map. Sorry.
			return AtCommand_Unknown;
		}
	}

	/* search start of parameters */
	while (isspace(*p))
		p++;

	/* check parameter length */
	if (strlen(p) > 99) {
		clif_displaymessage(fd, msg_txt(49)); // Command with too long parameters.
		sprintf(atcmd_output, msg_txt(154), received_command); // %s failed.
		clif_displaymessage(fd, atcmd_output);
	} else if (atcommand_info[i].level >= map[sd->bl.m].flag.nogmcmd) {
		clif_displaymessage(fd, msg_txt(269)); // This map can't perform this GM command, sorry.
	/* do GM command */
	} else {
		command[0] = atcommand_info[i].command[0]; /* set correct first symbol for after (inside the function). */
		if (atcommand_info[i].proc(fd, sd, received_command, command, p) == 0) {
			// log command if necessary
			if (log_gm_level <= atcommand_info[i].level) {
				command[0] = command_symbol; /* to have correct display */
				log_atcommand(sd, command, p);
			}
		} else {
			/* Command can not be executed */
			command[0] = command_symbol; /* to have correct display */
			sprintf(atcmd_output, msg_txt(154), received_command); // %s failed.
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return atcommand_info[i].type;
}

/*==========================================
 *
 *------------------------------------------
 */
void atcommand_config_read(const char *cfgName) {
	static int read_counter = 0; // to count every time we enter in the function (for 'import' option)
	char line[512], w1[512], w2[512];
	int i, level;
	FILE* fp;

	/* init time of last spawn */
	last_spawn = time(NULL); /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */

	if ((fp = fopen(cfgName, "r")) == NULL) {
			printf("At commands configuration file not found: %s\n", cfgName);
			return;
	}

	read_counter++;

	while (fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')

		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%[^:]:%s", w1, w2) < 2)
			continue;
		for (i = 0; w1[i]; i++) /* speed up, only 1 lowercase for all loops */
			w1[i] = tolower((unsigned char)w1[i]); // tolower needs unsigned char
		/* searching gm command */
		for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
			if (strcmp(atcommand_info[i].command + 1, w1) == 0)
				break;
		if (atcommand_info[i].type != AtCommand_Unknown) {
			level = atoi(w2);
			if (level > 100)
				level = 100;
			else if (level < 0)
				level = 0;
			atcommand_info[i].level = level;
		} else if (strcmp(w1, "command_symbol") == 0) {
			if (!iscntrl((int)w2[0]) && // w2[0] > 31 &&
			    w2[0] != '/' && // symbol of standard ragnarok GM commands
			    w2[0] != '%' && // symbol of party chat speaking
			    w2[0] != char_command_symbol && // symbol of 'remote' GM commands
			    w2[0] != main_channel_symbol) // symbol of the Main channel
				command_symbol = w2[0];
		} else if (strcmp(w1, "char_command_symbol") == 0) {
			if (!iscntrl((int)w2[0]) && // w2[0] > 31 &&
			    w2[0] != '/' && // symbol of standard ragnarok GM commands
			    w2[0] != '%' && // symbol of party chat speaking
			    w2[0] != command_symbol && // symbol of 'normal' GM commands
			    w2[0] != main_channel_symbol) // symbol of the Main channel
				char_command_symbol = w2[0];
		} else if (strcmp(w1, "main_channel_symbol") == 0) {
			if (!iscntrl((int)w2[0]) && // w2[0] > 31 &&
			    w2[0] != '/' && // symbol of standard ragnarok GM commands
			    w2[0] != '%' && // symbol of party chat speaking
			    w2[0] != command_symbol && // symbol of 'normal' GM commands
			    w2[0] != char_command_symbol) // symbol of 'remote' GM commands
				main_channel_symbol = w2[0];
		} else if (strcmp(w1, "messages_filename") == 0) {
			memset(messages_filename, 0, sizeof(messages_filename));
			strncpy(messages_filename, w2, sizeof(messages_filename) - 1);
// import
		} else if (strcmp(w1, "import") == 0) {
			printf("atcommand_config_read: Import file: %s.\n", w2);
			atcommand_config_read(w2);
// unknown option/command
		} else {
			printf("Unknown GM command: '%s'.\n", w1);
		}
	}
	fclose(fp);

	printf("File '" CL_WHITE "%s" CL_RESET "' readed.\n", cfgName);

	read_counter--;
	if (read_counter == 0) {
		printf("Symbols:\n");
		printf("\t- '" CL_WHITE "%c" CL_RESET "' for GM commands.\n", command_symbol);
		printf("\t- '" CL_WHITE "%c" CL_RESET "' for char GM commands.\n", char_command_symbol);
		printf("\t- '" CL_WHITE "%c" CL_RESET "' for main channel.\n", main_channel_symbol);
	}

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
void atcommand_custom_free() {
	int i;

	for (i = 0; i < synonym_count; i++) {
		FREE(synonym_table[i].synonym);
		FREE(synonym_table[i].command);
	}
	FREE(synonym_table);

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
void atcommand_custom_read(const char *cfgName) {
	static int read_counter = 0; // to count every time we enter in the function (for 'import' option)
	int i;
	char line[512], w1[512], w2[512];
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
			printf("At commands configuration file not found: %s\n", cfgName);
			return;
	}

	// free synonym structure if it is first call (import files are possible)
	if (read_counter == 0)
		atcommand_custom_free();

	read_counter++;

	while (fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')

		line[sizeof(line) - 1] = '\0'; // be sure to have NULL at end
		if (strlen(line) > 99) // 100: max size of a GM command, but here, no command_symbol -> 99
			continue;
		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%[^:]:%s", w1, w2) < 2 &&
			  sscanf(line, "%[^,],%s", w1, w2) < 2 &&
			  sscanf(line, "%[^\t]\t%s", w1, w2) < 2 &&
			  sscanf(line, "%s %s", w1, w2) < 2)
			continue;
		for (i = 0; w1[i]; i++) /* speed up, only 1 lowercase for all loops */
			w1[i] = tolower((unsigned char)w1[i]); // tolower needs unsigned char
// import
		if (strcmp(w1, "import") == 0) {
			printf("%s: Import file: %s.\n", cfgName, w2);
			atcommand_custom_read(w2);
		} else {
			/* searching if synonym is not a gm command */
			for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
				if (strcmp(atcommand_info[i].command + 1, w1) == 0) {
					printf(CL_RED "Error in %s file:" CL_RESET "GM synonym '%s' is not a synonym, but a GM command.\n", cfgName, w1);
					break;
				}
			// if synonym is ok
			if (atcommand_info[i].type == AtCommand_Unknown) {
				/* searching if gm command exist */
				for (i = 0; w2[i]; i++) /* speed up, only 1 lowercase for all loops */
					w2[i] = tolower((unsigned char)w2[i]); // tolower needs unsigned char
				for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
					if (strcmp(atcommand_info[i].command + 1, w2) == 0) {
						// GM command found, create synonym
						//printf("new synonym: %s->%s\n", w1, w2);
						if (synonym_count == 0) {
							MALLOC(synonym_table, struct synonym_table, 1);
						} else {
							REALLOC(synonym_table, struct synonym_table, synonym_count + 1);
						}
						CALLOC(synonym_table[synonym_count].synonym, char, strlen(w1) + 1);
						strcpy(synonym_table[synonym_count].synonym, w1);
						CALLOC(synonym_table[synonym_count].command, char, strlen(w2) + 1);
						strcpy(synonym_table[synonym_count].command, w2);
						synonym_count++;
						break;
					}
				if (atcommand_info[i].type == AtCommand_Unknown) {
					printf(CL_RED "Error in %s file:" CL_RESET "GM command '%s' doesn't exist." CL_RESET "\n", cfgName, w2);
				}
			}
		}
	}
	fclose(fp);

	printf("File '" CL_WHITE "%s" CL_RESET "' readed.\n", cfgName);

	read_counter--;
	if (read_counter == 0) {
		switch (synonym_count) {
		case 0:
			printf("\tNo GM command synonym used.\n");
			break;
		case 1:
			printf("\t1 GM command synonym used.\n");
			break;
		default:
			printf("\t%d GM command synonyms used.\n", synonym_count);
			break;
		}
	}

	return;
}

//--------------------------------------------------
// Return the message string of the specified number
//--------------------------------------------------
char * msg_txt(int msg_number) {
	if (msg_number >= 0 && msg_number < MSG_NUMBER && msg_table[msg_number] != NULL && msg_table[msg_number][0] != '\0')
		return msg_table[msg_number];

	return "<no message>";
}

/*==========================================
 * Add Message in table
 *------------------------------------------
 */
void add_msg(int msg_number, const char *Message) {
	int len, i, var1, var2;

	len = strlen(Message);
	if (len > 0) {
		if (len > MAX_MSG_LEN)
			len = MAX_MSG_LEN;
		if (msg_config_read_done) { /* if already read, some checks */
			if (msg_table[msg_number] == NULL) {
				printf(CL_YELLOW "Warning: You add the message #%d. By default this message doesn't exist." CL_RESET "\n", msg_number);
			/* else, check number of % */
			} else {
				var1 = 0;
				for(i = 0; msg_table[msg_number][i]; i++)
					if (msg_table[msg_number][i] == '%') {
						if (msg_table[msg_number][i + 1] == '%') { /* if %% to display % */
							i++;
						} else {
							var1++;
						}
					}
				var2 = 0;
				for(i = 0; Message[i] && i <= len; i++)
					if (Message[i] == '%') {
						if (Message[i + 1] == '%') { /* if %% to display % */
							i++;
						} else {
							var2++;
						}
					}
				if (var1 != var2) {
					printf(CL_YELLOW "Warning: You add the message #%d with %d variable%s and replaced message have %d variable%s." CL_RESET " Note: a variable begin by %%.\n", msg_number, var2, (var2 > 1) ? "s" : "", var1, (var1 > 1) ? "s" : "");
				}
			}
		}
		/* add new message */
		FREE(msg_table[msg_number]);
		CALLOC(msg_table[msg_number], char, len + 1); // + NULL
		strncpy(msg_table[msg_number], Message, len);
	} else {
		printf(CL_RED "Error in conf file: You try to add the void message #%d." CL_RESET "\n", msg_number);
	}

	return;
}

/*==========================================
 * Set default messages
 *------------------------------------------
 */
void set_default_msg() {
	// Messages of GM commands
	add_msg(  0, "Warped.");
	add_msg(  1, "Map not found.");
	add_msg(  2, "Coordinates out of range.");
	add_msg(  3, "Character not found.");
	add_msg(  4, "Jump to '%s'.");
	add_msg(  5, "Jump to %d %d");
	add_msg(  6, "Character data respawn point saved.");
	add_msg(  7, "Warping to respawn point.");
	add_msg(  8, "Speed changed.");
	add_msg(  9, "Options changed.");
	add_msg( 10, "Invisible: Off.");
	add_msg( 11, "Invisible: On.");
	add_msg( 12, "Your job has been changed.");
	add_msg( 13, "A pity! You've died.");
	add_msg( 14, "Character killed.");
	add_msg( 15, "Player warped (message sends to player too).");
	add_msg( 16, "You've been revived! It's a miracle!");
	add_msg( 17, "HP, SP recovered.");
	add_msg( 18, "Item created.");
	add_msg( 19, "Invalid item ID or name.");
	add_msg( 20, "All of your inventory items have been removed.");
	add_msg( 21, "Base level raised.");
	add_msg( 22, "Base level lowered.");
	add_msg( 23, "Job level can't go any higher.");
	add_msg( 24, "Job level raised.");
	add_msg( 25, "Job level lowered.");
	add_msg( 26, "Help commands:");
	add_msg( 27, "File 'help.txt' not found.");
	add_msg( 28, "No player found.");
	add_msg( 29, "1 player found.");
	add_msg( 30, "%d players found.");
	add_msg( 31, "PvP: Off.");
	add_msg( 32, "PvP: On.");
	add_msg( 33, "GvG: Off.");
	add_msg( 34, "GvG: On.");
	add_msg( 35, "You can't use this clothes color with this class.");
	add_msg( 36, "Appearence changed.");
	add_msg( 37, "An invalid number was specified.");
	add_msg( 38, "Invalid location number or name.");
	add_msg( 39, "All monsters summoned!");
	add_msg( 40, "Invalid monster ID or name.");
	add_msg( 41, "Impossible to decrease the number/value.");
	add_msg( 42, "Stat changed.");
	add_msg( 43, "You're not in a guild.");
	add_msg( 44, "You're not the master of your guild.");
	add_msg( 45, "Guild level change failed.");
	add_msg( 46, "'%s' recalled!");
	add_msg( 47, "Base level can't go any higher.");
	add_msg( 48, "Character's job changed.");
	add_msg( 49, "Command with too long parameters.");
	add_msg( 50, "You already have some GM powers.");
	add_msg( 51, "Character revived.");
	add_msg( 52, "This option cannot be used in PK Mode.");
	add_msg( 53, "'%s' (account: %d) stats:");
	add_msg( 54, "No player found in map '%s'.");
	add_msg( 55, "1 player found in map '%s'.");
	add_msg( 56, "%d players found in map '%s'.");
	add_msg( 57, "Character's respawn point changed.");
	add_msg( 58, "Character's options changed.");
	add_msg( 59, "Night has fallen.");
	add_msg( 60, "Day has arrived.");
	add_msg( 61, "The holy messenger has given judgement.");
	add_msg( 62, "Judgement was made.");
	add_msg( 63, "Mercy has been shown.");
	add_msg( 64, "Mercy has been granted.");
	add_msg( 65, "Character's base level raised.");
	add_msg( 66, "Character's base level lowered.");
	add_msg( 67, "Character's job level can't go any higher.");
	add_msg( 68, "character's job level raised.");
	add_msg( 69, "Character's job level lowered.");
	add_msg( 70, "You have learned the skill.");
	add_msg( 71, "You have forgotten the skill.");
	add_msg( 72, "Guild siege warfare start!");
	add_msg( 73, "Already it has started siege warfare.");
	add_msg( 74, "Guild siege warfare end!");
	add_msg( 75, "Siege warfare hasn't started yet.");
	add_msg( 76, "You have received all skills.");
	add_msg( 77, "The reference result of '%s' (name: id):");
	add_msg( 78, "%s: %d");
	add_msg( 79, "It is %d affair above.");
	add_msg( 80, "Give a display name and monster name/id please.");
	add_msg( 81, "Your GM level don't authorize you to do this action on this player.");
	add_msg( 82, "Please, use one of this numbers/names:");
	add_msg( 83, "Cannot spawn emperium.");
	add_msg( 84, "All stats changed!");
	add_msg( 85, "Invalid time for ban command.");
	add_msg( 86, "Sorry, but a player name has at least 4 characters.");
	add_msg( 87, "Sorry, but a player name has 23 characters maximum.");
	add_msg( 88, "Character name sends to char-server to ask it.");
	add_msg( 89, "Sorry, it's already the night. Impossible to execute the command.");
	add_msg( 90, "Sorry, it's already the day. Impossible to execute the command.");
	add_msg( 91, "Character's base level can't go any higher.");
	add_msg( 92, "All characters recalled!");
	add_msg( 93, "All online characters of the '%s' guild are near you.");
	add_msg( 94, "Incorrect name/ID, or no one from the guild is online.");
	add_msg( 95, "All online characters of the '%s' party are near you.");
	add_msg( 96, "Incorrect name or ID, or no one from the party is online.");
	add_msg( 97, "Item database reloaded.");
	add_msg( 98, "Monster database reloaded.");
	add_msg( 99, "Skill database reloaded.");
	add_msg(100, "Scripts reloaded.");
	add_msg(101, "Login-server asked to reload GM accounts and their level.");
	add_msg(102, "Mounted Peco.");
	add_msg(103, "No longer spying on the '%s' guild.");
	add_msg(104, "Spying on the '%s' guild.");
	add_msg(105, "No longer spying on the '%s' party.");
	add_msg(106, "Spying on the '%s' party.");
	add_msg(107, "All items have been repaired.");
	add_msg(108, "No item need to be repaired.");
	add_msg(109, "Player has been nuked!");
	add_msg(110, "Npc Enabled.");
	add_msg(111, "This NPC doesn't exist.");
	add_msg(112, "Npc Disabled.");
	add_msg(113, "%d item(s) removed by a GM.");
	add_msg(114, "%d item(s) removed from the player.");
	add_msg(115, "%d item(s) removed. Player had only %d on %d items.");
	add_msg(116, "Character does not have the item.");
	add_msg(117, "GM has send you in jails for %s.");
	add_msg(118, "Player warped in jails for %s.");
	add_msg(119, "This player is not in jails.");
	add_msg(120, "GM has discharge you.");
	add_msg(121, "Player warped to Prontera.");
	add_msg(122, "Disguise applied.");
	add_msg(123, "Monster/NPC name/id hasn't been found.");
	add_msg(124, "Undisguise applied.");
	add_msg(125, "You're not disguised.");
	add_msg(126, "You accept any wisp (no wisper is refused).");
	add_msg(127, "You accept any wisp, except thoses from %d player(s):");
	add_msg(128, "You refuse all wisps (no specifical wisper is refused).");
	add_msg(129, "You refuse all wisps, AND refuse wisps from %d player(s):");
	add_msg(130, "'%s' accept any wisp (no wisper is refused).");
	add_msg(131, "'%s' accept any wisp, except thoses from %d player(s):");
	add_msg(132, "'%s' refuse all wisps (no specifical wisper is refused).");
	add_msg(133, "'%s' refuse all wisps, AND refuse wisps from %d player(s):");
	add_msg(134, "'%s' already accepts all wispers.");
	add_msg(135, "'%s' now accepts all wispers.");
	add_msg(136, "A GM has authorized all wispers for you.");
	add_msg(137, "'%s' already blocks all wispers.");
	add_msg(138, "'%s' blocks now all wispers.");
	add_msg(139, "A GM has blocked all wispers for you.");
	add_msg(140, "Character's disguise applied.");
	add_msg(141, "Character's undisguise applied.");
	add_msg(142, "Character is not disguised.");
	add_msg(143, "Give a monster name/id please.");
	add_msg(144, "Invalid actual email. If you have default e-mail, type a@a.com.");
	add_msg(145, "Invalid new email. Please enter a real e-mail.");
	add_msg(146, "New email must be a real e-mail.");
	add_msg(147, "New email must be different of the actual e-mail.");
	add_msg(148, "Information sent to login-server via char-server.");
	add_msg(149, "Impossible to increase the number/value.");
	add_msg(150, "No GM found.");
	add_msg(151, "1 GM found.");
	add_msg(152, "%d GMs found.");
	add_msg(153, "%s is Unknown Command.");
	add_msg(154, "%s failed.");
	add_msg(155, "Impossible to change your job.");
	add_msg(156, "HP or/and SP modified.");
	add_msg(157, "HP and SP are already with the good values.");
	add_msg(158, "Base level can't go any lower.");
	add_msg(159, "Job level can't go any lower.");
	add_msg(160, "PvP is already Off.");
	add_msg(161, "PvP is already On.");
	add_msg(162, "GvG is already Off.");
	add_msg(163, "GvG is already On.");
	add_msg(164, "Your memo point #%d doesn't exist.");
	add_msg(165, "All monsters killed!");
	add_msg(166, "No item has been refined!");
	add_msg(167, "1 item has been refined!");
	add_msg(168, "%d items have been refined!");
	add_msg(169, "This item (%d: '%s') is not an equipment.");
	add_msg(170, "This item is not an equipment.");
	add_msg(171, "%d - void");
	add_msg(172, "You replace previous memo position %d - %s (%d,%d).");
	add_msg(173, "Note: you don't have the 'Warp' skill level to use it.");
	add_msg(174, "Number of status points changed!");
	add_msg(175, "Number of skill points changed!");
	add_msg(176, "Number of zenys changed!");
	add_msg(177, "Impossible to decrease a stat.");
	add_msg(178, "Impossible to increase a stat.");
	add_msg(179, "Guild level changed.");
	add_msg(180, "The monter/egg name/id doesn't exist.");
	add_msg(181, "You already have a pet.");
	add_msg(182, "Pet friendly value changed!");
	add_msg(183, "Pet friendly is already the good value.");
	add_msg(184, "Sorry, but you have no pet.");
	add_msg(185, "Pet hungry value changed!");
	add_msg(186, "Pet hungry is already the good value.");
	add_msg(187, "You can now rename your pet.");
	add_msg(188, "You can already rename your pet.");
	add_msg(189, "This player can now rename his/her pet.");
	add_msg(190, "This player can already rename his/her pet.");
	add_msg(191, "Sorry, but this player has no pet.");
	add_msg(192, "Impossible to change the character's job.");
	add_msg(193, "Character's base level can't go any lower.");
	add_msg(194, "Character's job level can't go any lower.");
	add_msg(195, "All players have been kicked!");
	add_msg(196, "You already have this quest skill.");
	add_msg(197, "This skill number doesn't exist or isn't a quest skill.");
	add_msg(198, "This skill number doesn't exist.");
	add_msg(199, "This player has learned the skill.");
	add_msg(200, "This player already has this quest skill.");
	add_msg(201, "You don't have this quest skill.");
	add_msg(202, "This player has forgotten the skill.");
	add_msg(203, "This player doesn't have this quest skill.");
	add_msg(204, "WARNING: more than 400 spiritballs can CRASH your client!");
	add_msg(205, "You already have this number of spiritballs.");
	add_msg(206, "'%s' skill points reseted!");
	add_msg(207, "'%s' stats points reseted!");
	add_msg(208, "'%s' skill and stats points reseted!");
	add_msg(209, "Character's number of skill points changed!");
	add_msg(210, "Character's number of status points changed!");
	add_msg(211, "Character's number of zenys changed!");
	add_msg(212, "Cannot mount a Peco while in disguise.");
	add_msg(213, "You can not mount a peco with your job.");
	add_msg(214, "Unmounted Peco.");
	add_msg(215, "This player cannot mount a Peco while in disguise.");
	add_msg(216, "Now, this player mounts a peco.");
	add_msg(217, "This player can not mount a peco with his/her job.");
	add_msg(218, "Now, this player has not more peco.");
	add_msg(219, "%d day");
	add_msg(220, "%d days");
	add_msg(221, "%s %d hour");
	add_msg(222, "%s %d hours");
	add_msg(223, "%s %d minute");
	add_msg(224, "%s %d minutes");
	add_msg(225, "%s and %d second");
	add_msg(226, "%s and %d seconds");
	add_msg(227, "Cannot wear disguise while riding a Peco.");
	add_msg(228, "Character cannot wear disguise while riding a Peco.");
	add_msg(229, "Your Effect Has Changed.");
	add_msg(230, "Server time (normal time): %A, %B %d %Y %X.");
	add_msg(231, "Game time: The game is in permanent daylight.");
	add_msg(232, "Game time: The game is in permanent night.");
	add_msg(233, "Game time: The game is actualy in night for %s.");
	add_msg(234, "Game time: After, the game will be in permanent daylight.");
	add_msg(235, "Game time: The game is actualy in daylight for %s.");
	add_msg(236, "Game time: After, the game will be in permanent night.");
	add_msg(237, "Game time: After, the game will be in night for %s.");
	add_msg(238, "Game time: A day cycle has a normal duration of %s.");
	add_msg(239, "Game time: After, the game will be in daylight for %s.");
	add_msg(240, "%d monster(s) summoned!");
	add_msg(241, "All inventory items of the player have been removed.");
	add_msg(242, "All your items have been checked.");
	add_msg(243, "Map skills are off.");
	add_msg(244, "Map skills are on.");
	add_msg(245, "Server Uptime: %s.");
	add_msg(246, "Message of the day:");
	add_msg(247, "Your display option (HP of players) is now set to OFF.");
	add_msg(248, "Your display option (HP of players) is now set to ON.");
	add_msg(249, "Your display experience option is now set to OFF.");
	add_msg(250, "Your display experience option is now set to ON.");
	add_msg(251, "Your request has not been sent. Request system is disabled.");
	add_msg(252, "Your request has been sent. If there are no GMs online, your request is lost.");
	add_msg(253, "(map message)");

	add_msg(254, "List of monsters (with current drop rate) that drop '%s (%s)' (id: %d):");
	add_msg(255, "No monster drops item '%s (%s)' (id: %d).");
	add_msg(256, "1 monster drops item '%s (%s)' (id: %d).");
	add_msg(257, "%d monsters drop item '%s (%s)' (id: %d).");

	add_msg(258, "Invalid party name. Party name must have between 1 to 24 characters.");
	add_msg(259, "Invalid guild name. Guild name must have between 1 to 24 characters.");

	add_msg(260, "You're now a killer.");
	add_msg(261, "You're no longer a killer.");
	add_msg(262, "The player is now a killer.");
	add_msg(263, "The player is no longer a killer.");
	add_msg(264, "You're now killable.");
	add_msg(265, "You're no longer killable.");
	add_msg(266, "The player is now killable.");
	add_msg(267, "The player is no longer killable.");

	add_msg(268, "You're not authorized to use this command on this map. Sorry.");
	add_msg(269, "This map can't perform this GM command, sorry.");

	add_msg(270, "Your current autoloot option is disabled.");
	add_msg(271, "Your current autoloot option is set to get rate between 0 to %0.02f%%.");
	add_msg(272, "Your auto loot option for looted items is now set to OFF.");
	add_msg(273, "Your auto loot option for looted items is now set to ON.");
	add_msg(274, "And you get drops of looted items.");
	add_msg(275, "And you don't get drops of looted items.");
	add_msg(276, "Invalid drop rate. Value must be between 0 (no autoloot) to 100 (autoloot all drops).");
	add_msg(277, "Autoloot of monster drops disabled.");
	add_msg(278, "Set autoloot of monster drops when they are between 0 to %0.02f%%.");

	add_msg(279, "Invalid coordinates (can't move on).");
	add_msg(280, "Invalid coordinates (a NPC is already at this position).");
	add_msg(281, "NPC moved.");

	// Messages of others (not for GM commands)
	add_msg(500, "Actually, it's the night...");
	add_msg(501, "Your account time limit is: %d-%m-%Y %H:%M:%S.");
	add_msg(502, "The day has arrived!");
	add_msg(503, "The night has fallen...");
	add_msg(504, "Hack on global message (normal message): character '%s' (account: %d) uses an other name.");
	add_msg(505, " This player sends a void name and a void message.");
	add_msg(506, " This player sends (name:message): '%s'.");
	add_msg(507, " This player has been banned for %d minute(s).");
	add_msg(508, " This player hasn't been banned (Ban option is disabled).");
	add_msg(509, "You can not page yourself. Sorry.");
	add_msg(510, "Unknown monster or item.");
	add_msg(511, "Muting is disabled.");
	add_msg(512, "It's impossible to block this player.");
	add_msg(513, "It's impossible to unblock this player.");
	add_msg(514, "This player is already blocked.");
	add_msg(515, "Character '%s' (account: %d) has tried AGAIN to block wisps from '%s' (wisp name of the server). Bot user? Check please.");
	add_msg(516, "Character '%s' (account: %d) has tried to block wisps from '%s' (wisp name of the server). Bot user?");
	add_msg(517, "Add me in your ignore list, doesn't block my wisps.");
	add_msg(518, "You can not block more people.");
	add_msg(519, "This player is not blocked by you.");
	add_msg(520, "You already block everyone.");
	add_msg(521, "You already allow everyone.");
	add_msg(522, "This name (for a friend) doesn't exist.");
	add_msg(523, "This player is already a friend.");
	add_msg(524, "Please wait. This player must already answer to an invitation.");
	add_msg(525, "Friend name not found in list.");
	add_msg(526, "Change sex failed. Account %d not found.");
	add_msg(527, "Sex of '%s' changed.");
	add_msg(528, "Sex of account %d changed.");
	add_msg(529, "You cannot wear disguise when riding a Peco.");
	add_msg(530, "Item has been repaired.");
	add_msg(531, "%s stole an unknown item.");
	add_msg(532, "%s stole a(n) %s.");
	add_msg(533, "%s has not stolen the item because of being overweight.");
	add_msg(534, "You cannot mount a peco while in disguise.");
	add_msg(535, "Alliances cannot be made during Guild Wars!");
	add_msg(536, "Alliances cannot be broken during Guild Wars!");
	add_msg(537, "You broke target's weapon.");
	add_msg(538, "Hack on trade: character '%s' (account: %d) try to trade more items that he has.");
	add_msg(539, "This player has %d of a kind of item (id: %d), and try to trade %d of them.");
	add_msg(540, " This player has been definitivly blocked.");
	add_msg(541, "(to GM >= %d) ");
	add_msg(542, "The player '%s' doesn't exist.");
	add_msg(543, "Login-server has been asked to block the player '%s'.");
	add_msg(544, "Your GM level don't authorize you to block the player '%s'.");
	add_msg(545, "Login-server is offline. Impossible to block the player '%s'.");
	add_msg(546, "Login-server has been asked to ban the player '%s'.");
	add_msg(547, "Your GM level don't authorize you to ban the player '%s'.");
	add_msg(548, "Login-server is offline. Impossible to ban the player '%s'.");
	add_msg(549, "Login-server has been asked to unblock the player '%s'.");
	add_msg(550, "Your GM level don't authorize you to unblock the player '%s'.");
	add_msg(551, "Login-server is offline. Impossible to unblock the player '%s'.");
	add_msg(552, "Login-server has been asked to unban the player '%s'.");
	add_msg(553, "Your GM level don't authorize you to unban the player '%s'.");
	add_msg(554, "Login-server is offline. Impossible to unban the player '%s'.");
	add_msg(555, "Login-server has been asked to change the sex of the player '%s'.");
	add_msg(556, "Your GM level don't authorize you to change the sex of the player '%s'.");
	add_msg(557, "Login-server is offline. Impossible to change the sex of the player '%s'.");
	add_msg(558, "GM modification success.");
	add_msg(559, "Failure of GM modification.");
	add_msg(560, "Your sex has been changed (need disconnection by the server)...");
	add_msg(561, "Your account has been deleted (disconnection)...");
	add_msg(562, "Your account has 'Unregistered'.");
	add_msg(563, "Your account has an 'Incorrect Password'...");
	add_msg(564, "Your account has expired.");
	add_msg(565, "Your account has been rejected from server.");
	add_msg(566, "Your account has been blocked by the GM Team.");
	add_msg(567, "Your Game's EXE file is not the latest version.");
	add_msg(568, "Your account has been prohibited to log in.");
	add_msg(569, "Server is jammed due to over populated.");
	add_msg(570, "Your account has not more authorized.");
	add_msg(571, "Your account has been totally erased.");
	add_msg(572, "Your account has been banished until ");
	add_msg(573, "%m-%d-%Y %H:%M:%S");

	// mail system
	//----------------------
	add_msg(574, "You have no message.");
	add_msg(575, "%d - From : %s (New - Priority)");
	add_msg(576, "%d - From : %s (New)");
	add_msg(577, "%d - From : %s");
	add_msg(578, "You have %d new messages.");
	add_msg(579, "You have %d unread priority messages.");
	add_msg(580, "You have no new message.");
	add_msg(581, "Message not found.");
	add_msg(582, "Reading message from %s");
	add_msg(583, "Cannot delete unread priority message.");
	add_msg(584, "You have recieved new messages, use @listmail before deleting.");
	add_msg(585, "Message deleted.");
	add_msg(586, "You must wait 10 minutes before sending another message.");
	add_msg(587, "Access Denied.");
	add_msg(588, "Character does not exist.");
	add_msg(589, "Message has been sent.");
	add_msg(590, "You have new message.");

	add_msg(591, "Don't flood server with emotion icons, please.");
	add_msg(592, "Trade can not be done, because one of your doesn't have enough free slots in its inventory.");
	add_msg(593, "(to all players) ");
	add_msg(594, "Experience gained: Base:%d, Job:%d.");
	add_msg(595, "trade done");
	add_msg(596, "gives");
	add_msg(597, "lvl");
	add_msg(598, "card(s)");
	add_msg(599, "1 zeny trades by this player.");
	add_msg(600, "zenys trade by this player.");
	add_msg(601, "No zeny trades by this player.");
	add_msg(602, "Nothing trades by this player.");
	add_msg(603, "(Main) '%s': ");
	add_msg(604, "(Request from '%s'): ");

	// Supernovice's Guardian Angel
	// actually. new client msgtxt file contains these 3 lines...
	add_msg(605, "Guardian Angel, can you hear my voice? ^^;");
	add_msg(606, "My name is %s, and I'm a Super Novice~");
	add_msg(607, "Please help me~ T.T");

	add_msg(608, "Your Main channel is OFF.");
	add_msg(609, "Your Main channel is ON.");
	add_msg(610, "Experience gained: Base:%d (%2.02f%%), Job:%d (%2.02f%%).");
	add_msg(611, "Unable to read motd.txt file.");
	add_msg(612, "You are not active. In order to provide better service to other players, you will be disconnected in 30 seconds.");
	add_msg(613, "You were disconnected because of your inactivity.");

	add_msg(614, "Vending done for %d z");
	add_msg(615, "Vendor");
	add_msg(616, "Buyer");
	add_msg(617, "price: %d x %d z = %d z.");
	add_msg(618, "price: %d z.");

	add_msg(619, "Your e-mail has NOT been changed (impossible to change it).");
	add_msg(620, "Your e-mail has been changed to '%s'.");

	add_msg(621, "Possible use of BOT (99%% of chance) or modified client by '%s' (account: %d, char_id: %d). This player ask your name when you are hidden.");
	add_msg(622, "Character '%s' (account: %d) try to use a bot (it tries to detect a fake player).");
	add_msg(623, "Character '%s' (account: %d) try to use a bot (it tries to detect a fake mob).");

	add_msg(624, "You have been discharged.");
	add_msg(625, "Invalid time for jail command.");
	add_msg(626, "Remember, you are in jails.");
	add_msg(627, "Invalid final time for jail command.");
	add_msg(628, "No change done for jail time of this player.");
	add_msg(629, "Jail time of the player mofified by %+d second%s.");
	add_msg(630, "Map error, please reconnect.");
	add_msg(631, "%s has been discharged from jail.");
	add_msg(632, "A GM has discharged %s from jail.");
	add_msg(633, "Character is now in jail for %s.");
	add_msg(634, "Your jail time has been mofified by %+d second%s.");
	add_msg(635, "You are now in jail for %s.");
	add_msg(636, "You are actually in jail for %s.");
	add_msg(637, "You are not in jail.");
	add_msg(638, "This player is actually in jail for %s.");
	add_msg(639, "This player is not in jail.");
	add_msg(640, "%s has been put in jail for %s.");

	add_msg(641, " This player hasn't been banned.");
	add_msg(642, "Possible hack on global message (normal message): character '%s' (account: %d) tries to send a big message (%d characters).");
	add_msg(643, "Possible hack on wisp message (PM): character '%s' (account: %d) tries to send a big message (%d characters).");
	add_msg(644, "Possible hack on GM message: character '%s' (account: %d) tries to send a big message (%d characters).");
	add_msg(645, "Possible hack on local GM message: character '%s' (account: %d) tries to send a big message (%d characters).");
	add_msg(646, "Possible hack on party message: character '%s' (account: %d) tries to send a big message (%d characters).");
	add_msg(647, "Possible hack on guild message: character '%s' (account: %d) tries to send a big message (%d characters).");

	add_msg(648, "A merchant with a vending shop can not join a chat, sorry.");

	add_msg(649, "You have too much zenys. You can not get more zenys.");

	add_msg(650, "Rudeness is not authorized in this game.");
	add_msg(651, "Rudeness is not a good way to have fun in this game.");

	add_msg(652, "Hack on party message: character '%s' (account: %d) uses an other name.");
	add_msg(653, "Hack on guild message: character '%s' (account: %d) uses an other name.");

	add_msg(654, "You are not authorized to use '\"' in a guild name.");

	add_msg(655, "Sorry, but trade is not allowed on this map.");

	add_msg(656, "Monster dropped %d %s (drop rate: %%%0.02f).");
	add_msg(657, "Invalid drop rate. Value must be between 0 (no display) to 100 (display all drops).");
	add_msg(658, "Set displaying of monster drops when they are between 0 to %0.02f%%.");
	add_msg(659, "Displaying of monster drops disabled.");
	add_msg(660, "Monster dropped %d %s (it was a loot).");
	add_msg(661, "Your current display drop option is disabled.");
	add_msg(662, "Your current display drop option is set to display rate between 0 to %0.02f%%.");

	add_msg(663, "You display now drops of looted items.");
	add_msg(664, "You don't more display drops of looted items.");
	add_msg(665, "And you display drops of looted items.");
	add_msg(666, "And you don't display drops of looted items.");

	add_msg(667, "Server has adjusted your speed to your follow target.");

	add_msg(668, "(From %s) The War Of Emperium is running...");

	add_msg(669, "%s has broken.");

	add_msg(670, "You can't create a chat when you are in one.");

	add_msg(671, "Login-server has been asked to change the GM level of the player '%s'.");
	add_msg(672, "Your GM level don't authorize you to change the GM level of the player '%s'.");
	add_msg(673, "Login-server is offline. Impossible to change the GM level of the player '%s'.");
	add_msg(674, "Player (account: %d) that you want to change the GM level doesn't exist.");
	add_msg(675, "You are not authorised to change the GM level of this player (account: %d).");
	add_msg(676, "The player (account: %d) already has the specified GM level.");
	add_msg(677, "GM level of the player (account: %d) changed to %d.");

	add_msg(678, "For the record: War of Emperium is actually running. Because you are member of a guild, you can not use 'Main channel'.");
	add_msg(679, "War of Emperium is actually running. Because you are member of a guild, you can not use 'Main channel'.");

	return;
}

/*==========================================
 * Read Message Data
 *------------------------------------------
 */
int msg_config_read(const char *cfgName) {
	int msg_number;
	char line[MAX_MSG_LEN], w1[MAX_MSG_LEN], w2[MAX_MSG_LEN];
	int i;
	FILE *fp;

	if (msg_config_read_done == 0) {
		memset(&msg_table[0], 0, sizeof(msg_table[0]) * MSG_NUMBER);
		set_default_msg();
		msg_config_read_done = 1;
	}

	if ((fp = fopen(cfgName, "r")) == NULL) {
//		if ((fp = fopen("conf/msg_athena.conf", "r")) == NULL) { // not try default, possible infinite loop with import
			printf("Messages file not found: %s.\n", cfgName);
			// prepare check for supernovice angel
			for (i = 0; msg_table[605][i]; i++) // Guardian Angel, can you hear my voice? ^^;
				msg_table[605][i] = tolower((unsigned char)msg_table[605][i]); // tolower needs unsigned char
			for (i = 0; msg_table[607][i]; i++) // Please help me~ T.T
				msg_table[607][i] = tolower((unsigned char)msg_table[607][i]); // tolower needs unsigned char
			return 1;
//		}
	}

	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		/* remove carriage return if exist */
		while(line[0] != '\0' && (line[(i = strlen(line) - 1)] == '\n' || line[i] == '\r'))
			line[i] = '\0';
		// it's not necessary to remove 'carriage return ('\n' or '\r')

		memset(w2, 0, sizeof(w2));
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
// import
			if (strcasecmp(w1, "import") == 0) {
				printf("msg_config_read: Import file: %s.\n", w2);
				msg_config_read(w2);
			} else {
				msg_number = atoi(w1);
				if (msg_number >= 0 && msg_number < MSG_NUMBER) {
					add_msg(msg_number, w2);
				//	printf("Message #%d: '%s'.\n", msg_number, msg_table[msg_number]);
				}
			}
		}
	}
	fclose(fp);

	// prepare check for supernovice angel
	for (i = 0; msg_table[605][i]; i++) // Guardian Angel, can you hear my voice? ^^;
		msg_table[605][i] = tolower((unsigned char)msg_table[605][i]); // tolower needs unsigned char
	for (i = 0; msg_table[607][i]; i++) // Please help me~ T.T
		msg_table[607][i] = tolower((unsigned char)msg_table[607][i]); // tolower needs unsigned char

	printf("File '" CL_WHITE "%s" CL_RESET "' readed.\n", cfgName);

	return 0;
}

/*==========================================
 * Free Message Data
 *------------------------------------------
 */
void do_final_msg_config() {
	int msg_number;

	for (msg_number = 0; msg_number < MSG_NUMBER; msg_number++) {
		FREE(msg_table[msg_number]);
	}

	return;
}

/*==========================================
// @ command processing functions
 *------------------------------------------
 */

/*==========================================
 *
 *------------------------------------------
 */
static int atkillmonster_sub(struct block_list *bl, va_list ap) {
	int flag = va_arg(ap, int);

	nullpo_retr(0, bl);

	if (flag)
		mob_damage(NULL, (struct mob_data *)bl, ((struct mob_data *)bl)->hp, 2);
	else
		mob_delete((struct mob_data *)bl);

	return 0;
}

/*==========================================
 * to send usage of a GM command to a player
 *------------------------------------------
 */
void send_usage(int fd, char *fmt, ...) {
	va_list ap;
	char tmpstr[2048];

	va_start(ap, fmt);

	vsprintf(tmpstr, fmt, ap);
	clif_displaymessage(fd, tmpstr);

	va_end(ap);

	return;
}

/*==========================================
 * @rura+
 *------------------------------------------
 */
ATCOMMAND_FUNC(rurap) {
	int x = 0, y = 0;
	struct map_session_data *pl_sd;
	int m;

	if (!message || !*message || (sscanf(message, "%s %d %d %[^\n]", atcmd_mapname, &x, &y, atcmd_name) < 4 &&
	                              sscanf(message, "%[^, ],%d,%d %[^\n]", atcmd_mapname, &x, &y, atcmd_name) < 4)) {
		send_usage(fd, "Usage: %s <mapname> <x> <y> <char name|account_id>", original_command);
		return -1;
	}

	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same GM level
			if ((m = map_checkmapname(atcmd_mapname)) == -1) { // if map doesn't exist in all map-servers
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
			if (m >= 0) { // if on this map-server
				if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
					clif_displaymessage(fd, "You are not authorized to warp someone to this map.");
					return -1;
				}
				if (x < 1)
					x = rand() % (map[m].xs - 2) + 1;
				else if (x >= map[m].xs)
					x = map[m].xs - 1;
				if (y < 1)
					y = rand() % (map[m].ys - 2) + 1;
				else if (y >= map[m].ys)
					y = map[m].ys - 1;
			}
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp this player from its actual map.");
				return -1;
			}
			if (pc_setpos(pl_sd, atcmd_mapname, x, y, 3) == 0) {
				clif_displaymessage(pl_sd->fd, msg_txt(0)); // Warped.
				if (pl_sd != sd)
					clif_displaymessage(fd, msg_txt(15)); // Player warped (message sends to player too).
			} else {
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

// @rura
/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(rura) {
	int x = 0, y = 0;
	int m;

	if (!message || !*message || (sscanf(message, "%[^, ],%d,%d", atcmd_mapname, &x, &y) < 3 &&
	                              sscanf(message, "%s %d %d", atcmd_mapname, &x, &y) < 1)) {
		send_usage(fd, "Please, enter a map (usage: %s <mapname> <x> <y>).", original_command);
		return -1;
	}

	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if ((m = map_checkmapname(atcmd_mapname)) == -1) { // if map doesn't exist in all map-servers
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	if (m >= 0) { // if on this map-server
		if (x < 1)
			x = rand() % (map[m].xs - 2) + 1;
		else if (x >= map[m].xs)
			x = map[m].xs - 1;
		if (y < 1)
			y = rand() % (map[m].ys - 2) + 1;
		else if (y >= map[m].ys)
			y = map[m].ys - 1;

		if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
			clif_displaymessage(fd, "You are not authorized to warp you to this map.");
			return -1;
		}
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
		return -1;
	}

	if (pc_setpos(sd, atcmd_mapname, x, y, 3) == 0)
		clif_displaymessage(fd, msg_txt(0)); // Warped.
	else {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(where) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, sd->status.name, 24);
	}

	if (((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) &&
	    !((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
		sprintf(atcmd_output, "%s: %s (%d,%d)", pl_sd->status.name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(jumpto) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp you to the map of this player.");
				return -1;
			}
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
				return -1;
			}
			if (pc_setpos(sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3) == 0) { /* not found ? */
				sprintf(atcmd_output, msg_txt(4), pl_sd->status.name); // Jump to '%s'.
				clif_displaymessage(fd, atcmd_output);
			} else {
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(3)); // Character not found.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(jump) {
	int x = 0, y = 0;

	sscanf(message, "%d %d", &x, &y);

	if (x < 1)
		x = rand() % (map[sd->bl.m].xs - 2) + 1;
	else if (x >= map[sd->bl.m].xs)
		x = map[sd->bl.m].xs - 1;
	if (y < 1)
		y = rand() % (map[sd->bl.m].ys - 2) + 1;
	else if (y >= map[sd->bl.m].ys)
		y = map[sd->bl.m].ys - 1;

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp you to your actual map.");
		return -1;
	}
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
		return -1;
	}

	pc_setpos(sd, sd->mapname, x, y, 3);
	sprintf(atcmd_output, msg_txt(5), x, y); // Jump to %d %d
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @users
 * simplified by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(users) {
	char buf[50];
	int i, m, users_all;
	struct map_session_data *pl_sd;
	int *map_users;

	// Addition of all players
	users_all = 0;
	CALLOC(map_users, int, map_num);
	for (i = 0; i < fd_max; i++)
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) // only lower or same level
				if ((m = pl_sd->bl.m) >= 0 && m < map_num) {
					map_users[m]++;
					users_all++;
				}

	// display number of players in all map
	for(m = 0; m < map_num; m++)
		if (map_users[m] > 0) {
			if (m == sd->bl.m)
				sprintf(buf, "%s : %d (%02.02f%%) - your map", map[m].name, map_users[m], ((float)map_users[m] * 100. / (float)users_all)); // 16 + 3 + 4 (1000) + 2 + 6 (100.00) + 13 + 1 (NULL) = 45
			else
				sprintf(buf, "%s : %d (%02.02f%%)", map[m].name, map_users[m], ((float)map_users[m] * 100. / (float)users_all)); // 16 + 3 + 4 (1000) + 2 + 6 (100.00) + 2 + 1 (NULL) = 34
			clif_displaymessage(fd, buf);
		}

	FREE(map_users);

	// display total number of players
	sprintf(buf, "all : %d", users_all);
	clif_displaymessage(fd, buf);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(who) {
	struct map_session_data *pl_sd;
	int i, j, count;
	char match_text[100];

	memset(match_text, 0, sizeof(match_text));

	if (sscanf(message, "%[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower((unsigned char)match_text[j]); // tolower needs unsigned char

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				memset(atcmd_name, 0, sizeof(atcmd_name));
				for (j = 0; pl_sd->status.name[j]; j++)
					atcmd_name[j] = tolower((unsigned char)pl_sd->status.name[j]); // tolower needs unsigned char
				if (strstr(atcmd_name, match_text) != NULL) { // search with no case sensitive
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					else
						sprintf(atcmd_output, "Name: %s | Location: %s %d %d", pl_sd->status.name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(who2) {
	struct map_session_data *pl_sd;
	int i, j, count;
	char match_text[100];

	memset(match_text, 0, sizeof(match_text));

	if (sscanf(message, "%[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower((unsigned char)match_text[j]); // tolower needs unsigned char

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				memset(atcmd_name, 0, sizeof(atcmd_name));
				for (j = 0; pl_sd->status.name[j]; j++)
					atcmd_name[j] = tolower((unsigned char)pl_sd->status.name[j]); // tolower needs unsigned char
				if (strstr(atcmd_name, match_text) != NULL) { // search with no case sensitive
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level);
					else
						sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(who3) {
	struct map_session_data *pl_sd;
	int i, j, count;
	char match_text[100];
	struct guild *g;
	struct party *p;

	memset(match_text, 0, sizeof(match_text));

	if (sscanf(message, "%[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower((unsigned char)match_text[j]); // tolower needs unsigned char

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				memset(atcmd_name, 0, sizeof(atcmd_name));
				for (j = 0; pl_sd->status.name[j]; j++)
					atcmd_name[j] = tolower((unsigned char)pl_sd->status.name[j]); // tolower needs unsigned char
				if (strstr(atcmd_name, match_text) != NULL) { // search with no case sensitive
					p = party_search(pl_sd->status.party_id);
					g = guild_search(pl_sd->status.guild_id);
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_sd->GM_level, (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
					else
						sprintf(atcmd_output, "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(whomap) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->bl.m == map_id) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					else
						sprintf(atcmd_output, "Name: %s | Location: %s %d %d", pl_sd->status.name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(whomap2) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id = 0;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->bl.m == map_id) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level);
					else
						sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(whomap3) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id = 0;
	struct guild *g;
	struct party *p;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->bl.m == map_id) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					p = party_search(pl_sd->status.party_id);
					g = guild_search(pl_sd->status.guild_id);
					if (pl_sd->GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_sd->GM_level, (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
					else
						sprintf(atcmd_output, "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
	else
		sprintf(atcmd_output, msg_txt(56), count, map[map_id].name); // %d players found in map '%s'.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(whogm) {
	struct map_session_data *pl_sd;
	int i, j, count;
	char match_text[100];
	struct guild *g;
	struct party *p;

	memset(match_text, 0, sizeof(match_text));

	if (sscanf(message, "%[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower((unsigned char)match_text[j]); // tolower needs unsigned char

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->GM_level > battle_config.atcommand_max_player_gm_level) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					memset(atcmd_name, 0, sizeof(atcmd_name));
					for (j = 0; pl_sd->status.name[j]; j++)
						atcmd_name[j] = tolower((unsigned char)pl_sd->status.name[j]); // tolower needs unsigned char
					if (strstr(atcmd_name, match_text) != NULL) { // search with no case sensitive
						sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
						clif_displaymessage(fd, atcmd_output);
						sprintf(atcmd_output, "       BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level);
						clif_displaymessage(fd, atcmd_output);
						p = party_search(pl_sd->status.party_id);
						g = guild_search(pl_sd->status.guild_id);
						sprintf(atcmd_output, "       Party: '%s' | Guild: '%s'", (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
						clif_displaymessage(fd, atcmd_output);
						count++;
					}
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(150)); // No GM found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(151)); // 1 GM found.
	else {
		sprintf(atcmd_output, msg_txt(152), count); // %d GMs found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @whozeny - quick sorting function
 *------------------------------------------
 */
void quick_sorting(int tableau[], int premier, int dernier) {
	int temp, vmin, vmax, separateur_de_listes;
	int cpm_result;
	char *p_name;

	vmin = premier;
	vmax = dernier;

	separateur_de_listes = ((struct map_session_data *)session[tableau[(premier + dernier) / 2]]->session_data)->status.zeny;
	p_name = ((struct map_session_data *)session[tableau[(premier + dernier) / 2]]->session_data)->status.name;
	do {
		while((cpm_result = ((struct map_session_data *)session[tableau[vmin]]->session_data)->status.zeny) < separateur_de_listes ||
		      // if same number of zenys, we sort by name.
		      (cpm_result == separateur_de_listes &&
		       strcasecmp(((struct map_session_data *)session[tableau[vmin]]->session_data)->status.name, p_name) < 0))
			vmin++;
		while((cpm_result = ((struct map_session_data *)session[tableau[vmax]]->session_data)->status.zeny) > separateur_de_listes ||
		      // if same number of zenys, we sort by name.
		      (cpm_result == separateur_de_listes &&
		       strcasecmp(((struct map_session_data *)session[tableau[vmax]]->session_data)->status.name, p_name) > 0))
			vmax--;

		if (vmin <= vmax) {
			temp = tableau[vmin];
			tableau[vmin++] = tableau[vmax];
			tableau[vmax--] = temp;
		}
	} while(vmin <= vmax);

	if (premier < vmax)
		quick_sorting(tableau, premier, vmax);
	if (vmin < dernier)
		quick_sorting(tableau, vmin, dernier);
}

/*==========================================
 * @whozeny
 *------------------------------------------
 */
ATCOMMAND_FUNC(whozeny) {
	int *id;
	char match_text[100];
	int i, j, players;
	struct map_session_data *pl_sd;

	memset(match_text, 0, sizeof(match_text));

	if (sscanf(message, "%[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (i = 0; match_text[i]; i++)
		match_text[i] = tolower((unsigned char)match_text[i]); // tolower needs unsigned char

	CALLOC(id, int, fd_max);

	// Get number of online players, id of each online players
	players = 0;
	// sort online characters.
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				memset(atcmd_name, 0, sizeof(atcmd_name));
				for (j = 0; pl_sd->status.name[j]; j++)
					atcmd_name[j] = tolower((unsigned char)pl_sd->status.name[j]); // tolower needs unsigned char
				if (strstr(atcmd_name, match_text) != NULL) { /* search with no case sensitive */
					id[players] = i;
					players++;
				}
			}
		}
	}
	if (players > 1)
		quick_sorting(id, 0, players - 1);

	i = 0;
	if (players > 50)
		i = players - 50;
	while(i < players) {
		pl_sd = session[id[i]]->session_data;
		if (pl_sd->GM_level > 0)
			sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, pl_sd->status.zeny);
		else
			sprintf(atcmd_output, "Name: %s | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->status.base_level, pl_sd->status.zeny);
		clif_displaymessage(fd, atcmd_output);
		i++;
	}

	if (players == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (players == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), players); // %d players found.
		clif_displaymessage(fd, atcmd_output);
		if (players > 50)
			clif_displaymessage(fd, "But only the 50 richest players are displayed.");
	}

	/* display abnormal amount of zeny */
	if (players > 0) {
		j = 0;
		for (i = 0; i < players; i++) {
			pl_sd = session[id[i]]->session_data;
			if (pl_sd->status.zeny < 0 || pl_sd->status.zeny > MAX_ZENY) {
				if (j == 0)
					clif_displaymessage(fd, "Abnormal amount of zenys (negativ or upper than maximum):");
				if (pl_sd->GM_level > 0)
					sprintf(atcmd_output, "*** Name: %s (GM:%d) | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, pl_sd->status.zeny);
				else
					sprintf(atcmd_output, "*** Name: %s | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->status.base_level, pl_sd->status.zeny);
				clif_displaymessage(fd, atcmd_output);
				j++;
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "No player has abnormal amount of zenys.");
		else if (j == 1)
			clif_displaymessage(fd, "1 player has abnormal amount of zenys.");
		else {
			sprintf(atcmd_output, "%d players have abnormal amount of zenys.", j); // %d players found.
			clif_displaymessage(fd, atcmd_output);
		}
	}

	FREE(id);

	return 0;
}

/*==========================================
 * @whozenymap
 *------------------------------------------
 */
ATCOMMAND_FUNC(whozenymap) {
	int *id;
	int i, j, players;
	int map_id = 0;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	CALLOC(id, int, fd_max);

	// Get number of online players, id of each online players
	players = 0;
	// sort online characters.
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				if (pl_sd->bl.m == map_id) {
					id[players] = i;
					players++;
				}
			}
		}
	}
	if (players > 1)
		quick_sorting(id, 0, players - 1);

	i = 0;
	if (players > 50)
		i = players - 50;
	while(i < players) {
		pl_sd = session[id[i]]->session_data;
		if (pl_sd->GM_level > 0)
			sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, pl_sd->status.zeny);
		else
			sprintf(atcmd_output, "Name: %s | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->status.base_level, pl_sd->status.zeny);
		clif_displaymessage(fd, atcmd_output);
		i++;
	}

	if (players == 0) {
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
		clif_displaymessage(fd, atcmd_output);
	} else if (players == 1) {
		sprintf(atcmd_output, msg_txt(55), map[map_id].name); // 1 player found in map '%s'.
		clif_displaymessage(fd, atcmd_output);
	} else {
		sprintf(atcmd_output, msg_txt(56), players, map[map_id].name); // %d players found in map '%s'.
		clif_displaymessage(fd, atcmd_output);
		if (players > 50)
			clif_displaymessage(fd, "But only the 50 richest players are displayed.");
	}

	/* display abnormal amount of zeny */
	if (players > 0) {
		j = 0;
		for (i = 0; i < players; i++) {
			pl_sd = session[id[i]]->session_data;
			if (pl_sd->status.zeny < 0 || pl_sd->status.zeny > MAX_ZENY) {
				if (j == 0)
					clif_displaymessage(fd, "Abnormal amount of zenys (negativ or upper than maximum):");
				if (pl_sd->GM_level > 0)
					sprintf(atcmd_output, "*** Name: %s (GM:%d) | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->GM_level, pl_sd->status.base_level, pl_sd->status.zeny);
				else
					sprintf(atcmd_output, "*** Name: %s | BLvl: %d | Zeny: %d", pl_sd->status.name, pl_sd->status.base_level, pl_sd->status.zeny);
				clif_displaymessage(fd, atcmd_output);
				j++;
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "No player has abnormal amount of zenys.");
		else if (j == 1)
			clif_displaymessage(fd, "1 player has abnormal amount of zenys.");
		else {
			sprintf(atcmd_output, "%d players have abnormal amount of zenys.", j); // %d players found.
			clif_displaymessage(fd, atcmd_output);
		}
	}

	FREE(id);

	return 0;
}

/*==========================================
 * @whohas
 *------------------------------------------
 */
ATCOMMAND_FUNC(whohas) {
	char item_name[100];
	char position[100];
	int i, j, k, item_id, players;
	struct map_session_data *pl_sd;
	struct item_data *item_data;
	struct storage *stor;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s", item_name) < 1) {
		send_usage(fd, "Please, enter an item name/id (usage: %s <item_name|ID>).", original_command);
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		// Get number of online players, id of each online players
		players = 0;
		// sort online characters.
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					memset(position, 0, sizeof(position));
					// search in inventory
					for (j = 0; j < MAX_INVENTORY; j++) {
						if (pl_sd->status.inventory[j].nameid == item_id)
							break;
						// search for cards
						if (pl_sd->status.inventory[j].nameid > 0 && (item_data = itemdb_search(pl_sd->status.inventory[j].nameid)) != NULL) {
							for (k = 0; k < item_data->slot; k++)
								if (pl_sd->status.inventory[j].card[k] == item_id)
									break;
							if (k != item_data->slot)
								break;
						}
					}
					if (j != MAX_INVENTORY)
						strcpy(position, "inventory, ");
					// search in cart
					for (j = 0; j < MAX_CART; j++) {
						if (pl_sd->status.cart[j].nameid == item_id)
							break;
						// search for cards
						if (pl_sd->status.cart[j].nameid > 0 && (item_data = itemdb_search(pl_sd->status.cart[j].nameid)) != NULL) {
							for (k = 0; k < item_data->slot; k++)
								if (pl_sd->status.cart[j].card[k] == item_id)
									break;
							if (k != item_data->slot)
								break;
						}
					}
					if (j != MAX_CART)
						strcat(position, "cart, ");
					// search in storage
					if ((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
						for (j = 0; j < MAX_STORAGE; j++) {
							if (stor->storage[j].nameid == item_id)
								break;
							// search for cards
							if (stor->storage[j].nameid > 0 && (item_data = itemdb_search(stor->storage[j].nameid)) != NULL) {
								for (k = 0; k < item_data->slot; k++)
									if (stor->storage[j].card[k] == item_id)
										break;
								if (k != item_data->slot)
									break;
							}
						}
					}
					if (j != MAX_STORAGE)
						strcat(position, "storage, ");
					// if found in one of inventory system of the player
					if (position[0]) {
						position[strlen(position)-2] = '\0'; // remove last ', '
						if (pl_sd->GM_level > 0)
							sprintf(atcmd_output, "Name: %s (GM:%d) | %s (%d/%d) | found in %s", pl_sd->status.name, pl_sd->GM_level, job_name(pl_sd->status.class), pl_sd->status.base_level, pl_sd->status.job_level, position);
						else
							sprintf(atcmd_output, "Name: %s | %s (%d/%d) | found in %s", pl_sd->status.name, job_name(pl_sd->status.class), pl_sd->status.base_level, pl_sd->status.job_level, position);
						clif_displaymessage(fd, atcmd_output);
						players++;
					}
				}
			}
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	if (players == 0)
		clif_displaymessage(fd, "No player has this item.");
	else if (players == 1)
		clif_displaymessage(fd, "1 player has this item.");
	else {
		sprintf(atcmd_output, "%d players have this item.", players);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @whohasmap
 *------------------------------------------
 */
ATCOMMAND_FUNC(whohasmap) {
	char item_name[100];
	char position[100];
	int i, j, k, map_id, item_id, players;
	struct map_session_data *pl_sd;
	struct item_data *item_data;
	struct storage *stor;

	memset(item_name, 0, sizeof(item_name));
	memset(atcmd_mapname, 0, sizeof(atcmd_mapname));

	if (!message || !*message || sscanf(message, "%s %s", item_name, atcmd_mapname) < 1) {
		send_usage(fd, "Please, enter an item name/id (usage: %s <item_name|ID> [map]).", original_command);
		return -1;
	}

	if (!atcmd_mapname[0])
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		// Get number of online players, id of each online players
		players = 0;
		// sort online characters.
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
					if (pl_sd->bl.m == map_id) {
						memset(position, 0, sizeof(position));
						// search in inventory
						for (j = 0; j < MAX_INVENTORY; j++) {
							if (pl_sd->status.inventory[j].nameid == item_id)
								break;
							// search for cards
							if (pl_sd->status.inventory[j].nameid > 0 && (item_data = itemdb_search(pl_sd->status.inventory[j].nameid)) != NULL) {
								for (k = 0; k < item_data->slot; k++)
									if (pl_sd->status.inventory[j].card[k] == item_id)
										break;
								if (k != item_data->slot)
									break;
							}
						}
						if (j != MAX_INVENTORY)
							strcpy(position, "inventory, ");
						// search in cart
						for (j = 0; j < MAX_CART; j++) {
							if (pl_sd->status.cart[j].nameid == item_id)
								break;
							// search for cards
							if (pl_sd->status.cart[j].nameid > 0 && (item_data = itemdb_search(pl_sd->status.cart[j].nameid)) != NULL) {
								for (k = 0; k < item_data->slot; k++)
									if (pl_sd->status.cart[j].card[k] == item_id)
										break;
								if (k != item_data->slot)
									break;
							}
						}
						if (j != MAX_CART)
							strcat(position, "cart, ");
						// search in storage
						if ((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
							for (j = 0; j < MAX_STORAGE; j++) {
								if (stor->storage[j].nameid == item_id)
									break;
								// search for cards
								if (stor->storage[j].nameid > 0 && (item_data = itemdb_search(stor->storage[j].nameid)) != NULL) {
									for (k = 0; k < item_data->slot; k++)
										if (stor->storage[j].card[k] == item_id)
											break;
									if (k != item_data->slot)
										break;
								}
							}
						}
						if (j != MAX_STORAGE)
							strcat(position, "storage, ");
						// if found in one of inventory system of the player
						if (position[0]) {
							position[strlen(position)-2] = '\0'; // remove last ', '
							if (pl_sd->GM_level > 0)
								sprintf(atcmd_output, "Name: %s (GM:%d) | %s (%d/%d) | found in %s", pl_sd->status.name, pl_sd->GM_level, job_name(pl_sd->status.class), pl_sd->status.base_level, pl_sd->status.job_level, position);
							else
								sprintf(atcmd_output, "Name: %s | %s (%d/%d) | found in %s", pl_sd->status.name, job_name(pl_sd->status.class), pl_sd->status.base_level, pl_sd->status.job_level, position);
							clif_displaymessage(fd, atcmd_output);
							players++;
						}
					}
				}
			}
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	if (players == 0)
		sprintf(atcmd_output, "No player has this item on map '%s'.", map[map_id].name);
	else if (players == 1)
		sprintf(atcmd_output, "1 player has this item on map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "%d players have this item on map '%s'.", players, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * Cause random emote on all online players [Valaris]
 *------------------------------------------
 */
ATCOMMAND_FUNC(happyhappyjoyjoy) {
	struct map_session_data *pl_sd;
	int i, e, count;

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!(pl_sd->status.option & OPTION_HIDE)) { // if GM is hiden, don't display on it
				while((e = rand() % 48) == 34);
				clif_emotion(&pl_sd->bl, e);
				count++;
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "Emotion icons apply to no player.");
	else if (count == 1)
		clif_displaymessage(fd, "Emotion icons apply to 1 player.");
	else {
		sprintf(atcmd_output, "Emotion icons apply to %d players.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Cause random emote on online players of a specifical map
 *------------------------------------------
 */
ATCOMMAND_FUNC(happyhappyjoyjoymap) {
	struct map_session_data *pl_sd;
	int i, e, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
		    pl_sd->bl.m == map_id) {
			if (!(pl_sd->status.option & OPTION_HIDE)) { // if GM is hiden, don't display on it
				while((e = rand() % 48) == 34);
				clif_emotion(&pl_sd->bl, e);
				count++;
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, "Emotion icons apply to no player on map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "Emotion icons apply to 1 player on map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "Emotion icons apply to %d players on map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(save) {
	pc_setsavepoint(sd, sd->mapname, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif_save_petdata(sd->status.account_id, &sd->pet);
	chrif_save(sd); // do pc_makesavestatus and save storage + account_reg/account_reg2 too
	clif_displaymessage(fd, msg_txt(6)); // Character data respawn point saved.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(load) {
	int m;

	if ((m = map_checkmapname(sd->status.save_point.map)) == -1) { // if map doesn't exist in all map-servers
		clif_displaymessage(fd, "Save map not found.");
		return -1;
	}

	if (m >= 0) { // if on this map-server
		if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
			clif_displaymessage(fd, "You are not authorized to warp you to your save map.");
			return -1;
		}
	}
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
		return -1;
	}
	pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 0);
	clif_displaymessage(fd, msg_txt(7)); // Warping to respawn point.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charload) {
	struct map_session_data *pl_sd;
	int m;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if ((m = map_checkmapname(pl_sd->status.save_point.map)) == -1) { // if map doesn't exist in all map-servers
				clif_displaymessage(fd, "Save map not found.");
				return -1;
			}
			if (m >= 0) { // if on this map-server
				if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
					clif_displaymessage(fd, "You are not authorized to warp this character to its save map.");
					return -1;
				}
			}
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp this character from its actual map.");
				return -1;
			}
			pc_setpos(pl_sd, pl_sd->status.save_point.map, pl_sd->status.save_point.x, pl_sd->status.save_point.y, 0);
			clif_displaymessage(fd, "Character warped to its respawn point.");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charloadmap) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->bl.m == map_id) {
				if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
					if (pl_sd != sd) { // not load yourself
						pc_setpos(pl_sd, pl_sd->status.save_point.map, pl_sd->status.save_point.x, pl_sd->status.save_point.y, 0);
						count++;
					}
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, "No player recalled from map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "1 player recalled from map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "%d players recalled from map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charloadall) {
	struct map_session_data *pl_sd;
	int i, count;

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (pl_sd != sd) { // not load yourself
					pc_setpos(pl_sd, pl_sd->status.save_point.map, pl_sd->status.save_point.x, pl_sd->status.save_point.y, 0);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "No player recalled.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player recalled.");
	else {
		sprintf(atcmd_output, "%d players recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(speed) {
	int speed = 0;

	sscanf(message, "%d", &speed);

	if (speed != 0 && (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED)) {
		send_usage(fd, "Please, enter a valid speed value (usage: %s [0(restore normal speed)|<%d-%d>]).", original_command, MIN_WALK_SPEED, MAX_WALK_SPEED);
		return -1;
	}

	if (speed == 0)
		status_calc_pc(sd, 3);
	else {
		sd->speed = speed;
		clif_updatestatus(sd, SP_SPEED);
	}
	clif_displaymessage(fd, msg_txt(8)); // Speed changed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charspeed) {
	struct map_session_data *pl_sd;
	int speed = 0;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &speed, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a speed value and a player name (usage: %s [0(restore normal speed)|<%d-%d>] <player name>).", original_command, MIN_WALK_SPEED, MAX_WALK_SPEED);
		return -1;
	}

	if (speed != 0 && (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED)) {
		send_usage(fd, "Please, enter a valid speed value (usage: %s 0(restore normal speed)|<%d-%d> <player name>).", original_command, MIN_WALK_SPEED, MAX_WALK_SPEED);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (speed == 0)
				status_calc_pc(pl_sd, 3);
			else {
				pl_sd->speed = speed;
				clif_updatestatus(pl_sd, SP_SPEED);
			}
			clif_displaymessage(fd, "Character speed changed.");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charspeedmap) {
	struct map_session_data *pl_sd;
	int i, count;
	int speed = 0;
	int map_id;

	memset(atcmd_mapname, 0, sizeof(atcmd_mapname));

	sscanf(message, "%d %[^\n]", &speed, atcmd_mapname);

	if (speed != 0 && (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED)) {
		send_usage(fd, "Please, enter a valid speed value (usage: %s [0(restore normal speed)|<%d-%d>] [map name]).", original_command, MIN_WALK_SPEED, MAX_WALK_SPEED);
		return -1;
	}

	if (!atcmd_mapname[0])
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (pl_sd->bl.m == map_id) {
				if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
					if (speed == 0)
						status_calc_pc(pl_sd, 3);
					else {
						pl_sd->speed = speed;
						clif_updatestatus(pl_sd, SP_SPEED);
					}
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, "Speed not changed - no player found on map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "Speed changed at 1 player on map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "Speed changed at %d players on map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charspeedall) {
	struct map_session_data *pl_sd;
	int i, count;
	int speed = 0;

	sscanf(message, "%d", &speed);

	if (speed != 0 && (speed < MIN_WALK_SPEED || speed > MAX_WALK_SPEED)) {
		send_usage(fd, "Please, enter a valid speed value (usage: %s [0(restore normal speed)|<%d-%d>]).", original_command, MIN_WALK_SPEED, MAX_WALK_SPEED);
		return -1;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (speed == 0)
					status_calc_pc(pl_sd, 3);
				else {
					pl_sd->speed = speed;
					clif_updatestatus(pl_sd, SP_SPEED);
				}
				count++;
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "Speed not changed - no player found.");
	else if (count == 1)
		clif_displaymessage(fd, "Speed changed at 1 player.");
	else {
		sprintf(atcmd_output, "Speed changed at %d players.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(storage) {
	struct storage *stor;

	if (sd->state.storage_flag == 1) {
		clif_displaymessage(fd, "You have opened your guild storage. Close it before.");
		return -1;
	}

	if ((stor = account2storage2(sd->status.account_id)) != NULL && stor->storage_status == 1) {
		clif_displaymessage(fd, "You have already opened your storage.");
		return -1;
	}

	if (!battle_config.atcommand_storage_on_pvp_map && map[sd->bl.m].flag.pvp) {
		clif_displaymessage(fd, "You can not open your storage on a PVP map.");
		return -1;
	}

	storage_storageopen(sd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charstorage) {
	struct map_session_data *pl_sd;
	struct storage *stor;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->state.storage_flag == 1) {
				clif_displaymessage(fd, "The player has opened its guild storage. He/She must close it before.");
				return -1;
			}
			if ((stor = account2storage2(pl_sd->status.account_id)) != NULL && stor->storage_status == 1) {
				clif_displaymessage(fd, "The player have already opened his/her storage.");
				return -1;
			}
			if (!battle_config.atcommand_storage_on_pvp_map && map[pl_sd->bl.m].flag.pvp) {
				clif_displaymessage(fd, "This player is on a pvp map. You can not open his/her storage.");
				return -1;
			}
			storage_storageopen(pl_sd);
			if (pl_sd != sd)
				clif_displaymessage(fd, "Player's storage is now opened (for the player).");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(guildstorage) {
	struct storage *stor;

	if (sd->status.guild_id > 0) {
		if (sd->state.storage_flag == 1) {
			clif_displaymessage(fd, "You have already opened your guild storage.");
			return -1;
		}
		if ((stor = account2storage2(sd->status.account_id)) != NULL && stor->storage_status == 1) {
			clif_displaymessage(fd, "Your storage is opened. Close it before.");
			return -1;
		}
		if (!battle_config.atcommand_gstorage_on_pvp_map && map[sd->bl.m].flag.pvp) {
			clif_displaymessage(fd, "You can not open your guild storage on a PVP map.");
			return -1;
		}
		storage_guild_storageopen(sd);
	} else {
		clif_displaymessage(fd, "You are not in a guild.");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charguildstorage) {
	struct map_session_data *pl_sd;
	struct storage *stor;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->status.guild_id > 0) {
				if (pl_sd->state.storage_flag == 1) {
					clif_displaymessage(fd, "The guild storage of the player is already opened.");
					return -1;
				}
				if ((stor = account2storage2(pl_sd->status.account_id)) != NULL && stor->storage_status == 1) {
					clif_displaymessage(fd, "The player's storage is opened. He/She must close it before.");
					return -1;
				}
				if (!battle_config.atcommand_gstorage_on_pvp_map && map[pl_sd->bl.m].flag.pvp) {
					clif_displaymessage(fd, "This player is on a pvp map. You can not open his/her guild storage.");
					return -1;
				}
				storage_guild_storageopen(pl_sd);
				if (pl_sd != sd)
					clif_displaymessage(fd, "Player's guild storage is now opened (for the player).");
			} else {
				clif_displaymessage(fd, "This player is not in a guild.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(option) {
	int opt1 = 0, opt2 = 0, opt3 = 0;

	if (!message || !*message || sscanf(message, "%d %d %d", &opt1, &opt2, &opt3) < 1 || opt1 < 0 || opt2 < 0 || opt3 < 0) {
		send_usage(fd, "Please, enter at least a option (usage: %s <opt1:0+> <opt2:0+> <opt3:0+>).", original_command);
		return -1;
	}

	sd->opt1 = opt1;
	sd->opt2 = opt2;
	if (!(sd->status.option & CART_MASK) && (opt3 & CART_MASK)) {
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
	}
	sd->status.option = opt3;
	// fix pecopeco display
	if (sd->status.class ==   13 || sd->status.class ==   21 ||
	    sd->status.class == 4014 || sd->status.class == 4022 ||
	    sd->status.class == 4036 || sd->status.class == 4044) {
		if (!pc_isriding(sd)) { // sd have the new value...
			// normal classes
			if (sd->status.class == 13)
				sd->status.class = sd->view_class = 7;
			else if (sd->status.class == 21)
				sd->status.class = sd->view_class = 14;
			// high classes
			else if (sd->status.class == 4014)
				sd->status.class = sd->view_class = 4008;
			else if (sd->status.class == 4022)
				sd->status.class = sd->view_class = 4015;
			// baby classes
			else if (sd->status.class == 4036)
				sd->status.class = sd->view_class = 4030;
			else if (sd->status.class == 4044)
				sd->status.class = sd->view_class = 4037;
		}
	} else {
		if (pc_isriding(sd)) { // sd have the new value...
			if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
				sd->status.option &= ~0x0020;
			} else {
				// normal classes
				if (sd->status.class == 7)
					sd->status.class = sd->view_class = 13;
				else if (sd->status.class == 14)
					sd->status.class = sd->view_class = 21;
				// high classes
				else if (sd->status.class == 4008)
					sd->status.class = sd->view_class = 4014;
				else if (sd->status.class == 4015)
					sd->status.class = sd->view_class = 4022;
				// baby classes
				else if (sd->status.class == 4030)
					sd->status.class = sd->view_class = 4036;
				else if (sd->status.class == 4037)
					sd->status.class = sd->view_class = 4044;
				else
					sd->status.option &= ~0x0020;
			}
		}
	}

	clif_changeoption(&sd->bl);
	status_calc_pc(sd, 0);
	clif_displaymessage(fd, msg_txt(9)); // Options changed.
	if (pc_isfalcon(sd) && pc_iscarton(sd) && (sd->status.option & 0x0008) != 0x0008)
		clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(optionadd) {
	int opt1 = 0, opt2 = 0, opt3 = 0;

	if (!message || !*message || sscanf(message, "%d %d %d", &opt1, &opt2, &opt3) < 1 || opt1 < 0 || opt2 < 0 || opt3 < 0 || (opt1 == 0 && opt2 == 0 && opt3 == 0)) {
		send_usage(fd, "Please, enter at least a option (usage: %s <opt1:0+> <opt2:0+> <opt3:0+>).", original_command);
		return -1;
	}

	sd->opt1 |= opt1;
	sd->opt2 |= opt2;
	if (!(sd->status.option & CART_MASK) && (opt3 & CART_MASK)) {
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
	}
	sd->status.option |= opt3;
	// fix pecopeco display
	if (sd->status.class ==   13 || sd->status.class ==   21 ||
	    sd->status.class == 4014 || sd->status.class == 4022 ||
	    sd->status.class == 4036 || sd->status.class == 4044) {
		if (!pc_isriding(sd)) { // sd have the new value...
			// normal classes
			if (sd->status.class == 13)
				sd->status.class = sd->view_class = 7;
			else if (sd->status.class == 21)
				sd->status.class = sd->view_class = 14;
			// high classes
			else if (sd->status.class == 4014)
				sd->status.class = sd->view_class = 4008;
			else if (sd->status.class == 4022)
				sd->status.class = sd->view_class = 4015;
			// baby classes
			else if (sd->status.class == 4036)
				sd->status.class = sd->view_class = 4030;
			else if (sd->status.class == 4044)
				sd->status.class = sd->view_class = 4037;
		}
	} else {
		if (pc_isriding(sd)) { // sd have the new value...
			if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
				sd->status.option &= ~0x0020;
			} else {
				// normal classes
				if (sd->status.class == 7)
					sd->status.class = sd->view_class = 13;
				else if (sd->status.class == 14)
					sd->status.class = sd->view_class = 21;
				// high classes
				else if (sd->status.class == 4008)
					sd->status.class = sd->view_class = 4014;
				else if (sd->status.class == 4015)
					sd->status.class = sd->view_class = 4022;
				// baby classes
				else if (sd->status.class == 4030)
					sd->status.class = sd->view_class = 4036;
				else if (sd->status.class == 4037)
					sd->status.class = sd->view_class = 4044;
				else
					sd->status.option &= ~0x0020;
			}
		}
	}

	clif_changeoption(&sd->bl);
	status_calc_pc(sd, 0);
	clif_displaymessage(fd, msg_txt(9)); // Options changed.
	if (pc_isfalcon(sd) && pc_iscarton(sd) && (sd->status.option & 0x0008) != 0x0008)
		clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(optionremove) {
	int opt1 = 0, opt2 = 0, opt3 = 0;

	if (!message || !*message || sscanf(message, "%d %d %d", &opt1, &opt2, &opt3) < 1 || opt1 < 0 || opt2 < 0 || opt3 < 0 || (opt1 == 0 && opt2 == 0 && opt3 == 0)) {
		send_usage(fd, "Please, enter at least a option (usage: %s <opt1:0+> <opt2:0+> <opt3:0+>).", original_command);
		return -1;
	}

	sd->opt1 &= ~opt1;
	sd->opt2 &= ~opt2;
	sd->status.option &= ~opt3;
	// fix pecopeco display
	if (sd->status.class ==   13 || sd->status.class ==   21 ||
	    sd->status.class == 4014 || sd->status.class == 4022 ||
	    sd->status.class == 4036 || sd->status.class == 4044) {
		if (!pc_isriding(sd)) { // sd have the new value...
			// normal classes
			if (sd->status.class == 13)
				sd->status.class = sd->view_class = 7;
			else if (sd->status.class == 21)
				sd->status.class = sd->view_class = 14;
			// high classes
			else if (sd->status.class == 4014)
				sd->status.class = sd->view_class = 4008;
			else if (sd->status.class == 4022)
				sd->status.class = sd->view_class = 4015;
			// baby classes
			else if (sd->status.class == 4036)
				sd->status.class = sd->view_class = 4030;
			else if (sd->status.class == 4044)
				sd->status.class = sd->view_class = 4037;
		}
	} else {
		if (pc_isriding(sd)) { // sd have the new value...
			if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
				sd->status.option &= ~0x0020;
			} else {
				// normal classes
				if (sd->status.class == 7)
					sd->status.class = sd->view_class = 13;
				else if (sd->status.class == 14)
					sd->status.class = sd->view_class = 21;
				// high classes
				else if (sd->status.class == 4008)
					sd->status.class = sd->view_class = 4014;
				else if (sd->status.class == 4015)
					sd->status.class = sd->view_class = 4022;
				// baby classes
				else if (sd->status.class == 4030)
					sd->status.class = sd->view_class = 4036;
				else if (sd->status.class == 4037)
					sd->status.class = sd->view_class = 4044;
				else
					sd->status.option &= ~0x0020;
			}
		}
	}

	clif_changeoption(&sd->bl);
	status_calc_pc(sd, 0);
	clif_displaymessage(fd, msg_txt(9)); // Options changed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(hide) {
	if (sd->status.option & OPTION_HIDE) {
		sd->status.option &= ~OPTION_HIDE;
		clif_displaymessage(fd, msg_txt(10)); // Invisible: Off.
	} else {
		sd->status.option |= OPTION_HIDE;
		clif_displaymessage(fd, msg_txt(11)); // Invisible: On.
	}
	clif_changeoption(&sd->bl);

	return 0;
}

/*==========================================
 * Job Change
 *------------------------------------------
 */
ATCOMMAND_FUNC(jobchange) {
	int job = 0, upper = -1;
	int flag, i;
	char jobname[100];

	const struct { char name[20]; int id; } jobs[] = {
		{ "novice",                0 },
		{ "swordman",              1 },
		{ "swordsman",             1 },
		{ "mage",                  2 },
		{ "archer",                3 },
		{ "acolyte",               4 },
		{ "acolite",               4 },
		{ "merchant",              5 },
		{ "thief",                 6 },
		{ "theif",                 6 },
		{ "knight",                7 },
		{ "priest",                8 },
		{ "priestess",             8 },
		{ "wizard",                9 },
		{ "blacksmith",           10 },
		{ "hunter",               11 },
		{ "assassin",             12 },
		{ "peco knight",          13 },
		{ "peco-knight",          13 },
		{ "peco_knight",          13 },
		{ "pecoknight",           13 },
		{ "peko knight",          13 },
		{ "peko-knight",          13 },
		{ "peko_knight",          13 },
		{ "pekoknight",           13 },
		{ "crusader",             14 },
		{ "monk",                 15 },
		{ "sage",                 16 },
		{ "rogue",                17 },
		{ "alchemist",            18 },
		{ "bard",                 19 },
		{ "dancer",               20 },
		{ "peco crusader",        21 },
		{ "peco-crusader",        21 },
		{ "peco_crusader",        21 },
		{ "pecocrusader",         21 },
		{ "peko crusader",        21 },
		{ "peko-crusader",        21 },
		{ "peko_crusader",        21 },
		{ "pekocrusader",         21 },
		{ "super novice",         23 },
		{ "super-novice",         23 },
		{ "super_novice",         23 },
		{ "supernovice",          23 },
		{ "novice high",        4001 },
		{ "novice_high",        4001 },
		{ "high novice",        4001 },
		{ "high-novice",        4001 },
		{ "high_novice",        4001 },
		{ "highnovice",         4001 },
		{ "swordman high",      4002 },
		{ "swordsman high",     4002 },
		{ "mage high",          4003 },
		{ "archer high",        4004 },
		{ "acolyte high",       4005 },
		{ "acolite high",       4005 },
		{ "merchant high",      4006 },
		{ "thief high",         4007 },
		{ "theif high",         4007 },
		{ "lord knight",        4008 },
		{ "high priest",        4009 },
		{ "high_priest",        4009 },
		{ "highpriest",         4009 },
		{ "high priestess",     4009 },
		{ "high_priestess",     4009 },
		{ "highpriestess",      4009 },
		{ "high wizard",        4010 },
		{ "high_wizard",        4010 },
		{ "highwizard",         4010 },
		{ "whitesmith",         4011 },
		{ "sniper",             4012 },
		{ "assassin cross",     4013 },
		{ "peco lord knight",   4014 },
		{ "peko lord knight",   4014 },
		{ "paladin",            4015 },
		{ "champion",           4016 },
		{ "professor",          4017 },
		{ "stalker",            4018 },
		{ "creator",            4019 },
		{ "clown",              4020 },
		{ "gypsy",              4021 },
		{ "peco paladin",       4022 },
		{ "peco-paladin",       4022 },
		{ "peco_paladin",       4022 },
		{ "pecopaladin",        4022 },
		{ "peko paladin",       4022 },
		{ "peko-paladin",       4022 },
		{ "peko_paladin",       4022 },
		{ "pekopaladin",        4022 },
		{ "baby novice",        4023 },
		{ "baby swordman",      4024 },
		{ "baby swordsman",     4024 },
		{ "baby mage",          4025 },
		{ "baby archer",        4026 },
		{ "baby acolyte",       4027 },
		{ "baby acolite",       4027 },
		{ "baby merchant",      4028 },
		{ "baby thief",         4029 },
		{ "baby theif",         4029 },
		{ "baby knight",        4030 },
		{ "baby priest",        4031 },
		{ "baby priestess",     4031 },
		{ "baby wizard",        4032 },
		{ "baby blacksmith",    4033 },
		{ "baby hunter",        4034 },
		{ "baby assassin",      4035 },
		{ "baby peco knight",   4036 },
		{ "baby peco-knight",   4036 },
		{ "baby peco_knight",   4036 },
		{ "baby pecoknight",    4036 },
		{ "baby peko knight",   4036 },
		{ "baby peko-knight",   4036 },
		{ "baby peko_knight",   4036 },
		{ "baby pekoknight",    4036 },
		{ "baby crusader",      4037 },
		{ "baby monk",          4038 },
		{ "baby sage",          4039 },
		{ "baby rogue",         4040 },
		{ "baby alchemist",     4041 },
		{ "baby bard",          4042 },
		{ "baby dancer",        4043 },
		{ "baby peco crusader", 4044 },
		{ "baby peco-crusader", 4044 },
		{ "baby peco_crusader", 4044 },
		{ "baby pecocrusader",  4044 },
		{ "baby peko crusader", 4044 },
		{ "baby peko-crusader", 4044 },
		{ "baby peko_crusader", 4044 },
		{ "baby pekocrusader",  4044 },
		{ "super baby",         4045 },
		{ "super-baby",         4045 },
		{ "super_baby",         4045 },
		{ "superbaby",          4045 },
	};

	if (!message || !*message || sscanf(message, "%d %d", &job, &upper) < 1 || job < 0 || job >= MAX_PC_CLASS || (job > 23 && job < 4001)) {
		/* search class name */
		i = (int)(sizeof(jobs) / sizeof(jobs[0]));
		if (sscanf(message, "\"%[^\"]\" %d", jobname, &upper) >= 1 ||
		    sscanf(message, "%s %d", jobname, &upper) >= 1) {
			for (i = 0; i < (int)(sizeof(jobs) / sizeof(jobs[0])); i++) {
				if (strcasecmp(jobname, jobs[i].name) == 0 || strcasecmp(message, jobs[i].name) == 0) {
					job = jobs[i].id;
					break;
				}
			}
		}
		/* if class name not found */
		if ((i == (int)(sizeof(jobs) / sizeof(jobs[0])))) {
			send_usage(fd, "Please, enter a valid job ID (usage: %s <job ID|\"job name\"> [upper]).", original_command);
			send_usage(fd, "   0 Novice            7 Knight           14 Crusader       22 Formal");
			send_usage(fd, "   1 Swordman          8 Priest           15 Monk           23 Super Novice");
			send_usage(fd, "   2 Mage              9 Wizard           16 Sage");
			send_usage(fd, "   3 Archer           10 Blacksmith       17 Rogue");
			send_usage(fd, "   4 Acolyte          11 Hunter           18 Alchemist");
			send_usage(fd, "   5 Merchant         12 Assassin         19 Bard");
			send_usage(fd, "   6 Thief            13 Peco-Knight      20 Dancer         21 Peco-Crusader");
			send_usage(fd, "4001 Novice High    4008 Lord Knight      4015 Paladin      4022 Peco-Paladin");
			send_usage(fd, "4002 Swordman High  4009 High Priest      4016 Champion");
			send_usage(fd, "4003 Mage High      4010 High Wizard      4017 Professor");
			send_usage(fd, "4004 Archer High    4011 Whitesmith       4018 Stalker");
			send_usage(fd, "4005 Acolyte High   4012 Sniper           4019 Creator");
			send_usage(fd, "4006 Merchant High  4013 Assassin Cross   4020 Clown");
			send_usage(fd, "4007 Thief High     4014 Peco Knight      4021 Gypsy");
			send_usage(fd, "4023 Baby Novice    4030 Baby Knight      4037 Baby Crusader  4044 Baby Peco-Crusader");
			send_usage(fd, "4024 Baby Swordsman 4031 Baby Priest      4038 Baby Monk      4045 Super Baby");
			send_usage(fd, "4025 Baby Mage      4032 Baby Wizard      4039 Baby Sage");
			send_usage(fd, "4026 Baby Archer    4033 Baby Blacksmith  4040 Baby Rogue");
			send_usage(fd, "4027 Baby Acolyte   4034 Baby Hunter      4041 Baby Alchemist");
			send_usage(fd, "4028 Baby Merchant  4035 Baby Assassin    4042 Baby Bard");
			send_usage(fd, "4029 Baby Thief     4036 Baby Peco-Knight 4043 Baby Dancer");
			send_usage(fd, "[upper]: -1 (default) to automatically determine the 'level', 0 to force normal job, 1 to force high job.");
			return -1;
		}
	}

	if (job == sd->status.class) {
		clif_displaymessage(fd, "Your job is already the wished job.");
		return -1;
	}

	// fix pecopeco display
	if ((job != 13 && job != 21 && job != 4014 && job != 4022 && job != 4036 && job != 4044)) {
		if (pc_isriding(sd)) {
			// normal classes
			if (sd->status.class == 13)
				sd->status.class = sd->view_class = 7;
			else if (sd->status.class == 21)
				sd->status.class = sd->view_class = 14;
			// high classes
			else if (sd->status.class == 4014)
				sd->status.class = sd->view_class = 4008;
			else if (sd->status.class == 4022)
				sd->status.class = sd->view_class = 4015;
			// baby classes
			else if (sd->status.class == 4036)
				sd->status.class = sd->view_class = 4030;
			else if (sd->status.class == 4044)
				sd->status.class = sd->view_class = 4037;
			sd->status.option &= ~0x0020;
			clif_changeoption(&sd->bl);
			status_calc_pc(sd, 0);
		}
	} else {
		if (!pc_isriding(sd)) {
			// normal classes
			if (job == 13)
				job = 7;
			else if (job == 21)
				job = 14;
			// high classes
			else if (job == 4014)
				job = 4008;
			else if (job == 4022)
				job = 4015;
			// baby classes
			else if (job == 4036)
				job = 4030;
			else if (job == 4044)
				job = 4037;
		}
	}

	flag = 0;
	for (i = 0; i < MAX_INVENTORY; i++)
		if (sd->status.inventory[i].nameid && (sd->status.inventory[i].equip & 34) != 0) { // righ hand (2) + left hand (32)
			pc_unequipitem(sd, i, 3); /* unequip weapon to avoid sprite error */
			flag = 1;
		}
	if (flag)
		clif_displaymessage(fd, "Weapon unequiped to avoid sprite error.");

	if (pc_jobchange(sd, job, upper) == 0)
		clif_displaymessage(fd, msg_txt(12)); // Your job has been changed.
	else {
		clif_displaymessage(fd, msg_txt(155)); // Impossible to change your job.
		return -1;
	}

	return 0;
}

// === SUICIDES CHARACTER ===
// ==========================
ATCOMMAND_FUNC(die)
{
	if(pc_isdead(sd))
	{
		clif_displaymessage(fd, "You are already dead.");
		return -1;
	}
	clif_specialeffect(&sd->bl, 450, 0);
	pc_damage(NULL, sd, sd->status.hp + 1);
	clif_displaymessage(fd, msg_txt(13));
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kill) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(fd, msg_txt(14)); // Character killed.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(alive) {
	/* if not dead, call @heal command */
	if (!pc_isdead(sd))
		return atcommand_heal(fd, sd, original_command, "@heal", "");

	sd->status.hp = sd->status.max_hp;
	sd->status.sp = sd->status.max_sp;
	clif_skill_nodamage(&sd->bl, &sd->bl, ALL_RESURRECTION, 4, 1);
	pc_setstand(sd);
	if (battle_config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
	clif_updatestatus(sd, SP_HP);
	clif_updatestatus(sd, SP_SP);
	clif_resurrection(&sd->bl, 1);
	clif_displaymessage(fd, msg_txt(16)); // You've been revived! It's a miracle!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(heal) {
	int hp = 0, sp = 0;

	if(pc_isdead(sd))
	{
		clif_displaymessage(fd, "You are dead. You must resurrect before to heal you.");
		return -1;
	}

	sscanf(message, "%d %d", &hp, &sp);

	if (hp == 0 && sp == 0) {
		hp = sd->status.max_hp - sd->status.hp;
		sp = sd->status.max_sp - sd->status.sp;
	} else {
		if (hp > 0 && (hp > sd->status.max_hp || hp > (sd->status.max_hp - sd->status.hp))) // fix positiv overflow
			hp = sd->status.max_hp - sd->status.hp;
		else if (hp < 0 && (hp < -sd->status.max_hp || hp < (1 - sd->status.hp))) // fix negativ overflow
			hp = 1 - sd->status.hp;
		if (sp > 0 && (sp > sd->status.max_sp || sp > (sd->status.max_sp - sd->status.sp))) // fix positiv overflow
			sp = sd->status.max_sp - sd->status.sp;
		else if (sp < 0 && (sp < -sd->status.max_sp || sp < (1 - sd->status.sp))) // fix negativ overflow
			sp = 1 - sd->status.sp;
	}

	if (hp > 0) // display like heal
		clif_heal(fd, SP_HP, hp);
	else if (hp < 0) // display like damage
		clif_damage(&sd->bl,&sd->bl, gettick_cache, 0, 0, -hp, 0 , 4, 0);
	if (sp > 0) // no display when we lost SP
		clif_heal(fd, SP_SP, sp);

	if (hp != 0 || sp != 0) {
		pc_heal(sd, hp, sp);
		if (hp >= 0 && sp >= 0)
			clif_displaymessage(fd, msg_txt(17)); // HP, SP recovered.
		else
			clif_displaymessage(fd, msg_txt(156)); // HP or/and SP modified.
	} else {
		clif_displaymessage(fd, msg_txt(157)); // HP and SP are already with the good values.
		return -1;
	}

	return 0;
}

/*==========================================
 * Kami + Color (@kamic), based on [LuzZza]'s code
 * Added to Nezumi by [akrus]
 *------------------------------------------
 */
ATCOMMAND_FUNC(kami) {
	unsigned int color;

	switch (*(command + 5)) {
	case 'c': // command is convert to lower before to be called
		if (!message || !*message || sscanf(message, "%x %[^\n]", &color, atcmd_output) < 2) {
			send_usage(fd, "Please, enter color and message (usage: %s <hex_color> <message>).", original_command);
			return -1;
		}
		if (color > 0xFFFFFF) {
			clif_displaymessage(fd, "Invalid color.");
			return -1;
		}
		intif_announce(atcmd_output, color, 0);
		break;
	default:
		if (!message || !*message) {
			send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
			return -1;
		}
		intif_GMmessage((char*)message, 0);
		break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kamib) {
	if (!message || !*message) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (check_bad_word(message, strlen(message), sd))
		return -1; // check_bad_word function display message if necessary

	intif_GMmessage((char*)message, 0x10);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kamiGM) {
	int min_level;
	char message_to_gm[100];
	char message_to_gm2[125];

	memset(message_to_gm2, 0, sizeof(message_to_gm2));

	if (!message || !*message || sscanf(message, "%d %[^\n]", &min_level, message_to_gm) < 2) {
		send_usage(fd, "Please, enter a level and a message (usage: %s <min_gm_level> <message>).", original_command);
		return -1;
	}

	if (check_bad_word(message_to_gm, strlen(message_to_gm), sd))
		return -1; // check_bad_word function display message if necessary

	intif_wis_message_to_gm(sd->status.name, min_level, message_to_gm);

	if (min_level == 0)
		sprintf(message_to_gm2, msg_txt(593), min_level); // (to all players) 
	else
		sprintf(message_to_gm2, msg_txt(541), min_level); // (to GM >= %d) 
	strncpy(message_to_gm2 + strlen(message_to_gm2), message_to_gm, strlen(message_to_gm));
	clif_wis_message(fd, "You", message_to_gm2, strlen(message_to_gm2) + 1); // R 0097 <len>.w <nick>.24B <message>.?B

	return 0;
}

/*==========================================
 * function to check if an item is not forbidden for creation
 * return 0: not authorized
 * return not 0: authorized
 *------------------------------------------
 */
int check_item_authorization(const int item_id, const unsigned char gm_level) {
	FILE *fp;
	char line[512];
	int id, level;
	const char *filename = "db/item_deny.txt";

	if ((fp = fopen(filename, "r")) == NULL) {
		//printf("Items deny file not found: %s.\n", filename); // not display every time
		return 1; // by default: not deny -> authorized
	}

	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		level = 100; // default: if no mentioned GM level, no creation -> 100
		if (sscanf(line, "%d,%d", &id, &level) < 1)
			continue;
		if (id == item_id) {
			fclose(fp);
			if (gm_level < level)
				return 0; // not authorized
			else
				return 1; // authorized
		}
	}
	fclose(fp);

	return 1; // autorised
}

/*==========================================
 * @item command (usage: @item <name/id_of_item> [quantity]) (modified by [Yor] for pet_egg)
 *------------------------------------------
 */
ATCOMMAND_FUNC(item) {
	char item_name[100];
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d", item_name, &number) < 1) {
		send_usage(fd, "Please, enter an item name/id (usage: %s <item name | ID> [quantity]).", original_command);
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		if (check_item_authorization(item_id, sd->GM_level)) {
			get_count = number;
			// check pet egg
			pet_id = search_petDB_index(item_id, PET_EGG);
			if (item_data->type == 4 || item_data->type == 5 || // 4 = weapons, 5 = armors
				item_data->type == 7 || item_data->type == 8) { // 7 = eggs, 8 = pets accessories
				get_count = 1;
			}
			for (i = 0; i < number; i += get_count) {
				// if pet egg
				if (pet_id >= 0) {
					sd->catch_target_class = pet_db[pet_id].class;
					intif_create_pet(sd->status.account_id, sd->status.char_id,
					                 pet_db[pet_id].class, mob_db[pet_db[pet_id].class].lv,
					                 pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
					                 100, 0, 1, pet_db[pet_id].jname);
				// if not pet egg
				} else {
					memset(&item_tmp, 0, sizeof(item_tmp));
					item_tmp.nameid = item_id;
					item_tmp.identify = 1;
					if ((battle_config.atcommand_item_creation_name_input && !item_data->slot) ||
					    battle_config.atcommand_item_creation_name_input > 1) {
						item_tmp.card[0] = 0x00fe; // add GM name
						item_tmp.card[1] = 0;
						*((unsigned long *)(&item_tmp.card[2])) = sd->char_id;
					}
					if ((flag = pc_additem(sd, &item_tmp, get_count)))
						clif_additem(sd, 0, 0, flag);
				}
			}
			clif_displaymessage(fd, msg_txt(18)); // Item created.
		} else {
			clif_displaymessage(fd, "You are not authorized to create this item.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charitem/@giveitem command (usage: @charitem/@giveitem <name/id_of_item> <quantity>) (modified by [Yor] for pet_egg)
 *------------------------------------------
 */
ATCOMMAND_FUNC(charitem) {
	struct map_session_data *pl_sd;
	char item_name[100];
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d %[^\n]", item_name, &number, atcmd_name) < 3) {
		send_usage(fd, "Please, enter an item name/id and a player name (usage: %s <item name | ID> <quantity> <char name|account_id>).", original_command);
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		if (check_item_authorization(item_id, sd->GM_level)) {
			get_count = number;
			// check pet egg
			pet_id = search_petDB_index(item_id, PET_EGG);
			if (item_data->type == 4 || item_data->type == 5 || // 4 = weapons, 5 = armors
				item_data->type == 7 || item_data->type == 8) { // 7 = eggs, 8 = pets accessories
				get_count = 1;
			}
			if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
				for (i = 0; i < number; i += get_count) {
					// if pet egg
					if (pet_id >= 0) {
						pl_sd->catch_target_class = pet_db[pet_id].class;
						intif_create_pet(pl_sd->status.account_id, pl_sd->status.char_id,
						                 pet_db[pet_id].class, mob_db[pet_db[pet_id].class].lv,
						                 pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
						                 100, 0, 1, pet_db[pet_id].jname);
					// if not pet egg
					} else {
						memset(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = item_id;
						item_tmp.identify = 1;
						if ((battle_config.atcommand_item_creation_name_input && !item_data->slot) ||
						    battle_config.atcommand_item_creation_name_input > 1) {
							item_tmp.card[0] = 0x00fe; // add GM name
							item_tmp.card[1] = 0;
							*((unsigned long *)(&item_tmp.card[2])) = sd->char_id;
						}
						if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
							clif_additem(pl_sd, 0, 0, flag);
					}
				}
				sprintf(atcmd_output, "Player received %d %s.", number, item_data->jname);
				clif_displaymessage(fd, atcmd_output);
			} else {
				clif_displaymessage(fd, msg_txt(3)); // Character not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, "You are not authorized to create this item.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * @itemall/@charitemall/@giveitemall command (usage: @itemall/@charitemall/@giveitemall <name/id_of_item> [quantity])
 *------------------------------------------
 */
ATCOMMAND_FUNC(charitemall) {
	struct map_session_data *pl_sd;
	char item_name[100];
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, j, pet_id, c;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d", item_name, &number) < 1) {
		send_usage(fd, "Please, enter an item name/id (usage: %s <item name | ID> [quantity]).", original_command);
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		if (check_item_authorization(item_id, sd->GM_level)) {
			get_count = number;
			// check pet egg
			pet_id = search_petDB_index(item_id, PET_EGG);
			if (item_data->type == 4 || item_data->type == 5 || // 4 = weapons, 5 = armors
				item_data->type == 7 || item_data->type == 8) { // 7 = eggs, 8 = pets accessories
				get_count = 1;
			}
			c = 0;
			for (j = 0; j < fd_max; j++) {
				if (session[j] && (pl_sd = session[j]->session_data) && pl_sd->state.auth) {
					for (i = 0; i < number; i += get_count) {
						// if pet egg
						if (pet_id >= 0) {
							pl_sd->catch_target_class = pet_db[pet_id].class;
							intif_create_pet(pl_sd->status.account_id, pl_sd->status.char_id,
							                 pet_db[pet_id].class, mob_db[pet_db[pet_id].class].lv,
							                 pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
							                 100, 0, 1, pet_db[pet_id].jname);
						// if not pet egg
						} else {
							memset(&item_tmp, 0, sizeof(item_tmp));
							item_tmp.nameid = item_id;
							item_tmp.identify = 1;
							if ((battle_config.atcommand_item_creation_name_input && !item_data->slot) ||
							    battle_config.atcommand_item_creation_name_input > 1) {
								item_tmp.card[0] = 0x00fe; // add GM name
								item_tmp.card[1] = 0;
								*((unsigned long *)(&item_tmp.card[2])) = sd->char_id;
							}
							if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
								clif_additem(pl_sd, 0, 0, flag);
						}
					}
					sprintf(atcmd_output, "You got %d %s.", number, item_data->jname);
					clif_displaymessage(pl_sd->fd, atcmd_output);
					c++;
				}
			}
			sprintf(atcmd_output, "Everyone received %d %s.", number, item_data->jname);
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, "You are not authorized to create this item.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(item2) {
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag;
	int loop, get_count, i;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9) {
		send_usage(fd, "Please, enter all informations (usage: %s <item name or ID> <quantity>", original_command);
		send_usage(fd, "  <Identify_flag> <refine> <attribut> <Card1> <Card2> <Card3> <Card4>).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		if (check_item_authorization(item_id, sd->GM_level)) {
			loop = 1;
			get_count = number;
			if (item_data->type == 4 || item_data->type == 5 || // 4 = weapons, 5 = armors
				item_data->type == 7 || item_data->type == 8) { // 7 = eggs, 8 = pets accessories
				loop = number;
				get_count = 1;
				if (item_data->type == 7) { // 7 = eggs
					identify = 1;
					refine = 0;
				}
				if (item_data->type == 8) // 8 = pets accessories
					refine = 0;
			} else {
				identify = 1;
				refine = 0;
				attr = 0;
			}
			if (refine > 10)
				refine = 10;
			else if (refine < 0)
				refine = 0;
			for (i = 0; i < loop; i++) {
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = item_id;
				item_tmp.identify = identify;
				item_tmp.refine = refine;
				item_tmp.attribute = attr;
				item_tmp.card[0] = c1;
				item_tmp.card[1] = c2;
				item_tmp.card[2] = c3;
				item_tmp.card[3] = c4;
				if ((flag = pc_additem(sd, &item_tmp, get_count)))
					clif_additem(sd, 0, 0, flag);
			}
			clif_displaymessage(fd, msg_txt(18)); // Item created.
		} else {
			clif_displaymessage(fd, "You are not authorized to create this item.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(itemreset) {
	int i;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount && sd->status.inventory[i].equip == 0)
			pc_delitem(sd, i, sd->status.inventory[i].amount, 0);
	}
	clif_displaymessage(fd, msg_txt(20)); // All of your inventory items have been removed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charitemreset) {
	struct map_session_data *pl_sd;
	int i;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			for (i = 0; i < MAX_INVENTORY; i++) {
				if (pl_sd->status.inventory[i].amount && pl_sd->status.inventory[i].equip == 0)
					pc_delitem(pl_sd, i, pl_sd->status.inventory[i].amount, 0);
			}
			clif_displaymessage(sd->fd, msg_txt(20)); // All of your inventory items have been removed.
			clif_displaymessage(fd, msg_txt(241)); // All inventory items of the player have been removed.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(itemcheck) {
	pc_checkitem(sd);
	clif_displaymessage(sd->fd, msg_txt(242)); // All your items have been checked.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charitemcheck) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			pc_checkitem(pl_sd);
			if (pl_sd != sd)
				clif_displaymessage(pl_sd->fd, "All your items have been checked by a GM.");
			clif_displaymessage(fd, "Item check on the player done.");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(baselevelup) {
	int level, i, modified_stat;
	short modified_status[6]; // need to update only modifed stats

	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		send_usage(fd, "Please, enter a level adjustement (usage: %s <number of levels>).", original_command);
		return -1;
	}

	if (level > 0) {
		if (sd->status.base_level == battle_config.maximum_level) {	// check for max level by Valaris
			clif_displaymessage(fd, msg_txt(47)); // Base level can't go any higher.
			return -1;
		}
		if (level > battle_config.maximum_level || level > (battle_config.maximum_level - sd->status.base_level)) // fix positiv overflow
			level = battle_config.maximum_level - sd->status.base_level;
		for (i = 1; i <= level; i++)
			sd->status.status_point += (sd->status.base_level + i + 14) / 5;
		// if player have max in all stats, don't give status_point
		if (sd->status.str  >= battle_config.max_parameter &&
		    sd->status.agi  >= battle_config.max_parameter &&
		    sd->status.vit  >= battle_config.max_parameter &&
		    sd->status.int_ >= battle_config.max_parameter &&
		    sd->status.dex  >= battle_config.max_parameter &&
		    sd->status.luk  >= battle_config.max_parameter)
			sd->status.status_point = 0;
		sd->status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		status_calc_pc(sd, 0);
		pc_heal(sd, sd->status.max_hp, sd->status.max_sp);
		clif_misceffect(&sd->bl, 0);
		clif_displaymessage(fd, msg_txt(21)); // Base level raised.
	} else {
		if (sd->status.base_level == 1) {
			clif_displaymessage(fd, msg_txt(158)); // Base level can't go any lower.
			return -1;
		}
		if (level < -battle_config.maximum_level || level < (1 - sd->status.base_level)) // fix negativ overflow
			level = 1 - sd->status.base_level;
		for (i = 0; i > level; i--)
			sd->status.status_point -= (sd->status.base_level + i + 14) / 5;
		if (sd->status.status_point < 0) {
			short* status[] = {
				&sd->status.str,  &sd->status.agi, &sd->status.vit,
				&sd->status.int_, &sd->status.dex, &sd->status.luk
			};
			// remove points from stats begining by the upper
			for (i = 0; i <= MAX_STATUS_TYPE; i++)
				modified_status[i] = 0;
			while (sd->status.status_point < 0 &&
			       sd->status.str > 0 &&
			       sd->status.agi > 0 &&
			       sd->status.vit > 0 &&
			       sd->status.int_ > 0 &&
			       sd->status.dex > 0 &&
			       sd->status.luk > 0 &&
			       (sd->status.str + sd->status.agi + sd->status.vit + sd->status.int_ + sd->status.dex + sd->status.luk > 6)) {
				modified_stat = 0;
				for (i = 1; i <= MAX_STATUS_TYPE; i++)
					if (*status[modified_stat] < *status[i])
						modified_stat = i;
				sd->status.status_point += (*status[modified_stat] + 8) / 10 + 1; // ((val - 1) + 9) / 10 + 1
				*status[modified_stat] = *status[modified_stat] - ((short)1);
				modified_status[modified_stat] = 1; // flag
			}
			for (i = 0; i <= MAX_STATUS_TYPE; i++)
				if (modified_status[i]) {
					clif_updatestatus(sd, SP_STR + i);
					clif_updatestatus(sd, SP_USTR + i);
				}
		}
		clif_updatestatus(sd, SP_STATUSPOINT);
		sd->status.base_level += level; // note: here, level is negativ
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(22)); // Base level lowered.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(joblevelup) {
	int up_level, level;
	struct pc_base_job s_class;

	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		send_usage(fd, "Please, enter a level adjustement (usage: %s <number of levels>).", original_command);
		return -1;
	}

	s_class = pc_calc_base_job(sd->status.class);

	up_level = 50;
	if (s_class.job == 0) // novice
		up_level -= 40;
	// super novices can go up to 99 [celest]
	else if (s_class.job == 23)
		up_level += 49;
	else if (sd->status.class > 4007 && sd->status.class < 4023)
		up_level += 20;

	if (level > 0) {
		if (sd->status.job_level == up_level) {
			clif_displaymessage(fd, msg_txt(23)); // Job level can't go any higher.
			return -1;
		}
		if (level > up_level || level > (up_level - sd->status.job_level)) // fix positiv overflow
			level = up_level - sd->status.job_level;
		// check with maximum authorized level
		if (sd->status.class == 0) { // Novice
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_novice) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_novice)
				level = battle_config.atcommand_max_job_level_novice - sd->status.job_level;
		} else if (sd->status.class <= 6) { // 1st Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_job1) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_job1)
				level = battle_config.atcommand_max_job_level_job1 - sd->status.job_level;
		} else if (sd->status.class <= 22) { // 2nd Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_job2) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_job2)
				level = battle_config.atcommand_max_job_level_job2 - sd->status.job_level;
		} else if (sd->status.class == 23) { // Super Novice
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_supernovice) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_supernovice)
				level = battle_config.atcommand_max_job_level_supernovice - sd->status.job_level;
		} else if (sd->status.class == 4001) { // High Novice
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_highnovice) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_highnovice)
				level = battle_config.atcommand_max_job_level_highnovice - sd->status.job_level;
		} else if (sd->status.class <= 4007) { // High 1st Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_highjob1) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_highjob1)
				level = battle_config.atcommand_max_job_level_highjob1 - sd->status.job_level;
		} else if (sd->status.class <= 4022) { // High 2nd Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_highjob2) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_highjob2)
				level = battle_config.atcommand_max_job_level_highjob2 - sd->status.job_level;
		} else if (sd->status.class == 4023) { // Baby Novice
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_babynovice) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_babynovice)
				level = battle_config.atcommand_max_job_level_babynovice - sd->status.job_level;
		} else if (sd->status.class <= 4029) { // Baby 1st Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_babyjob1) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_babyjob1)
				level = battle_config.atcommand_max_job_level_babyjob1 - sd->status.job_level;
		} else if (sd->status.class <= 4044) { // Baby 2nd Job
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_babyjob2) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_babyjob2)
				level = battle_config.atcommand_max_job_level_babyjob2 - sd->status.job_level;
		} else if (sd->status.class == 4045) { // Super Baby
			if (sd->status.job_level >= battle_config.atcommand_max_job_level_superbaby) {
				clif_displaymessage(fd, "You're not authorized to increase more your job level.");
				return -1;
			} else if (sd->status.job_level + level >= battle_config.atcommand_max_job_level_superbaby)
				level = battle_config.atcommand_max_job_level_superbaby - sd->status.job_level;
		}
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		sd->status.skill_point += level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		status_calc_pc(sd, 0);
		clif_misceffect(&sd->bl, 1);
		clif_displaymessage(fd, msg_txt(24)); // Job level raised.
	} else {
		if (sd->status.job_level == 1) {
			clif_displaymessage(fd, msg_txt(159)); // Job level can't go any lower.
			return -1;
		}
		if (level < -up_level || level < (1 - sd->status.job_level)) // fix negativ overflow
			level = 1 - sd->status.job_level;
		// don't check maximum authorized if we reduce level
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		if (sd->status.skill_point > 0) {
			sd->status.skill_point += level; // note: here, level is negativ
			if (sd->status.skill_point < 0) {
				level = sd->status.skill_point;
				sd->status.skill_point = 0;
			}
			clif_updatestatus(sd, SP_SKILLPOINT);
		}
		if (level < 0) { // if always negativ, skill points must be removed from skills
			// to add: remove skill points from skills
		}
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(25)); // Job level lowered.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(help) {
	char buf[256], w1[256], w2[256];
	char cmd_line[1000][256];
	char key[256];
	struct {
		char key_name[256];
		int key_line;
	} keys[200];
	int i, j, k, counter;
	int counter_keys, founded_key;
	int page, first_line, last_line;
	char *p;
	FILE* fp;

	memset(buf, 0, sizeof(buf));
	memset(w1, 0, sizeof(w1));
	memset(w2, 0, sizeof(w2));
	memset(cmd_line, 0, sizeof(cmd_line));
	memset(key, 0, sizeof(key));
	memset(keys, 0, sizeof(keys));

	if ((fp = fopen(help_txt, "r")) == NULL) {
		// if not found, try default help file name */
		if ((fp = fopen("conf/help.txt", "r")) == NULL) {
			clif_displaymessage(fd, msg_txt(27)); // File 'help.txt' not found.
			return -1;
		}
	}

	if ((page = atoi(message)) < 0) {
		page = 0;
		strcpy(key, "0");
	} else {
		for (i = 0; message[i]; i++)
			key[i] = tolower((unsigned char)message[i]); // tolower needs unsigned char
	}

	counter = 0;
	counter_keys = 0;
	founded_key = -1;
	for (i = 0; i < (int)(sizeof(keys) / sizeof(keys[0])); i++)
		keys[i].key_line = sizeof(cmd_line) / sizeof(cmd_line[0]); // init finish to max value
	// get lines to display
	while(fgets(buf, sizeof(buf), fp) != NULL && counter < (int)(sizeof(cmd_line) / sizeof(cmd_line[0]))) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if (buf[0] == '/' && buf[1] == '/')
			continue;
		for (i = 0; buf[i] != '\0'; i++) {
			if (buf[i] == '\r' || buf[i] == '\n') {
				buf[i] = '\0';
				break;
			}
		}
		memset(w2, 0, sizeof(w2));
		// help line without GM level
		if (sscanf(buf, "%255[^:]:%255[^\n]", w1, w2) < 2) {
			strcpy(cmd_line[counter], buf);
			counter++;
		} else {
			/* for compare, reduce in lowercase */
			for (i = 0; w1[i]; i++)
				w1[i] = tolower((unsigned char)w1[i]); // tolower needs unsigned char
			/* remove space at start */
			while (w1[0] == 32)
				for (i = 0; w1[i]; i++)
					w1[i] = w1[i + 1];
			/* check space at last */
			while ((i = strlen(w1)) > 0 && w1[i - 1] == 32)
				w1[i - 1] = '\0';
			// keys saving
			if (strcmp(w1, "key") == 0) {
				if (counter_keys < (int)(sizeof(keys) / sizeof(keys[0]))) {
					for (i = 0; w2[i]; i++)
						w2[i] = tolower((unsigned char)w2[i]); // tolower needs unsigned char
					sprintf(keys[counter_keys].key_name, "%s,", w2); // add a final ',' to check key+','
					keys[counter_keys].key_line = counter;
					sprintf(w2, "%s,", key);
					if (strstr(keys[counter_keys].key_name, w2) != NULL) // search with no case sensitive
						founded_key = counter_keys;
					counter_keys++;
				}
			// help line with GM level
			} else {
				// search gm command type
				i = 0;
				while (atcommand_info[i].type != AtCommand_Unknown) {
					if (strcmp(w1 + 1, atcommand_info[i].command + 1) == 0) {
						if (sd->GM_level >= atcommand_info[i].level) {
							strcpy(cmd_line[counter], w2);
							counter++;
						}
						break;
					}
					i++;
				}
				// if not found
				if (atcommand_info[i].type == AtCommand_Unknown)
					if (sd->GM_level >= atoi(w1)) {
						strcpy(cmd_line[counter], w2);
						counter++;
					}
			}
		}
	}

	fclose(fp);

	// remove @ to replace them by right command_symbol
	if (command_symbol != '@') {
		for(i = 0; i < counter; i++) {
			char* first_arobas;
			first_arobas = cmd_line[i] - 1;
			while((first_arobas = strchr(first_arobas + 1, '@')) != NULL) {
				j = 0;
				while (atcommand_info[j].type != AtCommand_Unknown) {
					if (strncasecmp(first_arobas + 1, atcommand_info[j].command + 1, strlen(atcommand_info[j].command + 1)) == 0) {
						if (strncasecmp(first_arobas + 1, "email.", 6) != 0) { // don't change a email :) new@email.com for example
							first_arobas[0] = command_symbol;
							break;
						}
					}
					j++;
				}
			}
		}
	}

	// Displaying of lines (pages method, or all pages)
	if (page > 0 || !message || !*message || strcmp(key, "0") == 0) {
		// Calculation of first and last line
		if (page == 0) { // we display all lines
			first_line = 0;
			last_line = counter;
		} else {
			if ((page - 1) * 50 > counter)
				page = (counter / 50) + 1;
			first_line = (page - 1) * 50;
			last_line = first_line + 50;
			if ((counter - 10) < last_line) // display all rest of GM command if there is less 10 GM command in last page
				last_line = counter;
			if ((counter - 10) < first_line && first_line >= 50) {
				first_line = first_line - 50;
				page--;
			}
		}
		// Displaying of the help
		if (first_line == 0 && last_line == counter)
			clif_displaymessage(fd, msg_txt(26)); // Help commands:
		else {
			sprintf(buf, "----- Help commands (page %d):", page);
			clif_displaymessage(fd, buf);
		}
		for(i = first_line; i < last_line; i++) {
			clif_displaymessage(fd, cmd_line[i]);
		}

	// Displaying of lines (key method)
	} else {
		// if asked key not found or display of all keywords
		if (founded_key == -1 || strncmp(key, "key", 3) == 0) { // 'key' or 'keyword'
			if (strncmp(key, "key", 3) != 0) {
				sprintf(buf, "----- Help commands keys (key: %s) - key not found!", key);
				clif_displaymessage(fd, buf);
			} else {
				sprintf(atcmd_output, "----- Help command keys (%s <key_word>):", original_command);
				clif_displaymessage(fd, atcmd_output);
			}
			memset(cmd_line, 0, sizeof(cmd_line)); // so save all keywords and not repeat them
			counter = 0;
			for(i = 0; i < counter_keys; i++) {
				p = keys[i].key_name;
				while(sscanf(p, "%255[^,],%n", w2, &j) == 1 && counter < (int)(sizeof(cmd_line) / sizeof(cmd_line[0]))) {
					if (strcmp(w2, "all") != 0) { // all is a special key
						// check duplicate keywords
						for(k = 0; k < counter; k++)
							if (strcmp(cmd_line[k], w2) == 0)
								break;
						if (k == counter) {
							strcpy(cmd_line[counter], w2);
							counter++;
						}
					}
					p = p + j;
				}
			}
			if (counter == 0)
				clif_displaymessage(fd, "There is no keyword in the help file.");
			else {
				for(i = 0; i < counter; i++)
					clif_displaymessage(fd, cmd_line[i]);
			}
		// if key found
		} else {
			sprintf(buf, "----- Help commands (key: %s):", key);
			clif_displaymessage(fd, buf);
			strcat(key, ",");
			for(j = 0; j < counter_keys; j++) {
				if (strstr(keys[j].key_name, "all,") != NULL || strstr(keys[j].key_name, key) != NULL) {
					first_line = keys[j].key_line;
					if ((j - 1) < counter_keys) {
						last_line = keys[j+1].key_line;
						if (last_line > counter)
							last_line = counter;
					} else
						last_line = counter;
					for(i = first_line; i < last_line; i++)
						clif_displaymessage(fd, cmd_line[i]);
					if (last_line == counter)
						break;
				}
			}
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(gm) {
	char password[100];

	memset(password, 0, sizeof(password));

	if (!message || !*message || sscanf(message, "%[^\n]", password) < 1) {
		send_usage(fd, "Please, enter a password (usage: %s <password>).", original_command);
		return -1;
	}

	if (sd->GM_level <= battle_config.atcommand_max_player_gm_level) { // a GM can not use this function. only a normal player (become gm is not for gm!)
		clif_displaymessage(fd, msg_txt(50)); // You already have some GM powers.
		return -1;
	} else
		chrif_changegm(sd->status.account_id, password, strlen(password) + 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(pvpoff) {
	struct map_session_data *pl_sd;
	int i;

	if (battle_config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_txt(52)); // This option cannot be used in PK Mode.
		return -1;
	}

	if (map[sd->bl.m].flag.pvp) {
		map[sd->bl.m].flag.pvp = 0;
		clif_send0199(sd->bl.m, 0);
		for (i = 0; i < fd_max; i++) { //�l�������[�v
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m) {
					clif_pvpset(pl_sd, 0, 0, 2); // send packet to player to say: pvp area
					if (pl_sd->pvp_timer != -1) {
						delete_timer(pl_sd->pvp_timer, pc_calc_pvprank_timer);
						pl_sd->pvp_timer = -1;
					}
				}
			}
		}
		clif_displaymessage(fd, msg_txt(31)); // PvP: Off.
	} else {
		clif_displaymessage(fd, msg_txt(160)); // PvP is already Off.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(pvpon) {
	struct map_session_data *pl_sd;
	int i;

	if (battle_config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_txt(52)); // This option cannot be used in PK Mode.
		return -1;
	}

	if (!map[sd->bl.m].flag.pvp) {
		map[sd->bl.m].flag.pvp = 1;
		clif_send0199(sd->bl.m, 1);
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m && pl_sd->pvp_timer == -1) {
					pl_sd->pvp_timer = add_timer(gettick_cache + 200, pc_calc_pvprank_timer, pl_sd->bl.id, 0);
					pl_sd->pvp_rank = 0;
					pl_sd->pvp_lastusers = 0;
					pl_sd->pvp_point = 5;
				}
			}
		}
		clif_displaymessage(fd, msg_txt(32)); // PvP: On.
	} else {
		clif_displaymessage(fd, msg_txt(161)); // PvP is already On.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(gvgoff) {
	if (map[sd->bl.m].flag.gvg) {
		map[sd->bl.m].flag.gvg = 0;
		clif_send0199(sd->bl.m, 0);
		clif_displaymessage(fd, msg_txt(33)); // GvG: Off.
	} else {
		clif_displaymessage(fd, msg_txt(162)); // GvG is already Off.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(gvgon) {
	if (!map[sd->bl.m].flag.gvg) {
		map[sd->bl.m].flag.gvg = 1;
		clif_send0199(sd->bl.m, 3);
		clif_displaymessage(fd, msg_txt(34)); // GvG: On.
	} else {
		clif_displaymessage(fd, msg_txt(163)); // GvG is already On.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(model) {
	int hair_style = 0, hair_color = 0, cloth_color = 0;

	if (!message || !*message || sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1) {
		send_usage(fd, "Please, enter at least a value (usage: %s <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).",
		           original_command, battle_config.min_hair_style, battle_config.max_hair_style, battle_config.min_hair_color, battle_config.max_hair_color, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	if (hair_style >= battle_config.min_hair_style && hair_style <= battle_config.max_hair_style &&
		hair_color >= battle_config.min_hair_color && hair_color <= battle_config.max_hair_color &&
		cloth_color >= battle_config.min_cloth_color && cloth_color <= battle_config.max_cloth_color) {
		//���̐F�ύX
		if (!battle_config.clothes_color_for_assassin &&
		    cloth_color != 0 && sd->status.sex == 1 && (sd->status.class == 12 ||  sd->status.class == 17)) {
			//���̐F�������E�̔���
			clif_displaymessage(fd, msg_txt(35)); // You can't use this clothes color with this class.
			return -1;
		} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		send_usage(fd, "Please, enter a valid value (usage: %s <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).",
		           original_command, battle_config.min_hair_style, battle_config.max_hair_style, battle_config.min_hair_color, battle_config.max_hair_color, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	return 0;
}

/*==========================================
 * @dye && @ccolor
 *------------------------------------------
 */
ATCOMMAND_FUNC(dye) {
	int cloth_color;

	if (!message || !*message || sscanf(message, "%d", &cloth_color) < 1) {
		send_usage(fd, "Please, enter a clothes color (usage: %s <clothes color: %d-%d>).", original_command, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	if (cloth_color >= battle_config.min_cloth_color && cloth_color <= battle_config.max_cloth_color) {
		if (!battle_config.clothes_color_for_assassin &&
		    cloth_color != 0 && sd->status.sex == 1 && (sd->status.class == 12 || sd->status.class == 17)) {
			clif_displaymessage(fd, msg_txt(35)); // You can't use this clothes color with this class.
			return -1;
		} else {
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		send_usage(fd, "Please, enter a valid clothes color (usage: %s <clothes color: %d-%d>).", original_command, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	return 0;
}

/*==========================================
 * @chardye / @charccolor
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardye) {
	int cloth_color;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &cloth_color, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a clothes color and a player name (usage: %s <clothes color: %d-%d> <char name|account_id>).", original_command, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (cloth_color >= battle_config.min_cloth_color && cloth_color <= battle_config.max_cloth_color) {
				if (!battle_config.clothes_color_for_assassin &&
				    cloth_color != 0 && pl_sd->status.sex == 1 && (pl_sd->status.class == 12 || pl_sd->status.class == 17)) {
					clif_displaymessage(fd, "You can't use this clothes color with the class of this player.");
					return -1;
				} else {
					pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
					clif_displaymessage(fd, "Player's appearence changed.");
				}
			} else {
				clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
				send_usage(fd, "Please, enter a valid clothes color (usage: %s <clothes color: %d-%d> <char name|account_id>).", original_command, battle_config.min_cloth_color, battle_config.max_cloth_color);
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @hairstyle && @hstyle
 *------------------------------------------
 */
ATCOMMAND_FUNC(hair_style) {
	int hair_style;

	if (!message || !*message || sscanf(message, "%d", &hair_style) < 1) {
		send_usage(fd, "Please, enter a hair style (usage: %s <hair ID: %d-%d>).", original_command, battle_config.min_hair_style, battle_config.max_hair_style);
		return -1;
	}

	if (hair_style >= battle_config.min_hair_style && hair_style <= battle_config.max_hair_style) {
		pc_changelook(sd, LOOK_HAIR, hair_style);
		clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		send_usage(fd, "Please, enter a valid hair style (usage: %s <hair ID: %d-%d>).", original_command, battle_config.min_hair_style, battle_config.max_hair_style);
		return -1;
	}

	return 0;
}

/*==========================================
 * @charhairstyle / @charhstyle
 *------------------------------------------
 */
ATCOMMAND_FUNC(charhairstyle) {
	int hair_style;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &hair_style, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a hair style and a player name (usage: %s <hair ID: %d-%d> <char name|account_id>).", original_command, battle_config.min_hair_style, battle_config.max_hair_style);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (hair_style >= battle_config.min_hair_style && hair_style <= battle_config.max_hair_style) {
				pc_changelook(pl_sd, LOOK_HAIR, hair_style);
				clif_displaymessage(fd, "Player's appearence changed.");
			} else {
				clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
				send_usage(fd, "Please, enter a valid hair style and a player name (usage: %s <hair ID: %d-%d> <char name|account_id>).", original_command, battle_config.min_hair_style, battle_config.max_hair_style);
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @haircolor && @hcolor
 *------------------------------------------
 */
ATCOMMAND_FUNC(hair_color) {
	int hair_color;

	if (!message || !*message || sscanf(message, "%d", &hair_color) < 1) {
		send_usage(fd, "Please, enter a hair color (usage: %s <hair color: %d-%d>).", original_command, battle_config.min_hair_color, battle_config.max_hair_color);
		return -1;
	}

	if (hair_color >= battle_config.min_hair_color && hair_color <= battle_config.max_hair_color) {
		pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
		clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		send_usage(fd, "Please, enter a valid hair color (usage: %s <hair color: %d-%d>).", original_command, battle_config.min_hair_color, battle_config.max_hair_color);
		return -1;
	}

	return 0;
}

/*==========================================
 * @charhaircolor / @charhcolor
 *------------------------------------------
 */
ATCOMMAND_FUNC(charhaircolor) {
	int hair_color;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &hair_color, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a hair color and a player name (usage: %s <hair color: %d-%d> <char name|account_id>).", original_command, battle_config.min_hair_color, battle_config.max_hair_color);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (hair_color >= battle_config.min_hair_color && hair_color <= battle_config.max_hair_color) {
				pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
				clif_displaymessage(fd, "Player's appearence changed.");
			} else {
				clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
				send_usage(fd, "Please, enter a valid hair color and a player name (usage: %s <hair color: %d-%d> <char name|account_id>).", original_command, battle_config.min_hair_color, battle_config.max_hair_color);
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @go [city_number/city_name]: improved by [yor] to add city names and help
 *------------------------------------------
 */
ATCOMMAND_FUNC(go) {
	int i;
	int town;
	int m;

	const struct { char map[17];  int x,   y; } data[] = { // 16 + NULL
	             { "prontera.gat",    157, 191 },	//	 0=Prontera
	             { "morocc.gat",      156,  93 },	//	 1=Morroc
	             { "geffen.gat",      119,  59 },	//	 2=Geffen
	             { "payon.gat",       162, 233 },	//	 3=Payon
	             { "alberta.gat",     192, 147 },	//	 4=Alberta
	             { "izlude.gat",      128, 114 },	//	 5=Izlude
	             { "aldebaran.gat",   140, 131 },	//	 6=Al de Baran
	             { "xmas.gat",        147, 134 },	//	 7=Lutie
	             { "comodo.gat",      209, 143 },	//	 8=Comodo
	             { "yuno.gat",        157,  51 },	//	 9=Yuno
	             { "amatsu.gat",      198,  84 },	//	10=Amatsu
	             { "gonryun.gat",     160, 120 },	//	11=Gon Ryun
	             { "umbala.gat",       89, 157 },	//	12=Umbala
	             { "niflheim.gat",     21, 153 },	//	13=Niflheim
	             { "louyang.gat",     217,  40 },	//	14=Lou Yang
	             { "new_1-1.gat",      53, 111 },	//	15=Start point
	             { "sec_pri.gat",      23,  61 },	//	16=Prison
	             { "valkyrie.gat",     49,  48 },	//	17=Valkyrie
	             { "gef_fild10.gat",   66, 335 },	//	18=Orc dungeon
	             { "pay_arche.gat",    56, 132 },	//	19=village (payon)
	             { "glast_01.gat",    200, 129 },	//	20=Glast Heim
	             { "jawaii.gat",      249, 127 },	//	21=Jawaii
	             { "ayothaya.gat",    151, 117 },	//	22=Ayothaya
	             { "einbroch.gat",     64, 200 },	//	23=Einbroch
	             { "lighthalzen.gat", 159,  93 },	//	24=Lighthalzen
	             { "einbech.gat",      70,  95 },	//	25=Einbech
	             { "hugel.gat",        96, 145 },	//	26=Hugel
	};

	if (map[sd->bl.m].flag.nogo && battle_config.any_warp_GM_min_level > sd->GM_level) {
		send_usage(fd, "You can not use %s on this map.", original_command);
		return -1;
	}

	// get the number
	town = atoi(message);

	// if no value, display all value
	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1 || town < -3 || town >= (int)(sizeof(data) / sizeof(data[0]))) {
		send_usage(fd, msg_txt(38)); // Invalid location number or name.
		send_usage(fd, msg_txt(82)); // Please, use one of this numbers/names:
		send_usage(fd, "-3=(Memo point 2)   7=Lutie         17=Valkyrie");
		send_usage(fd, "-2=(Memo point 1)   8=Comodo        18=Orc dungeon");
		send_usage(fd, "-1=(Memo point 0)   9=Yuno          19=village (payon)");
		send_usage(fd, " 0=Prontera        10=Amatsu        20=Glast Heim");
		send_usage(fd, " 1=Morroc          11=Gon Ryun      21=Jawaii");
		send_usage(fd, " 2=Geffen          12=Umbala        22=Ayothaya");
		send_usage(fd, " 3=Payon           13=Niflheim      23=Einbroch");
		send_usage(fd, " 4=Alberta         14=Lou Yang      24=Lighthalzen");
		send_usage(fd, " 5=Izlude          15=Start point   25=Einbech");
		send_usage(fd, " 6=Al de Baran     16=Prison        26=Hugel");
		return -1;
	} else {
		// get possible name of the city and add .gat if not in the name
		for (i = 0; atcmd_mapname[i]; i++)
			atcmd_mapname[i] = tolower((unsigned char)atcmd_mapname[i]); // tolower needs unsigned char
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		// try to see if it's a name, and not a number (try a lot of possibilities, write errors and abbreviations too)
		if (strncmp(atcmd_mapname, "prontera.gat", 3) == 0) { // 3 first characters
			town = 0;
		} else if (strncmp(atcmd_mapname, "morocc.gat", 3) == 0) { // 3 first characters
			town = 1;
		} else if (strncmp(atcmd_mapname, "geffen.gat", 3) == 0) { // 3 first characters
			town = 2;
		} else if (strncmp(atcmd_mapname, "payon.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "paion.gat", 3) == 0) { // writing error (3 first characters)
			town = 3;
		} else if (strncmp(atcmd_mapname, "alberta.gat", 3) == 0) { // 3 first characters
			town = 4;
		} else if (strncmp(atcmd_mapname, "izlude.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "islude.gat", 3) == 0) { // writing error (3 first characters)
			town = 5;
		} else if (strncmp(atcmd_mapname, "aldebaran.gat", 3) == 0 || // 3 first characters
		           strcmp(atcmd_mapname,  "al.gat") == 0) { // al (de baran)
			town = 6;
		} else if (strncmp(atcmd_mapname, "lutie.gat", 3) == 0 || // name of the city, not name of the map (3 first characters)
		           strcmp(atcmd_mapname,  "christmas.gat") == 0 || // name of the symbol
		           strncmp(atcmd_mapname, "xmas.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "x-mas.gat", 3) == 0) { // writing error (3 first characters)
			town = 7;
		} else if (strncmp(atcmd_mapname, "comodo.gat", 3) == 0) { // 3 first characters
			town = 8;
		} else if (strncmp(atcmd_mapname, "yuno.gat", 3) == 0) { // 3 first characters
			town = 9;
		} else if (strncmp(atcmd_mapname, "amatsu.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "ammatsu.gat", 3) == 0) { // writing error (3 first characters)
			town = 10;
		} else if (strncmp(atcmd_mapname, "gonryun.gat", 3) == 0) { // 3 first characters
			town = 11;
		} else if (strncmp(atcmd_mapname, "umbala.gat", 3) == 0) { // 3 first characters
			town = 12;
		} else if (strncmp(atcmd_mapname, "niflheim.gat", 3) == 0) { // 3 first characters
			town = 13;
		} else if (strncmp(atcmd_mapname, "louyang.gat", 3) == 0) { // 3 first characters
			town = 14;
		} else if (strncmp(atcmd_mapname, "new_1-1.gat", 3) == 0 || // 3 first characters (or "newbies")
		           strncmp(atcmd_mapname, "startpoint.gat", 3) == 0 || // name of the position (3 first characters)
		           strncmp(atcmd_mapname, "begining.gat", 3) == 0) { // name of the position (3 first characters)
			town = 15;
		} else if (strncmp(atcmd_mapname, "sec_pri.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "prison.gat", 3) == 0 || // name of the position (3 first characters)
		           strncmp(atcmd_mapname, "jails.gat", 3) == 0) { // name of the position
			town = 16;
		} else if (strncmp(atcmd_mapname, "valkyrie.gat", 3) == 0) { // 3 first characters
			town = 17;
		} else if (strncmp(atcmd_mapname, "gef_fild10.gat", 4) == 0 || // 4 first characters
		           strncmp(atcmd_mapname, "orc dungeon.gat", 3) == 0) { // name of the position (3 first characters)
			town = 18;
		} else if (strncmp(atcmd_mapname, "pay_arche.gat", 4) == 0 || // 4 first characters
		           strncmp(atcmd_mapname, "village.gat", 3) == 0 || // name of the position (3 first characters)
		           strncmp(atcmd_mapname, "archer.gat", 3) == 0) { // name of the position (3 first characters)
			town = 19;
		} else if (strncmp(atcmd_mapname, "glast_01.gat", 3) == 0 || // 3 first characters
		           strncmp(atcmd_mapname, "gh.gat", 3) == 0) { // name of the position (3 first characters)
			town = 20;
		} else if (strncmp(atcmd_mapname, "jawaii.gat", 3) == 0) { // 3 first characters
			town = 21;
		} else if (strncmp(atcmd_mapname, "ayothaya.gat", 3) == 0) { // 3 first characters
			town = 22;
		} else if (strncmp(atcmd_mapname, "einbroch.gat", 5) == 0) { // 5 first characters
			town = 23;
		} else if (strncmp(atcmd_mapname, "lighthalzen.gat", 3) == 0) { // 3 first characters
			town = 24;
		} else if (strncmp(atcmd_mapname, "einbech.gat", 5) == 0) { // 5 first characters
			town = 25;
		} else if (strncmp(atcmd_mapname, "hugel.gat", 3) == 0) { // 3 first characters
			town = 26;
		} else if (sscanf(message, "%d", &i) < 1) { /* not a number */
			send_usage(fd, msg_txt(38)); // Invalid location number or name.
			send_usage(fd, msg_txt(82)); // Please, use one of this numbers/names:
			send_usage(fd, "-3=(Memo point 2)   7=Lutie         17=Valkyrie");
			send_usage(fd, "-2=(Memo point 1)   8=Comodo        18=Orc dungeon");
			send_usage(fd, "-1=(Memo point 0)   9=Yuno          19=village (payon)");
			send_usage(fd, " 0=Prontera        10=Amatsu        20=Glast Heim");
			send_usage(fd, " 1=Morroc          11=Gon Ryun      21=Jawaii");
			send_usage(fd, " 2=Geffen          12=Umbala        22=Ayothaya");
			send_usage(fd, " 3=Payon           13=Niflheim      23=Einbroch");
			send_usage(fd, " 4=Alberta         14=Lou Yang      24=Lighthalzen");
			send_usage(fd, " 5=Izlude          15=Start point   25=Einbech");
			send_usage(fd, " 6=Al de Baran     16=Prison        26=Hugel");
			return -1;
		}

		if (town >= -3 && town <= -1) { // MAX_PORTAL_MEMO
			if (sd->status.memo_point[-town-1].map[0]) {
				if ((m = map_checkmapname(sd->status.memo_point[-town-1].map)) == -1) { // if map doesn't exist in all map-servers
					clif_displaymessage(fd, msg_txt(1)); // Map not found.
					return -1;
				}
				if (m >= 0) { // if on this map-server
					if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
						clif_displaymessage(fd, "You are not authorized to warp you to this memo map.");
						return -1;
					}
				}
				if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
					clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
					return -1;
				}
				if (pc_setpos(sd, sd->status.memo_point[-town-1].map, sd->status.memo_point[-town-1].x, sd->status.memo_point[-town-1].y, 3) == 0) {
					clif_displaymessage(fd, msg_txt(0)); // Warped.
				} else {
					clif_displaymessage(fd, msg_txt(1)); // Map not found.
					return -1;
				}
			} else {
				sprintf(atcmd_output, msg_txt(164), -town-1); // Your memo point #%d doesn't exist.
				clif_displaymessage(fd, atcmd_output);
				return -1;
			}
		} else if (town >= 0 && town < (int)(sizeof(data) / sizeof(data[0]))) {
			if ((m = map_checkmapname((char*)data[town].map)) == -1) { // if map doesn't exist in all map-servers
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
			if (m >= 0) { // if on this map-server
				if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
					clif_displaymessage(fd, "You are not authorized to warp you to this destination map.");
					return -1;
				}
			}
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp you from your actual map.");
				return -1;
			}
			if (pc_setpos(sd, (char*)data[town].map, data[town].x, data[town].y, 3) == 0) {
				clif_displaymessage(fd, msg_txt(0)); // Warped.
			} else {
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
		} else { // if you arrive here, you have an error in town variable when reading of names
			clif_displaymessage(fd, msg_txt(38)); // Invalid location number or name.
			return -1;
		}
	}

	return 0;
}

/*==========================================
 * function to check if a mob is not forbidden for spawn
 * return 0: not authorized
 * return not 0: authorized
 *------------------------------------------
 */
int check_mob_authorization(const int mob_id, const unsigned char gm_level) {
	FILE *fp;
	char line[512];
	int id, level;
	const char *filename = "db/mob_deny.txt";

	if ((fp = fopen(filename, "r")) == NULL) {
		//printf("Mob deny file not found: %s.\n", filename); // not display every time
		return 1; // by default: not deny -> authorized
	}

	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		level = 100; // default: if no mentioned GM level, no spawn -> 100
		if (sscanf(line, "%d,%d", &id, &level) < 1)
			continue;
		if (id == mob_id) {
			fclose(fp);
			if (gm_level < level)
				return 0; // not authorized
			else
				return 1; // authorized
		}
	}
	fclose(fp);

	return 1; // autorised
}

/*==========================================
 * Calculation of number of monsters that the player can look aroung of him
 *------------------------------------------
 */
int quantity_visible_monster(struct map_session_data* sd) {
	int m, bx, by;
	int mob_num;
	struct block_list *bl;

	mob_num = 0;
	m = sd->bl.m;
	for(by = (sd->bl.y - AREA_SIZE) / BLOCK_SIZE - 1; by <= (sd->bl.y + AREA_SIZE) / BLOCK_SIZE + 1; by++) {
		if (by < 0)
			continue;
		else if (by >= map[m].bys)
			break;
		for(bx = (sd->bl.x - AREA_SIZE) / BLOCK_SIZE - 1; bx <= (sd->bl.x + AREA_SIZE) / BLOCK_SIZE + 1; bx++) {
			if (bx < 0)
				continue;
			else if (bx >= map[m].bxs)
				break;
			for (bl = map[m].block_mob[bx + by * map[m].bxs]; bl; bl = bl->next)
				mob_num++;
		}
	}

	return mob_num;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(spawn) {
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int limited_number; // for correct displaying
	int x = 0, y = 0;
	int count;
	int c, i, j, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size, agro;

	memset(name, 0, sizeof(name));
	memset(monster, 0, sizeof(monster));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%s %d \"%[^\"]\" %d %d", monster, &number, name, &x, &y) < 3 &&
	     sscanf(message, "%s %d %s %d %d", monster, &number, name, &x, &y) < 1)) {
		clif_displaymessage(fd, msg_txt(143)); // Give a monster name/id please.
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_txt(83)); // Cannot spawn emperium.
		return -1;
	}

	if (!check_mob_authorization(mob_id, sd->GM_level)) {
		clif_displaymessage(fd, "You are not authorized to spawn this monster.");
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (name[0] == '\0')
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	limited_number = number;
	if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
		limited_number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.atc_map_mob_limit > 0) {
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next)
				mob_num++;
		if (mob_num >= battle_config.atc_map_mob_limit) {
			clif_displaymessage(fd, "There is too many monsters on the map. You can not spawn more monsters.");
			return -1;
		} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
			sprintf(atcmd_output, "Due to a density of monsters on the map, spawn has been limited.");
			clif_displaymessage(fd, atcmd_output);
			limited_number = battle_config.atc_map_mob_limit - mob_num;
		}
	}

	if ((count = quantity_visible_monster(sd)) >= 2000) {
		clif_displaymessage(fd, "There is too many monsters around of you. Move to a sparsely populated area to spawn.");
		return -1;
	} else if (count + limited_number > 2000) {
		sprintf(atcmd_output, "Due to a density of monsters around of you, spawn has been limited.");
		clif_displaymessage(fd, atcmd_output);
		limited_number = 2000 - count;
	}

	if (battle_config.etc_log)
		printf("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, limited_number, x, y);

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to spawn a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	// check for agressiv monster
	if (strstr(command, "agro") != NULL)
		agro = 1; // agressiv
	else if (strstr(command, "neutral") != NULL)
		agro = -1; // no agressiv
	else
		agro = 0; // normal

	count = 0;
	range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
	for (i = 0; i < limited_number; i++) {
		j = 0;
		id = 0;
		while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
			do {
				if (x <= 0)
					mx = sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
				else
					mx = x;
				if (y <= 0)
					my = sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
				else
					my = y;
			} while ((c = map_getcell(sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
			if (!c) {
				id = mob_once_spawn(sd, "this", mx, my, name, mob_id + size, 1, "");
				if (agro == 1 && (md = (struct mob_data *)map_id2bl(id)))
					md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
				else if (agro == -1 && (md = (struct mob_data *)map_id2bl(id)))
					md->mode = mob_db[mob_id].mode & ~0x4; // remove agressiv flag
			}
		}
		count += (id != 0) ? 1 : 0;
	}

	if (count != 0) {
		if (number == count)
			clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
		slave_num = 0;
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next) {
				mob_num++;
				if (((struct mob_data *)bl)->master_id)
					slave_num++;
			}
		if (slave_num == 0)
			sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(spawnmap) {
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int limited_number; // for correct displaying
	int count;
	int c, i, j, id;
	int x, y;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size, agro;

	memset(name, 0, sizeof(name));
	memset(monster, 0, sizeof(monster));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %s %d", name, monster, &number) < 2 &&
	     sscanf(message, "%s \"%[^\"]\" %d", monster, name, &number) < 2 &&
	     sscanf(message, "%s %d \"%[^\"]\"", monster, &number, name) < 3 &&
	     sscanf(message, "%s %d %s", monster, &number, name) < 1)) {
		clif_displaymessage(fd, msg_txt(143)); // Give a monster name/id please.
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_txt(83)); // Cannot spawn emperium.
		return -1;
	}

	if (!check_mob_authorization(mob_id, sd->GM_level)) {
		clif_displaymessage(fd, "You are not authorized to spawn this monster.");
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (name[0] == '\0')
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	limited_number = number;
	if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
		limited_number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.atc_map_mob_limit > 0) {
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next)
				mob_num++;
		if (mob_num >= battle_config.atc_map_mob_limit) {
			clif_displaymessage(fd, "There is too many monsters on the map. You can not spawn more monsters.");
			return -1;
		} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
			sprintf(atcmd_output, "Due to a density of monsters on the map, spawn has been limited.");
			clif_displaymessage(fd, atcmd_output);
			limited_number = battle_config.atc_map_mob_limit - mob_num;
		}
	}

	if (battle_config.etc_log)
		printf("%s monster='%s' name='%s' id=%d count=%d\n", command, monster, name, mob_id, limited_number);

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	// check for agressiv monster
	if (strstr(command, "agro") != NULL)
		agro = 1; // agressiv
	else if (strstr(command, "neutral") != NULL)
		agro = -1; // no agressiv
	else
		agro = 0; // normal

	count = 0;
	for (i = 0; i < limited_number; i++) {
		j = 0;
		id = 0;
		while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
			do {
				x = rand() % (map[sd->bl.m].xs - 2) + 1;
				y = rand() % (map[sd->bl.m].ys - 2) + 1;
			} while ((c = map_getcell(sd->bl.m, x, y, CELL_CHKNOPASS)) && j++ < 64);
			if (!c) {
				id = mob_once_spawn(sd, "this", x, y, name, mob_id + size, 1, "");
				if (agro == 1 && (md = (struct mob_data *)map_id2bl(id)))
					md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
				else if (agro == -1 && (md = (struct mob_data *)map_id2bl(id)))
					md->mode = mob_db[mob_id].mode & ~0x4; // remove agressiv flag
			}
		}
		count += (id != 0) ? 1 : 0;
	}

	if (count != 0) {
		if (number == count)
			clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
		slave_num = 0;
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next) {
				mob_num++;
				if (((struct mob_data *)bl)->master_id)
					slave_num++;
			}
		if (slave_num == 0)
			sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(spawnall) {
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int limited_number; // for correct displaying
	int count;
	int c, i, j, k, id;
	int x, y, range;
	struct mob_data *md;
	int size, agro;
	struct map_session_data *pl_sd;

	memset(name, 0, sizeof(name));
	memset(monster, 0, sizeof(monster));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %s %d", name, monster, &number) < 2 &&
	     sscanf(message, "%s \"%[^\"]\" %d", monster, name, &number) < 2 &&
	     sscanf(message, "%s %d \"%[^\"]\"", monster, &number, name) < 3 &&
	     sscanf(message, "%s %d %s", monster, &number, name) < 1)) {
		clif_displaymessage(fd, msg_txt(143)); // Give a monster name/id please.
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_txt(83)); // Cannot spawn emperium.
		return -1;
	}

	if (!check_mob_authorization(mob_id, sd->GM_level)) {
		clif_displaymessage(fd, "You are not authorized to spawn this monster.");
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (name[0] == '\0')
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	limited_number = number;
	if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
		limited_number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.etc_log)
		printf("%s monster='%s' name='%s' id=%d count=%d\n", command, monster, name, mob_id, limited_number);

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	// check for agressiv monster
	if (strstr(command, "agro") != NULL)
		agro = 1; // agressiv
	else if (strstr(command, "neutral") != NULL)
		agro = -1; // no agressiv
	else
		agro = 0; // normal

	count = 0;
	range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				for (k = 0; k < limited_number; k++) {
					j = 0;
					id = 0;
					while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
						do {
							x = pl_sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
							y = pl_sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
						} while ((c = map_getcell(pl_sd->bl.m, x, y, CELL_CHKNOPASS)) && j++ < 64);
						if (!c) {
							id = mob_once_spawn(pl_sd, "this", x, y, name, mob_id + size, 1, "");
							if (agro == 1 && (md = (struct mob_data *)map_id2bl(id)))
								md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
							else if (agro == -1 && (md = (struct mob_data *)map_id2bl(id)))
								md->mode = mob_db[mob_id].mode & ~0x4; // remove agressiv flag
						}
					}
				}
				count++;
			}
		}
	}

	if (count == 0) // impossible (GM is online)
		clif_displaymessage(fd, "No monster has been spawned!");
	else {
		if (count == 1)
			sprintf(atcmd_output, "%d monster%s summoned on 1 player!", limited_number, (limited_number > 1) ? "s" : "");
		else
			sprintf(atcmd_output, "%d monster%s summoned on each player (%d players)!", limited_number, (limited_number > 1) ? "s" : "", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(deadbranch) {
	int mob_id;
	int number;
	int limited_number; // for correct displaying
	int count;
	int c, i, j, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size;

	number = 1;
	sscanf(message, "%d", &number);

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to spawn a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	if (number <= 0)
		number = 1;

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	limited_number = number;
	if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
		limited_number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.atc_map_mob_limit > 0) {
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next)
				mob_num++;
		if (mob_num >= battle_config.atc_map_mob_limit) {
			clif_displaymessage(fd, "There is too many monsters on the map. You can not spawn more monsters.");
			return -1;
		} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
			sprintf(atcmd_output, "Due to a density of monsters on the map, spawn has been limited.");
			clif_displaymessage(fd, atcmd_output);
			limited_number = battle_config.atc_map_mob_limit - mob_num;
		}
	}

	if ((count = quantity_visible_monster(sd)) >= 2000) {
		clif_displaymessage(fd, "There is too many monsters around of you. Move to a sparsely populated area to spawn.");
		return -1;
	} else if (count + limited_number > 2000) {
		sprintf(atcmd_output, "Due to a density of monsters around of you, spawn has been limited.");
		clif_displaymessage(fd, atcmd_output);
		limited_number = 2000 - count;
	}

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	count = 0;
	range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
	for (i = 0; i < limited_number; i++) {
		// seach monster id
		j = 0;
		do {
			mob_id = rand() % 1000 + 1001;
		} while((mob_db[mob_id].max_hp <= 0 || mob_db[mob_id].summonper[0] <= (rand() % 1000000) || // summonper[0] = db/mob_branch.txt
		         sd->status.base_level < mob_db[mob_id].lv ||
		         mob_id == 1288) && // Cannot spawn emperium.
		        (j++) < 2000);
		if (j >= 2000)
			mob_id = 1002; // poring

		j = 0;
		id = 0;
		while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
			do {
				mx = sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
				my = sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
			} while ((c = map_getcell(sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
			if (!c) {
				id = mob_once_spawn(sd, "this", mx, my, "--ja--", mob_id + size, 1, "");
				if ((md = (struct mob_data *)map_id2bl(id)))
					md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
			}
		}
		count += (id != 0) ? 1 : 0;
	}

	if (count != 0) {
		if (number == count)
			clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
		slave_num = 0;
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next) {
				mob_num++;
				if (((struct mob_data *)bl)->master_id)
					slave_num++;
			}
		if (slave_num == 0)
			sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, "No monster summoned.");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardeadbranch) {
	int mob_id;
	int number;
	int limited_number; // for correct displaying
	int count;
	int c, i, j, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size;
	struct map_session_data *pl_sd;

	number = 1;
	if (!message || !*message || sscanf(message, "%d %[^\n]", &number, atcmd_name) < 2) {
		number = 1;
		if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
			send_usage(fd, "Please, enter a player name (usage: %s [# of monsters] <char name|account_id>).", original_command);
			return -1;
		}
	}

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to spawn a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (number <= 0)
				number = 1;

			// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
			limited_number = number;
			if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
				limited_number = battle_config.atc_spawn_quantity_limit;

			if (battle_config.atc_map_mob_limit > 0) {
				mob_num = 0;
				for (b = 0; b < map[pl_sd->bl.m].bxs * map[pl_sd->bl.m].bys; b++)
					for (bl = map[pl_sd->bl.m].block_mob[b]; bl; bl = bl->next)
						mob_num++;
				if (mob_num >= battle_config.atc_map_mob_limit) {
					clif_displaymessage(fd, "There is too many monsters on the map. You can not spawn more monsters.");
					return -1;
				} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
					sprintf(atcmd_output, "Due to a density of monsters on the map, spawn has been limited.");
					clif_displaymessage(fd, atcmd_output);
					limited_number = battle_config.atc_map_mob_limit - mob_num;
				}
			}

			if ((count = quantity_visible_monster(pl_sd)) >= 2000) {
				clif_displaymessage(fd, "There is too many monsters around of the player. Wait he moves to a sparsely populated area to spawn.");
				return -1;
			} else if (count + limited_number > 2000) {
				sprintf(atcmd_output, "Due to a density of monsters around of the player, spawn has been limited.");
				clif_displaymessage(fd, atcmd_output);
				limited_number = 2000 - count;
			}

			// check for monster size
			if (strstr(command, "small") != NULL)
				size = MAX_MOB_DB; // +2000 small
			else if (strstr(command, "big") != NULL)
				size = (MAX_MOB_DB * 2); // +4000 big
			else
				size = 0; // normal

			count = 0;
			range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
			for (i = 0; i < limited_number; i++) {
				// seach monster id
				j = 0;
				do {
					mob_id = rand() % 1000 + 1001;
				} while((mob_db[mob_id].max_hp <= 0 || mob_db[mob_id].summonper[0] <= (rand() % 1000000) || // summonper[0] = db/mob_branch.txt
				         pl_sd->status.base_level < mob_db[mob_id].lv ||
				         mob_id == 1288) && // Cannot spawn emperium.
				        (j++) < 2000);
				if (j >= 2000)
					mob_id = 1002; // poring

				j = 0;
				id = 0;
				while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
					do {
						mx = pl_sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
						my = pl_sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
					} while ((c = map_getcell(pl_sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
					if (!c) {
						id = mob_once_spawn(pl_sd, "this", mx, my, "--ja--", mob_id + size, 1, "");
						if ((md = (struct mob_data *)map_id2bl(id)))
							md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
					}
				}
				count += (id != 0) ? 1 : 0;
			}

			if (count != 0) {
				if (number == count)
					clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
				else {
					sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
					clif_displaymessage(fd, atcmd_output);
				}
				slave_num = 0;
				mob_num = 0;
				for (b = 0; b < map[pl_sd->bl.m].bxs * map[pl_sd->bl.m].bys; b++)
					for (bl = map[pl_sd->bl.m].block_mob[b]; bl; bl = bl->next) {
						mob_num++;
						if (((struct mob_data *)bl)->master_id)
							slave_num++;
					}
				if (slave_num == 0)
					sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
				else
					sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
				clif_displaymessage(fd, atcmd_output);
			} else {
				clif_displaymessage(fd, "No monster summoned.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(deadbranchmap) {
	int map_id, mob_id;
	int number;
	int limited_number; // for correct displaying
	int count, players;
	int c, i, j, p, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size;
	struct map_session_data *pl_sd;

	memset(atcmd_mapname, 0, sizeof(atcmd_mapname));

	number = 1;
	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		if (sscanf(message, "%d %s", &number, atcmd_mapname) < 2)
			sscanf(message, "%s", atcmd_mapname);
		if (atcmd_mapname[0] == '\0')
			map_id = sd->bl.m;
		else {
			if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
				strcat(atcmd_mapname, ".gat");
			if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
				map_id = sd->bl.m;
		}
	}

	if (number <= 0)
		number = 1;

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to spawn a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	count = 0;
	players = 0;
	// sort online characters.
	for(p = 0; p < fd_max; p++) {
		if (session[p] && (pl_sd = session[p]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level
				if (pl_sd->bl.m == map_id) {

					// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
					limited_number = number;
					if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
						limited_number = battle_config.atc_spawn_quantity_limit;

					if (battle_config.atc_map_mob_limit > 0) {
						mob_num = 0;
						for (b = 0; b < map[pl_sd->bl.m].bxs * map[pl_sd->bl.m].bys; b++)
							for (bl = map[pl_sd->bl.m].block_mob[b]; bl; bl = bl->next)
								mob_num++;
						if (mob_num >= battle_config.atc_map_mob_limit) {
							break;
						} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
							limited_number = battle_config.atc_map_mob_limit - mob_num;
						}
					}

					if ((j = quantity_visible_monster(pl_sd)) >= 2000) {
						continue;
					} else if (j + limited_number > 2000) {
						limited_number = 2000 - j;
					}

					range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
					for (i = 0; i < limited_number; i++) {
						// seach monster id
						j = 0;
						do {
							mob_id = rand() % 1000 + 1001;
						} while((mob_db[mob_id].max_hp <= 0 || mob_db[mob_id].summonper[0] <= (rand() % 1000000) || // summonper[0] = db/mob_branch.txt
						         pl_sd->status.base_level < mob_db[mob_id].lv ||
						         mob_id == 1288) && // Cannot spawn emperium.
						        (j++) < 2000);
						if (j >= 2000)
							mob_id = 1002; // poring

						j = 0;
						id = 0;
						while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
							do {
								mx = pl_sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
								my = pl_sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
							} while ((c = map_getcell(pl_sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
							if (!c) {
								id = mob_once_spawn(pl_sd, "this", mx, my, "--ja--", mob_id + size, 1, "");
								if ((md = (struct mob_data *)map_id2bl(id)))
									md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
							}
						}
						count += (id != 0) ? 1 : 0;
					}
					players++;
				}
			}
		}
	}

	if (players == 0) {
		sprintf(atcmd_output, msg_txt(54), map[map_id].name); // No player found in map '%s'.
		clif_displaymessage(fd, atcmd_output);
	} else {
		if (count != 0) {
			if (number == count / players)
				clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
			else {
				sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
				clif_displaymessage(fd, atcmd_output);
			}
			slave_num = 0;
			mob_num = 0;
			for (b = 0; b < map[map_id].bxs * map[map_id].bys; b++)
				for (bl = map[map_id].block_mob[b]; bl; bl = bl->next) {
					mob_num++;
					if (((struct mob_data *)bl)->master_id)
						slave_num++;
				}
			if (slave_num == 0)
				sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
			else
				sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, "No monster summoned.");
			return -1;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(deadbranchall) {
	int mob_id;
	int number;
	int limited_number; // for correct displaying
	int count, players;
	int c, i, j, p, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num;
	int size;
	struct map_session_data *pl_sd;

	number = 1;
	sscanf(message, "%d", &number);

	if (number <= 0)
		number = 1;

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to spawn a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	count = 0;
	players = 0;
	// sort online characters.
	for(p = 0; p < fd_max; p++) {
		if (session[p] && (pl_sd = session[p]->session_data) && pl_sd->state.auth) {
			if (!((pl_sd->GM_level >= battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_sd->GM_level > sd->GM_level))) { // only lower or same level

				// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
				limited_number = number;
				if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
					limited_number = battle_config.atc_spawn_quantity_limit;

				if (battle_config.atc_map_mob_limit > 0) {
					mob_num = 0;
					for (b = 0; b < map[pl_sd->bl.m].bxs * map[pl_sd->bl.m].bys; b++)
						for (bl = map[pl_sd->bl.m].block_mob[b]; bl; bl = bl->next)
							mob_num++;
					if (mob_num >= battle_config.atc_map_mob_limit) {
						continue;
					} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
						limited_number = battle_config.atc_map_mob_limit - mob_num;
					}
				}

				if ((j = quantity_visible_monster(pl_sd)) >= 2000) {
					continue;
				} else if (j + limited_number > 2000) {
					limited_number = 2000 - j;
				}

				range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
				for (i = 0; i < limited_number; i++) {
					// seach monster id
					j = 0;
					do {
						mob_id = rand() % 1000 + 1001;
					} while((mob_db[mob_id].max_hp <= 0 || mob_db[mob_id].summonper[0] <= (rand() % 1000000) || // summonper[0] = db/mob_branch.txt
					         pl_sd->status.base_level < mob_db[mob_id].lv ||
					         mob_id == 1288) && // Cannot spawn emperium.
					        (j++) < 2000);
					if (j >= 2000)
						mob_id = 1002; // poring

					j = 0;
					id = 0;
					while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
						do {
							mx = pl_sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
							my = pl_sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
						} while ((c = map_getcell(pl_sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
						if (!c) {
							id = mob_once_spawn(pl_sd, "this", mx, my, "--ja--", mob_id + size, 1, "");
							if ((md = (struct mob_data *)map_id2bl(id)))
								md->mode = mob_db[mob_id].mode | (0x1 + 0x4 + 0x80); // like dead branch
						}
					}
					count += (id != 0) ? 1 : 0;
				}
				players++;
			}
		}
	}

	if (players == 0) {
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	} else {
		if (count != 0) {
			if (number == count / players)
				clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
			else {
				sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
				clif_displaymessage(fd, atcmd_output);
			}
		} else {
			clif_displaymessage(fd, "No monster summoned.");
			return -1;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(summon) {
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int limited_number; // for correct displaying
	int x = 0, y = 0;
	int count;
	int c, i, j, id;
	int mx, my, range;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int size;

	memset(name, 0, sizeof(name));
	memset(monster, 0, sizeof(monster));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%s %d \"%[^\"]\" %d %d", monster, &number, name, &x, &y) < 3 &&
	     sscanf(message, "%s %d %s %d %d", monster, &number, name, &x, &y) < 1)) {
		clif_displaymessage(fd, msg_txt(143)); // Give a monster name/id please.
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_txt(83)); // Cannot spawn emperium.
		return -1;
	}

	if (!check_mob_authorization(mob_id, sd->GM_level)) {
		clif_displaymessage(fd, "You are not authorized to summon this monster.");
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (name[0] == '\0')
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	limited_number = number;
	if (battle_config.atc_spawn_quantity_limit > 0 && number > battle_config.atc_spawn_quantity_limit)
		limited_number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.atc_map_mob_limit > 0) {
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next)
				mob_num++;
		if (mob_num >= battle_config.atc_map_mob_limit) {
			clif_displaymessage(fd, "There is too many monsters on the map. You can not summon more monsters.");
			return -1;
		} else if (mob_num + limited_number > battle_config.atc_map_mob_limit) {
			sprintf(atcmd_output, "Due to a density of monsters on the map, summon has been limited.");
			clif_displaymessage(fd, atcmd_output);
			limited_number = battle_config.atc_map_mob_limit - mob_num;
		}
	}

	if ((count = quantity_visible_monster(sd)) >= 2000) {
		clif_displaymessage(fd, "There is too many monsters around of you. Move to a sparsely populated area to summon.");
		return -1;
	} else if (count + limited_number > 2000) {
		sprintf(atcmd_output, "Due to a density of monsters around of you, summon has been limited.");
		clif_displaymessage(fd, atcmd_output);
		limited_number = 2000 - count;
	}

	if (battle_config.etc_log)
		printf("%s summon='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, limited_number, x, y);

	/* check latest spawn time */
	if (battle_config.atc_local_spawn_interval) {
		if (last_spawn > time(NULL)) { /* # of seconds 1/1/1970 (timestamp): to limit number of spawn at 1 every 2 seconds (reduction of lag) */
			sprintf(atcmd_output, "Please wait %d second(s) before to summon a monster to avoid lag around of you.", (int)(last_spawn - time(NULL)));
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
		last_spawn = time(NULL) + battle_config.atc_local_spawn_interval;
	}

	// check for monster size
	if (strstr(command, "small") != NULL)
		size = MAX_MOB_DB; // +2000 small
	else if (strstr(command, "big") != NULL)
		size = (MAX_MOB_DB * 2); // +4000 big
	else
		size = 0; // normal

	count = 0;
	range = sqrt(limited_number) * 2 + 1; // calculation of an odd number
	for (i = 0; i < limited_number; i++) {
		j = 0;
		id = 0;
		while(j++ < 64 && id == 0) { // try 64 times to spawn the monster (needed for close area)
			do {
				if (x <= 0)
					mx = sd->bl.x + (rand() % (range + j * 2) - ((range + j * 2) / 2));
				else
					mx = x;
				if (y <= 0)
					my = sd->bl.y + (rand() % (range + j * 2) - ((range + j * 2) / 2));
				else
					my = y;
			} while ((c = map_getcell(sd->bl.m, mx, my, CELL_CHKNOPASS)) && j++ < 64);
			if (!c) {
				id = mob_once_spawn(sd, "this", mx, my, name, mob_id + size, 1, "");
				if ((md = (struct mob_data *)map_id2bl(id))) {
					md->master_id = sd->bl.id;
					md->state.special_mob_ai = 1; // 0: nothing, 1: cannibalize, 2-3: spheremine
					md->mode = mob_db[mob_id].mode | 0x04;
					md->deletetimer = add_timer(gettick_cache + 60000, mob_timer_delete, id, 0);
					clif_misceffect2(&md->bl, 344); /* display teleport of monster */
				}
			}
		}
		count += (id != 0) ? 1 : 0;
	}

	if (count != 0) {
		if (number == count)
			clif_displaymessage(fd, msg_txt(39)); // All monsters summoned!
		else {
			sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
		slave_num = 0;
		mob_num = 0;
		for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
			for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next) {
				mob_num++;
				if (((struct mob_data *)bl)->master_id)
					slave_num++;
			}
		if (slave_num == 0)
			sprintf(atcmd_output, "Total mobs in map: %d (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "Total mobs in map: %d (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(killmonster) {
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	map_foreachinarea(atkillmonster_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_MOB, 1); // 0: no drop, 1: drop

	clif_displaymessage(fd, msg_txt(165)); // All monsters killed!


	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(killmonster2) {
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	map_foreachinarea(atkillmonster_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_MOB, 0); // 0: no drop, 1: drop

	clif_displaymessage(fd, msg_txt(165)); // All monsters killed!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(killmonsterarea) {
	int area_size;

	area_size = AREA_SIZE;
	sscanf(message, "%d", &area_size);
	if (area_size < 1)
		area_size = 1;

	map_foreachinarea(atkillmonster_sub, sd->bl.m, sd->bl.x - area_size, sd->bl.y - area_size, sd->bl.x + area_size, sd->bl.y + area_size, BL_MOB, 1); // 0: no drop, 1: drop

	clif_displaymessage(fd, msg_txt(165)); // All monsters killed!


	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(killmonster2area) {
	int area_size;

	area_size = AREA_SIZE;
	sscanf(message, "%d", &area_size);
	if (area_size < 1)
		area_size = 1;

	map_foreachinarea(atkillmonster_sub, sd->bl.m, sd->bl.x - area_size, sd->bl.y - area_size, sd->bl.x + area_size, sd->bl.y + area_size, BL_MOB, 0); // 0: no drop, 1: drop

	clif_displaymessage(fd, msg_txt(165)); // All monsters killed!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(refine) {
	int i, position = 0, refine = 0, current_position, final_refine;
	int count;

	sscanf(message, "%d %d", &position, &refine);
	if (position < 0) {
		send_usage(fd, "Please, enter a position and a amount (usage: %s [equip position [+/- amount]]).", original_command);
		send_usage(fd, "(usage: %s 0 [+/- amount]): refine all items.", original_command);
		return -1;
	}

	if (refine < -10)
		refine = -10;
	else if (refine > 10)
		refine = 10;
	else if (refine == 0)
		refine = 1;

	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid &&	// �Y�����̑����𐸘B����
		    (sd->status.inventory[i].equip & position ||
			(sd->status.inventory[i].equip && !position))) {
			final_refine = sd->status.inventory[i].refine + refine;
			if (final_refine > 10)
				final_refine = 10;
			else if (final_refine < 0)
				final_refine = 0;
			if (sd->status.inventory[i].refine != final_refine) {
				sd->status.inventory[i].refine = final_refine;
				current_position = sd->status.inventory[i].equip;
				pc_unequipitem(sd, i, 3);
				clif_refine(fd, sd, 0, i, sd->status.inventory[i].refine);
				clif_delitem(sd, i, 1);
				clif_additem(sd, i, 1, 0); // 0: you got...
				pc_equipitem(sd, i, current_position);
				count++;
			}
		}
	}
	/* do effect if at least 1 item is modified */
	if (count > 0)
		clif_misceffect((struct block_list*)&sd->bl, 3);

	if (count == 0)
		clif_displaymessage(fd, msg_txt(166)); // No item has been refined!
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(167)); // 1 item has been refined!
	else {
		sprintf(atcmd_output, msg_txt(168), count); // %d items have been refined!
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(refineall) {
	char message2[103]; // '0 ' (2) + message (max 100) + NULL (1) = 103

	// preparation of message
	sprintf(message2, "0 %s", message);

	// call refine function
	return atcommand_refine(fd, sd, original_command, "@refine", message2); // use local buffer to avoid problem (we call another function that can use global variables)
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(produce) {
	char item_name[100];
	int item_id, attribute = 0, star = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d %d", item_name, &attribute, &star) < 1) {
		send_usage(fd, "Please, enter at least an item name/id (usage: %s <equip name or equip ID> <element> <# of very's>).", original_command);
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (itemdb_exists(item_id) &&
	    (item_id <  501 || item_id > 1099) && // weapons: 1100-1999, armors/equipement: 2000-2999
	    (item_id < 4001 || item_id > 4211) && // cards: 4000-4999, headgears: 5000-6000
	    (item_id < 7001 || item_id > 10019) && // etc: 7000-7999, egg: 9000-9999, pet accessories: 10000-10999, history books: 11000-11999, scrolls/quivers: 12000-12999
	    itemdb_isequip(item_id)) {
		if (check_item_authorization(item_id, sd->GM_level)) {
			if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
				attribute = ATTRIBUTE_NORMAL;
			if (star < MIN_STAR || star > MAX_STAR)
				star = 0;
			memset(&tmp_item, 0, sizeof tmp_item);
			tmp_item.nameid = item_id;
			tmp_item.amount = 1;
			tmp_item.identify = 1;
			tmp_item.card[0] = 0x00ff;
			tmp_item.card[1] = ((star * 5) << 8) + attribute;
			*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
			clif_produceeffect(sd, 0, item_id); // �����G�t�F�N�g�p�P�b�g
			clif_misceffect(&sd->bl, 3); // ���l�ɂ�������ʒm
			if ((flag = pc_additem(sd, &tmp_item, 1)))
				clif_additem(sd, 0, 0, flag);
			// display is not necessary
		} else {
			clif_displaymessage(fd, "You are not authorized to create this item.");
			return -1;
		}
	} else {
		if (battle_config.error_log)
			printf("%s NOT WEAPON [%d]\n", original_command, item_id);
		if (item_id != 0 && itemdb_exists(item_id)) {
			sprintf(atcmd_output, msg_txt(169), item_id, item_data->name); // This item (%d: '%s') is not an equipment.
			clif_displaymessage(fd, atcmd_output);
		} else
			clif_displaymessage(fd, msg_txt(170)); // This item is not an equipment.
		return -1;
	}

	return 0;
}

/*==========================================
 * Sub-function to display actual memo points
 *------------------------------------------
 */
void atcommand_memo_sub(struct map_session_data* sd) {
	int i;

	clif_displaymessage(sd->fd, "Your actual memo positions are (except respawn point):");
	for (i = 0; i < MAX_PORTAL_MEMO; i++) {
		if (sd->status.memo_point[i].map[0])
			sprintf(atcmd_output, "%d - %s (%d,%d)", i, sd->status.memo_point[i].map, sd->status.memo_point[i].x, sd->status.memo_point[i].y);
		else
			sprintf(atcmd_output, msg_txt(171), i); // %d - void
		clif_displaymessage(sd->fd, atcmd_output);
	}

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(memo) {
	int position = 0;

	if (!message || !*message || sscanf(message, "%d", &position) < 1)
		atcommand_memo_sub(sd);
	else {
		if (position >= 0 && position < MAX_PORTAL_MEMO) {
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nomemo && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to memo this map.");
				return -1;
			}
			if (sd->status.memo_point[position].map[0]) {
				sprintf(atcmd_output, msg_txt(172), position, sd->status.memo_point[position].map, sd->status.memo_point[position].x, sd->status.memo_point[position].y); // You replace previous memo position %d - %s (%d,%d).
				clif_displaymessage(fd, atcmd_output);
			}
			memset(sd->status.memo_point[position].map, 0, sizeof(sd->status.memo_point[position].map));
			strncpy(sd->status.memo_point[position].map, map[sd->bl.m].name, 16); // 17 - NULL
			sd->status.memo_point[position].x = sd->bl.x;
			sd->status.memo_point[position].y = sd->bl.y;
			clif_skill_memo(sd, 0); // 00: success to take memo., 01: insuffisant skill level., 02: You don't know warp skill.
			if (pc_checkskill(sd, AL_WARP) <= (position + 1))
				clif_displaymessage(fd, msg_txt(173)); // Note: you don't have the 'Warp' skill level to use it.
			atcommand_memo_sub(sd);
		} else {
			send_usage(fd, "Please, enter a valid position (usage: %s <memo_position:%d-%d>).", original_command, 0, MAX_PORTAL_MEMO - 1);
			atcommand_memo_sub(sd);
			return -1;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(gat) {
	int y, x;

	clif_displaymessage(fd, "Ground informations around the player:");
	for (y = sd->bl.y + 2; y >= sd->bl.y - 2; y--) {
		if (y >= 0 && y < map[sd->bl.m].ys) {
			if (sd->bl.x < 2)
				x = 2;
			else if (sd->bl.x >= map[sd->bl.m].xs - 2)
				x = map[sd->bl.m].xs - 3;
			else
				x = sd->bl.x;
			sprintf(atcmd_output, "%s (x= %d-%d, y= %d) %02X %02X %02X %02X %02X",
			         map[sd->bl.m].name,   x - 2, x + 2, y,
			         map_getcell(sd->bl.m, x - 2, y, CELL_GETTYPE),
			         map_getcell(sd->bl.m, x - 1, y, CELL_GETTYPE),
			         map_getcell(sd->bl.m, x    , y, CELL_GETTYPE),
			         map_getcell(sd->bl.m, x + 1, y, CELL_GETTYPE),
			         map_getcell(sd->bl.m, x + 2, y, CELL_GETTYPE));
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return 0;
}

/*==========================================
 * @send (used for testing packet sends from the client)
 *------------------------------------------
 */
ATCOMMAND_FUNC(send) {
	int a;

	if (!message || !*message || sscanf(message, "%d", &a) < 1)
		return -1;

	clif_misceffect(&sd->bl, a);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(packet) {
	int x, y;

	if (!message || !*message || sscanf(message, "%d %d", &x, &y) < 2) {
		send_usage(fd, "Please, enter a status type/flag (usage: %s <status type> <flag>).", original_command);
		return -1;
	}

	clif_status_change(&sd->bl, x, y);

	return 0;
}

/*==========================================
 * @stpoint (Rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(statuspoint) {
	int point, new_status_point;

	if (!message || !*message || sscanf(message, "%d", &point) < 1 || point == 0) {
		send_usage(fd, "Please, enter a number (usage: %s <number of points>).", original_command);
		return -1;
	}

	new_status_point = (int)sd->status.status_point + point;
	if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
		new_status_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
		new_status_point = 0;

	if (new_status_point != (int)sd->status.status_point) {
		// if player have max in all stats, don't give status_point
		if (sd->status.str  >= battle_config.max_parameter &&
		    sd->status.agi  >= battle_config.max_parameter &&
		    sd->status.vit  >= battle_config.max_parameter &&
		    sd->status.int_ >= battle_config.max_parameter &&
		    sd->status.dex  >= battle_config.max_parameter &&
		    sd->status.luk  >= battle_config.max_parameter &&
		    (short)new_status_point != 0) {
			sd->status.status_point = 0;
			clif_updatestatus(sd, SP_STATUSPOINT);
			clif_displaymessage(fd, "You have max in each stat -> status points set to 0.");
			return -1;
		} else {
			sd->status.status_point = (short)new_status_point;
			clif_updatestatus(sd, SP_STATUSPOINT);
			clif_displaymessage(fd, msg_txt(174)); // Number of status points changed!
		}
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @skpoint (Rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(skillpoint) {
	int point, new_skill_point;

	if (!message || !*message || sscanf(message, "%d", &point) < 1 || point == 0) {
		send_usage(fd, "Please, enter a number (usage: %s <number of points>).", original_command);
		return -1;
	}

	new_skill_point = (int)sd->status.skill_point + point;
	if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
		new_skill_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
		new_skill_point = 0;

	if (new_skill_point != (int)sd->status.skill_point) {
		sd->status.skill_point = (short)new_skill_point;
		clif_updatestatus(sd, SP_SKILLPOINT);
		clif_displaymessage(fd, msg_txt(175)); // Number of skill points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @zeny (Rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(zeny) {
	int zeny, new_zeny;

	if (!message || !*message || sscanf(message, "%d", &zeny) < 1 || zeny == 0) {
		send_usage(fd, "Please, enter an amount (usage: %s <amount>).", original_command);
		return -1;
	}

	new_zeny = sd->status.zeny + zeny;
	if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
		new_zeny = MAX_ZENY;
	else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
		new_zeny = 0;

	if (new_zeny != sd->status.zeny) {
		sd->status.zeny = new_zeny;
		clif_updatestatus(sd, SP_ZENY);
		clif_displaymessage(fd, msg_txt(176)); // Number of zenys changed!
	} else {
		if (zeny < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(param) {
	int i, idx, value = 0, new_value;
	const char* param[] = { "@str", "@agi", "@vit", "@int", "@dex", "@luk", NULL };
	short* status[] = {
		&sd->status.str,  &sd->status.agi, &sd->status.vit,
		&sd->status.int_, &sd->status.dex, &sd->status.luk
	};

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		send_usage(fd, "Please, enter a valid value (usage: %s <+/-adjustement>).", original_command);
		return -1;
	}

	idx = -1;
	for (i = 0; param[i] != NULL; i++) {
		if (strcasecmp(command + 1, param[i] + 1) == 0) { // to suppress symbol of GM command
			idx = i;
			break;
		}
	}
	if (idx < 0 || idx > MAX_STATUS_TYPE) { // normaly impossible...
		send_usage(fd, "Please, enter a valid value (usage: %s <+/-adjustement>).", original_command);
		return -1;
	}

	new_value = (int)*status[idx] + value;
	if (value > 0 && (value > battle_config.max_parameter || new_value > battle_config.max_parameter)) // fix positiv overflow
		new_value = battle_config.max_parameter;
	else if (value < 0 && (value < -battle_config.max_parameter || new_value < 1)) // fix negativ overflow
		new_value = 1;

	if (new_value != (int)*status[idx]) {
		*status[idx] = ((short)new_value);
		clif_updatestatus(sd, SP_STR + idx);
		clif_updatestatus(sd, SP_USTR + idx);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_txt(42)); // Stat changed.
		// if player have max in all stats, don't give status_point
		if (sd->status.str  >= battle_config.max_parameter &&
		    sd->status.agi  >= battle_config.max_parameter &&
		    sd->status.vit  >= battle_config.max_parameter &&
		    sd->status.int_ >= battle_config.max_parameter &&
		    sd->status.dex  >= battle_config.max_parameter &&
		    sd->status.luk  >= battle_config.max_parameter &&
		    sd->status.status_point != 0) {
			sd->status.status_point = 0;
			clif_updatestatus(sd, SP_STATUSPOINT);
			clif_displaymessage(fd, "You have max in each stat -> status points set to 0.");
		}
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * Stat all by fritz (rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(stat_all) {
	int idx, count, value = 0, new_value;
	short* status[] = {
		&sd->status.str,  &sd->status.agi, &sd->status.vit,
		&sd->status.int_, &sd->status.dex, &sd->status.luk
	};

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0)
		value = battle_config.max_parameter;

	count = 0;
	for (idx = 0; idx < (int)(sizeof(status) / sizeof(status[0])); idx++) {

		new_value = (int)*status[idx] + value;
		if (value > 0 && (value > battle_config.max_parameter || new_value > battle_config.max_parameter)) // fix positiv overflow
			new_value = battle_config.max_parameter;
		else if (value < 0 && (value < -battle_config.max_parameter || new_value < 1)) // fix negativ overflow
			new_value = 1;

		if (new_value != (int)*status[idx]) {
			*status[idx] = ((short)new_value);
			clif_updatestatus(sd, SP_STR + idx);
			clif_updatestatus(sd, SP_USTR + idx);
			status_calc_pc(sd, 0);
			count++;
		}
	}

	if (count > 0) { // if at least 1 stat modified
		clif_displaymessage(fd, msg_txt(84)); // All stats changed!
		// if player have max in all stats, don't give status_point
		if (sd->status.str  >= battle_config.max_parameter &&
		    sd->status.agi  >= battle_config.max_parameter &&
		    sd->status.vit  >= battle_config.max_parameter &&
		    sd->status.int_ >= battle_config.max_parameter &&
		    sd->status.dex  >= battle_config.max_parameter &&
		    sd->status.luk  >= battle_config.max_parameter &&
		    sd->status.status_point != 0) {
			sd->status.status_point = 0;
			clif_updatestatus(sd, SP_STATUSPOINT);
			clif_displaymessage(fd, "You have max in each stat -> status points set to 0.");
		}
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(177)); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_txt(178)); // Impossible to increase a stat.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(guildlevelup) {
	int level;
	short added_level;
	struct guild *guild_info;

	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		send_usage(fd, "Please, enter a valid level (usage: %s <# of levels>).", original_command);
		return -1;
	}

	if (sd->status.guild_id <= 0 || (guild_info = guild_search(sd->status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_txt(43)); // You're not in a guild.
		return -1;
	}
	if (strcmp(sd->status.name, guild_info->master) != 0) {
		clif_displaymessage(fd, msg_txt(44)); // You're not the master of your guild.
		return -1;
	}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, 2); // 2= sizeof(added_level)
		clif_displaymessage(fd, msg_txt(179)); // Guild level changed.
	} else {
		clif_displaymessage(fd, msg_txt(45)); // Guild level change failed.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charguildlevelup) {
	int level;
	short added_level;
	struct guild *guild_info;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &level, atcmd_name) < 2 || level == 0) {
		send_usage(fd, "Please, enter a valid level (usage: %s <# of levels> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->status.guild_id <= 0 || (guild_info = guild_search(pl_sd->status.guild_id)) == NULL) {
				clif_displaymessage(fd, "This player is not in a guild.");
				return -1;
			}
			if (strcmp(pl_sd->status.name, guild_info->master) != 0) {
				clif_displaymessage(fd, "This player is not the master of its guild.");
				return -1;
			}

			added_level = (short)level;
			if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
				added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
			else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
				added_level = 1 - guild_info->guild_lv;

			if (added_level != 0) {
				intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, 2); // 2= sizeof(added_level)
				clif_displaymessage(fd, msg_txt(179)); // Guild level changed.
			} else {
				clif_displaymessage(fd, msg_txt(45)); // Guild level change failed.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(makeegg) {
	struct item_data *item_data;
	int id, pet_id;

	if (!message || !*message) {
		send_usage(fd, "Please, enter a monters/egg name/id (usage: %s <pet_id>).", original_command);
		return -1;
	}

	if ((item_data = itemdb_searchname(message)) != NULL) // for egg name
		id = item_data->nameid;
	else if ((id = mobdb_searchname(message)) == 0) // for monster name
		id = atoi(message);

	pet_id = search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd->catch_target_class = pet_db[pet_id].class;
		intif_create_pet(sd->status.account_id, sd->status.char_id,
		                 pet_db[pet_id].class, mob_db[pet_db[pet_id].class].lv,
		                 pet_db[pet_id].EggID, 0, pet_db[pet_id].intimate,
		                 100, 0, 1, pet_db[pet_id].jname);
	} else {
		clif_displaymessage(fd, msg_txt(180)); // The monter/egg name/id doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(hatch) {
	int i;

	if (sd->status.pet_id <= 0) {
		for(i = 0; i < MAX_INVENTORY; i++)
			if (sd->status.inventory[i].nameid > 0 && sd->inventory_data[i] != NULL &&
			    sd->inventory_data[i]->type == 7 && sd->status.inventory[i].amount > 0)
				break;
		if (i == MAX_INVENTORY)
			clif_displaymessage(fd, "You have no egg.");
		else
			clif_sendegg(sd);
	} else {
		clif_displaymessage(fd, msg_txt(181)); // You already have a pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(petfriendly) {
	int friendly;
	int t;

	if (!message || !*message || sscanf(message, "%d", &friendly) < 1 || friendly < 0 || friendly > 1000) {
		send_usage(fd, "Please, enter a valid value (usage: %s <0-1000>).", original_command);
		return -1;
	}

	if (sd->status.pet_id > 0 && sd->pd) {
		if (friendly != sd->pet.intimate) {
			t = sd->pet.intimate;
			sd->pet.intimate = friendly;
			clif_send_petstatus(sd);
			if (battle_config.pet_status_support) {
				if ((sd->pet.intimate > 0 && t <= 0) ||
				    (sd->pet.intimate <= 0 && t > 0)) {
					if (sd->bl.prev != NULL)
						status_calc_pc(sd, 0);
					else
						status_calc_pc(sd, 2);
				}
			}
			clif_displaymessage(fd, msg_txt(182)); // Pet friendly value changed!
		} else {
			clif_displaymessage(fd, msg_txt(183)); // Pet friendly is already the good value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(pethungry) {
	int hungry;

	if (!message || !*message || sscanf(message, "%d", &hungry) < 1 || hungry < 0 || hungry > 100) {
		send_usage(fd, "Please, enter a valid number (usage: %s <0-100>).", original_command);
		return -1;
	}

	if (sd->status.pet_id > 0 && sd->pd) {
		if (hungry != sd->pet.hungry) {
			sd->pet.hungry = hungry;
			clif_send_petstatus(sd);
			clif_displaymessage(fd, msg_txt(185)); // Pet hungry value changed!
		} else {
			clif_displaymessage(fd, msg_txt(186)); // Pet hungry is already the good value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(petrename) {
	if (sd->status.pet_id > 0 && sd->pd) {
		if (sd->pet.rename_flag != 0) {
			sd->pet.rename_flag = 0;
			intif_save_petdata(sd->status.account_id, &sd->pet);
			clif_send_petstatus(sd);
			clif_displaymessage(fd, msg_txt(187)); // You can now rename your pet.
		} else {
			clif_displaymessage(fd, msg_txt(188)); // You can already rename your pet.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(184)); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charpetrename) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (pl_sd->status.pet_id > 0 && pl_sd->pd) {
			if (pl_sd->pet.rename_flag != 0) {
				pl_sd->pet.rename_flag = 0;
				intif_save_petdata(pl_sd->status.account_id, &pl_sd->pet);
				clif_send_petstatus(pl_sd);
				clif_displaymessage(fd, msg_txt(189)); // This player can now rename his/her pet.
			} else {
				clif_displaymessage(fd, msg_txt(190)); // This player can already rename his/her pet.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(191)); // Sorry, but this player has no pet.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(recall) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp somenone to your actual map.");
				return -1;
			}
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level) {
				clif_displaymessage(fd, "You are not authorized to warp this player from its actual map.");
				return -1;
			}
			pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			sprintf(atcmd_output, msg_txt(46), pl_sd->status.name); // '%s' recalled!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Job Change
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_job) {
	struct map_session_data* pl_sd;
	int job = 0, upper = -1;
	int flag, i;
	char jobname[100];

	const struct { char name[20]; int id; } jobs[] = {
		{ "novice",                0 },
		{ "swordman",              1 },
		{ "swordsman",             1 },
		{ "mage",                  2 },
		{ "archer",                3 },
		{ "acolyte",               4 },
		{ "acolite",               4 },
		{ "merchant",              5 },
		{ "thief",                 6 },
		{ "theif",                 6 },
		{ "knight",                7 },
		{ "priest",                8 },
		{ "priestess",             8 },
		{ "wizard",                9 },
		{ "blacksmith",           10 },
		{ "hunter",               11 },
		{ "assassin",             12 },
		{ "peco knight",          13 },
		{ "peco-knight",          13 },
		{ "peco_knight",          13 },
		{ "pecoknight",           13 },
		{ "peko knight",          13 },
		{ "peko-knight",          13 },
		{ "peko_knight",          13 },
		{ "pekoknight",           13 },
		{ "crusader",             14 },
		{ "monk",                 15 },
		{ "sage",                 16 },
		{ "rogue",                17 },
		{ "alchemist",            18 },
		{ "bard",                 19 },
		{ "dancer",               20 },
		{ "peco crusader",        21 },
		{ "peco-crusader",        21 },
		{ "peco_crusader",        21 },
		{ "pecocrusader",         21 },
		{ "peko crusader",        21 },
		{ "peko-crusader",        21 },
		{ "peko_crusader",        21 },
		{ "pekocrusader",         21 },
		{ "super novice",         23 },
		{ "super-novice",         23 },
		{ "super_novice",         23 },
		{ "supernovice",          23 },
		{ "novice high",        4001 },
		{ "novice_high",        4001 },
		{ "high novice",        4001 },
		{ "high-novice",        4001 },
		{ "high_novice",        4001 },
		{ "highnovice",         4001 },
		{ "swordman high",      4002 },
		{ "swordsman high",     4002 },
		{ "mage high",          4003 },
		{ "archer high",        4004 },
		{ "acolyte high",       4005 },
		{ "acolite high",       4005 },
		{ "merchant high",      4006 },
		{ "thief high",         4007 },
		{ "theif high",         4007 },
		{ "lord knight",        4008 },
		{ "high priest",        4009 },
		{ "high_priest",        4009 },
		{ "highpriest",         4009 },
		{ "high priestess",     4009 },
		{ "high_priestess",     4009 },
		{ "highpriestess",      4009 },
		{ "high wizard",        4010 },
		{ "high_wizard",        4010 },
		{ "highwizard",         4010 },
		{ "whitesmith",         4011 },
		{ "sniper",             4012 },
		{ "assassin cross",     4013 },
		{ "peco lord knight",   4014 },
		{ "peko lord knight",   4014 },
		{ "paladin",            4015 },
		{ "champion",           4016 },
		{ "professor",          4017 },
		{ "stalker",            4018 },
		{ "creator",            4019 },
		{ "clown",              4020 },
		{ "gypsy",              4021 },
		{ "peco paladin",       4022 },
		{ "peco-paladin",       4022 },
		{ "peco_paladin",       4022 },
		{ "pecopaladin",        4022 },
		{ "peko paladin",       4022 },
		{ "peko-paladin",       4022 },
		{ "peko_paladin",       4022 },
		{ "pekopaladin",        4022 },
		{ "baby novice",        4023 },
		{ "baby swordman",      4024 },
		{ "baby swordsman",     4024 },
		{ "baby mage",          4025 },
		{ "baby archer",        4026 },
		{ "baby acolyte",       4027 },
		{ "baby acolite",       4027 },
		{ "baby merchant",      4028 },
		{ "baby thief",         4029 },
		{ "baby theif",         4029 },
		{ "baby knight",        4030 },
		{ "baby priest",        4031 },
		{ "baby priestess",     4031 },
		{ "baby wizard",        4032 },
		{ "baby blacksmith",    4033 },
		{ "baby hunter",        4034 },
		{ "baby assassin",      4035 },
		{ "baby peco knight",   4036 },
		{ "baby peco-knight",   4036 },
		{ "baby peco_knight",   4036 },
		{ "baby pecoknight",    4036 },
		{ "baby peko knight",   4036 },
		{ "baby peko-knight",   4036 },
		{ "baby peko_knight",   4036 },
		{ "baby pekoknight",    4036 },
		{ "baby crusader",      4037 },
		{ "baby monk",          4038 },
		{ "baby sage",          4039 },
		{ "baby rogue",         4040 },
		{ "baby alchemist",     4041 },
		{ "baby bard",          4042 },
		{ "baby dancer",        4043 },
		{ "baby peco crusader", 4044 },
		{ "baby peco-crusader", 4044 },
		{ "baby peco_crusader", 4044 },
		{ "baby pecocrusader",  4044 },
		{ "baby peko crusader", 4044 },
		{ "baby peko-crusader", 4044 },
		{ "baby peko_crusader", 4044 },
		{ "baby pekocrusader",  4044 },
		{ "super baby",         4045 },
		{ "super-baby",         4045 },
		{ "super_baby",         4045 },
		{ "superbaby",          4045 },
	};

	if (!message || !*message || sscanf(message, "%d %d %[^\n]", &job, &upper, atcmd_name) < 3 || job < 0 || job >= MAX_PC_CLASS || (job > 23 && job < 4001)) { //upper�w�肵�Ă���
		upper = -1;
		if (!message || !*message || sscanf(message, "%d %[^\n]", &job, atcmd_name) < 2 || job < 0 || job >= MAX_PC_CLASS || (job > 23 && job < 4001)) { //upper�w�肵�ĂȂ���ɉ�������Ȃ�
			/* search class name */
			i = (int)(sizeof(jobs) / sizeof(jobs[0]));
			if (sscanf(message, "\"%[^\"]\" %d %[^\n]", jobname, &upper, atcmd_name) == 3 ||
			    sscanf(message, "%s %d %[^\n]", jobname, &upper, atcmd_name) == 3) {
				for (i = 0; i < (int)(sizeof(jobs) / sizeof(jobs[0])); i++) {
					if (strcasecmp(jobname, jobs[i].name) == 0) {
						job = jobs[i].id;
						break;
					}
				}
			}
			/* if class name not found (with 3 parameters), try with 2 parameters */
			if ((i == (int)(sizeof(jobs) / sizeof(jobs[0])))) {
				upper = -1;
				if (sscanf(message, "\"%[^\"]\" %[^\n]", jobname, atcmd_name) == 2 ||
				    sscanf(message, "%s %[^\n]", jobname, atcmd_name) == 2) {
					for (i = 0; i < (int)(sizeof(jobs) / sizeof(jobs[0])); i++) {
						if (strcasecmp(jobname, jobs[i].name) == 0) {
							job = jobs[i].id;
							break;
						}
					}
				}
				/* if class name not found */
				if ((i == (int)(sizeof(jobs) / sizeof(jobs[0])))) {
					send_usage(fd, "Please, enter a job and a player name (usage: %s <job ID|\"job name\"> [upper] <char name|account_id>).", original_command);
					send_usage(fd, "   0 Novice            7 Knight           14 Crusader       22 Formal");
					send_usage(fd, "   1 Swordman          8 Priest           15 Monk           23 Super Novice");
					send_usage(fd, "   2 Mage              9 Wizard           16 Sage");
					send_usage(fd, "   3 Archer           10 Blacksmith       17 Rogue");
					send_usage(fd, "   4 Acolyte          11 Hunter           18 Alchemist");
					send_usage(fd, "   5 Merchant         12 Assassin         19 Bard");
					send_usage(fd, "   6 Thief            13 Peco-Knight      20 Dancer         21 Peco-Crusader");
					send_usage(fd, "4001 Novice High    4008 Lord Knight      4015 Paladin      4022 Peco-Paladin");
					send_usage(fd, "4002 Swordman High  4009 High Priest      4016 Champion");
					send_usage(fd, "4003 Mage High      4010 High Wizard      4017 Professor");
					send_usage(fd, "4004 Archer High    4011 Whitesmith       4018 Stalker");
					send_usage(fd, "4005 Acolyte High   4012 Sniper           4019 Creator");
					send_usage(fd, "4006 Merchant High  4013 Assassin Cross   4020 Clown");
					send_usage(fd, "4007 Thief High     4014 Peco Knight      4021 Gypsy");
					send_usage(fd, "4023 Baby Novice    4030 Baby Knight      4037 Baby Crusader  4044 Baby Peco-Crusader");
					send_usage(fd, "4024 Baby Swordsman 4031 Baby Priest      4038 Baby Monk      4045 Super Baby");
					send_usage(fd, "4025 Baby Mage      4032 Baby Wizard      4039 Baby Sage");
					send_usage(fd, "4026 Baby Archer    4033 Baby Blacksmith  4040 Baby Rogue");
					send_usage(fd, "4027 Baby Acolyte   4034 Baby Hunter      4041 Baby Alchemist");
					send_usage(fd, "4028 Baby Merchant  4035 Baby Assassin    4042 Baby Bard");
					send_usage(fd, "4029 Baby Thief     4036 Baby Peco-Knight 4043 Baby Dancer");
					send_usage(fd, "[upper]: -1 (default) to automatically determine the 'level', 0 to force normal job, 1 to force high job.");
					return -1;
				}
			}
		}
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if (job == pl_sd->status.class) {
				clif_displaymessage(fd, "The character's job is already the wished job.");
				return -1;
			}

			// fix pecopeco display
			if ((job != 13 && job != 21 && job != 4014 && job != 4022 && job != 4036 && job != 4044)) {
				if (pc_isriding(sd)) {
					// normal classes
					if (pl_sd->status.class == 13)
						pl_sd->status.class = pl_sd->view_class = 7;
					else if (pl_sd->status.class == 21)
						pl_sd->status.class = pl_sd->view_class = 14;
					// high classes
					else if (pl_sd->status.class == 4014)
						pl_sd->status.class = pl_sd->view_class = 4008;
					else if (pl_sd->status.class == 4022)
						pl_sd->status.class = pl_sd->view_class = 4015;
					// baby classes
					else if (pl_sd->status.class == 4036)
						pl_sd->status.class = pl_sd->view_class = 4030;
					else if (pl_sd->status.class == 4044)
						pl_sd->status.class = pl_sd->view_class = 4037;
					pl_sd->status.option &= ~0x0020;
					clif_changeoption(&pl_sd->bl);
					status_calc_pc(pl_sd, 0);
				}
			} else {
				if (!pc_isriding(sd)) {
					// normal classes
					if (job == 13)
						job = 7;
					else if (job == 21)
						job = 14;
					// high classes
					else if (job == 4014)
						job = 4008;
					else if (job == 4022)
						job = 4015;
					// baby classes
					else if (job == 4036)
						job = 4030;
					else if (job == 4044)
						job = 4037;
				}
			}

			flag = 0;
			for (i = 0; i < MAX_INVENTORY; i++)
				if (pl_sd->status.inventory[i].nameid && (pl_sd->status.inventory[i].equip & 34) != 0) { // righ hand (2) + left hand (32)
					pc_unequipitem(pl_sd, i, 3); /* unequip weapon to avoid sprite error */
					flag = 1;
				}
			if (flag)
				clif_displaymessage(sd->fd, "Weapon unequiped to avoid sprite error.");

			if (pc_jobchange(pl_sd, job, upper) == 0) {
				if (pl_sd != sd)
					clif_displaymessage(pl_sd->fd, "Your job has been changed by a GM.");
				clif_displaymessage(fd, msg_txt(48)); // Character's job changed.
			} else {
				clif_displaymessage(fd, msg_txt(192)); // Impossible to change the character's job.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charchangelevel - character set the level when the player changed of job (job 1 -> job 2)
 *------------------------------------------
 */
ATCOMMAND_FUNC(change_level) {
	struct map_session_data *pl_sd;
	int level;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &level, atcmd_name) < 2 || level < 40 || level > 50) {
		send_usage(fd, "Please, enter a right level and a player name (usage: %s <lvl:40-50> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if (pl_sd->change_level != level) {
				pl_sd->change_level = level;
				pc_setglobalreg(pl_sd, "jobchange_level", pl_sd->change_level);
				// save player immediatly (synchronize with global_reg)
				chrif_save(pl_sd); // do pc_makesavestatus and save storage + account_reg/account_reg2 too
				// recalculate skill tree
				status_calc_pc(pl_sd, 0);
				clif_displaymessage(fd, "Player's end level of job 1 changed!");
			} else {
				clif_displaymessage(fd, "Player already had this level when he became job 2.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(revive) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		/* if not dead, call @charheal command */
		if (!pc_isdead(pl_sd)) {
			char message2[105]; // '0 0 ' (4) + message (max 100) + NULL (1) = 105
			// preparation of message
			sprintf(message2, "0 0 %s", message);
			// call refine function
			return atcommand_charheal(fd, sd, original_command, "@charheal", message2); // use local buffer to avoid problem (we call another function that can use global variables)
		}
		pl_sd->status.hp = pl_sd->status.max_hp;
		pl_sd->status.sp = pl_sd->status.max_sp;
		clif_skill_nodamage(&pl_sd->bl, &pl_sd->bl, ALL_RESURRECTION, 4, 1);
		pc_setstand(pl_sd);
		if (battle_config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
		clif_updatestatus(pl_sd, SP_HP);
		clif_updatestatus(pl_sd, SP_SP);
		clif_resurrection(&pl_sd->bl, 1);
		clif_displaymessage(pl_sd->fd, msg_txt(16)); // You've been revived! It's a miracle!
		clif_displaymessage(fd, msg_txt(51)); // Character revived.
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charheal) {
	int hp = 0, sp = 0;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %[^\n]", &hp, &sp, atcmd_name) < 3) {
		sp = 0;
		if (!message || !*message || sscanf(message, "%d %[^\n]", &hp, atcmd_name) < 2) {
			hp = 0;
			if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
				send_usage(fd, "Please, enter a player name (usage: %s [<HP> [<SP>]] <char name|account_id>).", original_command);
				return -1;
			}
		}
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		/* if dead */
		if (pc_isdead(pl_sd)) {
			clif_displaymessage(fd, "This player is dead. You must resurrect him/her before to heal him/her.");
			return -1;
		}

		if (hp == 0 && sp == 0) {
			hp = pl_sd->status.max_hp - pl_sd->status.hp;
			sp = pl_sd->status.max_sp - pl_sd->status.sp;
		} else {
			if (hp > 0 && (hp > pl_sd->status.max_hp || hp > (pl_sd->status.max_hp - pl_sd->status.hp))) // fix positiv overflow
				hp = sd->status.max_hp - sd->status.hp;
			else if (hp < 0 && (hp < -pl_sd->status.max_hp || hp < (1 - pl_sd->status.hp))) // fix negativ overflow
				hp = 1 - pl_sd->status.hp;
			if (sp > 0 && (sp > pl_sd->status.max_sp || sp > (pl_sd->status.max_sp - pl_sd->status.sp))) // fix positiv overflow
				sp = pl_sd->status.max_sp - pl_sd->status.sp;
			else if (sp < 0 && (sp < -pl_sd->status.max_sp || sp < (1 - pl_sd->status.sp))) // fix negativ overflow
				sp = 1 - pl_sd->status.sp;
		}

		if (hp > 0) // display like heal
			clif_heal(pl_sd->fd, SP_HP, hp);
		else if (hp < 0) // display like damage
			clif_damage(&pl_sd->bl, &pl_sd->bl, gettick_cache, 0, 0, -hp, 0 , 4, 0);
		if (sp > 0) // no display when we lost SP
			clif_heal(pl_sd->fd, SP_SP, sp);

		if (hp != 0 || sp != 0) {
			pc_heal(pl_sd, hp, sp);
			if (hp >= 0 && sp >= 0)
				clif_displaymessage(fd, "HP, SP of the player recovered.");
			else
				clif_displaymessage(fd, "HP or/and SP of the player modified.");
		} else {
			clif_displaymessage(fd, "HP and SP of the player are already with the good value.");
			return -1;
		}

	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_stats) {
	char job_jobname[100];
	struct map_session_data *pl_sd;
	struct guild *g;
	struct party *p;
	int i;

	memset(job_jobname, 0, sizeof(job_jobname));

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		struct {
			const char* format;
			int value;
		} output_table[] = {
			{ "Str: %d",  pl_sd->status.str },
			{ "Agi: %d",  pl_sd->status.agi },
			{ "Vit: %d",  pl_sd->status.vit },
			{ "Int: %d",  pl_sd->status.int_ },
			{ "Dex: %d",  pl_sd->status.dex },
			{ "Luk: %d",  pl_sd->status.luk },
			{ "Zeny: %d", pl_sd->status.zeny },
			{ NULL, 0 }
		};

		if (pl_sd->GM_level > 0)
			sprintf(atcmd_output, "'%s' (GM:%d) (account: %d) stats:",   pl_sd->status.name, pl_sd->GM_level, (int)pl_sd->status.account_id);
		else
			sprintf(atcmd_output, msg_txt(53),             pl_sd->status.name, pl_sd->status.account_id); // '%s' (account: %d) stats:
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "Job: %s (level %d/%d)",     job_name(pl_sd->status.class), pl_sd->status.base_level, pl_sd->status.job_level);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "Location: %s %d %d",        pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
		clif_displaymessage(fd, atcmd_output);
		p = party_search(pl_sd->status.party_id);
		g = guild_search(pl_sd->status.guild_id);
		sprintf(atcmd_output, "Party: '%s' | Guild: '%s'", (p == NULL) ? "None" : p->name, (g == NULL) ? "None" : g->name);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "Hp: %d/%d",                 pl_sd->status.hp, pl_sd->status.max_hp);
		clif_displaymessage(fd, atcmd_output);
		sprintf(atcmd_output, "Sp: %d/%d",                 pl_sd->status.sp, pl_sd->status.max_sp);
		clif_displaymessage(fd, atcmd_output);
		for (i = 0; output_table[i].format != NULL; i++) {
			sprintf(atcmd_output, output_table[i].format,  output_table[i].value);
			clif_displaymessage(fd, atcmd_output);
		}
		sprintf(atcmd_output, "Speed: %d",                 pl_sd->speed);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Stats All by fritz
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_stats_all) {
	int i, count;
	struct map_session_data *pl_sd;

	count = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d) | HP: %d/%d | SP: %d/%d", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class), pl_sd->status.job_level, pl_sd->status.hp, pl_sd->status.max_hp, pl_sd->status.sp, pl_sd->status.max_sp);
			clif_displaymessage(fd, atcmd_output);
			if (pl_sd->GM_level > 0)
				sprintf(atcmd_output, "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %d | GM Lvl: %d", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, pl_sd->status.zeny, pl_sd->GM_level);
			else
				sprintf(atcmd_output, "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %d", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, pl_sd->status.zeny);
			clif_displaymessage(fd, atcmd_output);
			clif_displaymessage(fd, "--------");
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_option) {
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %d %[^\n]", &opt1, &opt2, &opt3, atcmd_name) < 4 || opt1 < 0 || opt2 < 0 || opt3 < 0) {
		send_usage(fd, "Please, enter valid options and a player name (usage: %s <opt1:0+> <opt2:0+> <opt3:0+> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pl_sd->opt1 = opt1;
			pl_sd->opt2 = opt2;
			if (!(pl_sd->status.option & CART_MASK) && (opt3 & CART_MASK)) {
				clif_cart_itemlist(pl_sd);
				clif_cart_equiplist(pl_sd);
				clif_updatestatus(pl_sd, SP_CARTINFO);
			}
			pl_sd->status.option = opt3;
			// fix pecopeco display
			if (pl_sd->status.class ==   13 || pl_sd->status.class ==   21 ||
			    pl_sd->status.class == 4014 || pl_sd->status.class == 4022 ||
			    pl_sd->status.class == 4036 || pl_sd->status.class == 4044) {
				if (!pc_isriding(pl_sd)) { // pl_sd have the new value...
					// normal classes
					if (pl_sd->status.class == 13)
						pl_sd->status.class = pl_sd->view_class = 7;
					else if (pl_sd->status.class == 21)
						pl_sd->status.class = pl_sd->view_class = 14;
					// high classes
					else if (pl_sd->status.class == 4014)
						pl_sd->status.class = pl_sd->view_class = 4008;
					else if (pl_sd->status.class == 4022)
						pl_sd->status.class = pl_sd->view_class = 4015;
					// baby classes
					else if (pl_sd->status.class == 4036)
						pl_sd->status.class = pl_sd->view_class = 4030;
					else if (pl_sd->status.class == 4044)
						pl_sd->status.class = pl_sd->view_class = 4037;
				}
			} else {
				if (pc_isriding(pl_sd)) { // pl_sd have the new value...
					if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
						pl_sd->status.option &= ~0x0020;
					} else {
						// normal classes
						if (pl_sd->status.class == 7)
							pl_sd->status.class = pl_sd->view_class = 13;
						else if (pl_sd->status.class == 14)
							pl_sd->status.class = pl_sd->view_class = 21;
						// high classes
						else if (pl_sd->status.class == 4008)
							pl_sd->status.class = pl_sd->view_class = 4014;
						else if (pl_sd->status.class == 4015)
							pl_sd->status.class = pl_sd->view_class = 4022;
						// baby classes
						else if (pl_sd->status.class == 4030)
							pl_sd->status.class = pl_sd->view_class = 4036;
						else if (pl_sd->status.class == 4037)
							pl_sd->status.class = pl_sd->view_class = 4044;
						else
							pl_sd->status.option &= ~0x0020;
					}
				}
			}
			clif_changeoption(&pl_sd->bl);
			status_calc_pc(pl_sd, 0);
			clif_displaymessage(fd, msg_txt(58)); // Character's options changed.
			if (pc_isfalcon(pl_sd) && pc_iscarton(pl_sd) && (pl_sd->status.option & 0x0008) != 0x0008)
				clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_optionadd) {
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %d %[^\n]", &opt1, &opt2, &opt3, atcmd_name) < 4 || opt1 < 0 || opt2 < 0 || opt3 < 0 || (opt1 == 0 && opt2 == 0 && opt3 == 0)) {
		send_usage(fd, "Please, enter valid options and a player name (usage: %s <opt1:0+> <opt2:0+> <opt3:0+> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pl_sd->opt1 |= opt1;
			pl_sd->opt2 |= opt2;
			if (!(pl_sd->status.option & CART_MASK) && (opt3 & CART_MASK)) {
				clif_cart_itemlist(pl_sd);
				clif_cart_equiplist(pl_sd);
				clif_updatestatus(pl_sd, SP_CARTINFO);
			}
			pl_sd->status.option |= opt3;
			// fix pecopeco display
			if (pl_sd->status.class ==   13 || pl_sd->status.class ==   21 ||
			    pl_sd->status.class == 4014 || pl_sd->status.class == 4022 ||
			    pl_sd->status.class == 4036 || pl_sd->status.class == 4044) {
				if (!pc_isriding(pl_sd)) { // pl_sd have the new value...
					// normal classes
					if (pl_sd->status.class == 13)
						pl_sd->status.class = pl_sd->view_class = 7;
					else if (pl_sd->status.class == 21)
						pl_sd->status.class = pl_sd->view_class = 14;
					// high classes
					else if (pl_sd->status.class == 4014)
						pl_sd->status.class = pl_sd->view_class = 4008;
					else if (pl_sd->status.class == 4022)
						pl_sd->status.class = pl_sd->view_class = 4015;
					// baby classes
					else if (pl_sd->status.class == 4036)
						pl_sd->status.class = pl_sd->view_class = 4030;
					else if (pl_sd->status.class == 4044)
						pl_sd->status.class = pl_sd->view_class = 4037;
				}
			} else {
				if (pc_isriding(pl_sd)) { // pl_sd have the new value...
					if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
						pl_sd->status.option &= ~0x0020;
					} else {
						// normal classes
						if (pl_sd->status.class == 7)
							pl_sd->status.class = pl_sd->view_class = 13;
						else if (pl_sd->status.class == 14)
							pl_sd->status.class = pl_sd->view_class = 21;
						// high classes
						else if (pl_sd->status.class == 4008)
							pl_sd->status.class = pl_sd->view_class = 4014;
						else if (pl_sd->status.class == 4015)
							pl_sd->status.class = pl_sd->view_class = 4022;
						// baby classes
						else if (pl_sd->status.class == 4030)
							pl_sd->status.class = pl_sd->view_class = 4036;
						else if (pl_sd->status.class == 4037)
							pl_sd->status.class = pl_sd->view_class = 4044;
						else
							pl_sd->status.option &= ~0x0020;
					}
				}
			}
			clif_changeoption(&pl_sd->bl);
			status_calc_pc(pl_sd, 0);
			clif_displaymessage(fd, msg_txt(58)); // Character's options changed.
			if (pc_isfalcon(pl_sd) && pc_iscarton(pl_sd) && (pl_sd->status.option & 0x0008) != 0x0008)
				clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_optionremove) {
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %d %[^\n]", &opt1, &opt2, &opt3, atcmd_name) < 4 || opt1 < 0 || opt2 < 0 || opt3 < 0 || (opt1 == 0 && opt2 == 0 && opt3 == 0)) {
		send_usage(fd, "Please, enter valid options and a player name (usage: %s <opt1:0+> <opt2:0+> <opt3:0+> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pl_sd->opt1 &= ~opt1;
			pl_sd->opt2 &= ~opt2;
			pl_sd->status.option &= ~opt3;
			// fix pecopeco display
			if (pl_sd->status.class ==   13 || pl_sd->status.class ==   21 ||
			    pl_sd->status.class == 4014 || pl_sd->status.class == 4022 ||
			    pl_sd->status.class == 4036 || pl_sd->status.class == 4044) {
				if (!pc_isriding(pl_sd)) { // pl_sd have the new value...
					// normal classes
					if (pl_sd->status.class == 13)
						pl_sd->status.class = pl_sd->view_class = 7;
					else if (pl_sd->status.class == 21)
						pl_sd->status.class = pl_sd->view_class = 14;
					// high classes
					else if (pl_sd->status.class == 4014)
						pl_sd->status.class = pl_sd->view_class = 4008;
					else if (pl_sd->status.class == 4022)
						pl_sd->status.class = pl_sd->view_class = 4015;
					// baby classes
					else if (pl_sd->status.class == 4036)
						pl_sd->status.class = pl_sd->view_class = 4030;
					else if (pl_sd->status.class == 4044)
						pl_sd->status.class = pl_sd->view_class = 4037;
				}
			} else {
				if (pc_isriding(pl_sd)) { // pl_sd have the new value...
					if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
						pl_sd->status.option &= ~0x0020;
					} else {
						// normal classes
						if (pl_sd->status.class == 7)
							pl_sd->status.class = pl_sd->view_class = 13;
						else if (pl_sd->status.class == 14)
							pl_sd->status.class = pl_sd->view_class = 21;
						// high classes
						else if (pl_sd->status.class == 4008)
							pl_sd->status.class = pl_sd->view_class = 4014;
						else if (pl_sd->status.class == 4015)
							pl_sd->status.class = pl_sd->view_class = 4022;
						// baby classes
						else if (pl_sd->status.class == 4030)
							pl_sd->status.class = pl_sd->view_class = 4036;
						else if (pl_sd->status.class == 4037)
							pl_sd->status.class = pl_sd->view_class = 4044;
						else
							pl_sd->status.option &= ~0x0020;
					}
				}
			}
			clif_changeoption(&pl_sd->bl);
			status_calc_pc(pl_sd, 0);
			clif_displaymessage(fd, msg_txt(58)); // Character's options changed.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * changesex command (usage: changesex)
 *------------------------------------------
 */
ATCOMMAND_FUNC(change_sex) {
	chrif_char_ask_name(sd->status.account_id, sd->status.name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
	clif_displaymessage(fd, "Your character name has been sent to char-server to ask it.");

	return 0;
}

/*==========================================
 * charchangesex command (usage: charchangesex <player_name>)
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_change_sex) {
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, pl_sd->status.name, 24);
	}

	// check player name
	if (strlen(atcmd_name) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name has at least 4 characters.
		return -1;
	} else if (strlen(atcmd_name) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name has 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charblock command (usage: charblock <player_name>)
 * This command do a definitiv ban on a player
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_block) {
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, pl_sd->status.name, 24);
	}

	// check player name
	if (strlen(atcmd_name) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name has at least 4 characters.
		return -1;
	} else if (strlen(atcmd_name) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name has 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charban command (usage: charban <time> <player_name>)
 * This command do a limited ban on a player
 * Time is done as follows:
 *   Adjustment value (-1, 1, +1, etc...)
 *   Modified element:
 *     a or y: year
 *     m:  month
 *     j or d: day
 *     h:  hour
 *     mn: minute
 *     s:  second
 * <example> @ban +1m-2mn1s-6y test_player
 *           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_ban) {
	struct map_session_data* pl_sd;
	char modif[100];
	char * modif_p;
	int year, month, day, hour, minute, second, value;

	memset(modif, 0, sizeof(modif));

	if (!message || !*message || sscanf(message, "%s %[^\n]", modif, atcmd_name) < 2) {
		send_usage(fd, "Please, enter ban time and a player name (usage: %s <time> <char name|account_id>).", original_command);
		send_usage(fd, "time usage: adjustement (+/- value) and element (y/a, m, d/j, h, mn, s)");
		send_usage(fd, "Example: %s +1m-2mn1s-6y testplayer", original_command);
		return -1;
	}

	modif_p = modif;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second += value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute += value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour += value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day += value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month += value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year += value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif_displaymessage(fd, msg_txt(85)); // Invalid time for ban command.
		return -1;
	}

	if ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, pl_sd->status.name, 24);
	}

	// check player name
	if (strlen(atcmd_name) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name has at least 4 characters.
		return -1;
	} else if (strlen(atcmd_name) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name has 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_name, 2, year, month, day, hour, minute, second); // type: 2 - ban
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charunblock command (usage: charunblock <player_name>)
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_unblock) {
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, pl_sd->status.name, 24);
	}

	// check player name
	if (strlen(atcmd_name) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name has at least 4 characters.
		return -1;
	} else if (strlen(atcmd_name) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name has 23 characters maximum.
		return -1;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd->status.account_id, atcmd_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charunban command (usage: charunban <player_name>)
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_unban) {
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth) {
		memset(atcmd_name, 0, sizeof(atcmd_name));
		strncpy(atcmd_name, pl_sd->status.name, 24);
	}

	// check player name
	if (strlen(atcmd_name) < 4) {
		clif_displaymessage(fd, msg_txt(86)); // Sorry, but a player name has at least 4 characters.
		return -1;
	} else if (strlen(atcmd_name) > 23) {
		clif_displaymessage(fd, msg_txt(87)); // Sorry, but a player name has 23 characters maximum.
		return -1;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd->status.account_id, atcmd_name, 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban
		clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_save) {
	struct map_session_data* pl_sd;
	int x = 0, y = 0;
	int m;

	if (!message || !*message || sscanf(message, "%s %d %d %[^\n]", atcmd_mapname, &x, &y, atcmd_name) < 4 || x < 0 || y < 0) {
		send_usage(fd, "Please, enter a valid save point and a player name (usage: %s <map> <x> <y> <char name|account_id>).", original_command);
		return -1;
	}

	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if ((m = map_checkmapname(atcmd_mapname)) == -1) { // if map doesn't exist in all map-servers
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
			if (x < 0 || y < 0) {
				clif_displaymessage(fd, msg_txt(2)); // Coordinates out of range.
				return -1;
			}
			if (m >= 0) {
				if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
					clif_displaymessage(fd, "You are not authorized to set this map as a save map.");
					return -1;
				}
				if (x >= map[m].xs || y >= map[m].ys) {
					clif_displaymessage(fd, msg_txt(2)); // Coordinates out of range.
					return -1;
				}
				if (map_getcell(m, x, y, CELL_CHKNOPASS)) {
					clif_displaymessage(fd, "Invalid coordinates for a respawn point.");
					return -1;
				}
			}
			pc_setsavepoint(pl_sd, atcmd_mapname, x, y);
			clif_displaymessage(fd, msg_txt(57)); // Character's respawn point changed.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @night
 *------------------------------------------
 */
ATCOMMAND_FUNC(night) {
	struct map_session_data *pl_sd;
	int i, msglen;

	if (!night_flag) { // night_flag: 0=day, 1=night [Yor]
		night_flag = 1; // 0=day, 1=night [Yor]
		msglen = strlen(msg_txt(59)); // Night has fallen.
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && !map[pl_sd->bl.m].flag.indoors) {
				if (!pl_sd->state.night) {
					clif_status_change(&pl_sd->bl, 149, 1); // Turning on the effect
					pl_sd->state.night = 1;
				}
				clif_wis_message(pl_sd->fd, wisp_server_name, msg_txt(59), msglen + 1); // Night has fallen.
			}
		}
	} else {
		clif_displaymessage(fd, msg_txt(89)); // Sorry, it's already the night. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 * @day
 *------------------------------------------
 */
ATCOMMAND_FUNC(day) {
	struct map_session_data *pl_sd;
	int i, msglen;

	if (night_flag) { // night_flag: 0=day, 1=night [Yor]
		night_flag = 0; // 0=day, 1=night [Yor]
		msglen = strlen(msg_txt(60)); // Day has arrived.
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) { // even if there's 'indoors' flag we have to turn the effect off
				if (pl_sd->state.night) {
					clif_status_change(&pl_sd->bl, 149, 0); // Turning off the effect
					pl_sd->state.night = 0;
					clif_wis_message(pl_sd->fd, wisp_server_name, msg_txt(60), msglen + 1); // Day has arrived.
				}
			}
		}
	} else {
		clif_displaymessage(fd, msg_txt(90)); // Sorry, it's already the day. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(doommap) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	clif_specialeffect(&sd->bl, 450, 2); // dark cross display around each player // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && i != fd && map_id == pl_sd->bl.m &&
		    sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
			count++;
		}
	}
	clif_displaymessage(fd, msg_txt(62)); // Judgement was made.
	if (count == 0)
		sprintf(atcmd_output, "No player killed on the map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "1 player killed on the map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "%d players killed on the map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(doom) {
	struct map_session_data *pl_sd;
	int i, count;

	clif_specialeffect(&sd->bl, 450, 3); // dark cross display around each player // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	count = 0;
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && i != fd &&
		    sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(pl_sd->fd, msg_txt(61)); // The holy messenger has given judgement.
			count++;
		}
	}
	clif_displaymessage(fd, msg_txt(62)); // Judgement was made.
	if (count == 0)
		clif_displaymessage(fd, "No player killed.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player killed.");
	else {
		sprintf(atcmd_output, "%d players killed.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static void atcommand_raise_sub(struct map_session_data* sd) {
	clif_skill_nodamage(&sd->bl, &sd->bl, ALL_RESURRECTION, 4, 1);
	sd->status.hp = sd->status.max_hp;
	sd->status.sp = sd->status.max_sp;
	pc_setstand(sd);
	clif_updatestatus(sd, SP_HP);
	clif_updatestatus(sd, SP_SP);
	clif_resurrection(&sd->bl, 1);
	if (battle_config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
	clif_displaymessage(sd->fd, msg_txt(63)); // Mercy has been shown.

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(raisemap) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && pc_isdead(pl_sd) && map_id == pl_sd->bl.m) {
			atcommand_raise_sub(pl_sd);
			count++;
		}
	}
	clif_displaymessage(fd, msg_txt(64)); // Mercy has been granted.
	if (count == 0)
		sprintf(atcmd_output, "No player raised on the map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "1 player raised on the map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "%d players raised on the map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(raise) {
	struct map_session_data *pl_sd;
	int i, count;

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && pc_isdead(pl_sd)) {
			atcommand_raise_sub(pl_sd);
			count++;
		}
	}
	clif_displaymessage(fd, msg_txt(64)); // Mercy has been granted.
	if (count == 0)
		sprintf(atcmd_output, "No player raised.");
	else if (count == 1)
		sprintf(atcmd_output, "1 player raised.");
	else
		sprintf(atcmd_output, "%d players raised.", count);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @charbaselvl - character base level change
 *------------------------------------------
*/
ATCOMMAND_FUNC(character_baselevel) {
	struct map_session_data *pl_sd;
	int level = 0, i, modified_stat;
	short modified_status[6]; // need to update only modifed stats

	if (!message || !*message || sscanf(message, "%d %[^\n]", &level, atcmd_name) < 2 || level == 0) {
		send_usage(fd, "Please, enter a level adjustement and a player name (usage: %s <#> <nickname>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if (level > 0) {
				if (pl_sd->status.base_level == battle_config.maximum_level) { // check for max level
					clif_displaymessage(fd, msg_txt(91)); // Character's base level can't go any higher.
					return 0;
				}
				if (level > battle_config.maximum_level || level > (battle_config.maximum_level - pl_sd->status.base_level)) // fix positiv overflow
					level = battle_config.maximum_level - pl_sd->status.base_level;
				for (i = 1; i <= level; i++)
					pl_sd->status.status_point += (pl_sd->status.base_level + i + 14) / 5;
				// if player have max in all stats, don't give status_point
				if (pl_sd->status.str  >= battle_config.max_parameter &&
				    pl_sd->status.agi  >= battle_config.max_parameter &&
				    pl_sd->status.vit  >= battle_config.max_parameter &&
				    pl_sd->status.int_ >= battle_config.max_parameter &&
				    pl_sd->status.dex  >= battle_config.max_parameter &&
				    pl_sd->status.luk  >= battle_config.max_parameter)
					pl_sd->status.status_point = 0;
				pl_sd->status.base_level += level;
				clif_updatestatus(pl_sd, SP_BASELEVEL);
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				status_calc_pc(pl_sd, 0);
				pc_heal(pl_sd, pl_sd->status.max_hp, pl_sd->status.max_sp);
				clif_misceffect(&pl_sd->bl, 0);
				clif_displaymessage(fd, msg_txt(65)); // Character's base level raised.
			} else {
				if (pl_sd->status.base_level == 1) {
					clif_displaymessage(fd, msg_txt(193)); // Character's base level can't go any lower.
					return -1;
				}
				if (level < -battle_config.maximum_level || level < (1 - pl_sd->status.base_level)) // fix negativ overflow
					level = 1 - pl_sd->status.base_level;
				for (i = 0; i > level; i--)
					pl_sd->status.status_point -= (pl_sd->status.base_level + i + 14) / 5;
				if (pl_sd->status.status_point < 0) {
					short* status[] = {
						&pl_sd->status.str,  &pl_sd->status.agi, &pl_sd->status.vit,
						&pl_sd->status.int_, &pl_sd->status.dex, &pl_sd->status.luk
					};
					// remove points from stats begining by the upper
					for (i = 0; i <= MAX_STATUS_TYPE; i++)
						modified_status[i] = 0;
					while (pl_sd->status.status_point < 0 &&
					       pl_sd->status.str > 0 &&
					       pl_sd->status.agi > 0 &&
					       pl_sd->status.vit > 0 &&
					       pl_sd->status.int_ > 0 &&
					       pl_sd->status.dex > 0 &&
					       pl_sd->status.luk > 0 &&
					       (pl_sd->status.str + pl_sd->status.agi + pl_sd->status.vit + pl_sd->status.int_ + pl_sd->status.dex + pl_sd->status.luk > 6)) {
						modified_stat = 0;
						for (i = 1; i <= MAX_STATUS_TYPE; i++)
							if (*status[modified_stat] < *status[i])
								modified_stat = i;
						pl_sd->status.status_point += (*status[modified_stat] + 8) / 10 + 1; // ((val - 1) + 9) / 10 + 1
						*status[modified_stat] = *status[modified_stat] - ((short)1);
						modified_status[modified_stat] = 1; // flag
					}
					for (i = 0; i <= MAX_STATUS_TYPE; i++)
						if (modified_status[i]) {
							clif_updatestatus(pl_sd, SP_STR + i);
							clif_updatestatus(pl_sd, SP_USTR + i);
						}
				}
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				pl_sd->status.base_level += level; // note: here, level is negativ
				clif_updatestatus(pl_sd, SP_BASELEVEL);
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
				status_calc_pc(pl_sd, 0);
				clif_displaymessage(fd, msg_txt(66)); // Character's base level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0; //����I��
}

/*==========================================
 * @charjoblvl - character job level change
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_joblevel) {
	struct map_session_data *pl_sd;
	int max_level, level = 0;
	//�]����{�q�̏ꍇ�̌��̐E�Ƃ��Z�o����
	struct pc_base_job pl_s_class;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &level, atcmd_name) < 2 || level == 0) {
		send_usage(fd, "Please, enter a level adjustement and a player name (usage: %s <#> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pl_s_class = pc_calc_base_job(pl_sd->status.class);
			max_level = 50;
			if (pl_s_class.job == 0) // novice
				max_level -= 40;
			// super novices can go up to 99 [celest]
			else if (pl_s_class.job == 23)
				max_level += 49;
			else if (pl_sd->status.class > 4007 && pl_sd->status.class < 4023) //�X�p�m�r�Ɠ]���E��Job���x���̍ō���70
				max_level += 20;

			if (level > 0) {
				if (pl_sd->status.job_level == max_level) {
					clif_displaymessage(fd, msg_txt(67)); // Character's job level can't go any higher.
					return -1;
				}
				if (level > max_level || level > (max_level - pl_sd->status.job_level)) // fix positiv overflow
					level = max_level - pl_sd->status.job_level;
				// check with maximum authorized level
				if (pl_sd->status.class == 0) { // Novice
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_novice) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_novice)
						level = battle_config.atcommand_max_job_level_novice - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 6) { // 1st Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_job1) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_job1)
						level = battle_config.atcommand_max_job_level_job1 - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 22) { // 2nd Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_job2) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_job2)
						level = battle_config.atcommand_max_job_level_job2 - pl_sd->status.job_level;
				} else if (pl_sd->status.class == 23) { // Super Novice
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_supernovice) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_supernovice)
						level = battle_config.atcommand_max_job_level_supernovice - pl_sd->status.job_level;
				} else if (pl_sd->status.class == 4001) { // High Novice
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_highnovice) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_highnovice)
						level = battle_config.atcommand_max_job_level_highnovice - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 4007) { // High 1st Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_highjob1) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_highjob1)
						level = battle_config.atcommand_max_job_level_highjob1 - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 4022) { // High 2nd Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_highjob2) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_highjob2)
						level = battle_config.atcommand_max_job_level_highjob2 - pl_sd->status.job_level;
				} else if (pl_sd->status.class == 4023) { // Baby Novice
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_babynovice) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_babynovice)
						level = battle_config.atcommand_max_job_level_babynovice - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 4029) { // Baby 1st Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_babyjob1) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_babyjob1)
						level = battle_config.atcommand_max_job_level_babyjob1 - pl_sd->status.job_level;
				} else if (pl_sd->status.class <= 4044) { // Baby 2nd Job
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_babyjob2) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_babyjob2)
						level = battle_config.atcommand_max_job_level_babyjob2 - pl_sd->status.job_level;
				} else if (pl_sd->status.class == 4045) { // Super Baby
					if (pl_sd->status.job_level >= battle_config.atcommand_max_job_level_superbaby) {
						clif_displaymessage(fd, "You're not authorized to increase more the job level of this player.");
						return -1;
					} else if (pl_sd->status.job_level + level >= battle_config.atcommand_max_job_level_superbaby)
						level = battle_config.atcommand_max_job_level_superbaby - pl_sd->status.job_level;
				}
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				pl_sd->status.skill_point += level;
				clif_updatestatus(pl_sd, SP_SKILLPOINT);
				status_calc_pc(pl_sd, 0);
				clif_misceffect(&pl_sd->bl, 1);
				clif_displaymessage(fd, msg_txt(68)); // character's job level raised.
			} else {
				if (pl_sd->status.job_level == 1) {
					clif_displaymessage(fd, msg_txt(194)); // Character's job level can't go any lower.
					return -1;
				}
				if (level < -max_level || level < (1 - pl_sd->status.job_level)) // fix negativ overflow
					level = 1 - pl_sd->status.job_level;
				// don't check maximum authorized if we reduce level
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				if (pl_sd->status.skill_point > 0) {
					pl_sd->status.skill_point += level; // note: here, level is negativ
					if (pl_sd->status.skill_point < 0) {
						level = pl_sd->status.skill_point;
						pl_sd->status.skill_point = 0;
					}
					clif_updatestatus(pl_sd, SP_SKILLPOINT);
				}
				if (level < 0) { // if always negativ, skill points must be removed from skills
					// to add: remove skill points from skills
				}
				status_calc_pc(pl_sd, 0);
				clif_displaymessage(fd, msg_txt(69)); // Character's job level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kick) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) // only lower or same gm level
			clif_GM_kick(sd, pl_sd, 1);
		else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kickmap) {
	struct map_session_data *pl_sd;
	int i, count;
	int map_id;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && map_id == pl_sd->bl.m &&
		    sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if (fd != i)
				clif_GM_kick(sd, pl_sd, 0);
				count++;
			}
		}

	if (count == 0)
		sprintf(atcmd_output, "No player kicked from the map '%s'.", map[map_id].name);
	else if (count == 1)
		sprintf(atcmd_output, "1 player kicked from the map '%s'.", map[map_id].name);
	else
		sprintf(atcmd_output, "%d players kicked from the map '%s'.", count, map[map_id].name);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(kickall) {
	struct map_session_data *pl_sd;
	int i, count;

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
		    sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			if (fd != i)
				clif_GM_kick(sd, pl_sd, 0);
				count++;
			}
		}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(28)); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_txt(29)); // 1 player found.
	else {
		sprintf(atcmd_output, msg_txt(30), count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}
	clif_displaymessage(fd, msg_txt(195)); // All players have been kicked!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(allskill) {
	pc_allskillup(sd); // all skills
	clif_displaymessage(fd, msg_txt(76)); // You have received all skills.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(questskill) {
	int skill_id;

	if (!message || !*message || sscanf(message, "%d", &skill_id) < 1 || skill_id < 0) {
		send_usage(fd, "Please, enter a quest skill number (usage: %s <#:0+>).", original_command);
		send_usage(fd, "Novice                 Swordsman                  Thief                Merchant");
		send_usage(fd, "142 = Emergency Care   144 = Moving HP Recovery   149 = Throw Sand     153 = Cart Revolution");
		send_usage(fd, "143 = Act dead         145 = Attack Weak Point    150 = Back Sliding   154 = Change Cart");
		send_usage(fd, "Archer                 146 = Auto Berserk         151 = Take Stone     155 = Crazy Uproar/Loud Voice");
		send_usage(fd, "147 = Arrow Creation   Acolyte                    152 = Stone Throw    Magician");
		send_usage(fd, "148 = Charge Arrows    156 = Holy Light                                157 = Energy Coat");
		send_usage(fd, "Knight                                            Priest");
		send_usage(fd, "1001 = Charge Attack                              1014 = Redemptio");
		send_usage(fd, "Assassin                                          Hunter");
		send_usage(fd, "1003 = Sonic Acceleration                         1009 = Phantasmic Arrow");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) == 0) {
				pc_skill(sd, skill_id, 1, 0);
				if (pc_checkskill(sd, skill_id) == 0) { // if always unknown
					clif_displaymessage(fd, "You can not learn this skill quest (incompatible job?).");
				} else {
					clif_displaymessage(fd, msg_txt(70)); // You have learned the skill.
				}
			} else {
				clif_displaymessage(fd, msg_txt(196)); // You already have this quest skill.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charquestskill) {
	struct map_session_data *pl_sd;
	int skill_id;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &skill_id, atcmd_name) < 2 || skill_id < 0) {
		send_usage(fd, "Please, enter a quest skill number and a player name (usage: %s <#:0+> <char name|account_id>).", original_command);
		send_usage(fd, "Novice                 Swordsman                  Thief                Merchant");
		send_usage(fd, "142 = Emergency Care   144 = Moving HP Recovery   149 = Throw Sand     153 = Cart Revolution");
		send_usage(fd, "143 = Act dead         145 = Attack Weak Point    150 = Back Sliding   154 = Change Cart");
		send_usage(fd, "Archer                 146 = Auto Berserk         151 = Take Stone     155 = Crazy Uproar/Loud Voice");
		send_usage(fd, "147 = Arrow Creation   Acolyte                    152 = Stone Throw    Magician");
		send_usage(fd, "148 = Charge Arrows    156 = Holy Light                                157 = Energy Coat");
		send_usage(fd, "Assassin                                          Hunter");
		send_usage(fd, "1003 = Sonic Acceleration                         1009 = Phantasmic Arrow");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
				if (pc_checkskill(pl_sd, skill_id) == 0) {
					pc_skill(pl_sd, skill_id, 1, 0);
					if (pc_checkskill(pl_sd, skill_id) == 0) { // if always unknown
						clif_displaymessage(fd, "This player can not learn this skill quest (incompatible job?).");
					} else {
						clif_displaymessage(fd, msg_txt(199)); // This player has learned the skill.
					}
				} else {
					clif_displaymessage(fd, msg_txt(200)); // This player already has this quest skill.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_txt(3)); // Character not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(lostskill) {
	int skill_id;

	if (!message || !*message || sscanf(message, "%d", &skill_id) < 1 || skill_id < 0) {
		send_usage(fd, "Please, enter a quest skill number (usage: %s <#:0+>).", original_command);
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) > 0) {
				sd->status.skill[skill_id].id = 0;
				sd->status.skill[skill_id].lv = 0;
				sd->status.skill[skill_id].flag = 0;
				clif_skillinfoblock(sd);
				clif_displaymessage(fd, msg_txt(71)); // You have forgotten the skill.
			} else {
				clif_displaymessage(fd, msg_txt(201)); // You don't have this quest skill.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charlostskill) {
	struct map_session_data *pl_sd;
	int skill_id;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &skill_id, atcmd_name) < 2 || skill_id < 0) {
		send_usage(fd, "Please, enter a quest skill number and a player name (usage: %s <#:0+> <char name|account_id>).", original_command);
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
				if (pc_checkskill(pl_sd, skill_id) > 0) {
					pl_sd->status.skill[skill_id].id = 0;
					pl_sd->status.skill[skill_id].lv = 0;
					pl_sd->status.skill[skill_id].flag = 0;
					clif_skillinfoblock(pl_sd);
					clif_displaymessage(fd, msg_txt(202)); // This player has forgotten the skill.
				} else {
					clif_displaymessage(fd, msg_txt(203)); // This player doesn't have this quest skill.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_txt(3)); // Character not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(spiritball) {
	int number;

	if (!message || !*message || sscanf(message, "%d", &number) < 1 || number < 0) {
		send_usage(fd, "Please, enter a valid spirit ball number (usage: %s <number: 0-500>).", original_command);
		return -1;
	}

	// set max number to avoid server/client crash (500 create big balls of several balls: no visual difference with more)
	if (number > 500)
		number = 500;

	if (sd->spiritball != number) {
		if (sd->spiritball > 0)
			pc_delspiritball(sd, sd->spiritball, 1);
			sd->spiritball = number;
			clif_spiritball(sd);
			// no message, player can look the difference
			if (number > 400)
				clif_displaymessage(fd, msg_txt(204)); // WARNING: more than 400 spiritballs can CRASH your client!
	} else {
		clif_displaymessage(fd, msg_txt(205)); // You already have this number of spiritballs.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(charspiritball) {
	struct map_session_data *pl_sd;
	int number;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &number, atcmd_name) < 2 || number < 0) {
		send_usage(fd, "Please, enter a spirit ball number and a player name (usage: %s <number: 0-500> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			// set max number to avoid server/client crash (500 create big balls of several balls: no visual difference with more)
			if (number > 500)
				number = 500;
			if (pl_sd->spiritball != number) {
				if (pl_sd->spiritball > 0)
					pc_delspiritball(pl_sd, pl_sd->spiritball, 1);
					pl_sd->spiritball = number;
					clif_spiritball(pl_sd);
				if (number > 0) {
					sprintf(atcmd_output, "The player has now %d spirit ball(s).", number);
					clif_displaymessage(fd, atcmd_output);
					if (number > 400)
						clif_displaymessage(fd, "WARNING: more than 400 spiritballs can CRASH the player's client!");
				} else
					clif_displaymessage(fd, "The player has no more spirit ball.");
			} else {
				clif_displaymessage(fd, "The player already has this number of spiritballs.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(party) {
	char party_name[100];

	memset(party_name, 0, sizeof(party_name));

	if (!message || !*message || (sscanf(message, "\"%[^\"]\"", party_name) < 1 && sscanf(message, "%[^\n]", party_name) < 1) || party_name[0] == '\0') {
		send_usage(fd, "Please, enter a party name (usage: %s \"party_name\"|<party_name>).", original_command);
		return -1;
	}

	if (strlen(party_name) <= 24)
		party_create(sd, party_name, 0, 0);
	else {
		clif_displaymessage(fd, msg_txt(258)); // Invalid party name. Party name must have between 1 to 24 characters.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(guild) {
	char guild_name[100];
	int prev;

	memset(guild_name, 0, sizeof(guild_name));

	if (!message || !*message || (sscanf(message, "\"%[^\"]\"", guild_name) < 1 && sscanf(message, "%[^\n]", guild_name) < 1) || guild_name[0] == '\0') {
		send_usage(fd, "Please, enter a guild name (usage: %s \"guild_name\"|<guild_name>).", original_command);
		return -1;
	}

	if (strchr(guild_name, '\"') != NULL) {
		clif_displaymessage(fd, msg_txt(654)); // You are not authorized to use '"' in a guild name. // if you use ", /breakguild will not work
		return -1;
	}

	if (strlen(guild_name) <= 24) {
		prev = battle_config.guild_emperium_check;
		battle_config.guild_emperium_check = 0;
		guild_create(sd, guild_name);
		battle_config.guild_emperium_check = prev;
	} else {
		clif_displaymessage(fd, msg_txt(259)); // Invalid guild name. Guild name must have between 1 to 24 characters.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(agitstart) {
	if (agit_flag == 1) {
		clif_displaymessage(fd, msg_txt(73)); // Already it has started siege warfare.
		return -1;
	}

	agit_flag = 1; // 0: WoE not starting, Woe is running
	guild_agit_start();
	clif_displaymessage(fd, msg_txt(72)); // Guild siege warfare start!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(agitend) {
	if (agit_flag == 0) {
		clif_displaymessage(fd, msg_txt(75)); // Siege warfare hasn't started yet.
		return -1;
	}

	pc_guardiansave(); // save guardians (if necessary)
	agit_flag = 0; // 0: WoE not starting, Woe is running
	guild_agit_end();
	clif_displaymessage(fd, msg_txt(74)); // Guild siege warfare end!

	return 0;
}

/*==========================================
 * @mapexit - shutdown the map-server
 *------------------------------------------
 */
ATCOMMAND_FUNC(mapexit) {
	struct map_session_data *pl_sd;
	int i;

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (fd != i)
				clif_GM_kick(sd, pl_sd, 0);
		}
	}
	clif_GM_kick(sd, sd, 0);

	runflag = 0; // terminate the main loop

	/* send all packets not sent */
	flush_fifos();

	return 0;
}

/*==========================================
 * @idsearch/@itemsearch <part_of_name>: rewritten by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(idsearch) {
	char item_name[100];
	char temp[100];
	int i, j, match;
	struct item_data *item;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s", item_name) < 0) {
		send_usage(fd, "Please, enter a part of item name (usage: %s <part_of_item_name>).", original_command);
		return -1;
	}

	sprintf(atcmd_output, msg_txt(77), item_name); // The reference result of '%s' (name: id):
	clif_displaymessage(fd, atcmd_output);
	match = 0;
	for(i = 0; item_name[i]; i++)
		item_name[i] = tolower((unsigned char)item_name[i]); // tolower needs unsigned char
	for(i = 0; i < 20000; i++) {
		if ((item = itemdb_exists(i)) != NULL) {
			memset(temp, 0, sizeof(temp));
			for(j = 0; item->jname[j]; j++)
				temp[j] = tolower((unsigned char)item->jname[j]); // tolower needs unsigned char
			if (strstr(temp, item_name) != NULL) {
				match++;
				sprintf(atcmd_output, msg_txt(78), item->jname, item->nameid); // %s: %d
				clif_displaymessage(fd, atcmd_output);
			}
		}
	}
	sprintf(atcmd_output, msg_txt(79), match); // It is %d affair above.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * whodrops <name/id_of_item> by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(whodrops) {
	char item_name[100];
	int i;
	int item_id, mob_id, count;
	struct item_data *item;
	struct mob_db *mob;
	double temp_rate, rate;
	int drop_rate;
	int ratemin, ratemax;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s", item_name) < 0) {
		send_usage(fd, "Please, enter an item name/id (usage: %s <name/id_of_item>).", original_command);
		return -1;
	}

	item_id = 0;
	if ((item = itemdb_searchname(item_name)) != NULL ||
	    (item = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item->nameid;

	if (item_id >= 500) {
		count = 0;
		for(mob_id = 1000; mob_id < MAX_MOB_DB; mob_id++) {
			if (!mobdb_checkid(mob_id)) // mob doesn't exist
				continue;
			mob = &mob_db[mob_id];
			// search within normal drops
			for (i = 0; i < MAX_MOB_DROP; i++) {
				if (mob->dropitem[i].nameid != item_id)
					continue;
				// get value for calculation of drop rate
				switch (itemdb_type(mob->dropitem[i].nameid)) {
				case 0:
					rate = (double)battle_config.item_rate_heal;
					ratemin = battle_config.item_drop_heal_min;
					ratemax = battle_config.item_drop_heal_max;
					break;
				case 2:
					rate = (double)battle_config.item_rate_use;
					ratemin = battle_config.item_drop_use_min;
					ratemax = battle_config.item_drop_use_max;
					break;
				case 4:
				case 5:
				case 8: // Changed to include Pet Equip
					rate = (double)battle_config.item_rate_equip;
					ratemin = battle_config.item_drop_equip_min;
					ratemax = battle_config.item_drop_equip_max;
					break;
				case 6:
					rate = (double)battle_config.item_rate_card;
					ratemin = battle_config.item_drop_card_min;
					ratemax = battle_config.item_drop_card_max;
					break;
				default:
					rate = (double)battle_config.item_rate_common;
					ratemin = battle_config.item_drop_common_min;
					ratemax = battle_config.item_drop_common_max;
					break;
				}
				// calculation of drop rate
				if (mob->dropitem[i].p <= 0)
					drop_rate = 0;
				else {
					temp_rate = ((double)mob->dropitem[i].p) * rate / 100.;
					if (temp_rate > 10000. || temp_rate < 0 || ((int)temp_rate) > 10000)
						drop_rate = 10000;
					else
						drop_rate = (int)temp_rate;
				}
				// check minimum/maximum with configuration
				if (drop_rate < ratemin)
					drop_rate = ratemin;
				else if (drop_rate > ratemax)
					drop_rate = ratemax;
				if (drop_rate <= 0 && !battle_config.drop_rate0item)
					drop_rate = 1;
				// display drops
				if (drop_rate > 0) {
					count++;
					if (count == 1) {
						sprintf(atcmd_output, msg_txt(254), item->jname, item->name, item_id); // List of monsters (with current drop rate) that drop '%s (%s)' (id: %d):
						clif_displaymessage(fd, atcmd_output);
					}
					sprintf(atcmd_output, " %s (%s)  %02.02f%%", mob->jname, mob->name, ((float)drop_rate) / 100.);
					clif_displaymessage(fd, atcmd_output);
					break;
				}
			}
			// if item was found
			//if (i != 10) // if an administrator sets same item in normal drop and mvp drop, don't stop here to display MVP drop rate
			//	continue;
			// within mvp drops
			if (mob->mexp) {
				for (i = 0; i < 3; i++) {
					if (mob->mvpitem[i].nameid != item_id)
						continue;
					// calculation of drop rate
					if (mob->mvpitem[i].p <= 0)
						drop_rate = 0;
					else {
						temp_rate = ((double)mob->mvpitem[i].p) * ((double)battle_config.mvp_item_rate) / 100.;
						if (temp_rate > 10000. || temp_rate < 0 || ((int)temp_rate) > 10000)
							drop_rate = 10000;
						else
							drop_rate = (int)temp_rate;
					}
					// check minimum/maximum with configuration
					if (drop_rate < battle_config.item_drop_mvp_min)
						drop_rate = battle_config.item_drop_mvp_min;
					else if (drop_rate > battle_config.item_drop_mvp_max)
						drop_rate = battle_config.item_drop_mvp_max;
					if (drop_rate <= 0 && !battle_config.drop_rate0item)
						drop_rate = 1;
					// display drops
					if (drop_rate > 0) {
						count++;
						if (count == 1) {
							sprintf(atcmd_output, msg_txt(254), item->jname, item->name, item_id); // List of monsters (with current drop rate) that drop '%s (%s)' (id: %d):
							clif_displaymessage(fd, atcmd_output);
						}
						sprintf(atcmd_output, " %s (%s)  %02.02f%% (MVP)", mob->jname, mob->name, ((float)drop_rate) / 100.);
						clif_displaymessage(fd, atcmd_output);
						break;
					}
				}
			}
		}
		// display number of monster
		if (count == 0) {
			sprintf(atcmd_output, msg_txt(255), item->jname, item->name, item_id); // No monster drops item '%s (%s)' (id: %d).
			clif_displaymessage(fd, atcmd_output);
		} else if (count == 1) {
			sprintf(atcmd_output, msg_txt(256), item->jname, item->name, item_id); // 1 monster drops item '%s (%s)' (id: %d).
			clif_displaymessage(fd, atcmd_output);
		} else {
			sprintf(atcmd_output, msg_txt(257), count, item->jname, item->name, item_id); // %d monsters drop item '%s (%s)' (id: %d).
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * GM Stat Reset
 *------------------------------------------
 */
ATCOMMAND_FUNC(resetstate) {
	pc_resetstate(sd);
	clif_displaymessage(fd, "Your stats points are reseted!");

	return 0;
}

/*==========================================
 * GM Skill Reset
 *------------------------------------------
 */
ATCOMMAND_FUNC(resetskill) {
	pc_resetskill(sd);
	clif_displaymessage(fd, "Your skill points are reseted!");

	return 0;
}

/*==========================================
 * Character Stat Reset
 *------------------------------------------
 */
ATCOMMAND_FUNC(charstreset) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pc_resetstate(pl_sd);
			sprintf(atcmd_output, msg_txt(207), pl_sd->status.name); // '%s' stats points reseted!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Skill Reset
 *------------------------------------------
 */
ATCOMMAND_FUNC(charskreset) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pc_resetskill(pl_sd);
			sprintf(atcmd_output, msg_txt(206), pl_sd->status.name); // '%s' skill points reseted!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Reset
 *------------------------------------------
 */
ATCOMMAND_FUNC(charreset) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same gm level
			pc_resetstate(pl_sd);
			pc_resetskill(pl_sd);
			sprintf(atcmd_output, msg_txt(208), pl_sd->status.name); // '%s' skill and stats points reseted!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Model by chbrules
 *------------------------------------------
 */
ATCOMMAND_FUNC(charmodel) {
	int hair_style, hair_color, cloth_color;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %d %[^\n]", &hair_style, &hair_color, &cloth_color, atcmd_name) < 4 || hair_style < 0 || hair_color < 0 || cloth_color < 0) {
		send_usage(fd, "Please, enter a valid model and a player name (usage: %s <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d> <name>).",
		           original_command, battle_config.min_hair_style, battle_config.max_hair_style, battle_config.min_hair_color, battle_config.max_hair_color, battle_config.min_cloth_color, battle_config.max_cloth_color);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (hair_style >= battle_config.min_hair_style && hair_style <= battle_config.max_hair_style &&
		    hair_color >= battle_config.min_hair_color && hair_color <= battle_config.max_hair_color &&
		    cloth_color >= battle_config.min_cloth_color && cloth_color <= battle_config.max_cloth_color) {

			if (!battle_config.clothes_color_for_assassin &&
			    cloth_color != 0 &&
			    pl_sd->status.sex == 1 &&
			    (pl_sd->status.class == 12 ||  pl_sd->status.class == 17)) {
				clif_displaymessage(fd, msg_txt(35)); // You can't use this clothes color with this class.
				return -1;
			} else {
				pc_changelook(pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
			}
		} else {
			clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Skill Point (Rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(charskpoint) {
	struct map_session_data *pl_sd;
	int new_skill_point;
	int point;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &point, atcmd_name) < 2 || point == 0) {
		send_usage(fd, "Please, enter a number and a player name (usage: %s <amount> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		new_skill_point = (int)pl_sd->status.skill_point + point;
		if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
			new_skill_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
			new_skill_point = 0;
		if (new_skill_point != (int)pl_sd->status.skill_point) {
			pl_sd->status.skill_point = new_skill_point;
			clif_updatestatus(pl_sd, SP_SKILLPOINT);
			clif_displaymessage(fd, msg_txt(209)); // Character's number of skill points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Status Point (rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(charstpoint) {
	struct map_session_data *pl_sd;
	int new_status_point;
	int point = 0;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &point, atcmd_name) < 2 || point == 0) {
		send_usage(fd, "Please, enter a number and a player name (usage: %s <amount> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		new_status_point = (int)pl_sd->status.status_point + point;
		if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
			new_status_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
			new_status_point = 0;
		if (new_status_point != (int)pl_sd->status.status_point) {
			// if player has max in all stats, don't give status_point
			if (pl_sd->status.str  >= battle_config.max_parameter &&
			    pl_sd->status.agi  >= battle_config.max_parameter &&
			    pl_sd->status.vit  >= battle_config.max_parameter &&
			    pl_sd->status.int_ >= battle_config.max_parameter &&
			    pl_sd->status.dex  >= battle_config.max_parameter &&
			    pl_sd->status.luk  >= battle_config.max_parameter &&
			    new_status_point != 0) {
				pl_sd->status.status_point = 0;
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				clif_displaymessage(fd, "This player have max in each stat -> status points set to 0.");
				return -1;
			} else {
				pl_sd->status.status_point = new_status_point;
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				clif_displaymessage(fd, msg_txt(210)); // Character's number of status points changed!
			}
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Zeny Point (Rewritten by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(charzeny) {
	struct map_session_data *pl_sd;
	int zeny, new_zeny;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &zeny, atcmd_name) < 2 || zeny == 0) {
		send_usage(fd, "Please, enter a number and a player name (usage: %s <zeny> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		new_zeny = pl_sd->status.zeny + zeny;
		if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
			new_zeny = MAX_ZENY;
		else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
			new_zeny = 0;
		if (new_zeny != pl_sd->status.zeny) {
			pl_sd->status.zeny = new_zeny;
			clif_updatestatus(pl_sd, SP_ZENY);
			clif_displaymessage(fd, msg_txt(211)); // Character's number of zenys changed!
		} else {
			if (zeny < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Recall All Characters Online To Your Location
 *------------------------------------------
 */
ATCOMMAND_FUNC(recallall) {
	struct map_session_data *pl_sd;
	int i;
	int count;

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp somenone to your actual map.");
		return -1;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && fd != i &&
		    sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level)
				count++;
			else
				pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
		}
	}

	clif_displaymessage(fd, msg_txt(92)); // All characters recalled!
	if (count) {
		sprintf(atcmd_output, "Because you are not authorized to warp from some maps, %d player(s) have not been recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Recall online characters of a guild to your location
 *------------------------------------------
 */
ATCOMMAND_FUNC(guildrecall) {
	struct map_session_data *pl_sd;
	int i;
	char guild_name[100];
	struct guild *g;
	int count;

	memset(guild_name, 0, sizeof(guild_name));

	if (!message || !*message || sscanf(message, "%[^\n]", guild_name) < 1) {
		send_usage(fd, "Please, enter a guild name/id (usage: %s <guild_name/id>).", original_command);
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp somenone to your actual map.");
		return -1;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		count = 0;
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
			    fd != i &&
			    pl_sd->status.guild_id == g->guild_id) {
				if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level)
					count++;
				else
					pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			}
		}
		sprintf(atcmd_output, msg_txt(93), g->name); // All online characters of the '%s' guild are near you.
		clif_displaymessage(fd, atcmd_output);
		if (count) {
			sprintf(atcmd_output, "Because you are not authorized to warp from some maps, %d player(s) have not been recalled.", count);
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	return 0;
}

/*==========================================
 * Recall online characters of a party to your location
 *------------------------------------------
 */
ATCOMMAND_FUNC(partyrecall) {
	int i;
	struct map_session_data *pl_sd;
	char party_name[100];
	struct party *p;
	int count;

	memset(party_name, 0, sizeof(party_name));

	if (!message || !*message || sscanf(message, "%[^\n]", party_name) < 1) {
		send_usage(fd, "Please, enter a party name/id (usage: %s <party_name/id>).", original_command);
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > sd->GM_level) {
		clif_displaymessage(fd, "You are not authorized to warp somenone to your actual map.");
		return -1;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		count = 0;
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
			    fd != i &&
			    pl_sd->status.party_id == p->party_id) {
				if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > sd->GM_level)
					count++;
				else
					pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			}
		}
		sprintf(atcmd_output, msg_txt(95), p->name); // All online characters of the '%s' party are near you.
		clif_displaymessage(fd, atcmd_output);
		if (count) {
			sprintf(atcmd_output, "Because you are not authorized to warp from some maps, %d player(s) have not been recalled.", count);
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(96)); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(reloaditemdb) {
	itemdb_reload();
	clif_displaymessage(fd, msg_txt(97)); // Item database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(reloadmobdb) {
	mob_reload();
	clif_displaymessage(fd, msg_txt(98)); // Monster database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(reloadskilldb) {
	skill_reload();
	clif_displaymessage(fd, msg_txt(99)); // Skill database reloaded.

	return 0;
}

int atkillnpc_sub(struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	npc_delete((struct npc_data *)bl);

	return 0;
}

void rehash(const int fd, struct map_session_data* sd) {
	int map_id;

	for (map_id = 0; map_id < map_num; map_id++) {
		map_foreachinarea(atkillmonster_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_MOB, 0);
		map_foreachinarea(atkillnpc_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_NPC, 0);
	}

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(reloadscript) {
	intif_GMmessage("Nezumi Server is Rehashing...", 0);
	intif_GMmessage("You will feel a bit of lag at this point !", 0);
	flush_fifos(); // send message to all players

	rehash(fd, sd);

	intif_GMmessage("Reloading NPCs...", 0);
	flush_fifos(); // send message to all players

	do_init_npc();
	do_init_script();

	npc_event_do_oninit();

	clif_displaymessage(fd, msg_txt(100)); // Scripts reloaded.

	return 0;
}

/*==========================================
 * @mapinfo <map name> [0-3] by MC_Cameri
 * => Shows information about the map [map name]
 * 0 = no additional information
 * 1 = Show users in that map and their location
 * 2 = Shows NPCs in that map
 * 3 = Shows the shops/chats in that map
 *------------------------------------------
 */
ATCOMMAND_FUNC(mapinfo) {
	struct map_session_data *pl_sd;
	struct npc_data *nd = NULL;
	struct chat_data *cd = NULL;
	char direction[12];
	int map_id, i, chat_num, list = 0;
	struct block_list *bl;
	int b, mob_num, slave_num;

	memset(atcmd_mapname, 0, sizeof(atcmd_mapname));
	memset(direction, 0, sizeof(direction));

	sscanf(message, "%d %[^\n]", &list, atcmd_mapname);

	if (list < 0 || list > 3) {
		send_usage(fd, "Please, enter at least a valid list number (usage: %s <0-3> [map]).", original_command);
		return -1;
	}

	if (atcmd_mapname[0] == '\0')
		strncpy(atcmd_mapname, sd->mapname, sizeof(atcmd_mapname) - 1);
	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) { // only from actual map-server
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	clif_displaymessage(fd, "------ Map Info ------");
	sprintf(atcmd_output, "Map Name: %s", atcmd_mapname);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Players In Map: %d", map[map_id].users);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "NPCs In Map: %d", map[map_id].npc_num);
	clif_displaymessage(fd, atcmd_output);
	slave_num = 0;
	mob_num = 0;
	for (b = 0; b < map[map_id].bxs * map[map_id].bys; b++)
		for (bl = map[map_id].block_mob[b]; bl; bl = bl->next) {
			mob_num++;
			if (((struct mob_data *)bl)->master_id)
				slave_num++;
		}
	if (slave_num == 0)
		sprintf(atcmd_output, "Mobs In Map: %d (of which is no slave).", mob_num);
	else
		sprintf(atcmd_output, "Mobs In Map: %d (of which are %d slaves).", mob_num, slave_num);
	clif_displaymessage(fd, atcmd_output);
	chat_num = 0;
	for (i = 0; i < fd_max; i++)
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
		    map_id == pl_sd->bl.m &&
		    (cd = (struct chat_data*)map_id2bl(pl_sd->chatID)))
			chat_num++;
	sprintf(atcmd_output, "Chats In Map: %d", chat_num);
	clif_displaymessage(fd, atcmd_output);
	clif_displaymessage(fd, "------ Map Flags ------");
	sprintf(atcmd_output, "Player vs Player: PvP: %s | No Guild: %s | No Party: %s",
	        (map[map_id].flag.pvp) ? "True" : "False",
	        (map[map_id].flag.pvp_noguild) ? "True" : "False",
	        (map[map_id].flag.pvp_noparty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "PVP options: nightmare drop: %s | No Calcul Rank: %s",
	        (map[map_id].flag.pvp_nightmaredrop) ? "True" : "False",
	        (map[map_id].flag.pvp_nocalcrank) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Guild vs Guild: %s | No Party: %s | Guild Dungeon: %s", (map[map_id].flag.gvg) ? "True" : "False", (map[map_id].flag.gvg_noparty) ? "True" : "False", (map[map_id].flag.guild_dungeon) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Dead Branch: %s", (map[map_id].flag.nobranch) ? "True" : "False"); // forbid usage of Dead Branch (604), Bloody Branch (12103) and Poring Box (12109)
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Memo: %s", (map[map_id].flag.nomemo) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Penalty: %s", (map[map_id].flag.nopenalty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Zeny Penalty: %s", (map[map_id].flag.nozenypenalty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Return: %s", (map[map_id].flag.noreturn) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Save: %s", (map[map_id].flag.nosave) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Teleport: %s", (map[map_id].flag.noteleport) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Monster Teleport: %s", (map[map_id].flag.monster_noteleport) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Warp options: No Warp: %s | No WarpTo: %s | No Go: %s",
	        (map[map_id].flag.nowarp) ? "True" : "False",
	        (map[map_id].flag.nowarpto) ? "True" : "False",
	        (map[map_id].flag.nogo) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Spells and Skills: No Spell: %s | No Skill: %s | No Ice Wall: %s",
	        (map[map_id].flag.nospell) ? "True" : "False",
	        (map[map_id].flag.noskill) ? "True" : "False",
	        (map[map_id].flag.noicewall) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Trade: %s", (map[map_id].flag.notrade) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Weathers: Snow: %s | Fog: %s | Sakura: %s | Leaves: %s | Rain: %s",
	        (map[map_id].flag.snow) ? "True" : "False",
	        (map[map_id].flag.fog) ? "True" : "False",
	        (map[map_id].flag.sakura) ? "True" : "False",
	        (map[map_id].flag.leaves) ? "True" : "False",
	        (map[map_id].flag.rain) ? "True" : "False");
	sprintf(atcmd_output, "Indoors: %s", (map[map_id].flag.indoors) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	if (map[map_id].flag.nogmcmd == 100)
		sprintf(atcmd_output, "GM Commands limits: None (All GM commands can be used).");
	else if (map[map_id].flag.nogmcmd == 0)
		sprintf(atcmd_output, "GM Commands limits: No GM command can be used.");
	else if (map[map_id].flag.nogmcmd == 1)
		sprintf(atcmd_output, "GM Commands limits: only GM commands with level 0 can be used.");
	else
		sprintf(atcmd_output, "GM Commands limits: only GM commands with level 0 to %d can be used.", map[map_id].flag.nogmcmd - 1);
	clif_displaymessage(fd, atcmd_output);
	if (map[map_id].flag.mingmlvl == 100)
		sprintf(atcmd_output, "Minimum GM level to use GM commands: All GM commands are forbiden.");
	else if (map[map_id].flag.mingmlvl == 0)
		sprintf(atcmd_output, "Minimum GM level to use GM commands: None (any player can use GM commands, if the player have access to the command).");
	else
		sprintf(atcmd_output, "Minimum GM level to use GM commands: %d.", map[map_id].flag.mingmlvl);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Other Flags: No Base EXP: %s | No Job EXP: %s | No Mob Drop: %s | No MVP Drop: %s",
	        (map[map_id].flag.nobaseexp) ? "True" : "False",
	        (map[map_id].flag.nojobexp) ? "True" : "False",
	        (map[map_id].flag.nomobdrop) ? "True" : "False",
	        (map[map_id].flag.nomvpdrop) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);

	switch (list) {
	case 0:
		// Do nothing. It's list 0, no additional display.
		break;
	case 1:
		clif_displaymessage(fd, "----- Players in Map -----");
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth && pl_sd->bl.m == map_id) {
				sprintf(atcmd_output, "Player '%s' (session #%d) | Location: %d,%d",
				        pl_sd->status.name, i, pl_sd->bl.x, pl_sd->bl.y);
				clif_displaymessage(fd, atcmd_output);
			}
		}
		break;
	case 2:
		clif_displaymessage(fd, "----- NPCs in Map -----");
		for (i = 0; i < map[map_id].npc_num; i++) {
			nd = map[map_id].npc[i];
			switch(nd->dir) {
			case 0:  strcpy(direction, "North"); break;
			case 1:  strcpy(direction, "North West"); break;
			case 2:  strcpy(direction, "West"); break;
			case 3:  strcpy(direction, "South West"); break;
			case 4:  strcpy(direction, "South"); break;
			case 5:  strcpy(direction, "South East"); break;
			case 6:  strcpy(direction, "East"); break;
			case 7:  strcpy(direction, "North East"); break;
			case 9:  strcpy(direction, "North"); break;
			default: strcpy(direction, "Unknown"); break;
			}
			sprintf(atcmd_output, "NPC %d (%s): %s | Direction: %s | Sprite: %d | Location: %d %d",
			        (i + 1), nd->exname, nd->name, direction, nd->class, nd->bl.x, nd->bl.y);
			clif_displaymessage(fd, atcmd_output);
		}
		break;
	case 3:
		clif_displaymessage(fd, "----- Chats in Map -----");
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth &&
			    pl_sd->bl.m == map_id &&
			    (cd = (struct chat_data*)map_id2bl(pl_sd->chatID)) &&
			    cd->usersd[0] == pl_sd) {
				sprintf(atcmd_output, "Chat %d: %s | Player: %s | Location: %d %d",
				        i, cd->title, pl_sd->status.name, cd->bl.x, cd->bl.y);
				clif_displaymessage(fd, atcmd_output);
				sprintf(atcmd_output, "   Users: %d/%d | Password: %s | Public: %s",
				        cd->users, cd->limit, cd->pass, (cd->pub) ? "Yes" : "No");
				clif_displaymessage(fd, atcmd_output);
			}
		}
		break;
	default: // normally impossible to arrive here
		send_usage(fd, "Please, enter at least a valid list number (usage: %s <0-3> [map]).", original_command);
		return -1;
		break;
	}

	return 0;
}

/*==========================================
* Show Monster DB Info   v 1.0
* originally by [Lupus] eAthena
*------------------------------------------
*/
ATCOMMAND_FUNC(mobinfo) {
	unsigned char msize[3][7] = {"Small", "Medium", "Large"};
	unsigned char mrace[12][11] = {"Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Boss", "Non-Boss"};
	unsigned char melement[11][8] = {"None", "Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead"};
	char output2[200];
	struct item_data *item_data;
	struct mob_db *mob;
	int mob_id;
	int i, j;
	double temp_rate, rate;
	int drop_rate, max_hp;
	int ratemin, ratemax;

	memset(atcmd_output, 0, sizeof(atcmd_output));
	memset(output2, 0, sizeof(output2));

	if (!message || !*message) {
		send_usage(fd, "Please, enter a Monster/NPC name/id (usage: %s <monster_name_or_monster_ID>).", original_command);
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(message));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
		return -1;
	}

	mob = &mob_db[mob_id];

	// stats
	if (mob->mexp)
		sprintf(atcmd_output, "Monster (MVP): '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	else
		sprintf(atcmd_output, "Monster: '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	clif_displaymessage(fd, atcmd_output);
	max_hp = mob->max_hp;
	if (mob->mexp > 0) {
		if (battle_config.mvp_hp_rate != 100)
			max_hp = max_hp * battle_config.mvp_hp_rate / 100;
	} else {
		if (battle_config.monster_hp_rate != 100)
			max_hp = max_hp * battle_config.monster_hp_rate / 100;
	}
	sprintf(atcmd_output, " Level:%d  HP:%d  SP:%d  Base EXP:%d  Job EXP:%d", mob->lv, max_hp, mob->max_sp, mob->base_exp, mob->job_exp);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, " DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d", mob->def, mob->mdef, mob->str, mob->agi, mob->vit, mob->int_, mob->dex, mob->luk);
	clif_displaymessage(fd, atcmd_output);
	if (mob->element < 20) {
		//Element - None, Level 0
		i = 0;
		j = 0;
	} else {
		i = mob->element % 20 + 1;
		j = mob->element / 20;
	}
	sprintf(atcmd_output, " ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)", mob->atk1, mob->atk2, mob->range, mob->range2 , mob->range3, msize[mob->size], mrace[mob->race], melement[i], j);
	clif_displaymessage(fd, atcmd_output);
	// drops
	clif_displaymessage(fd, " Drops:");
	strcpy(atcmd_output, " ");
	j = 0;
	for (i = 0; i < MAX_MOB_DROP; i++) {
		if (mob->dropitem[i].nameid <= 0 || (item_data = itemdb_search(mob->dropitem[i].nameid)) == NULL)
			continue;
		// get value for calculation of drop rate
		switch (itemdb_type(mob->dropitem[i].nameid)) {
		case 0:
			rate = (double)battle_config.item_rate_heal;
			ratemin = battle_config.item_drop_heal_min;
			ratemax = battle_config.item_drop_heal_max;
			break;
		case 2:
			rate = (double)battle_config.item_rate_use;
			ratemin = battle_config.item_drop_use_min;
			ratemax = battle_config.item_drop_use_max;
			break;
		case 4:
		case 5:
		case 8: // Changed to include Pet Equip
			rate = (double)battle_config.item_rate_equip;
			ratemin = battle_config.item_drop_equip_min;
			ratemax = battle_config.item_drop_equip_max;
			break;
		case 6:
			rate = (double)battle_config.item_rate_card;
			ratemin = battle_config.item_drop_card_min;
			ratemax = battle_config.item_drop_card_max;
			break;
		default:
			rate = (double)battle_config.item_rate_common;
			ratemin = battle_config.item_drop_common_min;
			ratemax = battle_config.item_drop_common_max;
			break;
		}
		// calculation of drop rate
		if (mob->dropitem[i].p <= 0)
			drop_rate = 0;
		else {
			temp_rate = ((double)mob->dropitem[i].p) * rate / 100.;
			if (temp_rate > 10000. || temp_rate < 0 || ((int)temp_rate) > 10000)
				drop_rate = 10000;
			else
				drop_rate = (int)temp_rate;
		}
		// check minimum/maximum with configuration
		if (drop_rate < ratemin)
			drop_rate = ratemin;
		else if (drop_rate > ratemax)
			drop_rate = ratemax;
		if (drop_rate <= 0 && !battle_config.drop_rate0item)
			drop_rate = 1;
		// display drops
		if (drop_rate > 0) {
			sprintf(output2, " - %s  %02.02f%%", item_data->name, ((float)drop_rate) / 100.);
			strcat(atcmd_output, output2);
			if (++j % 3 == 0) {
				clif_displaymessage(fd, atcmd_output);
				strcpy(atcmd_output, " ");
			}
		}
	}
	if (j == 0)
		clif_displaymessage(fd, "This monster has no drop.");
	else if (j % 3 != 0)
		clif_displaymessage(fd, atcmd_output);
	// mvp
	if (mob->mexp) {
		sprintf(atcmd_output, " MVP Bonus EXP:%d  %02.02f%%", mob->mexp, (float)mob->mexpper / 100.);
		clif_displaymessage(fd, atcmd_output);
		strcpy(atcmd_output, " MVP Items:");
		j = 0;
		for (i = 0; i < 3; i++) {
			if (mob->mvpitem[i].nameid <= 0 || (item_data = itemdb_search(mob->mvpitem[i].nameid)) == NULL)
				continue;
			// calculation of drop rate
			if (mob->mvpitem[i].p <= 0)
				drop_rate = 0;
			else {
				temp_rate = ((double)mob->mvpitem[i].p) * ((double)battle_config.mvp_item_rate) / 100.;
				if (temp_rate > 10000. || temp_rate < 0 || ((int)temp_rate) > 10000)
					drop_rate = 10000;
				else
					drop_rate = (int)temp_rate;
			}
			// check minimum/maximum with configuration
			if (drop_rate < battle_config.item_drop_mvp_min)
				drop_rate = battle_config.item_drop_mvp_min;
			else if (drop_rate > battle_config.item_drop_mvp_max)
				drop_rate = battle_config.item_drop_mvp_max;
			if (drop_rate <= 0 && !battle_config.drop_rate0item)
				drop_rate = 1;
			// display drops
			if (drop_rate > 0) {
				j++;
				if (j == 1)
					sprintf(output2, " %s  %02.02f%%", item_data->name, ((float)drop_rate) / 100.);
				else
					sprintf(output2, " - %s  %02.02f%%", item_data->name, ((float)drop_rate) / 100.);
				strcat(atcmd_output, output2);
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "This monster has no MVP drop.");
		else
			clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(mount_peco) {
	if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
		clif_displaymessage(fd, msg_txt(212)); // Cannot mount a Peco while in disguise.
		return -1;
	}

	if (!pc_isriding(sd)) { // if actually no peco
		if (sd->status.class ==    7 || sd->status.class ==   14 ||
		    sd->status.class == 4008 || sd->status.class == 4015 ||
		    sd->status.class == 4030 || sd->status.class == 4037) {
			// normal classes
			if (sd->status.class == 7)
				sd->status.class = sd->view_class = 13;
			else if (sd->status.class == 14)
				sd->status.class = sd->view_class = 21;
			// high classes
			else if (sd->status.class == 4008)
				sd->status.class = sd->view_class = 4014;
			else if (sd->status.class == 4015)
				sd->status.class = sd->view_class = 4022;
			// baby classes
			else if (sd->status.class == 4030)
				sd->status.class = sd->view_class = 4036;
			else if (sd->status.class == 4037)
				sd->status.class = sd->view_class = 4044;
			pc_setoption(sd, sd->status.option | 0x0020);
			clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
			if (sd->status.clothes_color > 0)
				clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
			clif_displaymessage(fd, msg_txt(102)); // Mounted Peco.
		} else {
			clif_displaymessage(fd, msg_txt(213)); // You can not mount a peco with your job.
			return -1;
		}
	} else {
		// normal classes
		if (sd->status.class == 13)
			sd->status.class = sd->view_class = 7;
		else if (sd->status.class == 21)
			sd->status.class = sd->view_class = 14;
		// high classes
		else if (sd->status.class == 4014)
			sd->status.class = sd->view_class = 4008;
		else if (sd->status.class == 4022)
			sd->status.class = sd->view_class = 4015;
		// baby classes
		else if (sd->status.class == 4036)
			sd->status.class = sd->view_class = 4030;
		else if (sd->status.class == 4044)
			sd->status.class = sd->view_class = 4037;
		pc_setoption(sd, sd->status.option & ~0x0020);
		clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
		if (sd->status.clothes_color > 0)
			clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
		clif_displaymessage(fd, msg_txt(214)); // Unmounted Peco.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_mount_peco) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
			clif_displaymessage(fd, msg_txt(215)); // This player cannot mount a Peco while in disguise.
			return -1;
		}

		if (!pc_isriding(pl_sd)) { // if actually no peco
			if (pl_sd->status.class ==    7 || pl_sd->status.class ==   14 ||
			    pl_sd->status.class == 4008 || pl_sd->status.class == 4015 ||
			    pl_sd->status.class == 4030 || pl_sd->status.class == 4037) {
				// normal classes
				if (pl_sd->status.class == 7)
					pl_sd->status.class = pl_sd->view_class = 13;
				else if (pl_sd->status.class == 14)
					pl_sd->status.class = pl_sd->view_class = 21;
				// high classes
				else if (pl_sd->status.class == 4008)
					pl_sd->status.class = pl_sd->view_class = 4014;
				else if (pl_sd->status.class == 4015)
					pl_sd->status.class = pl_sd->view_class = 4022;
				// baby classes
				else if (pl_sd->status.class == 4030)
					pl_sd->status.class = pl_sd->view_class = 4036;
				else if (pl_sd->status.class == 4037)
					pl_sd->status.class = pl_sd->view_class = 4044;
				pc_setoption(pl_sd, pl_sd->status.option | 0x0020);
				clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
				if (pl_sd->status.clothes_color > 0)
					clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
				clif_displaymessage(fd, msg_txt(216)); // Now, this player mounts a peco.
			} else {
				clif_displaymessage(fd, msg_txt(217)); // This player can not mount a peco with his/her job.
				return -1;
			}
		} else {
			// normal classes
			if (pl_sd->status.class == 13)
				pl_sd->status.class = pl_sd->view_class = 7;
			else if (pl_sd->status.class == 21)
				pl_sd->status.class = pl_sd->view_class = 14;
			// high classes
			else if (pl_sd->status.class == 4014)
				pl_sd->status.class = pl_sd->view_class = 4008;
			else if (pl_sd->status.class == 4022)
				pl_sd->status.class = pl_sd->view_class = 4015;
			// baby classes
			else if (pl_sd->status.class == 4036)
				pl_sd->status.class = pl_sd->view_class = 4030;
			else if (pl_sd->status.class == 4044)
				pl_sd->status.class = pl_sd->view_class = 4037;
			pc_setoption(pl_sd, pl_sd->status.option & ~0x0020);
			clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
			if (pl_sd->status.clothes_color > 0)
				clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
			clif_displaymessage(fd, msg_txt(218)); // Now, this player has not more peco.
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(falcon) {
	if (!pc_isfalcon(sd)) { // if actually no falcon
		pc_setoption(sd, sd->status.option | 0x0010);
		clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
		if (sd->status.clothes_color > 0)
			clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
		clif_displaymessage(fd, "Falcon is here.");
		if (pc_iscarton(sd) && (sd->status.option & 0x0008) != 0x0008)
			clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
	} else {
		pc_setoption(sd, sd->status.option & ~0x0010);
		clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
		if (sd->status.clothes_color > 0)
			clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
		clif_displaymessage(fd, "Falcon is go on.");
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_falcon) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (!pc_isfalcon(pl_sd)) { // if actually no falcon
			pc_setoption(pl_sd, pl_sd->status.option | 0x0010);
			clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
			if (pl_sd->status.clothes_color > 0)
				clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
			clif_displaymessage(fd, "Now, this player has a falcon.");
			if (pc_iscarton(pl_sd) && (pl_sd->status.option & 0x0008) != 0x0008)
				clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
		} else {
			pc_setoption(pl_sd, pl_sd->status.option & ~0x0010);
			clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
			if (pl_sd->status.clothes_color > 0)
				clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
			clif_displaymessage(fd, "Now, this player has not more falcon.");
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(cart) {
	short cart;

	if (command[5] != 0) { // cart0, cart1, cart2, cart3, cart4, cart5
		cart = (short)command[5] - 48;
		if (cart < 0 || cart > 5) {
			return -1;
		}
	} else if (!message || !*message || sscanf(message, "%hd", &cart) < 1 || cart < 0 || cart > 5) {
		send_usage(fd, "Please, enter a cart type (usage: %s <cart_type:0-5>).", original_command);
		return -1;
	}

	switch (cart) {
	case 0:
		return atcommand_remove_cart(fd, sd, original_command, "@removecart", "");
	case 1:
		cart = 0x0008;
		break;
	case 2:
		cart = 0x0080;
		break;
	case 3:
		cart = 0x0100;
		break;
	case 4:
		cart = 0x0200;
		break;
	case 5:
		cart = 0x0400;
		break;
	}

	if ((sd->status.option & cart) != cart) { // if not right cart
		if (!pc_iscarton(sd)) { // if actually no cart
			pc_setoption(sd, sd->status.option | cart); /* first */
			clif_cart_itemlist(sd);
			clif_cart_equiplist(sd);
			clif_updatestatus(sd, SP_CARTINFO);
		} else {
			sd->status.option &= ~((short)CART_MASK); /* suppress actual cart */
			pc_setoption(sd, sd->status.option | cart);
		}
		clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
		if (sd->status.clothes_color > 0)
			clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
		clif_displaymessage(fd, "Now, you have the desired cart.");
		if (pc_isfalcon(sd) && (cart & 0x0008) != 0x0008)
			clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
	} else {
		clif_displaymessage(fd, "You already have this cart.");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(remove_cart) {
	if (pc_iscarton(sd)) { // if actually a cart
		pc_setoption(sd, sd->status.option & ~CART_MASK);
		clif_changelook(&sd->bl, LOOK_BASE, sd->view_class); /* for button in the equip window */
		if (sd->status.clothes_color > 0)
			clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);
		clif_displaymessage(fd, "Now, you have no more cart.");
	} else {
		clif_displaymessage(fd, "You already have no cart.");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_cart) {
	short cart;
	struct map_session_data *pl_sd;

	if (command[9] != 0) { // charcart0, charcart1, charcart2, charcart3, charcart4, charcart5
		cart = (short)command[9] - 48;
		if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1 || cart < 0 || cart > 5) {
			send_usage(fd, "Please, enter a character name (usage: %s <char name|account_id>).", original_command);
			return -1;
		}
	} else if (!message || !*message || sscanf(message, "%hd %[^\n]", &cart, atcmd_name) < 2 || cart < 0 || cart > 5) {
		send_usage(fd, "Please, enter a cart type and a character name (usage: %s <cart_type:0-5> <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		switch (cart) {
		case 0:
			return atcommand_char_remove_cart(fd, sd, original_command, "@charremovecart", pl_sd->status.name); // not a common variable to all @ commands
		case 1:
			cart = 0x0008;
			break;
		case 2:
			cart = 0x0080;
			break;
		case 3:
			cart = 0x0100;
			break;
		case 4:
			cart = 0x0200;
			break;
		case 5:
			cart = 0x0400;
			break;
		}

		if ((pl_sd->status.option & cart) != cart) { // if not right cart
			if (!pc_iscarton(pl_sd)) { // if actually no cart
				pc_setoption(pl_sd, pl_sd->status.option | cart); /* first */
				clif_cart_itemlist(pl_sd);
				clif_cart_equiplist(pl_sd);
				clif_updatestatus(pl_sd, SP_CARTINFO);
			} else {
				pl_sd->status.option &= ~((short)CART_MASK); /* suppress actual cart */
				pc_setoption(pl_sd, pl_sd->status.option | cart);
			}
			clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
			if (pl_sd->status.clothes_color > 0)
				clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
			clif_displaymessage(fd, "Now, this player has the desired cart.");
			if (pc_isfalcon(pl_sd) && (cart & 0x0008) != 0x0008)
				clif_displaymessage(fd, "Falcon can display all carts like a basic cart (type 1).");
		} else {
			clif_displaymessage(fd, "This player already has this cart.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(char_remove_cart) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a character name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (pc_iscarton(pl_sd)) { // if actually a cart
			pc_setoption(pl_sd, pl_sd->status.option & ~CART_MASK);
			clif_changelook(&pl_sd->bl, LOOK_BASE, pl_sd->view_class); /* for button in the equip window */
			if (pl_sd->status.clothes_color > 0)
				clif_changelook(&pl_sd->bl, LOOK_CLOTHES_COLOR, pl_sd->status.clothes_color);
			clif_displaymessage(fd, "Now, This player has no more cart.");
		} else {
			clif_displaymessage(fd, "This player already has no cart.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *Spy Commands by Syrus22
 *------------------------------------------
 */
ATCOMMAND_FUNC(guildspy) {
	char guild_name[100];
	struct guild *g;

	memset(guild_name, 0, sizeof(guild_name));

	if (!message || !*message || sscanf(message, "%[^\n]", guild_name) < 1) {
		send_usage(fd, "Please, enter a guild name/id (usage: %s <guild_name/id>).", original_command);
		return -1;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		if (sd->guildspy == g->guild_id) {
			sd->guildspy = 0;
			sprintf(atcmd_output, msg_txt(103), g->name); // No longer spying on the '%s' guild.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->guildspy = g->guild_id;
			sprintf(atcmd_output, msg_txt(104), g->name); // Spying on the '%s' guild.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(94)); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(partyspy) {
	char party_name[100];
	struct party *p;

	memset(party_name, 0, sizeof(party_name));

	if (!message || !*message || sscanf(message, "%[^\n]", party_name) < 1) {
		send_usage(fd, "Please, enter a party name/id (usage: %s <party_name/id>).", original_command);
		return -1;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		if (sd->partyspy == p->party_id) {
			sd->partyspy = 0;
			sprintf(atcmd_output, msg_txt(105), p->name); // No longer spying on the '%s' party.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->partyspy = p->party_id;
			sprintf(atcmd_output, msg_txt(106), p->name); // Spying on the '%s' party.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_txt(96)); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	return 0;
}

/*==========================================
 * @repairall [Valaris]
 *------------------------------------------
 */
ATCOMMAND_FUNC(repairall) {
	int count, i;

	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && sd->status.inventory[i].attribute == 1) {
			sd->status.inventory[i].attribute = 0;
			clif_produceeffect(sd, 0, sd->status.inventory[i].nameid);
			count++;
		}
	}

	if (count > 0) {
		clif_misceffect(&sd->bl, 3);
		clif_equiplist(sd);
		clif_displaymessage(fd, msg_txt(107)); // All items have been repaired.
	} else {
		clif_displaymessage(fd, msg_txt(108)); // No item need to be repaired.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(nuke) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same GM level
			skill_castend_damage_id(&pl_sd->bl, &pl_sd->bl, NPC_SELFDESTRUCTION, 99, gettick_cache, 0);
			clif_displaymessage(fd, msg_txt(109)); // Player has been nuked!
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(enablenpc) {
	char NPCname[100];

	memset(NPCname, 0, sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%[^\n]", NPCname) < 1) {
		send_usage(fd, "Please, enter a NPC name (usage: %s <NPC_name>).", original_command);
		return -1;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 1);
		clif_displaymessage(fd, msg_txt(110)); // Npc Enabled.
	} else {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(disablenpc) {
	char NPCname[100];

	memset(NPCname, 0, sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%[^\n]", NPCname) < 1) {
		send_usage(fd, "Please, enter a NPC name (usage: %s <NPC_name>).", original_command);
		return -1;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 0);
		clif_displaymessage(fd, msg_txt(112)); // Npc Disabled.
	} else {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 * time in txt for time command (by [Yor])
 *------------------------------------------
 */
char * txt_time(unsigned int duration) {
	int days, hours, minutes, seconds;
	char temp[MAX_MSG_LEN + 100]; // max size of msg_txt + security (char name, char id, etc...) (100)
	static char temp1[MAX_MSG_LEN + 100]; // static: value is the return value

	memset(temp, 0, sizeof(temp));
	memset(temp1, 0, sizeof(temp1));

//	if (duration < 0) // unsigned int, never < 0
//		duration = 0;

	days = duration / (60 * 60 * 24);
	duration = duration - (60 * 60 * 24 * days);
	hours = duration / (60 * 60);
	duration = duration - (60 * 60 * hours);
	minutes = duration / 60;
	seconds = duration - (60 * minutes);

	if (days == 0) {
		// do nothing (temp is void)
	} else if (days > -2 && days < 2) // -1 0 1
		sprintf(temp, msg_txt(219), days); // %d day
	else
		sprintf(temp, msg_txt(220), days); // %d days

	if (hours == 0 && temp[0] == '\0') {
		// don't add hours if no day (temp1 is void)
	} else if (hours > -2 && hours < 2) // -1 0 1
		sprintf(temp1, msg_txt(221), temp, hours); // %s %d hour
	else
		sprintf(temp1, msg_txt(222), temp, hours); // %s %d hours

	if (minutes > -2 && minutes < 2) // -1 0 1
		sprintf(temp, msg_txt(223), temp1, minutes); // %s %d minute
	else
		sprintf(temp, msg_txt(224), temp1, minutes); // %s %d minutes

	// always add seconds!
	if (seconds > -2 && seconds < 2) // -1 0 1
		sprintf(temp1, msg_txt(225), temp, seconds); // %s and %d second
	else
		sprintf(temp1, msg_txt(226), temp, seconds); // %s and %d seconds

	return temp1;
}

/*==========================================
 * @time/@date/@server_date/@serverdate/@server_time/@servertime: Display the date/time of the server (by [Yor]
 * Calculation management of GM modification (@day/@night GM commands) is done
 *------------------------------------------
 */
ATCOMMAND_FUNC(servertime) {
	struct TimerData * timer_data;
	struct TimerData * timer_data2;
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...

	memset(atcmd_output, 0, sizeof(atcmd_output));

	time(&time_server); // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(atcmd_output, sizeof(atcmd_output) - 1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, atcmd_output);

	if (battle_config.night_duration == 0 && battle_config.day_duration == 0) {
		if (!night_flag) // night_flag: 0=day, 1=night [Yor]
			clif_displaymessage(fd, msg_txt(231)); // Game time: The game is in permanent daylight.
		else
			clif_displaymessage(fd, msg_txt(232)); // Game time: The game is in permanent night.
	} else if (battle_config.night_duration == 0)
		if (night_flag) { // we start with night // night_flag: 0=day, 1=night [Yor]
			timer_data = get_timer(day_timer_tid);
			sprintf(atcmd_output, msg_txt(233), txt_time((timer_data->tick - gettick_cache) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, atcmd_output);
			clif_displaymessage(fd, msg_txt(234)); // Game time: After, the game will be in permanent daylight.
		} else
			clif_displaymessage(fd, msg_txt(231)); // Game time: The game is in permanent daylight.
	else if (battle_config.day_duration == 0)
		if (!night_flag) { // we start with day
			timer_data = get_timer(night_timer_tid);
			sprintf(atcmd_output, msg_txt(235), txt_time((timer_data->tick - gettick_cache) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, atcmd_output);
			clif_displaymessage(fd, msg_txt(236)); // Game time: After, the game will be in permanent night.
		} else
			clif_displaymessage(fd, msg_txt(232)); // Game time: The game is in permanent night.
	else {
		if (!night_flag) { // night_flag: 0=day, 1=night [Yor]
			timer_data = get_timer(night_timer_tid);
			timer_data2 = get_timer(day_timer_tid);
			sprintf(atcmd_output, msg_txt(235), txt_time((timer_data->tick - gettick_cache) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, atcmd_output);
			if (timer_data->tick > timer_data2->tick)
				sprintf(atcmd_output, msg_txt(237), txt_time((timer_data->interval - abs(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in night for %s.
			else
				sprintf(atcmd_output, msg_txt(237), txt_time(abs(timer_data->tick - timer_data2->tick) / 1000)); // Game time: After, the game will be in night for %s.
			clif_displaymessage(fd, atcmd_output);
			sprintf(atcmd_output, msg_txt(238), txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, atcmd_output);
		} else {
			timer_data = get_timer(day_timer_tid);
			timer_data2 = get_timer(night_timer_tid);
			sprintf(atcmd_output, msg_txt(233), txt_time((timer_data->tick - gettick_cache) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, atcmd_output);
			if (timer_data->tick > timer_data2->tick)
				sprintf(atcmd_output, msg_txt(239), txt_time((timer_data->interval - abs(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in daylight for %s.
			else
				sprintf(atcmd_output, msg_txt(239), txt_time(abs(timer_data->tick - timer_data2->tick) / 1000)); // Game time: After, the game will be in daylight for %s.
			clif_displaymessage(fd, atcmd_output);
			sprintf(atcmd_output, msg_txt(238), txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, atcmd_output);
		}
	}

	return 0;
}

/*==========================================
 * @chardelitem <item_name_or_ID> <quantity> <char name|account_id> (by [Yor]
 * removes <quantity> item from a character
 * item can be equiped or not.
 * Inspired from a old command created by RoVeRT
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardelitem) {
	struct map_session_data *pl_sd;
	char item_name[100];
	int i, number, item_id, item_position, count;
	struct item_data *item_data;

	memset(item_name, 0, sizeof(item_name));

	if (!message || !*message || sscanf(message, "%s %d %[^\n]", item_name, &number, atcmd_name) < 3 || number < 1) {
		send_usage(fd, "Please, enter an item name/id, a quantity and a player name (usage: %s <item_name_or_ID> <quantity> <char name|account_id>).", original_command);
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				item_position = pc_search_inventory(pl_sd, item_id);
				if (item_position >= 0) {
					count = 0;
					for(i = 0; i < number && item_position >= 0; i++) {
						pc_delitem(pl_sd, item_position, 1, 0);
						count++;
						item_position = pc_search_inventory(pl_sd, item_id); // for next loop
					}
					if (pl_sd != sd) {
						sprintf(atcmd_output, msg_txt(113), count); // %d item(s) removed by a GM.
						clif_displaymessage(pl_sd->fd, atcmd_output);
					}
					if (number == count)
						sprintf(atcmd_output, msg_txt(114), count); // %d item(s) removed from the player.
					else
						sprintf(atcmd_output, msg_txt(115), count, count, number); // %d item(s) removed. Player had only %d on %d items.
					clif_displaymessage(fd, atcmd_output);
				} else {
					clif_displaymessage(fd, msg_txt(116)); // Character does not have the item.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(3)); // Character not found.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * @jail <time> <char name|account_id> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 * Time is done as follows:
 *   Adjustment value (-1, 1, +1, etc...)
 *   Modified element:
 *     a or y: year
 *     m:  month
 *     j or d: day
 *     h:  hour
 *     mn: minute
 *     s:  second
 * <example> @jail +1m-2mn1s-6y test_player
 *           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
 *------------------------------------------
 */
ATCOMMAND_FUNC(jail) {
	char modif[100];
	struct map_session_data *pl_sd;
	int x, y;
	char * modif_p;
	int year, month, day, hour, minute, second, value;
	time_t jail_time;

	memset(modif, 0, sizeof(modif));

	if (!message || !*message || sscanf(message, "%s %[^\n]", modif, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a player name (usage: %s <time> <char name|account_id>).", original_command);
		send_usage(fd, "time usage: adjustement (+/- value) and element (y/a, m, d/j, h, mn, s)");
		send_usage(fd, "Example: %s +1m-2mn1s-6y testplayer", original_command);
		return -1;
	}

	modif_p = modif;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second += value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute += value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour += value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day += value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month += value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year += value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif_displaymessage(fd, msg_txt(625)); // Invalid time for jail command.
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same GM
			if (map_checkmapname("sec_pri.gat") != -1) {
				time_t timestamp;
				struct tm *tmtime;
				jail_time = (time_t)pc_readglobalreg(pl_sd, "JAIL_TIMER");
				if (jail_time == 0 || jail_time < time(NULL))
					timestamp = time(NULL);
				else
					timestamp = jail_time;
				tmtime = localtime(&timestamp);
				tmtime->tm_year = tmtime->tm_year + year;
				tmtime->tm_mon = tmtime->tm_mon + month;
				tmtime->tm_mday = tmtime->tm_mday + day;
				tmtime->tm_hour = tmtime->tm_hour + hour;
				tmtime->tm_min = tmtime->tm_min + minute;
				tmtime->tm_sec = tmtime->tm_sec + second;
				timestamp = mktime(tmtime);
				if (timestamp != -1) {
					if (timestamp <= time(NULL))
						timestamp = 0;
					if (jail_time != timestamp) {
						if (timestamp != 0) {
							pc_setglobalreg(pl_sd, "JAIL_TIMER", timestamp);
							chrif_save(pl_sd); // do pc_makesavestatus and save storage + account_reg/account_reg2 too
							// if already in jail
							if (jail_time != 0) {
								// just change timer value
								addtick_timer(pl_sd->jailtimer, (timestamp - jail_time) * 1000);
								// send message to GM
								sprintf(atcmd_output, msg_txt(629), (timestamp - jail_time), // Jail time of the player mofified by %+d second%s.
								        ((timestamp - jail_time) > 1 || (timestamp - jail_time) < -1) ? "s" : "");
								clif_displaymessage(fd, atcmd_output);
								sprintf(atcmd_output, msg_txt(633), txt_time(timestamp - time(NULL))); // Character is now in jail for %s.
								clif_displaymessage(fd, atcmd_output);
								// send message to player
								sprintf(atcmd_output, msg_txt(634), (timestamp - jail_time), // Your jail time has been mofified by %+d second%s.
								        ((timestamp - jail_time) > 1 || (timestamp - jail_time) < -1) ? "s" : "");
								clif_displaymessage(pl_sd->fd, atcmd_output);
								sprintf(atcmd_output, msg_txt(635), txt_time(timestamp - time(NULL))); // You are now in jail for %s.
								clif_displaymessage(pl_sd->fd, atcmd_output);
							// player is not in jail actually
							} else {
								switch(rand() % 2) {
								case 0:
									x = 24;
									y = 75;
									break;
								default:
									x = 49;
									y = 75;
									break;
								}
								pc_setsavepoint(pl_sd, "sec_pri.gat", x, y); // Save Char Respawn Point in the jail room [Lupus]
								if (pl_sd != sd) {
									sprintf(atcmd_output, msg_txt(117), txt_time(timestamp - time(NULL))); // GM has send you in jails for %s.
									clif_displaymessage(pl_sd->fd, atcmd_output);
								}
								if (pl_sd->jailtimer != -1) // normally impossible, but we know...
									delete_timer(pl_sd->jailtimer, pc_jail_timer);
								pl_sd->jailtimer = add_timer(gettick_cache + ((timestamp - time(NULL)) * 1000), pc_jail_timer, pl_sd->bl.id, 0);
								if (pc_setpos(pl_sd, "sec_pri.gat", x, y, 3) == 0) {
									sprintf(atcmd_output, msg_txt(118), txt_time(timestamp - time(NULL))); // Player warped in jails for %s.
									clif_displaymessage(fd, atcmd_output);
									// send message to all players
									if (battle_config.jail_message) { // Do we send message to ALL players when a player is put in jail?
										sprintf(atcmd_output, msg_txt(640), pl_sd->status.name, txt_time(timestamp - time(NULL))); // %s has been put in jail for %s.
										intif_GMmessage(atcmd_output, 0);
									}
								} else {
									clif_displaymessage(fd, msg_txt(1)); // Map not found.
									return -1;
								}
							}
						} else {
							// call unjail function
							sprintf(modif, "%s", atcmd_name); // use local buffer to avoid problem (we call another function that can use global variables)
							return atcommand_unjail(fd, sd, original_command, "@unjail", modif);
						}
					} else {
						clif_displaymessage(fd, msg_txt(628)); // No change done for jail time of this player.
						return -1;
					}
				} else {
					clif_displaymessage(fd, msg_txt(627)); // Invalid final time for jail command.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_txt(1)); // Map not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @unjail/@discharge <char name|account_id> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------
 */
ATCOMMAND_FUNC(unjail) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same GM
			if (strcmp(pl_sd->status.last_point.map, "sec_pri.gat") != 0) {
				clif_displaymessage(fd, msg_txt(119)); // This player is not in jails.
				return -1;
			} else {
				if (pl_sd->jailtimer != -1) {
					delete_timer(pl_sd->jailtimer, pc_jail_timer);
					pl_sd->jailtimer = -1;
				}
				pc_setglobalreg(pl_sd, "JAIL_TIMER", 0);
				chrif_save(pl_sd); // do pc_makesavestatus and save storage + account_reg/account_reg2 too
				pc_setsavepoint(pl_sd, "prontera.gat", 156, 191); // Save char respawn point in Prontera
				// send message to player
				if (pl_sd != sd)
					clif_displaymessage(pl_sd->fd, msg_txt(120)); // GM has discharge you.
				// send message to all players
				if (battle_config.jail_discharge_message & 2) { // Do we send message to ALL players when a player is discharged?
					char *discharge_message;
					CALLOC(discharge_message, char, 16 + strlen(msg_txt(632)) + 1); // name (16) + message (A GM has discharged %s from jail.) + NULL (1)
					sprintf(discharge_message, msg_txt(632), pl_sd->status.name); // A GM has discharged %s from jail.
					intif_GMmessage(discharge_message, 0);
					FREE(discharge_message);
				}
				if (pc_setpos(pl_sd, "prontera.gat", 156, 191, 3) == 0) {
					clif_displaymessage(fd, msg_txt(121)); // Player warped to Prontera.
				} else {
					clif_displaymessage(fd, msg_txt(1)); // Map not found.
					return -1;
				}
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @jailtime (give remaining jail time)
 *------------------------------------------
 */
ATCOMMAND_FUNC(jailtime) {
	time_t jail_time;

	jail_time = (time_t)pc_readglobalreg(sd, "JAIL_TIMER");
	if (jail_time == 0)
		clif_displaymessage(fd, msg_txt(637)); // You are not in jail.
	else {
		sprintf(atcmd_output, msg_txt(636), txt_time(jail_time - time(NULL))); // You are actually in jail for %s.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @charjailtime (give remaining jail time of a player)
 *------------------------------------------
 */
ATCOMMAND_FUNC(charjailtime) {
	struct map_session_data* pl_sd;
	time_t jail_time;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			jail_time = (time_t)pc_readglobalreg(pl_sd, "JAIL_TIMER");
			if (jail_time == 0)
				clif_displaymessage(fd, msg_txt(639)); // This player is not in jail.
			else {
				sprintf(atcmd_output, msg_txt(638), txt_time(jail_time - time(NULL))); // This player is actually in jail for %s.
				clif_displaymessage(fd, atcmd_output);
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @disguise <mob_id> by [Valaris] (simplified by [Yor])
 *------------------------------------------
 */
ATCOMMAND_FUNC(disguise) {
	int mob_id;

	if (!message || !*message) {
		send_usage(fd, "Please, enter a Monster/NPC name/id (usage: %s <monster_name_or_monster_ID>).", original_command);
		return -1;
	}

	if ((mob_id = mobdb_searchname(message)) == 0) { // check name first (to avoid possible name begining by a number)
		if ((mob_id = mobdb_checkid(atoi(message))) == 0) {
			mob_id = atoi(message);
			if ((mob_id >= 46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 858)) { // NPC (id source: http://kalen.s79.xrea.com/npc/npce.shtml)
			} else {
				clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id hasn't been found.
				return -1;
			}
		}
	}

	if (pc_isriding(sd)) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
		clif_displaymessage(fd, msg_txt(227)); // Cannot wear disguise while riding a Peco.
		return -1;
	}

	pc_stop_walking(sd, 0);
	clif_clearchar(&sd->bl, 9);
	sd->disguise = mob_id;
	sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
	clif_changeoption(&sd->bl);
	clif_spawnpc(sd);

	clif_displaymessage(fd, msg_txt(122)); // Disguise applied.

	return 0;
}

/*==========================================
 * @undisguise by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(undisguise) {
	if (sd->disguise) {
		pc_stop_walking(sd, 0);
		clif_clearchar(&sd->bl, 9);
		sd->disguiseflag = 0; // reset to override items with disguise script [Valaris]
		sd->disguise = 0;
		clif_changeoption(&sd->bl);
		clif_spawnpc(sd);

		clif_displaymessage(fd, msg_txt(124)); // Undisguise applied.
	} else {
		clif_displaymessage(fd, msg_txt(125)); // You're not disguised.
		return -1;
	}

	return 0;
}

/*==========================================
 * @chardisguise <mob_id> <character> by Kalaspuff (based off Valaris' and Yor's work)
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardisguise) {
	int mob_id;
	char mob_name[100];
	struct map_session_data* pl_sd;

	memset(mob_name, 0, sizeof(mob_name));

	if (!message || !*message || sscanf(message, "%s %[^\n]", mob_name, atcmd_name) < 2) {
		send_usage(fd, "Please, enter a Monster/NPC name/id and a player name (usage: %s <monster_name_or_monster_ID> <char name|account_id>).", original_command);
		return -1;
	}

	if ((mob_id = mobdb_searchname(mob_name)) == 0) { // check name first (to avoid possible name begining by a number)
		if ((mob_id = mobdb_checkid(atoi(mob_name))) == 0) {
			mob_id = atoi(mob_name);
			if ((mob_id >= 46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 858)) { // NPC (id source: http://kalen.s79.xrea.com/npc/npce.shtml)
			} else {
				clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id hasn't been found.
				return -1;
			}
		}
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pc_isriding(pl_sd)) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
				clif_displaymessage(fd, msg_txt(228)); // Character cannot wear disguise while riding a Peco.
				return -1;
			}
			pc_stop_walking(pl_sd, 0);
			clif_clearchar(&pl_sd->bl, 9);
			pl_sd->disguise = mob_id;
			pl_sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
			clif_changeoption(&pl_sd->bl);
			clif_spawnpc(pl_sd);
			clif_displaymessage(pl_sd->fd, "You have been disguised.");

			clif_displaymessage(fd, msg_txt(140)); // Character's disguise applied.
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charundisguise <character> by Kalaspuff (based off Yor's work)
 *------------------------------------------
 */
ATCOMMAND_FUNC(charundisguise) {
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->disguise) {
				pc_stop_walking(pl_sd, 0);
				clif_clearchar(&pl_sd->bl, 9);
				pl_sd->disguiseflag = 0; // reset to override items with disguise script [Valaris]
				pl_sd->disguise = 0;
				clif_changeoption(&pl_sd->bl);
				clif_spawnpc(pl_sd);
				clif_displaymessage(pl_sd->fd, "You have been undisguised.");

				clif_displaymessage(fd, msg_txt(141)); // Character's undisguise applied.
			} else {
				clif_displaymessage(fd, msg_txt(142)); // Character is not disguised.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @chardisguisemap <mob_id> [map]
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardisguisemap) {
	struct map_session_data* pl_sd;
	char mob_name[100];
	int mob_id;
	int i, count;
	int map_id = 0;

	memset(mob_name, 0, sizeof(mob_name));
	memset(atcmd_mapname, 0, sizeof(atcmd_mapname));

	if (!message || !*message || sscanf(message, "%s %[^\n]", mob_name, atcmd_mapname) < 1) {
		send_usage(fd, "Please, enter a Monster/NPC name/id (usage: %s <monster_name_or_monster_ID> [map]).", original_command);
		return -1;
	}

	if (atcmd_mapname[0] == '\0')
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	if ((mob_id = mobdb_searchname(mob_name)) == 0) { // check name first (to avoid possible name begining by a number)
		if ((mob_id = mobdb_checkid(atoi(mob_name))) == 0) {
			mob_id = atoi(mob_name);
			if ((mob_id >= 46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 858)) { // NPC (id source: http://kalen.s79.xrea.com/npc/npce.shtml)
			} else {
				clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id hasn't been found.
				return -1;
			}
		}
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (pl_sd->bl.m == map_id) { // same map
					if (pc_isriding(pl_sd)) // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
						continue;
					pc_stop_walking(pl_sd, 0);
					clif_clearchar(&pl_sd->bl, 9);
					pl_sd->disguise = mob_id;
					pl_sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
					clif_changeoption(&pl_sd->bl);
					clif_spawnpc(pl_sd);
					clif_displaymessage(pl_sd->fd, "You have been disguised.");

					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "No player has been disguised.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player has been disguised.");
	else {
		sprintf(atcmd_output, "%d players have been disguised.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @charundisguisemap [map]
 *------------------------------------------
 */
ATCOMMAND_FUNC(charundisguisemap) {
	struct map_session_data* pl_sd;
	int i, count;
	int map_id = 0;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (pl_sd->bl.m == map_id) { // same map
					if (pl_sd->disguise) {
						pc_stop_walking(pl_sd, 0);
						clif_clearchar(&pl_sd->bl, 9);
						pl_sd->disguiseflag = 0; // reset to override items with disguise script [Valaris]
						pl_sd->disguise = 0;
						clif_changeoption(&pl_sd->bl);
						clif_spawnpc(pl_sd);
						clif_displaymessage(pl_sd->fd, "You have been undisguised.");

						count++;
					}
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "No disguised player found.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player has been undisguised.");
	else {
		sprintf(atcmd_output, "%d players have been undisguised.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @chardisguiseall <mob_id>
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardisguiseall) {
	struct map_session_data* pl_sd;
	char mob_name[100];
	int mob_id;
	int i, count;

	memset(mob_name, 0, sizeof(mob_name));

	if (!message || !*message || sscanf(message, "%s", mob_name) < 1) {
		send_usage(fd, "Please, enter a Monster/NPC name/id (usage: %s <monster_name_or_monster_ID>).", original_command);
		return -1;
	}

	if ((mob_id = mobdb_searchname(mob_name)) == 0) { // check name first (to avoid possible name begining by a number)
		if ((mob_id = mobdb_checkid(atoi(mob_name))) == 0) {
			mob_id = atoi(mob_name);
			if ((mob_id >= 46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 858)) { // NPC (id source: http://kalen.s79.xrea.com/npc/npce.shtml)
			} else {
				clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id hasn't been found.
				return -1;
			}
		}
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (pc_isriding(pl_sd)) // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
					continue;
				pc_stop_walking(pl_sd, 0);
				clif_clearchar(&pl_sd->bl, 9);
				pl_sd->disguise = mob_id;
				pl_sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
				clif_changeoption(&pl_sd->bl);
				clif_spawnpc(pl_sd);
				clif_displaymessage(pl_sd->fd, "You have been disguised.");

				count++;
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "No player has been disguised.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player has been disguised.");
	else {
		sprintf(atcmd_output, "%d players have been disguised.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @charundisguiseall
 *------------------------------------------
 */
ATCOMMAND_FUNC(charundisguiseall) {
	struct map_session_data* pl_sd;
	int i, count;

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
			if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
				if (pl_sd->disguise) {
					pc_stop_walking(pl_sd, 0);
					clif_clearchar(&pl_sd->bl, 9);
					pl_sd->disguiseflag = 0; // reset to override items with disguise script [Valaris]
					pl_sd->disguise = 0;
					clif_changeoption(&pl_sd->bl);
					clif_spawnpc(pl_sd);
					clif_displaymessage(pl_sd->fd, "You have been undisguised.");

					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, "No disguised player found.");
	else if (count == 1)
		clif_displaymessage(fd, "1 player has been undisguised.");
	else {
		sprintf(atcmd_output, "%d players have been undisguised.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @changelook by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(changelook) {
	int i, item_id;
	char item_name[100];
	struct item_data *item_data;

	if (!message || !*message || sscanf(message, "%s", item_name) < 1) {
		send_usage(fd, "Usage: %s <item name | ID>", original_command);
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		if (item_data->type == 4 || item_data->type == 5) { // 4 = weapons, 5 = armors
			i = 0;
			if (item_data->equip & 0x0100)
				i |= LOOK_HEAD_TOP;
			if (item_data->equip & 0x0200)
				i |= LOOK_HEAD_MID;
			if (item_data->equip & 0x0001)
				i |= LOOK_HEAD_BOTTOM;
			if (item_data->equip & 0x0002)
				i |= LOOK_WEAPON;
			if (item_data->equip & 0x0020)
				i |= LOOK_SHIELD;
			if (item_data->equip & 0x0040)
				i |= LOOK_SHOES;
			if (i != 0) {
				clif_changelook(&sd->bl, i, item_data->look);
				clif_displaymessage(fd, "Your look has been changed (if item can be visible).");
				if (i & LOOK_WEAPON || i & LOOK_SHIELD || i & LOOK_SHOES)
					clif_displaymessage(fd, "Changing of Weapons, Shields and Shoes is probably not visible."); // it's a problem inside clif_changelook function (using what the player has equiped)
			} else {
				clif_displaymessage(fd, "This item is not visible -> Changelook canceled.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, "This item is not an equipment.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charchangelook by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(charchangelook) {
	int i, item_id;
	char item_name[100];
	struct item_data *item_data;
	struct map_session_data* pl_sd;

	if (!message || !*message || sscanf(message, "%s %[^\n]", item_name, atcmd_name) < 2) {
		send_usage(fd, "Usage: %s <item name | ID> <char name|account_id>", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			item_id = 0;
			if ((item_data = itemdb_searchname(item_name)) != NULL ||
			    (item_data = itemdb_exists(atoi(item_name))) != NULL)
				item_id = item_data->nameid;

			if (item_id >= 500) {
				if (item_data->type == 4 || item_data->type == 5) { // 4 = weapons, 5 = armors
					i = 0;
					if (item_data->equip & 0x0100)
						i |= LOOK_HEAD_TOP;
					if (item_data->equip & 0x0200)
						i |= LOOK_HEAD_MID;
					if (item_data->equip & 0x0001)
						i |= LOOK_HEAD_BOTTOM;
					if (item_data->equip & 0x0002)
						i |= LOOK_WEAPON;
					if (item_data->equip & 0x0020)
						i |= LOOK_SHIELD;
					if (item_data->equip & 0x0040)
						i |= LOOK_SHOES;
					if (i != 0) {
						clif_changelook(&pl_sd->bl, i, item_data->look);
						clif_displaymessage(fd, "Look of the player changed (if item can be visible).");
						if (i & LOOK_WEAPON || i & LOOK_SHIELD || i & LOOK_SHOES)
							clif_displaymessage(fd, "Changing of Weapons, Shields and Shoes is probably not visible."); // it's a problem inside clif_changelook function (using what the player has equiped)
					} else {
						clif_displaymessage(fd, "This item is not visible -> Changelook canceled.");
						return -1;
					}
				} else {
					clif_displaymessage(fd, "This item is not an equipment.");
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @broadcast by [Valaris]
 *------------------------------------------
 */
ATCOMMAND_FUNC(broadcast) {
	if (!message || !*message) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (check_bad_word(message, strlen(message), sd))
		return -1; // check_bad_word function display message if necessary

	sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	intif_GMmessage(atcmd_output, 0);

	return 0;
}

/*==========================================
 * @localbroadcast
 *------------------------------------------
 */
ATCOMMAND_FUNC(localbroadcast) {
	if (!message || !*message) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (check_bad_word(message, strlen(message), sd))
		return -1; // check_bad_word function display message if necessary

	if (battle_config.atcommand_add_local_message_info)
		sprintf(atcmd_output, "%s: %s %s", sd->status.name, msg_txt(253), message); // (map message)
	else
		sprintf(atcmd_output, "%s: %s", sd->status.name, message);
	clif_GMmessage(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, 1); // 1: ALL_SAMEMAP

	return 0;
}

/*==========================================
 * @nlb (without name of GM)
 *------------------------------------------
 */
ATCOMMAND_FUNC(localbroadcast2) {
	if (!message || !*message) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	// because /b give gm name, we check which sentence we propose for bad words
	if (strncmp(message, sd->status.name, strlen(sd->status.name)) == 0 && message[strlen(sd->status.name)] == ':') {
		//printf("/lb: Check with GM name: %s\n", message);
		if (check_bad_word(message + strlen(sd->status.name) + 1, strlen(message), sd))
			return -1; // check_bad_word function display message if necessary
	} else {
		//printf("/nlb or /mb: Check without GM name: %s\n", message);
		if (check_bad_word(message, strlen(message), sd))
			return -1; // check_bad_word function display message if necessary
	}

	if (battle_config.atcommand_add_local_message_info)
		sprintf(atcmd_output, "%s %s", msg_txt(253), message); // (map message)
	else
		strcpy(atcmd_output, message);
	clif_GMmessage(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, 1); // 1: ALL_SAMEMAP

	return 0;
}

/*==========================================
 * @ignorelist by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(ignorelist) {
	int i;

	if (sd->ignoreAll == 0)
		if (sd->ignore_num == 0)
			clif_displaymessage(fd, msg_txt(126)); // You accept any wisp (no wisper is refused).
		else {
			sprintf(atcmd_output, msg_txt(127), sd->ignore_num); // You accept any wisp, except thoses from %d player(s):
			clif_displaymessage(fd, atcmd_output);
		}
	else
		if (sd->ignore_num == 0)
			clif_displaymessage(fd, msg_txt(128)); // You refuse all wisps (no specifical wisper is refused).
		else {
			sprintf(atcmd_output, msg_txt(129), sd->ignore_num); // You refuse all wisps, AND refuse wisps from %d player(s):
			clif_displaymessage(fd, atcmd_output);
		}

	for(i = 0; i < sd->ignore_num; i++)
		clif_displaymessage(fd, sd->ignore[i].name);

	return 0;
}

/*==========================================
 * @charignorelist <player_name> by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(charignorelist) {
	struct map_session_data *pl_sd;
	int i;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {

		if (pl_sd->ignoreAll == 0)
			if (pl_sd->ignore_num == 0) {
				sprintf(atcmd_output, msg_txt(130), pl_sd->status.name); // '%s' accept any wisp (no wisper is refused).
				clif_displaymessage(fd, atcmd_output);
			} else {
				sprintf(atcmd_output, msg_txt(131), pl_sd->status.name, pl_sd->ignore_num); // '%s' accept any wisp, except thoses from %d player(s):
				clif_displaymessage(fd, atcmd_output);
			}
		else
			if (pl_sd->ignore_num == 0) {
				sprintf(atcmd_output, msg_txt(132), pl_sd->status.name); // '%s' refuse all wisps (no specifical wisper is refused).
				clif_displaymessage(fd, atcmd_output);
			} else {
				sprintf(atcmd_output, msg_txt(133), pl_sd->status.name, pl_sd->ignore_num); // '%s' refuse all wisps, AND refuse wisps from %d player(s):
				clif_displaymessage(fd, atcmd_output);
			}

		for(i = 0; i < pl_sd->ignore_num; i++)
			clif_displaymessage(fd, pl_sd->ignore[i].name);

	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @inall <player_name> by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(inall) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->ignoreAll == 0) {
				sprintf(atcmd_output, msg_txt(134), pl_sd->status.name); // '%s' already accepts all wispers.
				clif_displaymessage(fd, atcmd_output);
				return -1;
			} else {
				pl_sd->ignoreAll = 0;
				sprintf(atcmd_output, msg_txt(135), pl_sd->status.name); // '%s' now accepts all wispers.
				clif_displaymessage(fd, atcmd_output);
				// message to player
				clif_displaymessage(pl_sd->fd, msg_txt(136)); // A GM has authorized all wispers for you.
				WPACKETW(0) = 0x0d2; // R 00d2 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
				WPACKETB(2) = 1;
				WPACKETB(3) = 0; // success
				SENDPACKET(pl_sd->fd, 4); // packet_len_table[0x0d2]
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @exall <player_name> by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(exall) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->ignoreAll == 1) {
				sprintf(atcmd_output, msg_txt(137), pl_sd->status.name); // '%s' already blocks all wispers.
				clif_displaymessage(fd, atcmd_output);
				return -1;
			} else {
				pl_sd->ignoreAll = 1;
				sprintf(atcmd_output, msg_txt(138), pl_sd->status.name); // '%s' blocks now all wispers.
				clif_displaymessage(fd, atcmd_output);
				// message to player
				clif_displaymessage(pl_sd->fd, msg_txt(139)); // A GM has blocked all wispers for you.
				WPACKETW(0) = 0x0d2; // R 00d2 <type>.B <fail>.B: type: 0: deny, 1: allow, fail: 0: success, 1: fail
				WPACKETB(2) = 0;
				WPACKETB(3) = 0; // success
				SENDPACKET(pl_sd->fd, 4); // packet_len_table[0x0d2]
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @email <actual@email> <new@email> by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(email) {
	char actual_email[100];
	char new_email[100];

	memset(actual_email, 0, sizeof(actual_email));
	memset(new_email, 0, sizeof(new_email));

	if (!message || !*message || sscanf(message, "%s %s", actual_email, new_email) < 2) {
		send_usage(fd, "Please enter 2 emails (usage: %s <actual@email> <new@email>).", original_command);
		return -1;
	}

	if (e_mail_check(actual_email) == 0) {
		clif_displaymessage(fd, msg_txt(144)); // Invalid actual email. If you have default e-mail, give a@a.com.
		return -1;
	} else if (e_mail_check(new_email) == 0) {
		clif_displaymessage(fd, msg_txt(145)); // Invalid new email. Please enter a real e-mail.
		return -1;
	} else if (strcasecmp(new_email, "a@a.com") == 0) {
		clif_displaymessage(fd, msg_txt(146)); // New email must be a real e-mail.
		return -1;
	} else if (strcasecmp(actual_email, new_email) == 0) {
		clif_displaymessage(fd, msg_txt(147)); // New email must be different of the actual e-mail.
		return -1;
	} else {
		chrif_changeemail(sd->status.account_id, actual_email, new_email);
		clif_displaymessage(fd, msg_txt(148)); // Information sent to login-server via char-server.
	}

	return 0;
}

/*==========================================
 *@effect
 *------------------------------------------
 */
ATCOMMAND_FUNC(effect) {
	struct map_session_data *pl_sd;
	int type = 0, flag = 0, i;

	if (!message || !*message || sscanf(message, "%d %d", &type, &flag) < 2) {
		send_usage(fd, "Please, enter at least an option (usage: %s <type> <flag>).", original_command);
		return -1;
	}

	if (flag <= 0) {
		clif_specialeffect(&sd->bl, type, 0); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
		clif_displaymessage(fd, msg_txt(229)); // Your effect has changed.
	} else {
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				clif_specialeffect(&pl_sd->bl, type, flag); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
				clif_displaymessage(pl_sd->fd, msg_txt(229)); // Your effect has changed.
			}
		}
	}

	return 0;
}

/*==========================================
 * @charitemlist <character>: Displays the list of a player's items.
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_item_list) {
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, equip, count, counter, counter2;
	char equipstr[100], outputtmp[200];

	memset(equipstr, 0, sizeof(equipstr));
	memset(outputtmp, 0, sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_INVENTORY; i++) {
				if (pl_sd->status.inventory[i].nameid > 0 && (item_data = itemdb_search(pl_sd->status.inventory[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.inventory[i].amount;
					count++;
					if (count == 1) {
						sprintf(atcmd_output, "------ Items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, atcmd_output);
					}
					if ((equip = pl_sd->status.inventory[i].equip)) {
						strcpy(equipstr, "| equiped: ");
						if (equip & 4)
							strcat(equipstr, "robe/gargment, ");
						if (equip & 8)
							strcat(equipstr, "left accessory, ");
						if (equip & 16)
							strcat(equipstr, "body/armor, ");
						if ((equip & 34) == 2)
							strcat(equipstr, "right hand, ");
						if ((equip & 34) == 32)
							strcat(equipstr, "left hand, ");
						if ((equip & 34) == 34)
							strcat(equipstr, "both hands, ");
						if (equip & 64)
							strcat(equipstr, "feet, ");
						if (equip & 128)
							strcat(equipstr, "right accessory, ");
						if ((equip & 769) == 1)
							strcat(equipstr, "lower head, ");
						if ((equip & 769) == 256)
							strcat(equipstr, "top head, ");
						if ((equip & 769) == 257)
							strcat(equipstr, "lower/top head, ");
						if ((equip & 769) == 512)
							strcat(equipstr, "mid head, ");
						if ((equip & 769) == 512)
							strcat(equipstr, "lower/mid head, ");
						if ((equip & 769) == 769)
							strcat(equipstr, "lower/mid/top head, ");
						// remove final ', '
						equipstr[strlen(equipstr) - 2] = '\0';
					} else
						memset(equipstr, 0, sizeof(equipstr));
					if (pl_sd->status.inventory[i].refine)
						sprintf(atcmd_output, "%d %s %+d (%s %+d, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, pl_sd->status.inventory[i].refine, item_data->jname, pl_sd->status.inventory[i].refine, pl_sd->status.inventory[i].nameid, equipstr);
					else
						sprintf(atcmd_output, "%d %s (%s, id: %d) %s", pl_sd->status.inventory[i].amount, item_data->name, item_data->jname, pl_sd->status.inventory[i].nameid, equipstr);
					clif_displaymessage(fd, atcmd_output);
					memset(atcmd_output, 0, sizeof(atcmd_output));
					counter2 = 0;
					for (j = 0; j < item_data->slot; j++) {
						if (pl_sd->status.inventory[i].card[j]) {
							if ((item_temp = itemdb_search(pl_sd->status.inventory[i].card[j])) != NULL) {
								if (atcmd_output[0] == '\0')
									sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								else
									sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								strcat(atcmd_output, outputtmp);
							}
						}
					}
					if (atcmd_output[0] != '\0') {
						atcmd_output[strlen(atcmd_output) - 2] = ')';
						atcmd_output[strlen(atcmd_output) - 1] = '\0';
						clif_displaymessage(fd, atcmd_output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found on this player.");
			else {
				sprintf(atcmd_output, "%d item(s) found in %d kind(s) of items.", counter, count);
				clif_displaymessage(fd, atcmd_output);
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charstoragelist <character>: Displays the items list of a player's storage.
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_storage_list) {
	struct storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	char outputtmp[200];

	memset(outputtmp, 0, sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if ((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
				counter = 0;
				count = 0;
				for (i = 0; i < MAX_STORAGE; i++) {
					if (stor->storage[i].nameid > 0 && (item_data = itemdb_search(stor->storage[i].nameid)) != NULL) {
						counter = counter + stor->storage[i].amount;
						count++;
						if (count == 1) {
							sprintf(atcmd_output, "------ Storage items list of '%s' ------", pl_sd->status.name);
							clif_displaymessage(fd, atcmd_output);
						}
						if (stor->storage[i].refine)
							sprintf(atcmd_output, "%d %s %+d (%s %+d, id: %d)", stor->storage[i].amount, item_data->name, stor->storage[i].refine, item_data->jname, stor->storage[i].refine, stor->storage[i].nameid);
						else
							sprintf(atcmd_output, "%d %s (%s, id: %d)", stor->storage[i].amount, item_data->name, item_data->jname, stor->storage[i].nameid);
						clif_displaymessage(fd, atcmd_output);
						memset(atcmd_output, 0, sizeof(atcmd_output));
						counter2 = 0;
						for (j = 0; j < item_data->slot; j++) {
							if (stor->storage[i].card[j]) {
								if ((item_temp = itemdb_search(stor->storage[i].card[j])) != NULL) {
									if (atcmd_output[0] == '\0')
										sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									else
										sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									strcat(atcmd_output, outputtmp);
								}
							}
						}
						if (atcmd_output[0] != '\0') {
							atcmd_output[strlen(atcmd_output) - 2] = ')';
							atcmd_output[strlen(atcmd_output) - 1] = '\0';
							clif_displaymessage(fd, atcmd_output);
						}
					}
				}
				if (count == 0)
					clif_displaymessage(fd, "No item found in the storage of this player.");
				else {
					sprintf(atcmd_output, "%d item(s) found in %d kind(s) of items.", counter, count);
					clif_displaymessage(fd, atcmd_output);
				}
			} else {
				clif_displaymessage(fd, "This player has no storage.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charcartlist <character>: Displays the items list of a player's cart.
 *------------------------------------------
 */
ATCOMMAND_FUNC(character_cart_list) {
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	char outputtmp[200];

	memset(outputtmp, 0, sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_CART; i++) {
				if (pl_sd->status.cart[i].nameid > 0 && (item_data = itemdb_search(pl_sd->status.cart[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.cart[i].amount;
					count++;
					if (count == 1) {
						sprintf(atcmd_output, "------ Cart items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, atcmd_output);
					}
					if (pl_sd->status.cart[i].refine)
						sprintf(atcmd_output, "%d %s %+d (%s %+d, id: %d)", pl_sd->status.cart[i].amount, item_data->name, pl_sd->status.cart[i].refine, item_data->jname, pl_sd->status.cart[i].refine, pl_sd->status.cart[i].nameid);
					else
						sprintf(atcmd_output, "%d %s (%s, id: %d)", pl_sd->status.cart[i].amount, item_data->name, item_data->jname, pl_sd->status.cart[i].nameid);
					clif_displaymessage(fd, atcmd_output);
					memset(atcmd_output, 0, sizeof(atcmd_output));
					counter2 = 0;
					for (j = 0; j < item_data->slot; j++) {
						if (pl_sd->status.cart[i].card[j]) {
							if ((item_temp = itemdb_search(pl_sd->status.cart[i].card[j])) != NULL) {
								if (atcmd_output[0] == '\0')
									sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								else
									sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								strcat(atcmd_output, outputtmp);
							}
						}
					}
					if (atcmd_output[0] != '\0') {
						atcmd_output[strlen(atcmd_output) - 2] = ')';
						atcmd_output[strlen(atcmd_output) - 1] = '\0';
						clif_displaymessage(fd, atcmd_output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found in the cart of this player.");
			else {
				sprintf(atcmd_output, "%d item(s) found in %d kind(s) of items.", counter, count);
				clif_displaymessage(fd, atcmd_output);
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @killer by MouseJstr
 * enable killing players even when not in pvp
 *------------------------------------------
 */
ATCOMMAND_FUNC(killer) {
	sd->special_state.killer = !sd->special_state.killer;

	if (sd->special_state.killer)
		clif_displaymessage(fd, msg_txt(260)); // You're now a killer.
	else
		clif_displaymessage(fd, msg_txt(261)); // You're no longer a killer.

	return 0;
}

/*==========================================
 * @charkiller by Yor
 * enable killing players even when not in pvp
 *------------------------------------------
 */
ATCOMMAND_FUNC(charkiller) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		pl_sd->special_state.killer = !pl_sd->special_state.killer;
		if (pl_sd->special_state.killer)
			clif_displaymessage(fd, msg_txt(262)); // The player is now a killer.
		else
			clif_displaymessage(fd, msg_txt(263)); // The player is no longer a killer.
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @killable by MouseJstr
 * enable other people killing you
 *------------------------------------------
 */
ATCOMMAND_FUNC(killable) {
	sd->special_state.killable = !sd->special_state.killable;

	if (sd->special_state.killable)
		clif_displaymessage(fd, msg_txt(264)); // You're now killable.
	else
		clif_displaymessage(fd, msg_txt(265)); // You're no longer killable.

	return 0;
}

/*==========================================
 * @charkillable by MouseJstr
 * enable another player to be killed
 *------------------------------------------
 */
ATCOMMAND_FUNC(charkillable) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		pl_sd->special_state.killable = !pl_sd->special_state.killable;
		if (pl_sd->special_state.killable)
			clif_displaymessage(fd, msg_txt(266)); // The player is now killable.
		else
			clif_displaymessage(fd, msg_txt(267)); // The player is no longer killable.
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}


/*==========================================
 * @skillon [map] rewritten by [Yor]
 * turn skills on for the map
 *------------------------------------------
 */
ATCOMMAND_FUNC(skillon) {
	int map_id = 0;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	if (map[map_id].flag.noskill == 0) {
		clif_displaymessage(fd, "Map skills are already on.");
		return -1;
	} else {
		map[map_id].flag.noskill = 0;
		clif_displaymessage(fd, msg_txt(244)); // Map skills are on.
	}

	return 0;
}

/*==========================================
 * @skilloff [map] rewritten by [Yor]
 * Turn skills off on the map
 *------------------------------------------
 */
ATCOMMAND_FUNC(skilloff) {
	int map_id = 0;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	if (map[map_id].flag.noskill == 1) {
		clif_displaymessage(fd, "Map skills are already off.");
		return -1;
	} else {
		map[map_id].flag.noskill = 1;
		clif_displaymessage(fd, msg_txt(243)); // Map skills are off.
	}

	return 0;
}

/*==========================================
 * @nospell [map] rewritten by [Yor]
 * nospell flag
 *------------------------------------------
 */
ATCOMMAND_FUNC(nospell) {
	int map_id = 0;

	if (!message || !*message || sscanf(message, "%s", atcmd_mapname) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
			strcat(atcmd_mapname, ".gat");
		if ((map_id = map_mapname2mapid(atcmd_mapname)) < 0) // only from actual map-server
			map_id = sd->bl.m;
	}

	if (!map[map_id].flag.nospell) {
		map[map_id].flag.nospell = 1;
		clif_displaymessage(fd, "Nospell 'On' on the map.");
	} else {
		map[map_id].flag.nospell = 0;
		clif_displaymessage(fd, "Nospell 'Off' on the map.");
	}

	return 0;
}

/*==========================================
 * @npcmove <New_X> <New_Y> <NPC_name> rewritten by [Yor]
 * move a npc
 *------------------------------------------
 */
ATCOMMAND_FUNC(npcmove) {
	int x, y;
	struct npc_data *nd = 0;

	if (!message || !*message || sscanf(message, "%d %d %[^\n]", &x, &y, atcmd_name) < 3) {
		send_usage(fd, "Please, enter a NPC name and its new coordinates (usage: %s <New_X> <New_Y> <NPC_name>).", original_command);
		return -1;
	}

	nd = npc_name2id(atcmd_name);
	if (nd == NULL) {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	if (x < 0 || x >= map[nd->bl.m].xs || y < 0 || y >= map[nd->bl.m].ys) {
		clif_displaymessage(fd, msg_txt(2)); // Coordinates out of range.
		return -1;
	}

	if (map_getcell(nd->bl.m, x, y, CELL_CHKNOPASS)) {
		clif_displaymessage(fd, msg_txt(279)); // Invalid coordinates (can't move on).
		return -1;
	}

	if (map_getcell(nd->bl.m, x, y, CELL_CHKNPC)) {
		clif_displaymessage(fd, msg_txt(280)); // Invalid coordinates (a NPC is already at this position).
		return -1;
	}

	npc_enable(atcmd_name, 0);
	nd->bl.x = x;
	nd->bl.y = y;
	npc_enable(atcmd_name, 1);
	clif_displaymessage(fd, msg_txt(281)); // NPC moved.

	return 0;
}

/*==========================================
 * @addwarp rewritten by [Yor]
 *
 * Create a new static warp point.
 *------------------------------------------
 */
ATCOMMAND_FUNC(addwarp) {
	char w1[100], w3[100], w4[100];
	int x, y;

	if (!message || !*message || sscanf(message, "%s %d %d", atcmd_mapname, &x, &y) < 3) {
		send_usage(fd, "Please, enter a map name with a position (usage: %s <map name> <x_coord> <y_coord>).", original_command);
		return -1;
	}

	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if (map_checkmapname(atcmd_mapname) == -1) { // if map doesn't exist in all map-servers
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	sprintf(w1, "%s,%d,%d", sd->mapname, sd->bl.x, sd->bl.y);
	sprintf(w3, "%s%d%d%d%d", atcmd_mapname, sd->bl.x, sd->bl.y, x, y);
	// fix size of npc name (nor more than 24 char)
	w3[24] = '\0';
	sprintf(w4, "1,1,%s,%d,%d", atcmd_mapname, x, y);

	if (npc_parse_warp(w1, w3, w4, 0)) {
		clif_displaymessage(fd, "Warp can not be created (invalid coordinates, etc.).");
		return -1;
	} else {
		sprintf(atcmd_output, "New warp NPC => %s", w3);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @follow by [MouseJstr]
 * Follow a player (staying no more then 5 spaces away)
 * Improved by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(follow) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id> or %s off).", original_command, original_command);
		return -1;
	}

	if (strcasecmp(atcmd_name, "off") == 0) {
		if (sd->followtimer != -1) {
			delete_timer(sd->followtimer, pc_follow_timer);
			sd->followtimer = -1;
			clif_displaymessage(fd, "Follow OFF.");
		} else {
			clif_displaymessage(fd, "You don't follow anybody actually.");
			return -1;
		}
	} else {
		if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
			if (pl_sd == sd) {
				if (sd->followtimer != -1) {
					delete_timer(sd->followtimer, pc_follow_timer);
					sd->followtimer = -1;
					clif_displaymessage(fd, "Follow OFF.");
				} else {
					clif_displaymessage(fd, "You don't follow anybody actually.");
					return -1;
				}
			} else {
				pc_follow(sd, pl_sd->bl.id);
				send_usage(fd, "To cancel follow GM command, type: %s off.", original_command);
			}
		} else {
			clif_displaymessage(fd, msg_txt(3)); // Character not found.
			return -1;
		}
	}

	return 0;
}

/*==========================================
 * @unfollow by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(unfollow) {
	if (sd->followtimer != -1) {
		delete_timer(sd->followtimer, pc_follow_timer);
		sd->followtimer = -1;
		clif_displaymessage(fd, "Follow OFF.");
	} else {
		clif_displaymessage(fd, "You don't follow anybody actually.");
		return -1;
	}

	return 0;
}

/*==========================================
 * @chareffect by [MouseJstr]
 * Create a effect localized on another character
 *------------------------------------------
 */
ATCOMMAND_FUNC(chareffect) {
	struct map_session_data *pl_sd;
	int type = 0;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &type, atcmd_name) < 2) {
		send_usage(fd, "usage: %s <type+> <char name|account_id>.", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		clif_specialeffect(&pl_sd->bl, type, 0); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
		clif_displaymessage(fd, msg_txt(229)); // Your effect has changed.
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @dropall by [MouseJstr]
 * Drop all your possession on the ground
 *------------------------------------------
 */
ATCOMMAND_FUNC(dropall) {
	int i;

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if (sd->status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			pc_dropitem(sd, i, sd->status.inventory[i].amount);
		}
	}

	return 0;
}

/*==========================================
 * @chardropall by [MouseJstr]
 * Throw all the characters possessions on the ground.  Normally
 * done in response to them being disrespectful of a GM
 *------------------------------------------
 */
ATCOMMAND_FUNC(chardropall) {
	struct map_session_data *pl_sd;
	int i;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		for (i = 0; i < MAX_INVENTORY; i++) {
			if (pl_sd->status.inventory[i].amount) {
				if (pl_sd->status.inventory[i].equip != 0)
					pc_unequipitem(pl_sd, i, 3);
				pc_dropitem(pl_sd, i, pl_sd->status.inventory[i].amount);
			}
		}
		clif_displaymessage(pl_sd->fd, "Look your inventory around of you.");
		clif_displaymessage(fd, "It's done");
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @storeall by [MouseJstr]
 * Put everything into storage to simplify your inventory to make
 * debugging easie
 *------------------------------------------
 */
ATCOMMAND_FUNC(storeall) {
	short i;

	if (storage_storageopen(sd) == 1) {
		clif_displaymessage(fd, "No storage found. Run this command later.");
		return -1;
	}

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if (sd->status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			storage_storageadd(sd, i, sd->status.inventory[i].amount);
		}
	}
	storage_storageclose(sd);

	clif_displaymessage(fd, "It is done");

	return 0;
}

/*==========================================
 * @charstoreall by [MouseJstr]
 *
 * A way to screw with players who piss you off
 *------------------------------------------
 */
ATCOMMAND_FUNC(charstoreall) {
	struct map_session_data *pl_sd;
	short i;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (storage_storageopen(pl_sd) == 1) {
			clif_displaymessage(fd, "Had to open the characters storage window...");
			clif_displaymessage(fd, "Run this command later.");
			return -1;
		}
		for (i = 0; i < MAX_INVENTORY; i++) {
			if (pl_sd->status.inventory[i].amount) {
				if (pl_sd->status.inventory[i].equip != 0)
					pc_unequipitem(pl_sd, i, 3);
				storage_storageadd(pl_sd, i, pl_sd->status.inventory[i].amount);
			}
		}
		storage_storageclose(pl_sd);
		clif_displaymessage(pl_sd->fd, "Everything you own has been put away for safe keeping.");
		clif_displaymessage(pl_sd->fd, "Go to the nearest kafra to retrieve it.");
		clif_displaymessage(pl_sd->fd, "   -- the management");
		clif_displaymessage(fd, "It's done");
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @skillid by [MouseJstr]
 *
 * lookup a skill by name
 *------------------------------------------
 */
ATCOMMAND_FUNC(skillid) {
	int mes_len, idx = 0;

	if (!message || !*message)
		return -1;

	mes_len = strlen(message);
	while (skill_names[idx].id != 0) {
		if (strncasecmp(skill_names[idx].name, message, mes_len) == 0 ||
		    strncasecmp(skill_names[idx].desc, message, mes_len) == 0) {
			sprintf(atcmd_output, "skill %d: %s", skill_names[idx].id, skill_names[idx].desc);
			clif_displaymessage(fd, atcmd_output);
			break;
		}
		idx++;
	}

	if (skill_names[idx].id == 0) {
		clif_displaymessage(fd, "Unknown skill");
		return -1;
	}

	return 0;
}

/*==========================================
 * @useskill by [MouseJstr]
 *
 * A way of using skills without having to find them in the skills menu
 *------------------------------------------
 */
ATCOMMAND_FUNC(useskill) {
	struct map_session_data *pl_sd;
	int skillnum;
	int skilllv;
	int inf;

	if (!message || !*message || sscanf(message, "%d %d %[^\n]", &skillnum, &skilllv, atcmd_name) < 3) {
		send_usage(fd, "Usage: %s <skillnum> <skillv> <char name|account_id>", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		inf = skill_get_inf(skillnum);
		if (inf == 2 || inf == 32) // 2- place, 32- trap
			skill_use_pos(sd, pl_sd->bl.x, pl_sd->bl.y, skillnum, skilllv);
		else
			skill_use_id(sd, pl_sd->bl.id, skillnum, skilllv);
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @skilltree by [MouseJstr]
 * prints the skill tree for a player required to get to a skill
 *------------------------------------------
 */
ATCOMMAND_FUNC(skilltree) {
	struct map_session_data *pl_sd;
	int skillnum, skillidx;
	int meets, i, c, s;
	struct pc_base_job s_class;
	struct skill_tree_entry *ent;

	if (!message || !*message || sscanf(message, "%d %[^\r\n]", &skillnum, atcmd_name) < 2) {
		send_usage(fd, "Usage: %s <skillnum:1+> <char name|account_id>", original_command);
		return -1;
	}

	if (skillnum >= 10000 && skillnum < 10015) // guild skill????
		skillnum -= 9500;
	if (skillnum <= 0 || skillnum > MAX_SKILL) {
		clif_displaymessage(fd, "Unknown skill number.");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		s_class = pc_calc_base_job(pl_sd->status.class);
		c = s_class.job;
		s = s_class.upper;

		c = pc_calc_skilltree_normalize_job(c, s, pl_sd);

		sprintf(atcmd_output, "Player is using %s %s skill tree (%d basic points)", (s_class.upper == 2) ? "baby" : ((s_class.upper) ? "upper" : "lower"), job_name(c), pc_checkskill(pl_sd, 1));
		clif_displaymessage(fd, atcmd_output);

		i = 0;
		for (skillidx = 0; skillidx < MAX_SKILL_TREE && skill_tree[s][c][skillidx].id > 0; skillidx++)
			if (skill_tree[s][c][skillidx].id == skillnum) {
				i = 1;
				break;
			}

		if (i == 0) {
			clif_displaymessage(fd, "I do not believe the player can use that skill.");
			return 0;
		}

		ent = &skill_tree[s][c][skillidx];

		meets = 1;
		for(i = 0; i < 5; i++)
			if (ent->need[i].id && pc_checkskill(sd, ent->need[i].id) < ent->need[i].lv) {
				int idx = 0;
				char *desc;
				while (skill_names[idx].id != 0 && skill_names[idx].id != ent->need[i].id)
					idx++;
				if (skill_names[idx].id == 0)
					desc = "Unknown skill";
				else
					desc = skill_names[idx].desc;
				sprintf(atcmd_output, "player requires level %d of skill %s", ent->need[i].lv, desc);
				clif_displaymessage(fd, atcmd_output);
				meets = 0;
			}

		if (meets == 1) {
			clif_displaymessage(fd, "I believe the player meets all the requirements for that skill.");
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @marry
 * Marry two players
 *------------------------------------------
 */
ATCOMMAND_FUNC(marry) {
	struct map_session_data *pl_sd1;
	struct map_session_data *pl_sd2;
	char player1[100], player2[100];

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\",\"%[^\"]\"", player1, player2) < 2 &&
	     sscanf(message, "%[^,],\"%[^\"]\"", player1, player2) < 2 &&
	     sscanf(message, "\"%[^\"]\",%[^\r\n]", player1, player2) < 2 &&
	     sscanf(message, "%[^,],%[^\r\n]", player1, player2) < 2)) {
		send_usage(fd, "usage: %s \"<player1>\",\"<player2>\" or %s <player1>,<player2>.", original_command, original_command);
		return -1;
	}

	if((pl_sd1 = map_nick2sd(player1)) == NULL && ((pl_sd1 = map_id2sd(atoi(player1))) == NULL || !pl_sd1->state.auth)) {
		sprintf(atcmd_output, "Cannot find player %s online.", player1);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if((pl_sd2 = map_nick2sd(player2)) == NULL && ((pl_sd2 = map_id2sd(atoi(player2))) == NULL || !pl_sd2->state.auth)) {
		sprintf(atcmd_output, "Cannot find player %s online.", player2);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (pl_sd1->status.partner_id > 0 || pl_sd2->status.partner_id > 0) {
		clif_displaymessage(fd, "This marriage is impossible. One of these characters is already married!");
		return -1;
	}

	if (pc_marriage(pl_sd1, pl_sd2)) {
		clif_displaymessage(fd, "The marriage has failed. Class of the character is incorrect!");
		return -1;
	}

	clif_displaymessage(fd, "Marriage done. Congratulations to the newly wed.");

	return 0;
}

/*==========================================
 * @divorce
 * divorce two players
 *------------------------------------------
 */
ATCOMMAND_FUNC(divorce) {
	struct map_session_data *pl_sd1;
	struct map_session_data *pl_sd2;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd1 = map_nick2sd(atcmd_name)) != NULL || ((pl_sd1 = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd1->state.auth)) {
		if (pl_sd1->status.partner_id == 0) {
			clif_displaymessage(fd, "This player is not married.");
			return -1;
		}
		if ((pl_sd2 = map_nick2sd(map_charid2nick(pl_sd1->status.partner_id))) != NULL) {
			if (pc_divorce(pl_sd1)) {
				clif_displaymessage(fd, "The divorce has failed. Probably incorrect partner (bug?).");
				return -1;
			}
			clif_displaymessage(fd, "Divorce done.");
		} else {
			clif_displaymessage(fd, "The partner of this player is not online.");
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @rings
 * Give two players rings
 *------------------------------------------
 */
ATCOMMAND_FUNC(rings) {
	struct item item_tmp;
	int flag;

	memset(&item_tmp, 0, sizeof(struct item));

	item_tmp.nameid = 2634;
	item_tmp.amount = 1;
	item_tmp.identify = 1;
	if ((flag = pc_additem(sd, &item_tmp, 1))) {
		clif_additem(sd, 0, 0, flag);
		clif_displaymessage(fd, "You can not receive the Wedding Ring of Mister (id:2634) (overcharged?).");
		return -1;
	}

	memset(&item_tmp, 0, sizeof(struct item));
	item_tmp.nameid = 2635;
	item_tmp.amount = 1;
	item_tmp.identify = 1;
	if ((flag = pc_additem(sd, &item_tmp, 1))) {
		clif_additem(sd, 0, 0, flag);
		clif_displaymessage(fd, "You can not receive the Wedding Ring of Miss (id:2635) (overcharged?).");
		return -1;
	}

	// message is not necessary, because display is done when ring arrive in the inventory
	clif_displaymessage(fd, "You have rings! Give them to the lovers.");

	return 0;
}

/*==========================================
 * @grind (based on work of [MouseJstr]) (on test GM command)
 *------------------------------------------
 */
ATCOMMAND_FUNC(grind) {
	struct map_session_data *pl_sd;
	int skillnum;
	int inf;
	int hp, sp;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (Usage: %s <target>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		// save actual hp/sp
		hp = sd->status.hp;
		sp = sd->status.sp;
		for (skillnum = 1; skillnum < MAX_SKILL_DB; skillnum++) {
			if ((inf = skill_get_inf(skillnum)) != 0) { // 0: passiv skill, so no display
				if (pc_isdead(sd))
					atcommand_alive(fd, sd, original_command, "@alive", "");
				sd->status.hp = sd->status.max_hp;
				sd->status.sp = sd->status.max_sp;
				if (inf == 2 || inf == 32) // 2- place, 32- trap
					skill_use_pos(sd, pl_sd->bl.x + 5, pl_sd->bl.y + 5, skillnum, 1);
				else
					skill_use_id(sd, pl_sd->bl.id, skillnum, 1);
			}
		}
		// if target doesn't have play_dead, we must remove the skill because otherwise he is blocked
		if (pl_sd->sc_data[SC_TRICKDEAD].timer != -1)
			status_change_end(&pl_sd->bl, SC_TRICKDEAD, -1);
		// restore previous values of hp/sp
		hp = sd->status.hp;
		sp = sd->status.sp;
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		if (pc_isdead(sd))
			atcommand_alive(fd, sd, original_command, "@alive", "");
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @grind2: Spawn one of each monster's type around of the target.
 *------------------------------------------
 */
ATCOMMAND_FUNC(grind2) {
	struct map_session_data *pl_sd;
	int i, j, c, x, y, count;
	struct block_list *bl;
	int b, mob_num, slave_num;
	int *id;
	struct mob_db *mob;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1)
		strncpy(atcmd_name, sd->status.name, 24);

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		count = 0;
		CALLOC(id, int, 100);
		while (count < pl_sd->status.base_level * 2 && count < 100) {
			i = (rand() % (MAX_MOB_DB - 1000)) + 1001;
			// remove some mobs
			if (i == 1181 || // zombie dragon - sprite error
			    i == 1223 || // L_ORC - sprite error
			    i == 1284 || // Hugeling - sprite error
			    i == 1288 || // Emperium - never spawn an emperium
			    (i >= 1324 && i <= 1363)) // Treasure Chest - never spawn a Treasure Chest
				continue;
			// don't spawn same monster twice
			for (j = 0; j < count; j++)
				if (i == id[j])
					continue;
			if (mobdb_checkid(i)) { // if monster exists
				mob = &mob_db[i];
				// don't spawn monster with no HP
				if (mob->max_hp <= 0)
					continue;
				// don't spawn too much MVP
				if (mob->mexp && (rand() % 100) < 90)
					continue;
				// don't spawn too big monster
				if (mob->lv > pl_sd->status.base_level * 2)
					continue;
				if (mob->lv > pl_sd->status.base_level + 1 && (rand() % 100) < 90)
					continue;
				// spawn monster now
				id[count] = i;
				j = 0;
				do {
					x = pl_sd->bl.x + (rand() % 10 - 5);
					y = pl_sd->bl.y + (rand() % 10 - 5);
				} while ((c = map_getcell(pl_sd->bl.m, x, y, CELL_CHKNOPASS)) && j++ < 64);
				if (!c) {
					if (mob_once_spawn(pl_sd, "this", x, y, "--ja--", i, 1, "")) {
//						printf("id %d at (%d, %d)\n", i, x, y);
						count++;
					}
				}
			}
		}
		FREE(id);
		sprintf(atcmd_output, msg_txt(240), count); // %d monster(s) summoned!
		clif_displaymessage(fd, atcmd_output);
		slave_num = 0;
		mob_num = 0;
		for (b = 0; b < map[pl_sd->bl.m].bxs * map[pl_sd->bl.m].bys; b++)
			for (bl = map[pl_sd->bl.m].block_mob[b]; bl; bl = bl->next) {
				mob_num++;
				if (((struct mob_data *)bl)->master_id)
					slave_num++;
			}
		if (slave_num == 0)
			sprintf(atcmd_output, "Total mobs in target's map: %d (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "Total mobs in target's map: %d (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * It is made to rain.
 *------------------------------------------
 */
ATCOMMAND_FUNC(rain) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.rain) {
		map[sd->bl.m].flag.rain = 0;
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
				clif_displaymessage(pl_sd->fd, "The rain has stopped. If you want remove weather from your screen, warp you, change map, etc.");
	} else {
		map[sd->bl.m].flag.rain = 1;
		clif_specialeffect(&sd->bl, 161, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	}

	return 0;
}

/*==========================================
 * It is made to snow.
 *------------------------------------------
 */
ATCOMMAND_FUNC(snow) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.snow) {
		map[sd->bl.m].flag.snow = 0;
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
				clif_displaymessage(pl_sd->fd, "Snow has stopped. If you want remove weather from your screen, warp you, change map, etc.");
	} else {
		map[sd->bl.m].flag.snow = 1;
		clif_specialeffect(&sd->bl, 162, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	}

	return 0;
}

/*==========================================
 * Cherry tree snowstorm is made to fall. (Sakura)
 *------------------------------------------
 */
ATCOMMAND_FUNC(sakura) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.sakura) {
		map[sd->bl.m].flag.sakura = 0;
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
				clif_displaymessage(pl_sd->fd, "Cherry tree leaves has stopped. If you want remove weather from your screen, warp you, change map, etc.");
	} else {
		map[sd->bl.m].flag.sakura = 1;
		clif_specialeffect(&sd->bl, 163, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	}

	return 0;
}

/*==========================================
 * Fog hangs over.
 *------------------------------------------
 */
ATCOMMAND_FUNC(fog) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.fog) {
		map[sd->bl.m].flag.fog = 0;
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
				clif_displaymessage(pl_sd->fd, "The fog has stopped. If you want remove weather from your screen, warp you, change map, etc.");
	} else {
		map[sd->bl.m].flag.fog = 1;
		clif_specialeffect(&sd->bl, 233, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	}

	return 0;
}

/*==========================================
 * Fallen leaves fall.
 *------------------------------------------
 */
ATCOMMAND_FUNC(leaves) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.leaves) {
		map[sd->bl.m].flag.leaves = 0;
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth)
				clif_displaymessage(pl_sd->fd, "Leaves no longer fall. If you want remove weather from your screen, warp you, change map, etc.");
	} else {
		map[sd->bl.m].flag.leaves = 1;
		clif_specialeffect(&sd->bl, 333, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
	}

	return 0;
}

/*==========================================
 * Adds a rainbow when it's rain.
 *------------------------------------------
 */
ATCOMMAND_FUNC(rainbow) {
	if (!map[sd->bl.m].flag.rain) {
		clif_displaymessage(fd, "It doesn't rain. How a rainbow can be visible?");
		return -1;
	}

	clif_specialeffect(&sd->bl, 410, 2); // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)

	return 0;
}

/*==========================================
 * Clear all special weathers on a map.
 *------------------------------------------
 */
ATCOMMAND_FUNC(clsweather) {
	struct map_session_data *pl_sd;
	int i;

	if (map[sd->bl.m].flag.snow || map[sd->bl.m].flag.sakura || map[sd->bl.m].flag.leaves || map[sd->bl.m].flag.fog || map[sd->bl.m].flag.rain) {
		if (map[sd->bl.m].flag.rain)
			clif_specialeffect(&sd->bl, 410, 2); // add a rainbow // flag: 0: player see in the area (normal), 1: only player see only by player, 2: all players in a map that see only their (not see others), 3: all players that see only their (not see others)
		map[sd->bl.m].flag.snow = 0;
		map[sd->bl.m].flag.sakura = 0;
		map[sd->bl.m].flag.leaves = 0;
		map[sd->bl.m].flag.fog = 0;
		map[sd->bl.m].flag.rain = 0;
		clif_displaymessage(fd, "All special weathers are now removed from your map.");
		for (i = 0; i < fd_max; i++)
			if (session[i] && (pl_sd = session[i]->session_data) && pl_sd->state.auth) {
				clif_displaymessage(pl_sd->fd, "Special weather of the map has been removed.");
				clif_displaymessage(pl_sd->fd, "If you want remove weather from your screen, warp you, change map, etc.");
			}
	} else {
		clif_displaymessage(fd, "There is not special weather on your map.");
		return -1;
	}

	return 0;
}

/*===============================================================
 * Sound Command - plays a sound for everyone! [Yor] (based on poor code of codemaster)
 *---------------------------------------------------------------
 */
ATCOMMAND_FUNC(sound) {
	char sound_file[100];
	char sound_file2[150];

	memset(sound_file, 0, sizeof(sound_file));
	memset(sound_file2, 0, sizeof(sound_file2));

	if(!message || !*message || sscanf(message, "%s", sound_file) < 1) {
		send_usage(fd, "Please, enter a sound filename. (usage: %s <filename>).", original_command);
		return -1;
	}

	if (strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");

	if (strlen(sound_file) > 24) {
		sprintf(atcmd_output, "File name must not have more than 24 characters ('%s' has %d char).", sound_file, (int)strlen(sound_file));
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	sprintf(sound_file2, "data\\wav\\%s", sound_file);
	if (filelist_find(sound_file2) == NULL) {
		sprintf(sound_file2, "data\\wav\\effect\\%s", sound_file);
		if (strlen(sound_file) <= (24 - 7) && filelist_find(sound_file2) != NULL) { // 7 = effect-
			strncpy(sound_file, sound_file2 + 9, 24); // 9 = data-wav-
		} else {
			sprintf(atcmd_output, "'%s' is an unknown wav file.", sound_file);
			clif_displaymessage(fd, atcmd_output);
			return -1;
		}
	}

	clif_soundeffectall(&sd->bl, sound_file, 0);

	return 0;
}

/*==========================================
 * MOB Search
 *------------------------------------------
 */
ATCOMMAND_FUNC(mobsearch) {
	char monster[100];
	int mob_id;
	struct mob_data *md;
	struct block_list *bl;
	int b, mob_num, slave_num;

	memset(monster, 0, sizeof(monster));

	mob_id = -1;
	if (sscanf(message, "%s", monster) == 1) {
		// If monster identifier/name argument is a name
		if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
			if ((mob_id = mobdb_checkid(atoi(monster))) == 0) {
				if (atoi(monster) != -1) {
					clif_displaymessage(fd, msg_txt(40)); // Invalid monster ID or name.
					return -1;
				} else
					mob_id = -1;
			}
	}

	if (mob_id == -1)
		sprintf(atcmd_output, "Mob Search: all monsters on %s", sd->mapname);
	else
		sprintf(atcmd_output, "Mob Search: '%s' on %s", mob_db[mob_id].jname, sd->mapname);
	clif_displaymessage(fd, atcmd_output);

	slave_num = 0;
	mob_num = 0;
	for (b = 0; b < map[sd->bl.m].bxs * map[sd->bl.m].bys; b++)
		for (bl = map[sd->bl.m].block_mob[b]; bl; bl = bl->next) {
			md = (struct mob_data *)bl;
			if (md && md->name[0] && (mob_id == -1 || md->class == mob_id)) {
				if (mob_db[md->class].max_hp <= 0)
					sprintf(atcmd_output, "'%s' (class: %d-%s%s) at %d,%d", md->name, md->class, mob_db[md->class].jname, (md->master_id > 0) ? ", slave)" : "", bl->x, bl->y);
				else if (md->hp <= 0)
					sprintf(atcmd_output, "'%s' (class: %d-%s%s) at %d,%d [dead]", md->name, md->class, mob_db[md->class].jname, (md->master_id > 0) ? ", slave)" : "", bl->x, bl->y);
				else
					sprintf(atcmd_output, "'%s' (class: %d-%s%s) at %d,%d - hp: %d/%d", md->name, md->class, mob_db[md->class].jname, (md->master_id > 0) ? ", slave)" : "", bl->x, bl->y, md->hp, mob_db[md->class].max_hp);
				clif_displaymessage(fd, atcmd_output);
				mob_num++;
				if (md->master_id)
					slave_num++;
			}
		}
	if (mob_num == 0) {
		clif_displaymessage(fd, "No such mob found.");
	} else if (mob_num == 1) {
		sprintf(atcmd_output, "%d mob found.", mob_num);
		clif_displaymessage(fd, atcmd_output);
	} else {
		if (slave_num == 0)
			sprintf(atcmd_output, "%d mobs found (of which is no slave).", mob_num);
		else
			sprintf(atcmd_output, "%d mobs found (of which are %d slaves).", mob_num, slave_num);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * cleanmap
 *------------------------------------------
 */
static int atcommand_cleanmap_sub(struct block_list *bl, va_list ap) {
	nullpo_retr(0, bl);

	map_clearflooritem(bl->id);

	return 0;
}

ATCOMMAND_FUNC(cleanmap) {
	int area_size;

	area_size = AREA_SIZE * 2;
	sscanf(message, "%d", &area_size);
	if (area_size < 1)
		area_size = 1;

	map_foreachinarea(atcommand_cleanmap_sub, sd->bl.m,
	                  sd->bl.x - area_size, sd->bl.y - area_size,
	                  sd->bl.x + area_size, sd->bl.y + area_size,
	                  BL_ITEM);
	clif_displaymessage(fd, "All dropped items have been cleaned up.");

	return 0;
}

/*==========================================
 * @adjcmdlvl/@setcmdlvl
 *
 * Temp adjust the GM level required to use a GM command
 *
 * Used during beta testing to allow players to use GM commands
 * for short periods of time
 *------------------------------------------
 */
ATCOMMAND_FUNC(adjcmdlvl) {
	int i, newlev;
	char cmd[100];

	if (!message || !*message || sscanf(message, "%d %s", &newlev, cmd) != 2) {
		send_usage(fd, "usage: %s <lvl> <command>.", original_command);
		return -1;
	}

	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
		if (strcasecmp(cmd, atcommand_info[i].command + 1) == 0) {
			atcommand_info[i].level = newlev;
			sprintf(atcmd_output, "GM command '%s' level changed.", cmd);
			clif_displaymessage(fd, atcmd_output);
			return 0;
		}

	sprintf(atcmd_output, "GM command '%s' not found.", cmd);
	clif_displaymessage(fd, atcmd_output);

	return -1;
}

/*==========================================
 * @adjgmlvl/@setgmlvl
 *
 * Create a temp GM
 *
 * Used during beta testing to allow players to use GM commands
 * for short periods of time
 *------------------------------------------
 */
ATCOMMAND_FUNC(adjgmlvl) {
	int newlev;
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %[^\r\n]", &newlev, atcmd_name) != 2 || newlev < 0 || newlev > 99) {
		send_usage(fd, "usage: %s <lvl:0-99> <player>.", original_command);
		return -1;
	}

	if (newlev > sd->GM_level) // can not give upper GM level than its level
		newlev = sd->GM_level;

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level > pl_sd->GM_level || sd == pl_sd) { // only lower GM level (you can change TEMPORARILY you own GM level for some tests)
			if (pl_sd->GM_level != newlev) {
				sprintf(atcmd_output, "GM level of the player temporarily changed from %d to %d.", pl_sd->GM_level, newlev);
				clif_displaymessage(fd, atcmd_output);
				pc_set_gm_level(pl_sd->status.account_id, (signed char)newlev);
			} else {
				clif_displaymessage(fd, "This player already has this GM level.");
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @adjgmlvl2/@setgmlvl2
 *
 * Permanently modify a GM level of a player!
 *------------------------------------------
 */
ATCOMMAND_FUNC(adjgmlvl2) {
	int newlev;
	struct map_session_data *pl_sd;

	if (command[6] >= '0' && command[6] <= '9') {
		newlev = (int)(command[6] - '0');
		if (!message || !*message || sscanf(message, "%[^\r\n]", atcmd_name) != 1) {
			send_usage(fd, "usage: %s <player>.", original_command);
			return -1;
		}
	} else {
		if (!message || !*message || sscanf(message, "%d %[^\r\n]", &newlev, atcmd_name) != 2 || newlev < 0 || newlev > 99) {
			send_usage(fd, "usage: %s <lvl:0-99> <player>.", original_command);
			return -1;
		}
	}

	if (newlev > sd->GM_level) // can not give upper GM level than its level
		newlev = sd->GM_level;

	// try to foun dplayer online to check immediatly
	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd != pl_sd) { // you are not authorised to change your own GM level PERMANENTLY
			if (sd->GM_level > pl_sd->GM_level) { // only lower GM level
				if (pl_sd->GM_level == newlev) {
					clif_displaymessage(fd, "This player already has this GM level.");
					return -1;
				}
				// we can try to modify
			} else {
				clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
				return -1;
			}
		} else {
			clif_displaymessage(fd, "You can not change your own GM level permanently.");
			return -1;
		}
	} // else if not found, we can try to modify

	// if not stopped before, try to modify
	chrif_char_ask_name(sd->status.account_id, atcmd_name, 6, newlev, 0, 0, 0, 0, 0); // type: 6 - Change GM level
	clif_displaymessage(fd, msg_txt(88)); // Character name sends to char-server to ask it.

	return 0;
}

/*==========================================
 * @trade by [MouseJstr]
 *
 * Open a trade window with a remote player
 *
 * If I have to jump to a remote player one more time, I am gonna scream!
 *------------------------------------------
 */
ATCOMMAND_FUNC(trade) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (Usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		trade_traderequest(sd, pl_sd->bl.id, 0); // 1: check if near the other player, 0: doesnt check
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @setbattleflag/@adjbattleflag/@setbattleconf/@adjbattleconf by [MouseJstr]
 *
 * set a battle_config flag without having to reboot
 *------------------------------------------
 */
ATCOMMAND_FUNC(setbattleflag) {
	char flag[100], value[100];

	if (!message || !*message || sscanf(message, "%s %s", flag, value) < 2) {
		send_usage(fd, "usage: %s <flag> <value>.", original_command);
		return -1;
	}

	if (battle_set_value(flag, value) == 0)
		clif_displaymessage(fd, "unknown battle_config flag.");
	else
		clif_displaymessage(fd, "battle_config set as requested.");

	return 0;
}

/*==========================================
 * @setmapflag/@adjmapflag
 *
 * set a map flag without having to reboot
 *------------------------------------------
 */
ATCOMMAND_FUNC(setmapflag) {
	char mapflag[100], mapflaglower[100], option[100];
	int i, map_id;

	memset(option, 0, sizeof(option));
	memset(mapflaglower, 0, sizeof(mapflaglower));

	if (!message || !*message || sscanf(message, "%s %s %[^\n]", atcmd_mapname, mapflag, option) < 2) {
		send_usage(fd, "usage: %s <map> <mapflag> [option|value].", original_command);
		return -1;
	}

	if (strstr(atcmd_mapname, ".gat") == NULL && strstr(atcmd_mapname, ".afm") == NULL && strlen(atcmd_mapname) < 13) // 16 - 4 (.gat)
		strcat(atcmd_mapname, ".gat");

	if ((map_id = map_mapname2mapid(atcmd_mapname)) >= 0) { // only from actual map-server
		for(i = 0; mapflag[i]; i++)
			mapflaglower[i] = tolower((unsigned char)mapflag[i]); // tolower needs unsigned char
		/* use specific GM command if it exist */
		if (strcmp(mapflaglower, "pvp") == 0 && map_id == sd->bl.m)
			atcommand_pvpon(fd, sd, original_command, "@pvpon", "");
		else if (strcmp(mapflaglower, "gvg") == 0 && map_id == sd->bl.m)
			atcommand_gvgon(fd, sd, original_command, "@gvgon", "");
		else if (strcmp(mapflaglower, "noskill") == 0 && map_id == sd->bl.m)
			atcommand_skilloff(fd, sd, original_command, "@skilloff", "");
		else if (strcmp(mapflaglower, "snow") == 0 && map_id == sd->bl.m && !map[map_id].flag.snow)
			atcommand_snow(fd, sd, original_command, "@snow", "");
		else if (strcmp(mapflaglower, "fog") == 0 && map_id == sd->bl.m && !map[map_id].flag.fog)
			atcommand_fog(fd, sd, original_command, "@fog", "");
		else if (strcmp(mapflaglower, "sakura") == 0 && map_id == sd->bl.m && !map[map_id].flag.sakura)
			atcommand_sakura(fd, sd, original_command, "@sakura", "");
		else if (strcmp(mapflaglower, "leaves") == 0 && map_id == sd->bl.m && !map[map_id].flag.leaves)
			atcommand_leaves(fd, sd, original_command, "@leaves", "");
		else if (strcmp(mapflaglower, "rain") == 0 && map_id == sd->bl.m && !map[map_id].flag.rain)
			atcommand_rain(fd, sd, original_command, "@rain", "");
		else if (strcmp(mapflaglower, "nospell") == 0 && map_id == sd->bl.m && !map[map_id].flag.nospell)
			atcommand_nospell(fd, sd, original_command, "@nospell", "");
		/* general option */
		else if (npc_parse_mapflag(atcmd_mapname, mapflag, option, 0) == 1 && npc_parse_mapflag(atcmd_mapname, mapflaglower, option, 0) == 1) {
			clif_displaymessage(fd, "unknown map flag.");
			return -1;
		} else
			clif_displaymessage(fd, "map flag set as requested.");
	} else {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @uptime
 *------------------------------------------
 */
ATCOMMAND_FUNC(uptime) {
	sprintf(atcmd_output, msg_txt(245), txt_time(time(NULL) - start_time)); // Server Uptime: %s.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @clock
 *------------------------------------------
 */
ATCOMMAND_FUNC(clock) {
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...

	time(&time_server); // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	memset(atcmd_output, 0, sizeof(atcmd_output));
	strftime(atcmd_output, sizeof(atcmd_output) - 1, msg_txt(230), datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, msg_txt(245), txt_time(time(NULL) - start_time)); // Server Uptime: %s.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*===========================
 * @unmute [Valaris]
 *---------------------------
*/
ATCOMMAND_FUNC(unmute) {
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name|account_id>).", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			if (pl_sd->sc_data[SC_NOCHAT].timer != -1) {
				pl_sd->status.manner = 0; // have to set to 0 first [celest]
				status_change_end(&pl_sd->bl, SC_NOCHAT, -1);
				clif_displaymessage(sd->fd, "Player unmuted.");
			} else
				clif_displaymessage(sd->fd, "Player is not muted.");
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*================================================
 * @mute - Mutes a player for a set amount of time
 *------------------------------------------------
 */
ATCOMMAND_FUNC(mute) {
	struct map_session_data *pl_sd;
	int manner;

	if (!message || !*message || sscanf(message, "%d %[^\n]", &manner, atcmd_name) < 2) {
		send_usage(fd, "usage: %s <time> <character name>.", original_command);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_name)) != NULL || ((pl_sd = map_id2sd(atoi(atcmd_name))) != NULL && pl_sd->state.auth)) {
		if (sd->GM_level >= pl_sd->GM_level) { // only lower or same level
			pl_sd->status.manner -= manner;
			if (pl_sd->status.manner < 0)
				status_change_start(&pl_sd->bl, SC_NOCHAT, 0, 0, 0, 0, 0, 0);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorize you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @refresh (like @jumpto <<yourself>>)
 *------------------------------------------
 */
ATCOMMAND_FUNC(refresh) {
	pc_setpos(sd, sd->mapname, sd->bl.x, sd->bl.y, 3);

	return 0;
}

/*==========================================
 * @petid <part of pet name>
 * => Displays a list of matching pets.
 *------------------------------------------
 */
ATCOMMAND_FUNC(petid) {
	char searchtext[100];
	char temp0[100];
	char temp1[100];
	int cnt, i, j;

	if (!message || !*message || sscanf(message, "%s", searchtext) < 1) {
		send_usage(fd, "usage: %s <part_of_pet_name>.", original_command);
		return -1;
	}

	for (j = 0; searchtext[j]; j++)
		searchtext[j] = tolower((unsigned char)searchtext[j]); // tolower needs unsigned char
	sprintf(atcmd_output, "Search results for: '%s'.", searchtext);
	clif_displaymessage(fd, atcmd_output);

	cnt = 0;
	i = 0;
	while (i < MAX_PET_DB) {
		for (j = 0; pet_db[i].name[j]; j++)
			temp1[j] = tolower((unsigned char)pet_db[i].name[j]); // tolower needs unsigned char
		temp1[j] = 0;
		for (j = 0; pet_db[i].jname[j]; j++)
			temp0[j] = tolower((unsigned char)pet_db[i].jname[j]); // tolower needs unsigned char
		temp0[j] = 0;
		if (strstr(temp1, searchtext) || strstr(temp0, searchtext)) {
			sprintf(atcmd_output, "ID: %i -- Name: %s", pet_db[i].class, pet_db[i].jname);
			clif_displaymessage(fd, atcmd_output);
			cnt++;
		}
		i++;
	}

	sprintf(atcmd_output, "%i pets have '%s' in their name.", cnt, searchtext);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @identify
 * => GM's magnifier.
 *------------------------------------------
 */
ATCOMMAND_FUNC(identify) {
	int i, num;

	num = 0;
	for(i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify != 1)
			num++;
	}
	if (num > 0) {
		clif_item_identify_list(sd);
	} else {
		clif_displaymessage(fd, "There are no items to appraise.");
	}

	return 0;
}

/*==========================================
 * @motd (displaye again the MOTD) by [Yor]
 *------------------------------------------
 */
ATCOMMAND_FUNC(motd) {
	char buf[256];
	FILE *fp;
	int i;

	if ((fp = fopen(motd_txt, "r")) != NULL) {
		while (fgets(buf, sizeof(buf), fp) != NULL) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for(i = 0; buf[i]; i++) {
				if (buf[i] == '\r' || buf[i]== '\n') {
					buf[i] = 0;
					break;
				}
			}
			if (battle_config.motd_type)
				clif_disp_onlyself(sd, buf);
			else
				clif_displaymessage(sd->fd, buf);
		}
		fclose(fp);
	} else
		clif_displaymessage(fd, "Modt.txt file not found.");

	return 0;
}

/*==========================================
 * @gmotd (Global MOTD)
 * by davidsiaw :P
 *------------------------------------------
 */
ATCOMMAND_FUNC(gmotd) {
	char buf[256];
	FILE *fp;
	int i;

	if ((fp = fopen(motd_txt, "r")) != NULL) {
		intif_GMmessage(msg_txt(246), 0x10); // Message of the day:
		while (fgets(buf, sizeof(buf), fp) != NULL) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for(i = 0; buf[i]; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = 0;
					break;
				}
			}
			intif_GMmessage(buf, 0x10);
		}
		fclose(fp);
	} else
		clif_displaymessage(fd, "Modt.txt file not found.");

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(misceffect) {
	int effect;

	if (!message || !*message || sscanf(message, "%d", &effect) < 1)
		return -1;

	clif_misceffect(&sd->bl, effect);

	return 0;
}

/*==========================================
 * NPC/PET-related atcommands
 *------------------------------------------
 */
ATCOMMAND_FUNC(npctalk) {
	char npc_name[100], mes[100];
	struct npc_data *nd;

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %[^\n]", npc_name, mes) < 2 &&
	     sscanf(message, "%s %[^\n]", npc_name, mes) < 2)) {
		send_usage(fd, "Please, enter a NPC name and a message (usage: %s <npc_name> <message>).", original_command);
		return -1;
	}

	if (check_bad_word(mes, strlen(mes), sd))
		return -1; // check_bad_word function display message if necessary

	if ((nd = npc_name2id(npc_name)) != NULL) {
		clif_message(&nd->bl, mes);
		clif_displaymessage(fd, "Npc message displayed."); // display an answer, because npc can be not near the player
	} else {
		clif_displaymessage(fd, msg_txt(111)); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 * NPC/PET-related atcommands
 *------------------------------------------
 */
ATCOMMAND_FUNC(pettalk) {
	char mes[100];
	struct pet_data *pd;

	if (!message || !*message || sscanf(message, "%[^\n]", mes) < 1) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (!sd->status.pet_id || !(pd = sd->pd)) {
		clif_displaymessage(fd, "You have no pet.");
		return -1;
	}

	if (check_bad_word(mes, strlen(mes), sd))
		return -1; // check_bad_word function display message if necessary

	sprintf(atcmd_output, "%s : %s", sd->pet.name, mes);
	clif_message(&pd->bl, atcmd_output);

	return 0;
}

/*==========================================
* @autoloot % - [Yor]
*------------------------------------------
*/
ATCOMMAND_FUNC(autoloot) {
	char message2[100];
	double drop_rate, drop_rate2;
	int i, result_sscanf;

	memset(message2, 0, sizeof(message2));

	result_sscanf = 0;
	if (message && *message) {
		i = 0;
		while(message[i]) {
			if (message[i] == ',')
				message2[i] = '.';
			else
				message2[i] = message[i];
			i++;
		}
		result_sscanf = sscanf(message2, "%lf", &drop_rate2);
	}

	if (!message || !*message || sscanf(message, "%lf", &drop_rate) < 1) {
		if (result_sscanf != 0) {
			drop_rate = drop_rate2;
		} else {
			send_usage(fd, "Please, enter a max drop rate (usage: %s <max_drop_rate>).", original_command);
			if (sd->state.autoloot_rate == 0)
				clif_displaymessage(fd, msg_txt(270)); // Your current autoloot option is disabled.
			else {
				sprintf(atcmd_output, msg_txt(271), ((float)sd->state.autoloot_rate) / 100.); // Your current autoloot option is set to get rate between 0 to %0.02f%%.
				clif_displaymessage(fd, atcmd_output);
			}
			if (sd->state.autolootloot_flag)
				clif_displaymessage(fd, msg_txt(274)); // And you get drops of looted items.
			else
				clif_displaymessage(fd, msg_txt(275)); // And you don't get drops of looted items.
			return -1;
		}
	}

	if (result_sscanf != 0 && ((double)((long long)drop_rate)) == drop_rate && drop_rate != drop_rate2)
		drop_rate = drop_rate2;

	if (drop_rate >= 0. && drop_rate <= 100.) {
		sd->state.autoloot_rate = (int)(drop_rate * 100.);
		if (drop_rate == 0)
			clif_displaymessage(fd, msg_txt(277)); // Autoloot of monster drops disabled.
		else {
			sprintf(atcmd_output, msg_txt(278), drop_rate); // Set autoloot of monster drops when they are between 0 to %0.02f%%.
			clif_displaymessage(fd, atcmd_output);
		}
		if (sd->state.autolootloot_flag)
			clif_displaymessage(fd, msg_txt(274)); // And you get drops of looted items.
		else
			clif_displaymessage(fd, msg_txt(275)); // And you don't get drops of looted items.
	} else {
		clif_displaymessage(fd, msg_txt(276)); // Invalid drop rate. Value must be between 0 (no autoloot) to 100 (autoloot all drops).
		return -1;
	}

	return 0;
}

/*==========================================
* @autolootloot : On/Off [Yor]
*------------------------------------------
*/
ATCOMMAND_FUNC(autolootloot) {
	if (sd->state.autolootloot_flag) {
		sd->state.autolootloot_flag = 0; // 0: no auto-loot, 1: autoloot (for looted items)
		clif_displaymessage(fd, msg_txt(272)); // Your auto loot option for looted items is now set to OFF.
	} else {
		sd->state.autolootloot_flag = 1; // 0: no auto-loot, 1: autoloot (for looted items)
		clif_displaymessage(fd, msg_txt(273)); // Your auto loot option for looted items is now set to ON.
	}

	return 0;
}

/*==========================================
* Turns on/off Displayexp for a specific player
*------------------------------------------
*/
ATCOMMAND_FUNC(displayexp) {
	if (sd->state.displayexp_flag) {
		sd->state.displayexp_flag = 0; // 0 no xp display, 1: exp display
		clif_displaymessage(fd, msg_txt(249)); // Your display experience option is now set to OFF.
	} else {
		sd->state.displayexp_flag = 1; // 0 no xp display, 1: exp display
		clif_displaymessage(fd, msg_txt(250)); // Your display experience option is now set to ON.
	}

	return 0;
}

/*==========================================
* Set display drop for a specific player
*------------------------------------------
*/
ATCOMMAND_FUNC(displaydrop) {
	char message2[100];
	double drop_rate, drop_rate2;
	int i, result_sscanf;

	memset(message2, 0, sizeof(message2));

	result_sscanf = 0;
	if (message && *message) {
		i = 0;
		while(message[i]) {
			if (message[i] == ',')
				message2[i] = '.';
			else
				message2[i] = message[i];
			i++;
		}
		result_sscanf = sscanf(message2, "%lf", &drop_rate2);
	}

	if (!message || !*message || sscanf(message, "%lf", &drop_rate) < 1) {
		if (result_sscanf != 0) {
			drop_rate = drop_rate2;
		} else {
			send_usage(fd, "Please, enter a max drop rate (usage: %s <max_drop_rate>).", original_command);
			if (sd->state.displaydrop_rate == 0)
				clif_displaymessage(fd, msg_txt(661)); // Your current display drop option is disabled.
			else {
				sprintf(atcmd_output, msg_txt(662), ((float)sd->state.displaydrop_rate) / 100.); // Your current display drop option is set to display rate between 0 to %0.02f%%.
				clif_displaymessage(fd, atcmd_output);
			}
			if (sd->state.displaylootdrop)
				clif_displaymessage(fd, msg_txt(665)); // And you display drops of looted items.
			else
				clif_displaymessage(fd, msg_txt(666)); // And you don't display drops of looted items.
			return -1;
		}
	}

	if (result_sscanf != 0 && ((double)((long long)drop_rate)) == drop_rate && drop_rate != drop_rate2)
		drop_rate = drop_rate2;

	if (drop_rate >= 0. && drop_rate <= 100.) {
		sd->state.displaydrop_rate = (int)(drop_rate * 100.);
		if (drop_rate == 0)
			clif_displaymessage(fd, msg_txt(659)); // Displaying of monster drops disabled.
		else {
			sprintf(atcmd_output, msg_txt(658), drop_rate); // Set displaying of monster drops when they are between 0 to %0.02f%%.
			clif_displaymessage(fd, atcmd_output);
		}
		if (sd->state.displaylootdrop)
			clif_displaymessage(fd, msg_txt(665)); // And you display drops of looted items.
		else
			clif_displaymessage(fd, msg_txt(666)); // And you don't display drops of looted items.
	} else {
		clif_displaymessage(fd, msg_txt(657)); // Invalid drop rate. Value must be between 0 (no display) to 100 (display all drops).
		return -1;
	}

	return 0;
}

/*==========================================
* Set display drop of loot for a specific player
*------------------------------------------
*/
ATCOMMAND_FUNC(displaylootdrop) {
	if (sd->state.displaylootdrop) {
		sd->state.displaylootdrop = 0;
		clif_displaymessage(fd, msg_txt(664)); // You don't more display drops of looted items.
	} else {
		sd->state.displaylootdrop = 1;
		clif_displaymessage(fd, msg_txt(663)); // You display now drops of looted items.
	}

	return 0;
}

/*==========================================
* Turns on/off display of hp of all players
*------------------------------------------
*/
ATCOMMAND_FUNC(display_player_hp) {
	if (sd->state.display_player_hp) {
		sd->state.display_player_hp = 0; // 0 no hp display, 1: hp display
		clif_displaymessage(fd, msg_txt(247)); // Your display option (HP of players) is now set to OFF.
	} else {
		sd->state.display_player_hp = 1; // 0 no hp display, 1: hp display
		clif_displaymessage(fd, msg_txt(248)); // Your display option (HP of players) is now set to ON.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(main) {
	char message_to_all[100];

	if (!message || !*message || sscanf(message, "%[^\n]", message_to_all) < 1) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (strcasecmp(message_to_all, "on") == 0) {
		if (sd->state.main_flag == 1) {
			clif_displaymessage(fd, "Your Main channel is already ON.");
		} else {
			sd->state.main_flag = 1;
			clif_displaymessage(fd, "Your Main channel is now ON.");
		}
	} else if (strcasecmp(message_to_all, "off") == 0) {
		if (sd->state.main_flag == 0) {
			clif_displaymessage(fd, "Your Main channel is already OFF.");
		} else {
			sd->state.main_flag = 0;
			clif_displaymessage(fd, "Your Main channel is now OFF.");
		}
	} else if (sd->state.main_flag == 0)
		clif_displaymessage(fd, "Your Main channel is OFF. You can not speak on.");
	else {
		if (check_bad_word(message_to_all, strlen(message_to_all), sd))
			return -1; // check_bad_word function display message if necessary
		// check flag for WoE
		if (agit_flag == 1 && // 0: WoE not starting, Woe is running
		    sd->status.guild_id > 0 &&
		    battle_config.atcommand_main_channel_when_woe > sd->GM_level) { // is not possible to use @main when WoE and in guild
			clif_displaymessage(fd, msg_txt(679)); // War of Emperium is actually running. Because you are member of a guild, you can not use 'Main channel'.
			return -1;
		}
		intif_main_message(sd->status.name, message_to_all);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
ATCOMMAND_FUNC(request) {
	char message_to_GM[100];

	if (!message || !*message || sscanf(message, "%[^\n]", message_to_GM) < 1) {
		send_usage(fd, "Please, enter a message (usage: %s <message>).", original_command);
		return -1;
	}

	if (battle_config.atcommand_min_GM_level_for_request > 99)
		clif_displaymessage(fd, msg_txt(251)); // Your request has not been sent. Request system is disabled.
	else if (strcasecmp(message_to_GM, "on") == 0) {
		if (sd->GM_level < battle_config.atcommand_min_GM_level_for_request) {
			clif_displaymessage(fd, "Your option is refused. You are not a GM that can receive requests from players.");
			return -1;
		} else if (sd->state.refuse_request_flag == 0) {
			clif_displaymessage(fd, "You can already receive requests from players.");
		} else {
			sd->state.refuse_request_flag = 0;
			clif_displaymessage(fd, "You can now receive requests from players.");
		}
	} else if (strcasecmp(message_to_GM, "off") == 0) {
		if (sd->GM_level < battle_config.atcommand_min_GM_level_for_request) {
			clif_displaymessage(fd, "Your option is refused. You are not a GM that can receive requests from players.");
			return -1;
		} else if (sd->state.refuse_request_flag == 1) {
			clif_displaymessage(fd, "You already refuse requests from players.");
		} else {
			sd->state.refuse_request_flag = 1;
			clif_displaymessage(fd, "You refuse now requests from players.");
		}
	} else {
		if (check_bad_word(message_to_GM, strlen(message_to_GM), sd))
			return -1; // check_bad_word function display message if necessary
		// send message to online GMs
		if (intif_gm_message(sd->status.name, message_to_GM)) // Is there at least 1 online GM to receive the message (on this server)?
			clif_displaymessage(fd, "Your request has been delivered.");
		else if (map_is_alone) // not in multi-servers
			clif_displaymessage(fd, "Sorry, but there're no GMs online. Try to send your request later.");
		else // no online GM on this server, but perahps on some other map-servers
			clif_displaymessage(fd, msg_txt(252)); // Your request has been sent. If no GM is online, your request is lost.
	}

	return 0;
}

/*==========================================
 * @version - Displays version of Nezumi. +/- like 0x7530 admin packet
 *------------------------------------------
 */
ATCOMMAND_FUNC(version) {
	sprintf(atcmd_output, "Nezumi version %d.%d.%d %s", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION,
#ifdef USE_SQL
	"SQL");
#else
	"TXT");
#endif /* USE_SQL */
#ifdef SVN_REVISION
	if (SVN_REVISION >= 1) // in case of .svn directories have been deleted
		sprintf(atcmd_output + strlen(atcmd_output), " (SVN rev.: %d).", (int)SVN_REVISION);
#endif /* SVN_REVISION */

	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * @version2 - Displays version of Nezumi (with additional informations). +/- like 0x7535 admin packet
 *------------------------------------------
 */
ATCOMMAND_FUNC(version2) {
	sprintf(atcmd_output, "Nezumi version %d.%d.%d %s %s", ATHENA_MAJOR_VERSION, ATHENA_MINOR_VERSION, ATHENA_REVISION,
#ifdef USE_SQL
	"SQL", ATHENA_RELEASE_FLAG ? "beta" : "final");
#else
	"TXT", ATHENA_RELEASE_FLAG ? "beta" : "final");
#endif /* USE_SQL */
#ifdef SVN_REVISION
	if (SVN_REVISION >= 1) // in case of .svn directories have been deleted
		sprintf(atcmd_output + strlen(atcmd_output), " (SVN rev.: %d)", (int)SVN_REVISION);
#endif /* SVN_REVISION */
	sprintf(atcmd_output + strlen(atcmd_output), ", Athena Mod version %d.", ATHENA_MOD_VERSION);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * Duelling System commands by [Daven]
 *
 * It'd be great if some1 could 
 * improve these @commands...
 *------------------------------------------
 * @d | @duel <char_name> - initiate duel
 * @a | @accept - accept the duel request
 * @r | @reject - reject the duel request
 * @do | @dueloff - leave current duel
 * @di | @duelinfo - info about duellants
 *------------------------------------------
 * For more details u can contact me via 
 * Nezumi forums at http://nezumi.dns.st
 *------------------------------------------
 */

ATCOMMAND_FUNC(duel) {

	struct map_session_data *pl_sd = NULL;
	char msg[120];

	if (sd->d_status==3 && sd->d_id!=0){
		clif_displaymessage(fd, "Unable to send request. You have already been requested a duel.");
		return -1;
	}
	
	if (!message || !*message || sscanf(message, "%[^\n]", atcmd_name) < 1) {
		send_usage(fd, "Please, enter a player name (usage: %s <char name>).", original_command);
		return -1;
	}

	if ((pl_sd=map_nick2sd(atcmd_name))==NULL || (pl_sd!=NULL && !pl_sd->state.auth)){
		clif_displaymessage(fd, "Opponent not found.");
		return -1;
	}

	if(sd->char_id==pl_sd->char_id){
		clif_displaymessage(fd, "You can not request a duel from yourself!");
		return -1;
	} 

	if (sd->bl.m!=pl_sd->bl.m){
		clif_displaymessage(fd, "Your opponent must be on the same location, as you.");
		return -1;
	}

	if (/* map[sd->bl.m].flag.nopvp || */ map[sd->bl.m].flag.pvp || map[sd->bl.m].flag.gvg){ // flag nopvp commented out due to unavailability
		clif_displaymessage(fd, "This location is not allowed for duelling.");
		return -1;
	}
	
	// set the duel id for both players equal to the host's char id
	sd->d_id=sd->char_id;
	pl_sd->d_id=sd->char_id;
	
	// set the host(1) and request(3) state for both opponents
	sd->d_status=1;
	pl_sd->d_status=3;
	
	// reset the duellant counter
	sd->d_count=0;

	// message output
	sprintf(msg,"Player %s [%d/%d - %s] request a duel!", sd->status.name,sd->status.base_level,sd->status.job_level,job_name(pl_sd->status.class));
	clif_displaymessage(sd->fd, "Request sent. Awaiting reply...");
	clif_displaymessage(pl_sd->fd, msg);
	clif_disp_onlyself(pl_sd, "@accept - accept the duel request");
	clif_disp_onlyself(pl_sd, "@reject - reject the duel request");
	return 0;	
}

ATCOMMAND_FUNC(accept) {

	struct map_session_data *pl_sd;
	struct map_session_data *t_sd = NULL;
	int duelid, i;
	char msg1[120], msg2[120];
	pl_sd=NULL;

	if (sd->d_status!=3 || sd->d_id==0){
		clif_displaymessage(fd, "Unable to accept the duel request. No request was sent to you.");
		return -1;
	}

	// initial definitions
	pl_sd=map_charid2sd(sd->d_id); // duel host's session data
	duelid=sd->d_id; // duel id
	
	// duellants counter [+1]
	pl_sd->d_count++;

	// �������
	// clif_specialeffect(&pl_sd->bl, type, flag);

	// prepare messages
	sprintf(msg1,"%s [%d/%d - %s] has joined the duel!", sd->status.name,sd->status.base_level,sd->status.job_level,job_name(pl_sd->status.class));
	sprintf(msg2,"Current number of duellants: %d", pl_sd->d_count);

	// message output
	for (i = 0; i < fd_max; i++){
		if(session[i] && (t_sd = session[i]->session_data) && t_sd->state.auth && t_sd->d_id==duelid && t_sd!=sd) {
			clif_disp_onlyself(t_sd, msg1);
			clif_disp_onlyself(t_sd, msg2);
		}
	}
	
	sd->d_status=2;
	clif_displaymessage(sd->fd,"Duel accepted! Let's get ready to rumble!");
	clif_set0199(sd->fd, 1);
	if(pl_sd->d_count==1){
		clif_displaymessage(pl_sd->fd,"Duel accepted! Let's get ready to rumble!");
		clif_set0199(pl_sd->fd, 1);
	}
	return 0;	
}

ATCOMMAND_FUNC(reject) {

	struct map_session_data *pl_sd = NULL;
	int duelid;
	char msg1[120];

	if (sd->d_status!=3 || sd->d_id==0){
		clif_displaymessage(fd, "Unable to reject the duel request. No request was sent to you.");
		return -1;
	}

	// ��������� �������
	pl_sd=map_charid2sd(sd->d_id); // ����
	duelid=sd->d_id; // �� ����� ��������
	
	sprintf(msg1,"%s has rejected your duel request.", sd->status.name);

	// ��������� ���-�� ������� � ����� ���������
	sd->d_status=0;
	sd->d_id=0;
	if(pl_sd->d_count==0){
		pl_sd->d_status=0;
		pl_sd->d_id=0;
		clif_displaymessage(sd->fd,msg1);
	}
	
	clif_displaymessage(sd->fd,"Duel rejected.");
	return 0;	
}

ATCOMMAND_FUNC(dueloff) {

	struct map_session_data *pl_sd = NULL;
	struct map_session_data *t_sd = NULL;
	int duelid, i;
	char msg1[120],msg2[120];

	if (sd->d_status==0 || sd->d_status==3 || sd->d_id==0){
		clif_displaymessage(fd, "You are not duelling right now. Unable to leave");
		return -1;
	}

	// init
	pl_sd=map_charid2sd(sd->d_id); // duel host's session data
	duelid=sd->d_id; // duel id

	// duellants counter [-1]
	pl_sd->d_count--;
	
	sprintf(msg1,"%s has left the duel.", sd->status.name);
	sprintf(msg2,"Current number of duellants: %d", sd->d_count);

	// parsing number of duellants and their status
	if(sd==pl_sd){ // player is a host => end the duel for all duellants
		for (i = 0; i < fd_max; i++){
		if(session[i] && (t_sd = session[i]->session_data) && t_sd->state.auth && t_sd->d_id==duelid) {
				t_sd->d_id=0;
				t_sd->d_status=0;
				t_sd->d_count=0;
				clif_displaymessage(t_sd->fd,"The host has left the duel. No blood will spill any more. Duel over.");
				clif_set0199(t_sd->fd, 0);
			}
		}
	} else {
		sd->d_id=0;
		sd->d_status=0;
		sd->d_count=0;
		if(pl_sd->d_count==0){
			pl_sd->d_id=0;
			pl_sd->d_status=0;
			pl_sd->d_count=0;
			clif_displaymessage(pl_sd->fd,"All duellants have left. Duel over.");
		}
		clif_displaymessage(sd->fd,"You have left the duel.");
		clif_set0199(sd->fd, 0);
		for (i = 0; i < fd_max; i++){
			if(session[i] && (t_sd = session[i]->session_data) && t_sd->state.auth && t_sd->d_id==duelid && t_sd!=sd) {
				clif_disp_onlyself(t_sd, msg1);
				clif_disp_onlyself(t_sd, msg2);
			}
		}
	}
	return 0;	
}

ATCOMMAND_FUNC(duelinfo) {

	struct map_session_data *pl_sd = NULL;
	struct map_session_data *t_sd = NULL;
	int duelid, i;
	char msg1[120], msg2[120];

	if (sd->d_status==0 || sd->d_id==0){
		clif_displaymessage(fd, "You are not duelling right now. Unable to retrieve duel information.");
		return -1;
	}

	// init
	pl_sd=map_charid2sd(sd->d_id); // duel host's session data
	duelid=sd->d_id; // duel id
	
	// Prepare messages
	sprintf(msg2,"Total number of duellants: %d", pl_sd->d_count);

	// parse players output
	clif_disp_onlyself(sd, "------------------------------------------");
	clif_disp_onlyself(sd, "             [ Duel information ]            ");
	clif_disp_onlyself(sd, "------------------------------------------");
	clif_disp_onlyself(t_sd, msg2);
	
	// list duellants
		for (i = 0; i < fd_max; i++){
			if(session[i] && (t_sd = session[i]->session_data) && t_sd->state.auth && t_sd->d_id==duelid){
			sprintf(msg1,"[ %s ]-[ %d/%d - %s ]", t_sd->status.name,t_sd->status.base_level,t_sd->status.job_level,job_name(t_sd->status.class));
			clif_disp_onlyself(sd, msg1);
		}
	}
	return 0;

}


#ifdef USE_SQL  /* Begin SQL-Only commands */


/*==========================================
 * Mail System commands by [Valaris]
 *------------------------------------------
 */

ATCOMMAND_FUNC(listmail) {
	if (!battle_config.mail_system)
		return 0;

	if (strlen(command) == 12)
		mail_check(sd, 3); // type: 1:checkmail, 2:listmail, 3:listnewmail
	else if (strlen(command) == 9)
		mail_check(sd, 2); // type: 1:checkmail, 2:listmail, 3:listnewmail
	else
		mail_check(sd, 1); // type: 1:checkmail, 2:listmail, 3:listnewmail

	return 0;
}

ATCOMMAND_FUNC(readmail) {
	if (!battle_config.mail_system)
		return 0;

	if (!message || !*message) {
		clif_displaymessage(sd->fd, "You must specify a message number.");
		return 0;
	}

	if (strlen(command) == 11)
		mail_delete(sd, atoi(message));
	else
		mail_read(sd, atoi(message));

	return 0;
}

ATCOMMAND_FUNC(sendmail) {
	char text[100];

	if (!battle_config.mail_system)
		return 0;

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %79[^\n]", atcmd_name, text) < 2 &&
	     sscanf(message, "%s %79[^\n]", atcmd_name, text) < 2)) {
		clif_displaymessage(sd->fd, "You must specify a recipient and a message.");
		return 0;
	}

	if (check_bad_word(text, strlen(text), sd))
		return -1; // check_bad_word function display message if necessary

	if (strlen(command) == 17)
		mail_send(sd, atcmd_name, text, 1); // flag = 0: normal, 1: priority message
	else
		mail_send(sd, atcmd_name, text, 0); // flag = 0: normal, 1: priority message

	return 0;
}

#endif /* end sql only */

