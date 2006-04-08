// $Id$
#ifndef _STATUS_H_
#define _STATUS_H_

enum {
	SC_STONE = 0,
	SC_FREEZE,
	SC_STAN,
	SC_SLEEP,
	SC_POISON,
	SC_CURSE,
	SC_SILENCE,
	SC_CONFUSION,
	SC_BLIND,
	SC_BLEEDING,
	SC_DPOISON, //10

	SC_PROVOKE = 20,
	SC_ENDURE,
	SC_TWOHANDQUICKEN,
	SC_CONCENTRATE,
	SC_HIDING,
	SC_CLOAKING,
	SC_ENCPOISON,
	SC_POISONREACT,
	SC_QUAGMIRE,
	SC_ANGELUS,
	SC_BLESSING,
	SC_SIGNUMCRUCIS,
	SC_INCREASEAGI,
	SC_DECREASEAGI,
	SC_SLOWPOISON,
	SC_IMPOSITIO,
	SC_SUFFRAGIUM,
	SC_ASPERSIO,
	SC_BENEDICTIO,
	SC_KYRIE,
	SC_MAGNIFICAT, //40
	SC_GLORIA,
	SC_AETERNA,
	SC_ADRENALINE,
	SC_WEAPONPERFECTION,
	SC_OVERTHRUST,
	SC_MAXIMIZEPOWER,
	SC_TRICKDEAD,
	SC_LOUD,
	SC_ENERGYCOAT,
	SC_BROKENARMOR, //50
	SC_BROKENWEAPON,
	SC_HALLUCINATION,
	SC_WEIGHT50,
	SC_WEIGHT90,
	SC_SPEEDPOTION0,
	SC_SPEEDPOTION1,
	SC_SPEEDPOTION2,
	SC_SPEEDPOTION3,
	SC_SPEEDUP0,
	SC_SPEEDUP1, //60
	SC_ATKPOT,
	SC_MATKPOT,
	SC_WEDDING,
	SC_SLOWDOWN,
	SC_ANKLE,
	SC_KEEPING,
	SC_BARRIER,
	SC_STRIPWEAPON,
	SC_STRIPSHIELD,
	SC_STRIPARMOR, //70
	SC_STRIPHELM,
	SC_CP_WEAPON,
	SC_CP_SHIELD,
	SC_CP_ARMOR,
	SC_CP_HELM,
	SC_AUTOGUARD,
	SC_REFLECTSHIELD,
	SC_SPLASHER,
	SC_PROVIDENCE,
	SC_DEFENDER, //80
	SC_MAGICROD,
	SC_SPELLBREAKER,
	SC_AUTOSPELL,
	SC_SIGHTTRASHER,
	SC_AUTOBERSERK,
	SC_SPEARSQUICKEN,
	SC_AUTOCOUNTER,
	SC_SIGHT,
	SC_SAFETYWALL,
	SC_RUWACH, //90
	SC_EXTREMITYFIST,
	SC_EXPLOSIONSPIRITS,
	SC_COMBO,
	SC_BLADESTOP_WAIT,
	SC_BLADESTOP,
	SC_FLAMELAUNCHER,
	SC_FROSTWEAPON,
	SC_LIGHTNINGLOADER,
	SC_SEISMICWEAPON,
	SC_VOLCANO, //100
	SC_DELUGE,
	SC_VIOLENTGALE,
	SC_WATK_ELEMENT,
	SC_LANDPROTECTOR,
	SC_ARMOR_ELEMENT,
	SC_NOCHAT,
	SC_BABY,
	SC_AURABLADE,
	SC_PARRYING,
	SC_CONCENTRATION,
	SC_TENSIONRELAX,
	SC_BERSERK,
	SC_FURY,
	SC_GOSPEL,
	SC_ASSUMPTIO,
	SC_BASILICA,
	SC_GUILDAURA,
	SC_MAGICPOWER,
	SC_EDP,
	SC_TRUESIGHT,
	SC_WINDWALK,
	SC_MELTDOWN,
	SC_CARTBOOST,
	SC_CHASEWALK,
	SC_REJECTSWORD,
	SC_MARIONETTE,
	SC_MARIONETTE2,
	SC_MOONLIT,
	SC_JOINTBEAT,
	SC_MINDBREAKER,
	SC_MEMORIZE,
	SC_FOGWALL,
	SC_SPIDERWEB,
	SC_DEVOTION,
	SC_SACRIFICE,
	SC_STEELBODY,
	SC_ORCISH,
	SC_READYSTORM,
	SC_STORMKICK,
	SC_READYDOWN,
	SC_DOWNKICK,
	SC_READYTURN,
	SC_TURNKICK,
	SC_READYCOUNTER,
	SC_COUNTER,
	SC_DODGE,
	SC_JUMPKICK,
	SC_RUN,
	SC_SHADOWWEAPON,
	SC_ADRENALINE2,
	SC_GHOSTWEAPON,
	SC_NIGHT,
	SC_KAIZEL,
	SC_KAAHI,
	SC_KAUPE,
	SC_ONEHAND,
	SC_PRESERVE,
	SC_BATTLEORDERS,
	SC_REGENERATION,
	SC_DOUBLECASTING,
	SC_GRAVITATION,
	SC_MAXOVERTHRUST,
	SC_LONGING,
	SC_HERMODE,
	SC_SHRINK,
	SC_SIGHTBLASTER,
	SC_WINKCHARM,
	SC_CLOSECONFINE,
	SC_CLOSECONFINE2,
	SC_DANCING,
	SC_LULLABY,
	SC_RICHMANKIM,
	SC_ETERNALCHAOS,
	SC_DRUMBATTLE,
	SC_NIBELUNGEN,
	SC_ROKISWEIL,
	SC_INTOABYSS,
	SC_SIEGFRIED,
	SC_WHISTLE,
	SC_ASSNCROS,
	SC_POEMBRAGI,
	SC_APPLEIDUN,
	SC_UGLYDANCE,
	SC_HUMMING,
	SC_DONTFORGETME,
	SC_FORTUNE,
	SC_SERVICE4U,
	SC_INTRAVISION,
	SC_MODE,
	SC_STOP,
	SC_BROKNWEAPON,
	SC_BROKNARMOR,
	SC_DISSONANCE,
	SC_INCSTR,
	SC_RIDING,
	SC_FALCON,
	SC_PNEUMA,

	SC_INCATKRATE = 300,
	SC_INCMATKRATE,
	SC_INCDEFRATE,
	SC_INCHITRATE,
	SC_INCFLEERATE,

	SC_COMA = 310,

	SC_MAX,
};
extern int SkillStatusChangeTable[];

enum {
	SI_BLANK				= -1,
	SI_PROVOKE			= 0,
	SI_ENDURE			= 1,
	SI_TWOHANDQUICKEN		= 2,
	SI_CONCENTRATE		= 3,
	SI_HIDING			= 4,
	SI_CLOAKING			= 5,
	SI_ENCPOISON		= 6,
	SI_POISONREACT		= 7,
	SI_QUAGMIRE			= 8,
	SI_ANGELUS			= 9,
	SI_BLESSING			= 10,
	SI_SIGNUMCRUCIS		= 11,
	SI_INCREASEAGI		= 12,
	SI_DECREASEAGI		= 13,
	SI_SLOWPOISON		= 14,
	SI_IMPOSITIO  		= 15,
	SI_SUFFRAGIUM		= 16,
	SI_ASPERSIO			= 17,
	SI_BENEDICTIO		= 18,
	SI_KYRIE			= 19,
	SI_MAGNIFICAT		= 20,
	SI_GLORIA			= 21,
	SI_AETERNA			= 22,
	SI_ADRENALINE		= 23,
	SI_WEAPONPERFECTION	= 24,
	SI_OVERTHRUST		= 25,
	SI_MAXIMIZEPOWER		= 26,
	SI_RIDING			= 27,
	SI_FALCON			= 28,
	SI_TRICKDEAD		= 29,
	SI_LOUD			= 30,
	SI_ENERGYCOAT		= 31,
	SI_BROKENARMOR		= 32,
	SI_BROKENWEAPON		= 33,
	SI_HALLUCINATION		= 34,
	SI_WEIGHT50 		= 35,
	SI_WEIGHT90			= 36,
	SI_SPEEDPOTION0		= 37,
	SI_SPEEDPOTION1		= 38,
	SI_SPEEDPOTION2		= 39,
	SI_SPEEDPOTION3		= 40,
	SI_SPEEDUP0         = 41,
	SI_SPEEDUP1         = 42,
	SI_ATKPOT		= 43,
	SI_MATKPOT		= 44,
	SI_ANKLE				= 47,
	SI_STRIPWEAPON		= 50,
	SI_STRIPSHIELD		= 51,
	SI_STRIPARMOR		= 52,
	SI_STRIPHELM		= 53,
	SI_CP_WEAPON		= 54,
	SI_CP_SHIELD		= 55,
	SI_CP_ARMOR			= 56,
	SI_CP_HELM			= 57,
	SI_AUTOGUARD		= 58,
	SI_REFLECTSHIELD		= 59,
	SI_PROVIDENCE		= 61,
	SI_DEFENDER			= 62,
	SI_AUTOSPELL		= 65,
	SI_SIGHTTRASHER		= 66,
	SI_AUTOBERSERK		= 67,
	SI_SPEARQUICKEN		= 68,
	SI_AUTOCOUNTER		= 69,
	SI_SIGHT			= 70,
	SI_SAFETYWALL		= 71,
	SI_RUWACH			= 72,
	SI_PNEUMA			= 73,
	SI_STONE			= 74,
	SI_FREEZE			= 75,
	SI_STAN			= 76,
	SI_SLEEP			= 77,
	SI_POISON			= 78,
	SI_CURSE			= 79,
	SI_SILENCE			= 80,
	SI_CONFUSION		= 81,
	SI_BLIND			= 82,
	SI_DPOISON			= 84,
	SI_EXPLOSIONSPIRITS	= 86,
	SI_BLADESTOP_WAIT	= 88,
	SI_BLADESTOP		= 89,
	SI_FLAMELAUNCHER	= 90,
	SI_FROSTWEAPON		= 91,
	SI_LIGHTNINGLOADER	= 92,
	SI_SEISMICWEAPON	= 93,
	SI_VOLCANO			= 94,
	SI_DELUGE			= 95,
	SI_VIOLENTGALE		= 96,
	SI_NOCHAT			= 100,
	SI_AURABLADE		= 103,
	SI_PARRYING			= 104,
	SI_CONCENTRATION		= 105,
	SI_TENSIONRELAX		= 106,
	SI_BERSERK			= 107,
	SI_FURY			= 108,
	SI_GOSPEL			= 109,
	SI_ASSUMPTIO		= 110,
	SI_GUILDAURA		= 112,
	SI_MAGICPOWER		= 113,
	SI_EDP			= 114,
	SI_TRUESIGHT		= 115,
	SI_WINDWALK			= 116,
	SI_MELTDOWN			= 117,
	SI_CARTBOOST		= 118,
	SI_CHASEWALK		= 119,
	SI_REJECTSWORD		= 120,
	SI_MARIONETTE		= 121,
	SI_MARIONETTE2		= 122,
	SI_MOONLIT			= 123,
	SI_BLEEDING			= 124,
	SI_JOINTBEAT		= 125,
	SI_MINDBREAKER		= 126,
	SI_MEMORIZE			= 127,
	SI_FOGWALL			= 128,
	SI_SPIDERWEB		= 129,
	SI_DEVOTION			= 130,
	SI_SACRIFICE		= 131,
	SI_STEELBODY		= 132,
	SI_WIGGLE			= 134,
	SI_READYSTORM		= 135,
	SI_READYDOWN		= 137,
	SI_READYTURN		= 139,
	SI_READYCOUNTER		= 141,
	SI_DODGE			= 143,
	SI_JUMPKICK   	      = 144,
	SI_RUN			= 145,
	SI_ADRENALINE2    	= 147,
	SI_NIGHT			= 149,
	SI_KAIZEL			= 156,
	SI_KAAHI			= 157,
	SI_KAUPE			= 158,
	SI_ONEHAND			= 161,
	SI_PRESERVE			= 181,
	SI_BATTLEORDERS		= 182,
	SI_REGENERATION		= 183,
	SI_DOUBLECASTING		= 186,
	SI_GRAVITATION		= 187,
	SI_MAXOVERTHRUST		= 188,
	SI_LONGING			= 189,
	SI_HERMODE			= 190,
	SI_TAROT			= 191,
	SI_SHRINK			= 197,
	SI_SIGHTBLASTER		= 198,
	SI_WINKCHARM		= 199,
	SI_CLOSECONFINE		= 200,
	SI_CLOSECONFINE2		= 201,
};
extern int StatusIconChangeTable[];

extern int percentrefinery[5][10];	// (refine rates according to refine_db.txt)

extern int inv_index;

int status_getrefinebonus(int lv, int type);
int status_percentrefinery(struct map_session_data *sd, struct item *item);

int status_calc_pc(struct map_session_data*, int);
int status_calc_speed(struct map_session_data*);
int status_get_class(struct block_list *bl);
int status_get_dir(struct block_list *bl);
int status_get_lv(struct block_list *bl);
int status_get_range(struct block_list *bl);
int status_get_hp(struct block_list *bl);
int status_get_max_hp(struct block_list *bl);
int status_get_str(struct block_list *bl);
int status_get_agi(struct block_list *bl);
int status_get_vit(struct block_list *bl);
int status_get_int(struct block_list *bl);
int status_get_dex(struct block_list *bl);
int status_get_luk(struct block_list *bl);
int status_get_hit(struct block_list *bl);
int status_get_flee(struct block_list *bl);
int status_get_def(struct block_list *bl);
int status_get_mdef(struct block_list *bl);
int status_get_flee2(struct block_list *bl);
int status_get_critical(struct block_list *bl);
int status_get_def2(struct block_list *bl);
int status_get_mdef2(struct block_list *bl);
int status_get_baseatk(struct block_list *bl);
int status_get_atk(struct block_list *bl);
int status_get_atk_(struct block_list *bl);
int status_get_atk2(struct block_list *bl);
int status_get_atk_2(struct block_list *bl);
int status_get_matk1(struct block_list *bl);
int status_get_matk2(struct block_list *bl);
int status_get_speed(struct block_list *bl);
int status_get_adelay(struct block_list *bl);
int status_get_amotion(struct block_list *bl);
int status_get_dmotion(struct block_list *bl);
int status_get_element(struct block_list *bl);
int status_get_attack_element(struct block_list *bl);
int status_get_attack_element2(struct block_list *bl); //���蕐�푮���擾
#define status_get_elem_type(bl) (status_get_element(bl) % 10)
#define status_get_elem_level(bl) (status_get_element(bl) / 10 / 2)
int status_get_party_id(struct block_list *bl);
int status_get_guild_id(struct block_list *bl);
int status_get_race(struct block_list *bl);
int status_get_size(struct block_list *bl);
int status_get_mode(struct block_list *bl);
//int status_get_mexp(struct block_list *bl);
int status_get_race2(struct block_list *bl);
int status_isdead(struct block_list *bl);

struct status_change *status_get_sc_data(struct block_list *bl);
short *status_get_sc_count(struct block_list *bl);
short *status_get_opt1(struct block_list *bl);
short *status_get_opt2(struct block_list *bl);
short *status_get_opt3(struct block_list *bl);
short *status_get_option(struct block_list *bl);

int status_get_sc_def(struct block_list *bl, int type);
#define status_get_sc_def_mdef(bl) (status_get_sc_def(bl, SP_MDEF1))
#define status_get_sc_def_vit(bl) (status_get_sc_def(bl, SP_DEF2))
#define status_get_sc_def_int(bl) (status_get_sc_def(bl, SP_MDEF2))
#define status_get_sc_def_luk(bl) (status_get_sc_def(bl, SP_LUK))

int status_change_start(struct block_list *bl, int type, intptr_t val1, intptr_t val2, intptr_t val3, intptr_t val4, int tick, int flag);
int status_change_clear(struct block_list *bl, int type);
int status_change_end(struct block_list* bl, int type, int tid);
TIMER_FUNC(status_change_timer);
int status_change_timer_sub(struct block_list *bl, va_list ap);

int status_change_clear_buffs(struct block_list *bl);
int status_change_clear_debuffs(struct block_list *bl);

int do_init_status(void);

#endif
