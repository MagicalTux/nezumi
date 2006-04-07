// $Id$
#ifndef _ATCOMMAND_H_
#define _ATCOMMAND_H_

#define MAX_MSG_LEN 512

enum AtCommandType {
	AtCommand_None = -1,
	AtCommand_Broadcast = 0,
	AtCommand_LocalBroadcast,
	AtCommand_LocalBroadcast2,
	AtCommand_MapMove,
	AtCommand_RuraP,
	AtCommand_Rura,
	AtCommand_Warp,
	AtCommand_Where,
	AtCommand_JumpTo,
	AtCommand_Jump,
	AtCommand_Users,
	AtCommand_Who,
	AtCommand_Who2,
	AtCommand_Who3,
	AtCommand_WhoMap,
	AtCommand_WhoMap2,
	AtCommand_WhoMap3,
	AtCommand_WhoGM,
	AtCommand_Save,
	AtCommand_Load,
	AtCommand_CharLoad,
	AtCommand_CharLoadMap,
	AtCommand_CharLoadAll,
	AtCommand_Speed,
	AtCommand_CharSpeed,
	AtCommand_CharSpeedMap,
	AtCommand_CharSpeedAll,
	AtCommand_Storage,
	AtCommand_CharStorage,
	AtCommand_GuildStorage,
	AtCommand_CharGuildStorage,
	AtCommand_Option,
	AtCommand_OptionAdd,
	AtCommand_OptionRemove,
	AtCommand_Hide,
	AtCommand_JobChange,
	AtCommand_JobChange2,
	AtCommand_JobChange3,
	AtCommand_Die,
	AtCommand_Kill,
	AtCommand_Alive,
	AtCommand_Heal,
	AtCommand_Kami,
	AtCommand_KamiB,
	AtCommand_KamiC, // [LuzZza]
	AtCommand_KamiGM,
	AtCommand_Item,
	AtCommand_CharItem,
	AtCommand_CharItemAll,
	AtCommand_Item2,
	AtCommand_ItemReset,
	AtCommand_CharItemReset,
	AtCommand_ItemCheck,
	AtCommand_CharItemCheck,
	AtCommand_BaseLevelUp,
	AtCommand_JobLevelUp,
	AtCommand_H,
	AtCommand_Help,
	AtCommand_GM,
	AtCommand_PvPOff,
	AtCommand_PvPOn,
	AtCommand_GvGOff,
	AtCommand_GvGOn,
	AtCommand_Model,
	AtCommand_Go,

	AtCommand_Spawn,
	AtCommand_SpawnMap,
	AtCommand_SpawnAll,

	AtCommand_Summon,

	AtCommand_DeadBranch,
	AtCommand_CharDeadBranch,
	AtCommand_DeadBranchMap,
	AtCommand_DeadBranchAll,

	AtCommand_KillMonster,
	AtCommand_KillMonster2,
	AtCommand_KillMonsterArea,
	AtCommand_KillMonster2Area,

	AtCommand_Refine,
	AtCommand_RefineAll,
	AtCommand_Produce,
	AtCommand_Memo,
	AtCommand_GAT,
	AtCommand_Packet,
	AtCommand_StatusPoint,
	AtCommand_SkillPoint,
	AtCommand_Zeny,
	AtCommand_Param,
	AtCommand_Strength,
	AtCommand_Agility,
	AtCommand_Vitality,
	AtCommand_Intelligence,
	AtCommand_Dexterity,
	AtCommand_Luck,
	AtCommand_GuildLevelUp,
	AtCommand_CharGuildLevelUp,
	AtCommand_MakeEgg,
	AtCommand_PetFriendly,
	AtCommand_PetHungry,
	AtCommand_PetRename,
	AtCommand_CharPetRename,
	AtCommand_Recall,
	AtCommand_CharacterJob,
	AtCommand_ChangeLevel,
	AtCommand_Revive,
	AtCommand_CharacterHeal,
	AtCommand_CharacterStats,
	AtCommand_CharacterStatsAll,
	AtCommand_CharacterOption,
	AtCommand_CharacterOptionAdd,
	AtCommand_CharacterOptionRemove,
	AtCommand_CharacterSave,
	AtCommand_CharacterLoad,
	AtCommand_Night,
	AtCommand_Day,
	AtCommand_DoomMap,
	AtCommand_Doom,
	AtCommand_RaiseMap,
	AtCommand_Raise,
	AtCommand_CharacterBaseLevel,
	AtCommand_CharacterJobLevel,
	AtCommand_Kick,
	AtCommand_KickMap,
	AtCommand_KickAll,
	AtCommand_AllSkill,
	AtCommand_QuestSkill,
	AtCommand_CharQuestSkill,
	AtCommand_LostSkill,
	AtCommand_CharLostSkill,
	AtCommand_SpiritBall,
	AtCommand_CharSpiritBall,
	AtCommand_Party,
	AtCommand_Guild,
	AtCommand_AgitStart,
	AtCommand_AgitEnd,
	AtCommand_MapExit,
	AtCommand_IDSearch,
	AtCommand_WhoDrops,
	AtCommand_ResetState,
	AtCommand_ResetSkill,
	AtCommand_CharSkReset,
	AtCommand_CharStReset,
	AtCommand_CharReset,
	AtCommand_CharModel,
	AtCommand_CharSKPoint,
	AtCommand_CharSTPoint,
	AtCommand_CharZeny,
	AtCommand_RecallAll,
	AtCommand_ReloadItemDB,
	AtCommand_ReloadMobDB,
	AtCommand_ReloadSkillDB,
	AtCommand_ReloadScript,
//	AtCommand_ReloadGMDB, // removed, it's automatic now
	AtCommand_MapInfo,
	AtCommand_MobInfo,
	AtCommand_Dye,
	AtCommand_Hstyle,
	AtCommand_Hcolor,
	AtCommand_StatAll,
	AtCommand_ChangeSex,
	AtCommand_CharChangeSex,
	AtCommand_CharBlock,
	AtCommand_CharBan,
	AtCommand_CharUnBlock,
	AtCommand_CharUnBan,
	AtCommand_MountPeco,
	AtCommand_CharMountPeco,
	AtCommand_Falcon,
	AtCommand_CharFalcon,
	AtCommand_Cart,
	AtCommand_CharCart,
	AtCommand_RemoveCart,
	AtCommand_CharRemoveCart,
	AtCommand_GuildSpy,
	AtCommand_PartySpy,
	AtCommand_RepairAll,
	AtCommand_GuildRecall,
	AtCommand_PartyRecall,
	AtCommand_Nuke,
	AtCommand_Enablenpc,
	AtCommand_Disablenpc,
	AtCommand_ServerTime,
	AtCommand_CharDelItem,
	AtCommand_Jail,
	AtCommand_UnJail,
	AtCommand_JailTime,
	AtCommand_CharJailTime,
	AtCommand_Disguise,
	AtCommand_UnDisguise,
	AtCommand_CharDisguise,
	AtCommand_CharUnDisguise,
	AtCommand_CharDisguiseMap,
	AtCommand_CharUnDisguiseMap,
	AtCommand_CharDisguiseAll,
	AtCommand_CharUnDisguiseAll,
	AtCommand_ChangeLook,
	AtCommand_CharChangeLook,
	AtCommand_IgnoreList,
	AtCommand_CharIgnoreList,
	AtCommand_InAll,
	AtCommand_ExAll,
	AtCommand_EMail,
	AtCommand_Hatch,
	AtCommand_Effect,
	AtCommand_Char_Item_List,
	AtCommand_Char_Storage_List,
	AtCommand_Char_Cart_List,
	AtCommand_AddWarp,
	AtCommand_Follow,
	AtCommand_UnFollow,
	AtCommand_SkillOn,
	AtCommand_SkillOff,
	AtCommand_NoSpell,
	AtCommand_Killer,
	AtCommand_CharKiller,
	AtCommand_NpcMove,
	AtCommand_Killable,
	AtCommand_CharKillable,
	AtCommand_Chareffect,
	AtCommand_Chardye,
	AtCommand_Charhairstyle,
	AtCommand_Charhaircolor,
	AtCommand_Dropall,
	AtCommand_Chardropall,
	AtCommand_Storeall,
	AtCommand_Charstoreall,
	AtCommand_Skillid,
	AtCommand_Useskill,
	AtCommand_Rain,
	AtCommand_Snow,
	AtCommand_Sakura,
	AtCommand_Fog,
	AtCommand_Leaves,
	AtCommand_Rainbow,
	AtCommand_Clsweather,
	AtCommand_MobSearch,
	AtCommand_CleanMap,
	AtCommand_AdjGmLvl,
	AtCommand_AdjGmLvl2,
	AtCommand_AdjCmdLvl,
	AtCommand_Trade,
	AtCommand_Send,
	AtCommand_SetBattleFlag,
	AtCommand_SetMapFlag,
	AtCommand_UnMute,
	AtCommand_UpTime,
	AtCommand_Clock,
	AtCommand_Mute,
	AtCommand_WhoZeny,
	AtCommand_WhoZenyMap,
	AtCommand_WhoHas,
	AtCommand_WhoHasMap,
	AtCommand_HappyHappyJoyJoy,
	AtCommand_HappyHappyJoyJoyMap,
	AtCommand_Refresh,
	AtCommand_PetId,
	AtCommand_Identify,
	AtCommand_Motd,
	AtCommand_Gmotd,
	AtCommand_MiscEffect,
	AtCommand_SkillTree,
	AtCommand_Marry,
	AtCommand_Divorce,
	AtCommand_Rings,
	AtCommand_Grind,
	AtCommand_Grind2,
	AtCommand_Sound,

	AtCommand_NpcTalk,
	AtCommand_PetTalk,
	AtCommand_AutoLoot,
	AtCommand_AutoLootLoot,
	AtCommand_Displayexp,
	AtCommand_DisplayDrop,
	AtCommand_DisplayLootDrop,
	AtCommand_Display_Player_Hp,
	AtCommand_Main,
	AtCommand_Request,
	AtCommand_Version,
	AtCommand_Version2,

	// SQL-only commands start
#ifdef USE_SQL
	AtCommand_CheckMail,
	AtCommand_ListMail,
	AtCommand_ListNewMail,
	AtCommand_ReadMail,
	AtCommand_SendMail,
	AtCommand_DeleteMail,
	AtCommand_SendPriorityMail,
#endif /* USE_SQL */
	AtCommand_Unknown,
	AtCommand_MAX
};

typedef enum AtCommandType AtCommandType;

extern short log_gm_level;

char GM_Symbol();

AtCommandType is_atcommand(const int fd, struct map_session_data* sd, const char* message, unsigned char gmlvl);

int get_atcommand_level(const AtCommandType type);

char * msg_txt(int msg_number);

void atcommand_config_read(const char *cfgName);
void atcommand_custom_read(const char *cfgName);
void atcommand_custom_free();
int msg_config_read(const char *cfgName);
void do_final_msg_config();

#endif // _ATCOMMAND_H_
