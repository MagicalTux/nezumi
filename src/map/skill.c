// $Id$

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/utils.h"

#include "nullpo.h"
#include "skill.h"
#include "map.h"
#include "clif.h"
#include "pc.h"
#include "pet.h"
#include "mob.h"
#include "battle.h"
#include "party.h"
#include "itemdb.h"
#include "script.h"
#include "intif.h"
#include "chrif.h"
#include "guild.h"
#include "atcommand.h"
#include "grfio.h"
#include "status.h"
#include "mob.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define SKILLUNITTIMER_INVERVAL 100
#define STATE_BLIND 0x10
#define swap(x,y) { int t; t = x; x = y; y = t; }

const struct skill_name_db skill_names[] = {
 { AC_CHARGEARROW, "CHARGEARROW", "Charge_Arrow" } ,
 { AC_CONCENTRATION, "CONCENTRATION", "Improve_Concentration" } ,
 { AC_DOUBLE, "DOUBLE", "Double_Strafe" } ,
 { AC_MAKINGARROW, "MAKINGARROW", "Arrow_Creation" } ,
 { AC_OWL, "OWL", "Owl's_Eye" } ,
 { AC_SHOWER, "SHOWER", "Arrow_Shower" } ,
 { AC_VULTURE, "VULTURE", "Vulture's_Eye" } ,
 { ALL_RESURRECTION, "RESURRECTION", "Resurrection" } ,
 { AL_ANGELUS, "ANGELUS", "Angelus" } ,
 { AL_BLESSING, "BLESSING", "Blessing" } ,
 { AL_CRUCIS, "CRUCIS", "Signum_Crusis" } ,
 { AL_CURE, "CURE", "Cure" } ,
 { AL_DECAGI, "DECAGI", "Decrease_AGI" } ,
 { AL_DEMONBANE, "DEMONBANE", "Demon_Bane" } ,
 { AL_DP, "DP", "Divine_Protection" } ,
 { AL_HEAL, "HEAL", "Heal" } ,
 { AL_HOLYLIGHT, "HOLYLIGHT", "Holy_Light" } ,
 { AL_HOLYWATER, "HOLYWATER", "Aqua_Benedicta" } ,
 { AL_INCAGI, "INCAGI", "Increase_AGI" } ,
 { AL_PNEUMA, "PNEUMA", "Pneuma" } ,
 { AL_RUWACH, "RUWACH", "Ruwach" } ,
 { AL_TELEPORT, "TELEPORT", "Teleport" } ,
 { AL_WARP, "WARP", "Warp_Portal" } ,
 { AM_ACIDTERROR, "ACIDTERROR", "Acid_Terror" } ,
 { AM_AXEMASTERY, "AXEMASTERY", "Axe_Mastery" } ,
 { AM_BERSERKPITCHER, "BERSERKPITCHER", "Berserk Pitcher" } ,
 { AM_BIOETHICS, "BIOETHICS", "Bioethics" } ,
 { AM_BIOTECHNOLOGY, "BIOTECHNOLOGY", "Biotechnology" } ,
 { AM_CALLHOMUN, "CALLHOMUN", "Call_Homunculus" } ,
 { AM_CANNIBALIZE, "CANNIBALIZE", "Bio_Cannibalize" } ,
 { AM_CP_ARMOR, "ARMOR", "Chemical_Protection_Armor" } ,
 { AM_CP_HELM, "HELM", "Chemical_Protection_Helm" } ,
 { AM_CP_SHIELD, "SHIELD", "Chemical_Protection_Shield" } ,
 { AM_CP_WEAPON, "WEAPON", "Chemical_Protection_Weapon" } ,
 { AM_CREATECREATURE, "CREATECREATURE", "Life_Creation" } ,
 { AM_CULTIVATION, "CULTIVATION", "Cultivation" } ,
 { AM_DEMONSTRATION, "DEMONSTRATION", "Demonstration" } ,
 { AM_DRILLMASTER, "DRILLMASTER", "Drillmaster" } ,
 { AM_FLAMECONTROL, "FLAMECONTROL", "Flame_Control" } ,
 { AM_HEALHOMUN, "HEALHOMUN", "Heal_Homunculus" } ,
 { AM_LEARNINGPOTION, "LEARNINGPOTION", "AM_LEARNINGPOTION" } ,
 { AM_PHARMACY, "PHARMACY", "Pharmacy" } ,
 { AM_POTIONPITCHER, "POTIONPITCHER", "Potion_Pitcher" } ,
 { AM_REST, "REST", "Sabbath" } ,
 { AM_RESURRECTHOMUN, "RESURRECTHOMUN", "Ressurect_Homunculus" } ,
 { AM_SPHEREMINE, "SPHEREMINE", "Sphere_Mine" } ,
 { ASC_BREAKER, "BREAKER", "Breaker" } ,
 { ASC_CDP, "CDP", "Create_Deadly_Poison" } ,
 { ASC_EDP, "EDP", "Deadly_Poison_Enchantment" } ,
 { ASC_HALLUCINATION, "HALLUCINATION", "Hallucination_Walk" } ,
 { ASC_KATAR, "KATAR", "Advanced_Katar_Mastery" } ,
 { ASC_METEORASSAULT, "METEORASSAULT", "Meteor_Assault" } ,
 { AS_CLOAKING, "CLOAKING", "Cloaking" } ,
 { AS_ENCHANTPOISON, "ENCHANTPOISON", "Enchant_Poison" } ,
 { AS_GRIMTOOTH, "GRIMTOOTH", "Grimtooth" } ,
 { AS_KATAR, "KATAR", "Katar_Mastery" } ,
 { AS_LEFT, "LEFT", "Lefthand_Mastery" } ,
 { AS_POISONREACT, "POISONREACT", "Poison_React" } ,
 { AS_RIGHT, "RIGHT", "Righthand_Mastery" } ,
 { AS_SONICACCEL, "SONICACCEL", "Sonic_Acceleration" } ,
 { AS_SONICBLOW, "SONICBLOW", "Sonic_Blow" } ,
 { AS_SPLASHER, "SPLASHER", "Venom_Splasher" } ,
 { AS_VENOMDUST, "VENOMDUST", "Venom_Dust" } ,
 { BA_APPLEIDUN, "APPLEIDUN", "Apple_of_Idun" } ,
 { BA_ASSASSINCROSS, "ASSASSINCROSS", "Assassin_Cross" } ,
 { BA_DISSONANCE, "DISSONANCE", "Dissonance" } ,
 { BA_FROSTJOKE, "FROSTJOKE", "Dumb_Joke" } ,
 { BA_MUSICALLESSON, "MUSICALLESSON", "Musical_Lesson" } ,
 { BA_MUSICALSTRIKE, "MUSICALSTRIKE", "Musical_Strike" } ,
 { BA_POEMBRAGI, "POEMBRAGI", "Poem_of_Bragi" } ,
 { BA_WHISTLE, "WHISTLE", "Whistle" } ,
 { BD_ADAPTATION, "ADAPTATION", "Adaption" } ,
 { BD_DRUMBATTLEFIELD, "DRUMBATTLEFIELD", "Drumb_BattleField" } ,
 { BD_ENCORE, "ENCORE", "Encore" } ,
 { BD_ETERNALCHAOS, "ETERNALCHAOS", "Eternal_Chaos" } ,
 { BD_INTOABYSS, "INTOABYSS", "Into_the_Abyss" } ,
 { BD_LULLABY, "LULLABY", "Lullaby" } ,
 { BD_RAGNAROK, "RAGNAROK", "Ragnarok" } ,
 { BD_RICHMANKIM, "RICHMANKIM", "Rich_Mankim" } ,
 { BD_RINGNIBELUNGEN, "RINGNIBELUNGEN", "Ring_of_Nibelugen" } ,
 { BD_ROKISWEIL, "ROKISWEIL", "Loki's_Wail" } ,
 { BD_SIEGFRIED, "SIEGFRIED", "Invulnerable_Siegfried" } ,
 { BS_ADRENALINE, "ADRENALINE", "Adrenaline_Rush" } ,
 { BS_ADRENALINE2, "ADRENALINE2", "Adrenaline Rush 2" } ,
 { BS_AXE, "AXE", "Smith_Axe" } ,
 { BS_DAGGER, "DAGGER", "Smith_Dagger" } ,
 { BS_ENCHANTEDSTONE, "ENCHANTEDSTONE", "Enchantedstone_Craft" } ,
 { BS_FINDINGORE, "FINDINGORE", "Ore_Discovery" } ,
 { BS_HAMMERFALL, "HAMMERFALL", "Hammer_Fall" } ,
 { BS_HILTBINDING, "HILTBINDING", "Hilt_Binding" } ,
 { BS_IRON, "IRON", "Iron_Tempering" } ,
 { BS_KNUCKLE, "KNUCKLE", "Smith_Knucklebrace" } ,
 { BS_MACE, "MACE", "Smith_Mace" } ,
 { BS_MAXIMIZE, "MAXIMIZE", "Power_Maximize" } ,
 { BS_ORIDEOCON, "ORIDEOCON", "Orideocon_Research" } ,
 { BS_OVERTHRUST, "OVERTHRUST", "Power-Thrust" } ,
 { BS_REPAIRWEAPON, "REPAIRWEAPON", "Weapon_Repair" } ,
 { BS_SKINTEMPER, "SKINTEMPER", "Skin_Tempering" } ,
 { BS_SPEAR, "SPEAR", "Smith_Spear" } ,
 { BS_STEEL, "STEEL", "Steel_Tempering" } ,
 { BS_SWORD, "SWORD", "Smith_Sword" } ,
 { BS_TWOHANDSWORD, "TWOHANDSWORD", "Smith_Two-handed_Sword" } ,
 { BS_WEAPONPERFECT, "WEAPONPERFECT", "Weapon_Perfection" } ,
 { BS_WEAPONRESEARCH, "WEAPONRESEARCH", "Weaponry_Research" } ,
 { CG_ARROWVULCAN, "ARROWVULCAN", "Vulcan_Arrow" } ,
 { CG_HERMODE, "HERMODE", "Wand of Hermode" } ,
 { CG_LONGINGFREEDOM, "LONGINGFREEDOM", "Longing_for_Freedom" } ,
 { CG_MARIONETTE, "MARIONETTE", "Marionette_Control" } ,
 { CG_MOONLIT, "MOONLIT", "Moonlight_Petals" } ,
 { CG_TAROTCARD, "TAROTCARD", "Tarot_Card_of_Fate" } ,
 { CH_CHAINCRUSH, "CHAINCRUSH", "Chain_Crush_Combo" } ,
 { CH_PALMSTRIKE, "PALMSTRIKE", "Palm_Push_Strike" } ,
 { CH_SOULCOLLECT, "SOULCOLLECT", "Collect_Soul" } ,
 { CH_TIGERFIST, "TIGERFIST", "Tiger_Knuckle_Fist" } ,
 { CR_ACIDDEMONSTRATION, "CR_ACIDDEMONSTRATION", "Acid_Demonstration" } ,
 { CR_ALCHEMY, "ALCHEMY", "Alchemy" } ,
 { CR_SLIMPITCHER, "SLIMPITCHER", "Slim_Pitcher" } ,
 { CR_FULLPROTECTION, "FULLPROTECTION", "Full_Chemical_Protection" } ,
 { CR_AUTOGUARD, "AUTOGUARD", "Guard" } ,
 { CR_DEFENDER, "DEFENDER", "Defender" } ,
 { CR_DEVOTION, "DEVOTION", "Sacrifice" } ,
 { CR_GRANDCROSS, "GRANDCROSS", "Grand_Cross" } ,
 { CR_HOLYCROSS, "HOLYCROSS", "Holy_Cross" } ,
 { CR_PROVIDENCE, "PROVIDENCE", "Providence" } ,
 { CR_REFLECTSHIELD, "REFLECTSHIELD", "Shield_Reflect" } ,
 { CR_SHIELDBOOMERANG, "SHIELDBOOMERANG", "Shield_Boomerang" } ,
 { CR_SHIELDCHARGE, "SHIELDCHARGE", "Shield_Charge" } ,
 { CR_SPEARQUICKEN, "SPEARQUICKEN", "Spear_Quicken" } ,
 { CR_SYNTHESISPOTION, "SYNTHESISPOTION", "Potion_Synthesis" } ,
 { CR_TRUST, "TRUST", "Faith" } ,
 { DC_DANCINGLESSON, "DANCINGLESSON", "Dancing_Lesson" } ,
 { DC_DONTFORGETME, "DONTFORGETME", "Don't_Forget_Me" } ,
 { DC_FORTUNEKISS, "FORTUNEKISS", "Fortune_Kiss" } ,
 { DC_HUMMING, "HUMMING", "Humming" } ,
 { DC_SCREAM, "SCREAM", "Scream" } ,
 { DC_SERVICEFORYOU, "SERVICEFORYOU", "Prostitute" } ,
 { DC_THROWARROW, "THROWARROW", "Throw_Arrow" } ,
 { DC_UGLYDANCE, "UGLYDANCE", "Ugly_Dance" } ,
 { GD_BATTLEORDER, "BATTLEORDER", "Battle_Orders" } ,
 { GD_REGENERATION, "REGENERATION", "Regeneration" } ,
 { GD_RESTORE, "RESTORE", "Restore" } ,
 { GD_EMERGENCYCALL, "EMERGENCYCALL", "Emergency_Call" } ,
 { HP_ASSUMPTIO, "ASSUMPTIO", "Assumptio" } ,
 { HP_BASILICA, "BASILICA", "Basilica" } ,
 { HP_MANARECHARGE, "MANARECHARGE", "Mana_Recharge" } ,
 { HP_MEDITATIO, "MEDITATIO", "Meditation" } ,
 { HT_ANKLESNARE, "ANKLESNARE", "Ankle_Snare" } ,
 { HT_BEASTBANE, "BEASTBANE", "Beast_Bane" } ,
 { HT_BLASTMINE, "BLASTMINE", "Blast_Mine" } ,
 { HT_BLITZBEAT, "BLITZBEAT", "Blitz_Beat" } ,
 { HT_CLAYMORETRAP, "CLAYMORETRAP", "Claymore_Trap" } ,
 { HT_DETECTING, "DETECTING", "Detect" } ,
 { HT_FALCON, "FALCON", "Falconry_Mastery" } ,
 { HT_FLASHER, "FLASHER", "Flasher" } ,
 { HT_FREEZINGTRAP, "FREEZINGTRAP", "Freezing_Trap" } ,
 { HT_LANDMINE, "LANDMINE", "Land_Mine" } ,
 { HT_REMOVETRAP, "REMOVETRAP", "Remove_Trap" } ,
 { HT_SANDMAN, "SANDMAN", "Sandman" } ,
 { HT_SHOCKWAVE, "SHOCKWAVE", "Shockwave_Trap" } ,
 { HT_SKIDTRAP, "SKIDTRAP", "Skid_Trap" } ,
 { HT_SPRINGTRAP, "SPRINGTRAP", "Spring_Trap" } ,
 { HT_STEELCROW, "STEELCROW", "Steel_Crow" } ,
 { HT_TALKIEBOX, "TALKIEBOX", "Talkie_Box" } ,
 { HT_PHANTASMIC, "PHANTASMIC", "Phantasmic_Arrow" } ,
 { HW_GANBANTEIN, "GANBANTEIN", "Ganbantein" } ,
 { HW_GRAVITATION, "GRAVITATION", "Gravitation_Field" } ,
 { HW_MAGICCRASHER, "MAGICCRASHER", "Magic_Crasher" } ,
 { HW_MAGICPOWER, "MAGICPOWER", "Magic_Power" } ,
 { HW_NAPALMVULCAN, "NAPALMVULCAN", "Napalm_Vulcan" } ,
 { HW_SOULDRAIN, "SOULDRAIN", "Soul_Drain" } ,
 { ITM_TOMAHAWK, "TOMAHAWK", "Throw_Tomahawk" } ,
 { KN_AUTOCOUNTER, "AUTOCOUNTER", "Counter_Attack" } ,
 { KN_BOWLINGBASH, "BOWLINGBASH", "Bowling_Bash" } ,
 { KN_BRANDISHSPEAR, "BRANDISHSPEAR", "Brandish_Spear" } ,
 { KN_CAVALIERMASTERY, "CAVALIERMASTERY", "Cavalier_Mastery" } ,
 { KN_CHARGEATK, "CHARGEATK", "Charge_Attack" } ,
 { KN_PIERCE, "PIERCE", "Pierce" } ,
 { KN_RIDING, "RIDING", "Peco_Peco_Ride" } ,
 { KN_SPEARBOOMERANG, "SPEARBOOMERANG", "Spear_Boomerang" } ,
 { KN_SPEARMASTERY, "SPEARMASTERY", "Spear_Mastery" } ,
 { KN_SPEARSTAB, "SPEARSTAB", "Spear_Stab" } ,
 { KN_TWOHANDQUICKEN, "TWOHANDQUICKEN", "Twohand_Quicken" } ,
 { LK_AURABLADE, "AURABLADE", "Aura_Blade" } ,
 { LK_BERSERK, "BERSERK", "Berserk" } ,
 { LK_CONCENTRATION, "CONCENTRATION", "Concentration" } ,
 { LK_FURY, "FURY", "LK_FURY" } ,
 { LK_HEADCRUSH, "HEADCRUSH", "Head_Crusher" } ,
 { LK_JOINTBEAT, "JOINTBEAT", "Joint_Beat" } ,
 { LK_PARRYING, "PARRYING", "Parrying" } ,
 { LK_SPIRALPIERCE, "SPIRALPIERCE", "Spiral_Pierce" } ,
 { LK_TENSIONRELAX, "TENSIONRELAX", "Tension_Relax" } ,
 { MC_CARTREVOLUTION, "CARTREVOLUTION", "Cart_Revolution" } ,
 { MC_CHANGECART, "CHANGECART", "Change_Cart" } ,
 { MC_DISCOUNT, "DISCOUNT", "Discount" } ,
 { MC_IDENTIFY, "IDENTIFY", "Item_Appraisal" } ,
 { MC_INCCARRY, "INCCARRY", "Enlarge_Weight_Limit" } ,
 { MC_LOUD, "LOUD", "Lord_Exclamation" } ,
 { MC_MAMMONITE, "MAMMONITE", "Mammonite" } ,
 { MC_OVERCHARGE, "OVERCHARGE", "Overcharge" } ,
 { MC_PUSHCART, "PUSHCART", "Pushcart" } ,
 { MC_VENDING, "VENDING", "Vending" } ,
 { MG_COLDBOLT, "COLDBOLT", "Cold_Bolt" } ,
 { MG_ENERGYCOAT, "ENERGYCOAT", "Energy_Coat" } ,
 { MG_FIREBALL, "FIREBALL", "Fire_Ball" } ,
 { MG_FIREBOLT, "FIREBOLT", "Fire_Bolt" } ,
 { MG_FIREWALL, "FIREWALL", "Fire_Wall" } ,
 { MG_FROSTDIVER, "FROSTDIVER", "Frost_Diver" } ,
 { MG_LIGHTNINGBOLT, "LIGHTNINGBOLT", "Lightening_Bolt" } ,
 { MG_NAPALMBEAT, "NAPALMBEAT", "Napalm_Beat" } ,
 { MG_SAFETYWALL, "SAFETYWALL", "Safety_Wall" } ,
 { MG_SIGHT, "SIGHT", "Sight" } ,
 { MG_SOULSTRIKE, "SOULSTRIKE", "Soul_Strike" } ,
 { MG_SRECOVERY, "SRECOVERY", "Increase_SP_Recovery" } ,
 { MG_STONECURSE, "STONECURSE", "Stone_Curse" } ,
 { MG_THUNDERSTORM, "THUNDERSTORM", "Thunderstorm" } ,
 { MO_ABSORBSPIRITS, "ABSORBSPIRITS", "Absorb_Spirits" } ,
 { MO_BLADESTOP, "BLADESTOP", "Blade_Stop" } ,
 { MO_BODYRELOCATION, "BODYRELOCATION", "Body_Relocation" } ,
 { MO_CALLSPIRITS, "CALLSPIRITS", "Call_Spirits" } ,
 { MO_CHAINCOMBO, "CHAINCOMBO", "Chain_Combo" } ,
 { MO_COMBOFINISH, "COMBOFINISH", "Combo_Finish" } ,
 { MO_DODGE, "DODGE", "Dodge" } ,
 { MO_EXPLOSIONSPIRITS, "EXPLOSIONSPIRITS", "Explosion_Spirits" } ,
 { MO_EXTREMITYFIST, "EXTREMITYFIST", "Extremity_Fist" } ,
 { MO_FINGEROFFENSIVE, "FINGEROFFENSIVE", "Finger_Offensive" } ,
 { MO_INVESTIGATE, "INVESTIGATE", "Investigate" } ,
 { MO_IRONHAND, "IRONHAND", "Iron_Hand" } ,
 { MO_SPIRITSRECOVERY, "SPIRITSRECOVERY", "Spirit_Recovery" } ,
 { MO_STEELBODY, "STEELBODY", "Steel_Body" } ,
 { MO_TRIPLEATTACK, "TRIPLEATTACK", "Triple_Blows" } ,
 { NPC_ATTRICHANGE, "ATTRICHANGE", "NPC_ATTRICHANGE" } ,
 { NPC_BARRIER, "BARRIER", "NPC_BARRIER" } ,
 { NPC_BLINDATTACK, "BLINDATTACK", "NPC_BLINDATTACK" } ,
 { NPC_BLOODDRAIN, "BLOODDRAIN", "NPC_BLOODDRAIN" } ,
 { NPC_CHANGEDARKNESS, "CHANGEDARKNESS", "NPC_CHANGEDARKNESS" } ,
 { NPC_CHANGEFIRE, "CHANGEFIRE", "NPC_CHANGEFIRE" } ,
 { NPC_CHANGEGROUND, "CHANGEGROUND", "NPC_CHANGEGROUND" } ,
 { NPC_CHANGEHOLY, "CHANGEHOLY", "NPC_CHANGEHOLY" } ,
 { NPC_CHANGEPOISON, "CHANGEPOISON", "NPC_CHANGEPOISON" } ,
 { NPC_CHANGETELEKINESIS, "CHANGETELEKINESIS", "NPC_CHANGETELEKINESIS" } ,
 { NPC_CHANGEWATER, "CHANGEWATER", "NPC_CHANGEWATER" } ,
 { NPC_CHANGEWIND, "CHANGEWIND", "NPC_CHANGEWIND" } ,
 { NPC_COMBOATTACK, "COMBOATTACK", "NPC_COMBOATTACK" } ,
 { NPC_CRITICALSLASH, "CRITICALSLASH", "NPC_CRITICALSLASH" } ,
 { NPC_CURSEATTACK, "CURSEATTACK", "NPC_CURSEATTACK" } ,
 { NPC_DARKBLESSING, "DARKBLESSING", "NPC_DARKBLESSING" } ,
 { NPC_DARKBREATH, "DARKBREATH", "NPC_DARKBREATH" } ,
 { NPC_DARKCROSS, "DARKCROSS", "NPC_DARKCROSS" } ,
 { NPC_DARKNESSATTACK, "DARKNESSATTACK", "NPC_DARKNESSATTACK" } ,
 { NPC_DEFENDER, "DEFENDER", "NPC_DEFENDER" } ,
 { NPC_EMOTION, "EMOTION", "NPC_EMOTION" } ,
 { NPC_ENERGYDRAIN, "ENERGYDRAIN", "NPC_ENERGYDRAIN" } ,
 { NPC_FIREATTACK, "FIREATTACK", "NPC_FIREATTACK" } ,
 { NPC_GROUNDATTACK, "GROUNDATTACK", "NPC_GROUNDATTACK" } ,
 { NPC_GUIDEDATTACK, "GUIDEDATTACK", "NPC_GUIDEDATTACK" } ,
 { NPC_HALLUCINATION, "HALLUCINATION", "NPC_HALLUCINATION" } ,
 { NPC_HOLYATTACK, "HOLYATTACK", "NPC_HOLYATTACK" } ,
 { NPC_KEEPING, "KEEPING", "NPC_KEEPING" } ,
 { NPC_LICK, "LICK", "NPC_LICK" } ,
 { NPC_MAGICALATTACK, "MAGICALATTACK", "NPC_MAGICALATTACK" } ,
 { NPC_MENTALBREAKER, "MENTALBREAKER", "NPC_MENTALBREAKER" } ,
 { NPC_METAMORPHOSIS, "METAMORPHOSIS", "NPC_METAMORPHOSIS" } ,
 { NPC_PETRIFYATTACK, "PETRIFYATTACK", "NPC_PETRIFYATTACK" } ,
 { NPC_PIERCINGATT, "PIERCINGATT", "NPC_PIERCINGATT" } ,
 { NPC_CALLSLAVE, "CALLSLAVE", "NPC_CALLSLAVE" },
 { NPC_POISON, "POISON", "NPC_POISON" } ,
 { NPC_POISONATTACK, "POISONATTACK", "NPC_POISONATTACK" } ,
 { NPC_PROVOCATION, "PROVOCATION", "NPC_PROVOCATION" } ,
 { NPC_RANDOMATTACK, "RANDOMATTACK", "NPC_RANDOMATTACK" } ,
 { NPC_RANGEATTACK, "RANGEATTACK", "NPC_RANGEATTACK" } ,
 { NPC_REBIRTH, "REBIRTH", "NPC_REBIRTH" } ,
 { NPC_SELFDESTRUCTION, "SELFDESTRUCTION", "Kabooooom!" } ,
 { NPC_SILENCEATTACK, "SILENCEATTACK", "NPC_SILENCEATTACK" } ,
 { NPC_SLEEPATTACK, "SLEEPATTACK", "NPC_SLEEPATTACK" } ,
 { NPC_SMOKING, "SMOKING", "NPC_SMOKING" } ,
 { NPC_SPLASHATTACK, "SPLASHATTACK", "NPC_SPLASHATTACK" } ,
 { NPC_STUNATTACK, "STUNATTACK", "NPC_STUNATTACK" } ,
 { NPC_SUICIDE, "SUICIDE", "NPC_SUICIDE" } ,
 { NPC_SUMMONMONSTER, "SUMMONMONSTER", "NPC_SUMMONMONSTER" } ,
 { NPC_SUMMONSLAVE, "SUMMONSLAVE", "NPC_SUMMONSLAVE" } ,
 { NPC_TELEKINESISATTACK, "TELEKINESISATTACK", "NPC_TELEKINESISATTACK" } ,
 { NPC_TRANSFORMATION, "TRANSFORMATION", "NPC_TRANSFORMATION" } ,
 { NPC_WATERATTACK, "WATERATTACK", "NPC_WATERATTACK" } ,
 { NPC_WINDATTACK, "WINDATTACK", "NPC_WINDATTACK" } ,
 { NV_BASIC, "BASIC", "Basic_Skill" } ,
 { NV_FIRSTAID, "FIRSTAID", "First Aid" } ,
 { NV_TRICKDEAD, "TRICKDEAD", "Play_Dead" } ,
 { PA_GOSPEL, "GOSPEL", "Gospel" } ,
 { PA_PRESSURE, "PRESSURE", "Pressure" } ,
 { PA_SACRIFICE, "SACRIFICE", "Sacrificial_Ritual" } ,
 { PA_SHIELDCHAIN, "PA_SHIELDCHAIN", "Shield_Chain" } ,
 { PF_FOGWALL, "FOGWALL", "Wall_of_Fog" } ,
 { PF_HPCONVERSION, "HPCONVERSION", "Health_Conversion" } ,
 { PF_MEMORIZE, "MEMORIZE", "Memorize" } ,
 { PF_MINDBREAKER, "MINDBREAKER", "Mind_Breaker" } ,
 { PF_SOULBURN, "SOULBURN", "Soul_Burn" } ,
 { PF_SOULCHANGE, "SOULCHANGE", "Soul_Change" } ,
 { PF_SPIDERWEB, "SPIDERWEB", "Spider_Web" } ,
 { PF_DOUBLECASTING, "DOUBLECASTING", "Double_Casting" } ,
 { PR_ASPERSIO, "ASPERSIO", "Aspersio" } ,
 { PR_BENEDICTIO, "BENEDICTIO", "B.S_Sacramenti" } ,
 { PR_GLORIA, "GLORIA", "Gloria" } ,
 { PR_IMPOSITIO, "IMPOSITIO", "Impositio_Manus" } ,
 { PR_KYRIE, "KYRIE", "Kyrie_Eleison" } ,
 { PR_LEXAETERNA, "LEXAETERNA", "Lex_Aeterna" } ,
 { PR_LEXDIVINA, "LEXDIVINA", "Lex_Divina" } ,
 { PR_MACEMASTERY, "MACEMASTERY", "Mace_Mastery" } ,
 { PR_MAGNIFICAT, "MAGNIFICAT", "Magnificat" } ,
 { PR_MAGNUS, "MAGNUS", "Magnus_Exorcismus" } ,
 { PR_REDEMPTIO, "REDEMPTIO", "Redemptio" } ,
 { PR_SANCTUARY, "SANCTUARY", "Santuary" } ,
 { PR_SLOWPOISON, "SLOWPOISON", "Slow_Poison" } ,
 { PR_STRECOVERY, "STRECOVERY", "Status_Recovery" } ,
 { PR_SUFFRAGIUM, "SUFFRAGIUM", "Suffragium" } ,
 { PR_TURNUNDEAD, "TURNUNDEAD", "Turn_Undead" } ,
 { RG_BACKSTAP, "BACKSTAP", "Back_Stab" } ,
 { RG_CLEANER, "CLEANER", "Remover" } ,
 { RG_COMPULSION, "COMPULSION", "Compulsion_Discount" } ,
 { RG_FLAGGRAFFITI, "FLAGGRAFFITI", "Flag_Graffity" } ,
 { RG_GANGSTER, "GANGSTER", "Gangster's_Paradise" } ,
 { RG_GRAFFITI, "GRAFFITI", "Graffiti" } ,
 { RG_INTIMIDATE, "INTIMIDATE", "Intimidate" } ,
 { RG_PLAGIARISM, "PLAGIARISM", "Plagiarism" } ,
 { RG_RAID, "RAID", "Raid" } ,
 { RG_SNATCHER, "SNATCHER", "Snatcher" } ,
 { RG_STEALCOIN, "STEALCOIN", "Steal_Coin" } ,
 { RG_STRIPARMOR, "STRIPARMOR", "Strip_Armor" } ,
 { RG_STRIPHELM, "STRIPHELM", "Strip_Helm" } ,
 { RG_STRIPSHIELD, "STRIPSHIELD", "Strip_Shield" } ,
 { RG_STRIPWEAPON, "STRIPWEAPON", "Strip_Weapon" } ,
 { RG_TUNNELDRIVE, "TUNNELDRIVE", "Tunnel_Drive" } ,
 { SA_ABRACADABRA, "ABRACADABRA", "Hocus-pocus" } ,
 { SA_ADVANCEDBOOK, "ADVANCEDBOOK", "Advanced_Book" } ,
 { SA_AUTOSPELL, "AUTOSPELL", "Auto_Cast" } ,
 { SA_CASTCANCEL, "CASTCANCEL", "Cast_Cancel" } ,
 { SA_CLASSCHANGE, "CLASSCHANGE", "Class_Change" } ,
 { SA_COMA, "COMA", "Coma" } ,
 { SA_DEATH, "DEATH", "Death" } ,
 { SA_DELUGE, "DELUGE", "Deluge" } ,
 { SA_DISPELL, "DISPELL", "Dispel" } ,
 { SA_DRAGONOLOGY, "DRAGONOLOGY", "Dragonology" } ,
 { SA_FLAMELAUNCHER, "FLAMELAUNCHER", "Flame_Launcher" } ,
 { SA_FORTUNE, "FORTUNE", "Fortune" } ,
 { SA_FREECAST, "FREECAST", "Cast_Freedom" } ,
 { SA_FROSTWEAPON, "FROSTWEAPON", "Frost_Weapon" } ,
 { SA_FULLRECOVERY, "FULLRECOVERY", "Full_Recovery" } ,
 { SA_GRAVITY, "GRAVITY", "Gravity" } ,
 { SA_INSTANTDEATH, "INSTANTDEATH", "Instant_Death" } ,
 { SA_LANDPROTECTOR, "LANDPROTECTOR", "Land_Protector" } ,
 { SA_LEVELUP, "LEVELUP", "Level_Up" } ,
 { SA_LIGHTNINGLOADER, "LIGHTNINGLOADER", "Lightning_Loader" } ,
 { SA_MAGICROD, "MAGICROD", "Magic_Rod" } ,
 { SA_MONOCELL, "MONOCELL", "Monocell" } ,
 { SA_QUESTION, "QUESTION", "Question?" } ,
 { SA_REVERSEORCISH, "REVERSEORCISH", "Reverse_Orcish" } ,
 { SA_SEISMICWEAPON, "SEISMICWEAPON", "Seismic_Weapon" } ,
 { SA_SPELLBREAKER, "SPELLBREAKER", "Break_Spell" } ,
 { SA_SUMMONMONSTER, "SUMMONMONSTER", "Summon_Monster" } ,
 { SA_TAMINGMONSTER, "TAMINGMONSTER", "Taming_Monster" } ,
 { SA_VIOLENTGALE, "VIOLENTGALE", "Violent_Gale" } ,
 { SA_VOLCANO, "VOLCANO", "Volcano" } ,
 { SG_DEVIL, "DEVIL", "Devil" } ,
 { SG_FEEL, "FEEL", "Feel" } ,
 { SG_FRIEND, "FRIEND", "Friend" } ,
 { SG_FUSION, "FUSION", "Fusion" } ,
 { SG_HATE, "HATE", "Hate" } ,
 { SG_KNOWLEDGE, "KNOWLEDGE", "Knowledge" } ,
 { SG_MOON_ANGER, "ANGER", "Moon Anger" } ,
 { SG_MOON_BLESS, "BLESS", "Moon Bless" } ,
 { SG_MOON_COMFORT, "COMFORT", "Moon Comfort" } ,
 { SG_MOON_WARM, "WARM", "Moon Warm" } ,
 { SG_STAR_ANGER, "ANGER", "Star Anger" } ,
 { SG_STAR_BLESS, "BLESS", "Star Bless" } ,
 { SG_STAR_COMFORT, "COMFORT", "Star Comfort" } ,
 { SG_STAR_WARM, "WARM", "Star Warm" } ,
 { SG_SUN_ANGER, "ANGER", "Sun Anger" } ,
 { SG_SUN_BLESS, "BLESS", "Sun Bless" } ,
 { SG_SUN_COMFORT, "COMFORT", "Sun Comfort" } ,
 { SG_SUN_WARM, "WARM", "Sun Warm" } ,
 { SL_ALCHEMIST, "ALCHEMIST", "Alchemist" } ,
 { SL_ASSASIN, "ASSASIN", "Assasin" } ,
 { SL_BARDDANCER, "BARDDANCER", "Bard Dancer" } ,
 { SL_BLACKSMITH, "BLACKSMITH", "Black Smith" } ,
 { SL_CRUSADER, "CRUSADER", "Crusader" } ,
 { SL_HUNTER, "HUNTER", "Hunter" } ,
 { SL_KAAHI, "KAAHI", "Kaahi" } ,
 { SL_KAINA, "KAINA", "Kaina" } ,
 { SL_KAITE, "KAITE", "Kaite" } ,
 { SL_KAIZEL, "KAIZEL", "Kaizel" } ,
 { SL_KAUPE, "KAUPE", "Kaupe" } ,
 { SL_KNIGHT, "KNIGHT", "Knight" } ,
 { SL_MONK, "MONK", "Monk" } ,
 { SL_PRIEST, "PRIEST", "Priest" } ,
 { SL_ROGUE, "ROGUE", "Rogue" } ,
 { SL_SAGE, "SAGE", "Sage" } ,
 { SL_SKA, "SKA", "SKA" } ,
 { SL_SKE, "SKE", "SKE" } ,
 { SL_SMA, "SMA", "SMA" } ,
 { SL_SOULLINKER, "SOULLINKER", "Soul Linker" } ,
 { SL_STAR, "STAR", "Star" } ,
 { SL_STIN, "STIN", "Stin" } ,
 { SL_STUN, "STUN", "Stun" } ,
 { SL_SUPERNOVICE, "SUPERNOVICE", "Super Novice" } ,
 { SL_SWOO, "SWOO", "Swoo" } ,
 { SL_WIZARD, "WIZARD", "Wizard" } ,
 { SM_AUTOBERSERK, "AUTOBERSERK", "Auto_Berserk" } ,
 { SM_BASH, "BASH", "Bash" } ,
 { SM_ENDURE, "ENDURE", "Endure" } ,
 { SM_FATALBLOW, "FATALBLOW", "Attack_Weak_Point" } ,
 { SM_MAGNUM, "MAGNUM", "Magnum_Break" } ,
 { SM_MOVINGRECOVERY, "MOVINGRECOVERY", "Moving_HP_Recovery" } ,
 { SM_PROVOKE, "PROVOKE", "Provoke" } ,
 { SM_RECOVERY, "RECOVERY", "Increase_HP_Recovery" } ,
 { SM_SWORD, "SWORD", "Sword_Mastery" } ,
 { SM_TWOHAND, "TWOHAND", "Two-Handed_Sword_Mastery" } ,
 { SN_FALCONASSAULT, "FALCONASSAULT", "Falcon_Assault" } ,
 { SN_SHARPSHOOTING, "SHARPSHOOTING", "Sharpshooting" } ,
 { SN_SIGHT, "SIGHT", "True_Sight" } ,
 { SN_WINDWALK, "WINDWALK", "Wind_Walk" } ,
 { ST_CHASEWALK, "CHASEWALK", "Chase_Walk" } ,
 { ST_REJECTSWORD, "REJECTSWORD", "Reject_Sword" } ,
 { ST_STEALBACKPACK, "STEALBACKPACK", "Steal_Backpack" } ,
 { ST_PRESERVE, "PRESERVE", "Preserve" } ,
 { ST_FULLSTRIP, "FULLSTRIP", "Full_Strip" } ,
 { TF_BACKSLIDING, "BACKSLIDING", "Back_Sliding" } ,
 { TF_DETOXIFY, "DETOXIFY", "Detoxify" } ,
 { TF_DOUBLE, "DOUBLE", "Double_Attack" } ,
 { TF_HIDING, "HIDING", "Hiding" } ,
 { TF_MISS, "MISS", "Improve_Dodge" } ,
 { TF_PICKSTONE, "PICKSTONE", "Take_Stone" } ,
 { TF_POISON, "POISON", "Envenom" } ,
 { TF_SPRINKLESAND, "SPRINKLESAND", "Throw_Sand" } ,
 { TF_STEAL, "STEAL", "Steal" } ,
 { TF_THROWSTONE, "THROWSTONE", "Throw_Stone" } ,
 { TK_COUNTER, "COUNTER", "Counter" } ,
 { TK_DODGE, "DODGE", "Dodge" } ,
 { TK_DOWNKICK, "DOWNKICK", "Down Kick" } ,
 { TK_HIGHJUMP, "HIGHJUMP", "High Jump" } ,
 { TK_HPTIME, "HPTIME", "HP Time" } ,
 { TK_JUMPKICK, "JUMPKICK", "Jump Kick" } ,
 { TK_POWER, "POWER", "Power" } ,
 { TK_READYCOUNTER, "READYCOUNTER", "Ready Counter" } ,
 { TK_READYDOWN, "READYDOWN", "Ready Down" } ,
 { TK_READYSTORM, "READYSTORM", "Ready Storm" } ,
 { TK_READYTURN, "READYTURN", "Ready Turn" } ,
 { TK_RUN, "RUN", "TK_RUN" } ,
 { TK_SEVENWIND, "SEVENWIND", "Seven Wind" } ,
 { TK_SPTIME, "SPTIME", "SP Time" } ,
 { TK_STORMKICK, "STORMKICK", "Storm Kick" } ,
 { TK_TURNKICK, "TURNKICK", "Turn Kick" } ,
 { WE_BABY, "BABY", "Adopt_Baby" } ,
 { WE_CALLBABY, "CALLBABY", "Call_Baby" } ,
 { WE_CALLPARENT, "CALLPARENT", "Call_Parent" } ,
 { WE_CALLPARTNER, "CALLPARTNER", "I Want to See You" } ,
 { WE_FEMALE, "FEMALE", "I Only Look Up to You" } ,
 { WE_MALE, "MALE", "I Will Protect You" } ,
 { WS_CARTBOOST, "CARTBOOST", "Cart_Boost" } ,
 { WS_CARTTERMINATION, "WS_CARTTERMINATION", "Cart_Termination" } ,
 { WS_CREATECOIN, "CREATECOIN", "Create_Coins" } ,
 { WS_CREATENUGGET, "CREATENUGGET", "Create_Nuggets" } ,
 { WS_MELTDOWN, "MELTDOWN", "Meltdown" } ,
 { WS_MAXOVERTHRUST, "MAXOVERTHRUST", "Max_Overthrust" } ,
 { WS_SYSTEMCREATE, "SYSTEMCREATE", "Create_System_tower" } ,
 { WS_WEAPONREFINE, "WEAPONREFINE", "Weapon_Refine" } ,
 { WZ_EARTHSPIKE, "EARTHSPIKE", "Earth_Spike" } ,
 { WZ_ESTIMATION, "ESTIMATION", "Sense" } ,
 { WZ_FIREIVY, "FIREIVY", "Fire_Ivy" } ,
 { WZ_FIREPILLAR, "FIREPILLAR", "Fire_Pillar" } ,
 { WZ_FROSTNOVA, "FROSTNOVA", "Frost_Nova" } ,
 { WZ_HEAVENDRIVE, "HEAVENDRIVE", "Heaven's_Drive" } ,
 { WZ_ICEWALL, "ICEWALL", "Ice_Wall" } ,
 { WZ_JUPITEL, "JUPITEL", "Jupitel_Thunder" } ,
 { WZ_METEOR, "METEOR", "Meteor_Storm" } ,
 { WZ_QUAGMIRE, "QUAGMIRE", "Quagmire" } ,
 { WZ_SIGHTRASHER, "SIGHTRASHER", "Sightrasher" } ,
 { WZ_STORMGUST, "STORMGUST", "Storm_Gust" } ,
 { WZ_VERMILION, "VERMILION", "Lord_of_Vermilion" } ,
 { WZ_WATERBALL, "WATERBALL", "Water_Ball" } ,
 { 0, 0, 0 }
};

static const int dirx[8]={0,-1,-1,-1,0,1,1,1};
static const int diry[8]={1,1,0,-1,-1,-1,0,1};

static int rdamage;

struct skill_db skill_db[MAX_SKILL_DB];

struct skill_produce_db skill_produce_db[MAX_SKILL_PRODUCE_DB];

short num_skill_arrow_db = 0;
struct skill_arrow_db *skill_arrow_db = NULL;

struct skill_abra_db skill_abra_db[MAX_SKILL_ABRA_DB];

// macros to check for out of bounds errors
// i: Skill ID, l: Skill Level, var: Value to return after checking
// for values that don't require level use #define xxxx2
// macros with level
#define skill_chk(i, l) \
	if (i >= 10000 && i < 10015) i -= 9500; \
	if (i < 1 || i > MAX_SKILL_DB) return 0; \
	if (l <= 0 || l > MAX_SKILL_LEVEL) return 0
#define skill_get(var, i, l) \
	skill_chk(i, l); return var
// macros without level
#define skill_chk2(i) \
	if (i >= 10000 && i < 10015) i -= 9500; \
	if (i < 1 || i > MAX_SKILL_DB) return 0
#define skill_get2(var, i) \
	skill_chk2(i); return var

// Skill DB
int skill_get_hit(int id) { skill_get2(skill_db[id].hit, id); }
int skill_get_inf(int id) { skill_chk2(id); return (id < MAX_SKILL) ? skill_db[id].inf : guild_skill_get_inf(id); }
int skill_get_pl(int id) { skill_get2(skill_db[id].pl, id); }
int skill_get_nk(int id) { skill_get2(skill_db[id].nk, id); }
int skill_get_max(int id) { skill_chk2(id); return (id < MAX_SKILL) ? skill_db[id].max : guild_skill_get_max(id); }
int skill_get_range(int id, int lv) { skill_chk(id, lv); return (id < MAX_SKILL) ? skill_db[id].range[lv-1] : guild_skill_get_range(id); }
int skill_get_hp(int id, int lv) { skill_get(skill_db[id].hp[lv-1], id, lv); }
int skill_get_sp(int id, int lv) { skill_get(skill_db[id].sp[lv-1], id, lv); }
int skill_get_zeny(int id, int lv) { skill_get(skill_db[id].zeny[lv-1], id, lv); }
int skill_get_num(int id, int lv) { skill_get(skill_db[id].num[lv-1], id, lv); }
int skill_get_cast(int id, int lv) { skill_get(skill_db[id].cast[lv-1], id, lv); }
int skill_get_delay(int id, int lv) { skill_get(skill_db[id].delay[lv-1], id, lv); }
int skill_get_time(int id, int lv) { skill_get(skill_db[id].upkeep_time[lv-1], id, lv); }
int skill_get_time2(int id, int lv) { skill_get(skill_db[id].upkeep_time2[lv-1], id, lv); }
int skill_get_castdef(int id) { skill_get2(skill_db[id].cast_def_rate, id); }
int skill_get_weapontype(int id) { skill_get2(skill_db[id].weapon, id); }
int skill_get_inf2(int id) { skill_get2(skill_db[id].inf2, id); }
int skill_get_castcancel(int id) { skill_get2(skill_db[id].castcancel, id); }
int skill_get_maxcount(int id) { skill_get2(skill_db[id].maxcount, id); }
int skill_get_blewcount(int id, int lv) { skill_get(skill_db[id].blewcount[lv-1], id, lv); }
int skill_get_mhp(int id, int lv) { skill_get(skill_db[id].mhp[lv-1], id, lv); }
int skill_get_castnodex(int id, int lv) { skill_get(skill_db[id].castnodex[lv-1], id, lv); }
int skill_get_delaynodex(int id, int lv) { skill_get(skill_db[id].delaynodex[lv-1], id, lv); }
int skill_get_nocast(int id) { skill_get2(skill_db[id].nocast, id); }
int skill_get_type(int id) { skill_get2(skill_db[id].skill_type, id); }
int skill_get_unit_id(int id, int flag) { skill_get(skill_db[id].unit_id[flag], id, 1); }
int skill_get_unit_layout_type(int id, int lv) { skill_get(skill_db[id].unit_layout_type[lv-1], id, lv); }
int skill_get_unit_interval(int id) { skill_get2(skill_db[id].unit_interval, id); }
int skill_get_unit_range(int id) { skill_get2(skill_db[id].unit_range, id); }
int skill_get_unit_target(int id) { skill_get2(skill_db[id].unit_target, id); }
int skill_get_unit_flag(int id) { skill_get2(skill_db[id].unit_flag, id); }

int skill_tree_get_max(int id, int b_class) {
	struct pc_base_job s_class = pc_calc_base_job(b_class);
	int i, skillid;
	for(i = 0; i < MAX_SKILL_TREE && (skillid = skill_tree[s_class.upper][s_class.job][i].id) > 0; i++)
		if (id == skillid)
			return skill_tree[s_class.upper][s_class.job][i].max;

	return skill_get_max(id);
}

int skill_check_condition(struct map_session_data *sd, int type);
int skill_castend_damage_id(struct block_list* src, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag);
int skill_frostjoke_scream(struct block_list *bl, va_list ap);
int skill_attack_area(struct block_list *bl, va_list ap);
int skill_abra_dataset(int skilllv);
int skill_landprotector(struct block_list *bl, va_list ap );
int skill_ganbatein(struct block_list *bl, va_list ap);
int skill_trap_splash(struct block_list *bl, va_list ap );
int skill_count_target(struct block_list *bl, va_list ap );
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl, struct skill_unit_group *sg, int tick);
int skill_unit_onplace(struct skill_unit *src, struct block_list *bl, unsigned int tick);
int skill_unit_effect(struct block_list *bl, va_list ap);

// [MouseJstr] - skill ok to cast? and when?
int skillnotok(int skillid, struct map_session_data *sd) {
	if (sd == NULL)
		return 1;

	if (!(skillid >= 10000 && skillid < 10015) &&
	    (skillid < 0 || skillid > MAX_SKILL))
		return 1;

  {
	int i = skillid;
	if (i >= 10000 && i < 10015)
		i -= 9500;
	if (sd->blockskill[i] > 0)
		return 1;
  }

	//if (sd->GM_level >= 20)
	if (battle_config.gm_skilluncond > 0 && sd->GM_level >= battle_config.gm_skilluncond)
		return 0; // gm's can do anything damn thing they want

	// Check skill restrictions [Celest]
	if(!map[sd->bl.m].flag.pvp && !map[sd->bl.m].flag.gvg && skill_get_nocast(skillid) & 1)
		return 1;
	if(map[sd->bl.m].flag.pvp && skill_get_nocast(skillid) & 2)
		return 1;
	if(map[sd->bl.m].flag.gvg && skill_get_nocast(skillid) & 4)
		return 1;
	if (agit_flag && skill_get_nocast(skillid) & 8) // 0: WoE not starting, Woe is running
		return 1;
	if (battle_config.pk_mode && map[sd->bl.m].flag.pvp && skill_get_nocast(skillid) & 16)
		return 1;
	if (map[sd->bl.m].flag.nospell && (skill_get_nocast(skillid) & 32) == 0) // the nospell condition
		return 1;

	switch (skillid)
	{
		case AL_WARP:
			return 0;
		case AL_TELEPORT:
			return 0;
		case MC_VENDING:
			return 0;
		case MC_IDENTIFY:
			return 0;
		default:
			return(map[sd->bl.m].flag.noskill);
	}
}


static int distance(int x0, int y_0, int x1, int y_1)
{
	int dx, dy;

	dx = abs(x0  -  x1);
	dy = abs(y_0 - y_1);

	return dx > dy ? dx : dy;
}

struct skill_unit_layout skill_unit_layout[MAX_SKILL_UNIT_LAYOUT];
int firewall_unit_pos;
int icewall_unit_pos;

struct skill_unit_layout *skill_get_unit_layout(int skillid, int skilllv, struct block_list *src, int x, int y) {

	int pos = skill_get_unit_layout_type(skillid, skilllv);
	int dir;

	if (pos != -1)
		return &skill_unit_layout[pos];

	if (src->x == x && src->y == y)
		dir = 2;
	else
		dir = map_calc_dir(src,x,y);

	if (skillid == MG_FIREWALL)
		return &skill_unit_layout[firewall_unit_pos+dir];
	else if (skillid == WZ_ICEWALL)
		return &skill_unit_layout[icewall_unit_pos+dir];

	printf("unknown unit layout for skill %d, %d\n", skillid, skilllv);

	return &skill_unit_layout[0];
}

/*==========================================
 * Skill Additional Effect
 *------------------------------------------
 */
int skill_additional_effect(struct block_list* src, struct block_list *bl, int skillid, int skilllv, int attack_type, unsigned int tick)
{
	const int sc[] = {
		SC_POISON, SC_BLIND, SC_SILENCE, SC_STAN,
		SC_STONE, SC_CURSE, SC_SLEEP
	};
	const int sc2[]={
		MG_STONECURSE,MG_FROSTDIVER,NPC_STUNATTACK,
		NPC_SLEEPATTACK,TF_POISON,NPC_CURSEATTACK,
		NPC_SILENCEATTACK,0,NPC_BLINDATTACK
	};

	struct map_session_data *sd = NULL;
	struct map_session_data *dstsd = NULL;
	struct mob_data *md = NULL;
	struct mob_data *dstmd = NULL;
	struct pet_data *pd = NULL;

	int skill, skill2;
	int rate;

	int sc_def_mdef,sc_def_vit,sc_def_int,sc_def_luk;
	int sc_def_mdef2,sc_def_vit2,sc_def_int2,sc_def_luk2;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);

	if(skillid > 0 && skilllv <= 0) return 0;

	switch(src->type){
	case BL_PC:
		nullpo_retr(0, sd = (struct map_session_data *)src);
		break;
	case BL_MOB:
		nullpo_retr(0, md = (struct mob_data *)src);
		break;
	case BL_PET:
		nullpo_retr(0, pd = (struct pet_data *)src);
		break;
	}

	switch(bl->type){
	case BL_PC:
		nullpo_retr(0, dstsd = (struct map_session_data *)bl);
		break;
	case BL_MOB:
		nullpo_retr(0, dstmd = (struct mob_data *)bl);
		break;
	}

	sc_def_mdef = status_get_sc_def_mdef(bl);
	sc_def_vit = status_get_sc_def_vit(bl);
	sc_def_int = status_get_sc_def_int(bl);
	sc_def_luk = status_get_sc_def_luk(bl);

	sc_def_mdef2 = status_get_sc_def_mdef(src);
	sc_def_vit2 = status_get_sc_def_vit(src);
	sc_def_int2 = status_get_sc_def_int(src);
	sc_def_luk2 = status_get_sc_def_luk(src);

	switch(skillid){
	case 0:
		if(sd && pc_isfalcon(sd) && sd->status.weapon == 11 && (skill=pc_checkskill(sd,HT_BLITZBEAT))>0 &&
			rand()%1000 <= sd->paramc[5]*10/3+1 ) {
			int lv=(sd->status.job_level+9)/10;
			skill_castend_damage_id(src,bl,HT_BLITZBEAT,(skill<lv)?skill:lv,tick,0xf00000);
		}
		if(sd && sd->status.weapon != 11 && (skill=pc_checkskill(sd,RG_SNATCHER)) > 0)
			if((skill*15 + 55) + (skill2 = pc_checkskill(sd,TF_STEAL))*10 > rand()%1000) {
				if(pc_steal_item(sd,bl))
					clif_skill_nodamage(src,bl,TF_STEAL,skill2,1);
				else if (battle_config.display_snatcher_skill_fail)
					clif_skill_fail(sd, skillid, 0, 0); // it's annoying! =p [Celest]
			}
		if (sd && sd->sc_data[SC_EDP].timer != -1 && rand() % 10000 < sd->sc_data[SC_EDP].val2 * sc_def_vit) {
			int mhp = status_get_max_hp(bl);
			int hp = status_get_hp(bl);
			int lvl = sd->sc_data[SC_EDP].val1;
			int diff;
			if (hp > mhp >> 2) {
				if (bl->type == BL_PC) {
					diff = mhp * 10 / 100;
					if (hp - diff < mhp >> 2)
						diff = hp - (mhp >> 2);
					pc_heal(dstsd, -hp, 0);
				} else if(bl->type == BL_MOB) {
					struct mob_data *md = (struct mob_data *)bl;
					hp -= mhp * 15 / 100;
					if (hp > mhp >> 2)
						md->hp = hp;
					else
						md->hp = mhp >> 2;
				}
			}
			status_change_start(bl,SC_DPOISON,lvl,0,0,0,skill_get_time2(ASC_EDP,lvl),0);
		}
		break;

	case SM_BASH:
		if(sd && (skill = pc_checkskill(sd,SM_FATALBLOW)) > 0 ) {
			if(rand()%100 < 6 * (skilllv - 5) * sc_def_vit / 100)
				status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(SM_FATALBLOW,skilllv),0);
		}
		break;

	case TF_POISON:
	case AS_SPLASHER:
		if(rand()%100 < (2 * skilllv + 10) * sc_def_vit / 100)
		{
			status_change_start(bl, SC_POISON, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
			break;
		}
		if(sd)
			clif_skill_fail(sd, skillid, 0, 0);
		break;

	case AS_SONICBLOW:
		if( rand()%100 < (2*skilllv+10)*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case HT_FREEZINGTRAP:
		rate=skilllv*3+35;
		if(rand()%100 < rate*sc_def_mdef/100)
			status_change_start(bl,SC_FREEZE,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case MG_FROSTDIVER:
	case WZ_FROSTNOVA:
		rate=(skilllv*3+35)*sc_def_mdef/100-(status_get_int(bl)+status_get_luk(bl))/15;
		rate=rate<=5?5:rate;
		if(rand()%100 < rate)
			status_change_start(bl,SC_FREEZE,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		else if (sd && skillid == MG_FROSTDIVER)
			clif_skill_fail(sd, skillid, 0, 0);
		break;

	case WZ_STORMGUST:
	  {
		struct status_change *sc_data = status_get_sc_data(bl);
		if (sc_data) {
			sc_data[SC_FREEZE].val3++;
			if (sc_data[SC_FREEZE].val3 >= 3)
				status_change_start(bl,SC_FREEZE,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		}
	  }
		break;

	case HT_LANDMINE:	
		if( rand()%100 < (5*skilllv+30)*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case HT_SHOCKWAVE:
		if(map[bl->m].flag.pvp && dstsd)
		{
			dstsd->status.sp -= dstsd->status.sp * (5 + 15 * skilllv) / 100;
			clif_updatestatus(dstsd, SP_SP);
		}
		break;
	case HT_SANDMAN:
		if( rand()%100 < (5*skilllv+30)*sc_def_int/100 )
			status_change_start(bl,SC_SLEEP,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case TF_SPRINKLESAND:
		if( rand()%100 < 20*sc_def_int/100 )
			status_change_start(bl,SC_BLIND,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case TF_THROWSTONE:
		if( rand()%100 < 7*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case CR_HOLYCROSS:
		if( rand()%100 < 3*skilllv*sc_def_int/100 )
			status_change_start(bl,SC_BLIND,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case CR_GRANDCROSS:
	case NPC_DARKGRANDCROSS:
	  {
		int race = status_get_race(bl);
		if ((battle_check_undead(race, status_get_elem_type(bl)) || race == 6) && rand() % 100 < 100000 * sc_def_int / 100) //�����t�^�������S�ϐ��ɂ͖���
			status_change_start(bl, SC_BLIND, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
	  }
		break;

	case AM_ACIDTERROR:
		if( rand()%100 < (skilllv*3)*sc_def_vit/100 )
			status_change_start(bl,SC_BLEEDING,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case CR_SHIELDCHARGE:
		if(rand()%100 < (15 + skilllv*5)*sc_def_vit/100)
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case RG_RAID:
		if( rand()%100 < (10+3*skilllv)*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		if( rand()%100 < (10+3*skilllv)*sc_def_int/100 )
			status_change_start(bl,SC_BLIND,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case BA_FROSTJOKE:
		if(rand()%100 < (15+5*skilllv)*sc_def_mdef/100)
			status_change_start(bl,SC_FREEZE,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case DC_SCREAM:
		if( rand()%100 < (25+5*skilllv)*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case BD_LULLABY:
		if( rand()%100 < 15*sc_def_int/100 )
			status_change_start(bl,SC_SLEEP,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case WS_CARTTERMINATION:
		if(rand()%100 < skilllv * 5 - (sc_def_vit /2))
			status_change_start(bl, SC_STAN, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		break;
	case CR_ACIDDEMONSTRATION:
		if(dstsd && battle_config.equipment_breaking)
		{
			if(rand()%100 < skilllv)
				pc_breakweapon(dstsd);
			if(rand()%100 < skilllv)
				pc_breakarmor(dstsd);
		}
		break;

	case NPC_PETRIFYATTACK:
		if(rand()%100 < sc_def_mdef)
			status_change_start(bl,sc[skillid-NPC_POISON],skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case NPC_POISON:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
		if(rand()%100 < sc_def_vit && src->type!=BL_PET)
			status_change_start(bl,sc[skillid-NPC_POISON],skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		if(src->type==BL_PET)
			status_change_start(bl,sc[skillid-NPC_POISON],skilllv,0,0,0,skilllv*1000,0);
		break;
	case NPC_CURSEATTACK:
		if(rand()%100 < sc_def_luk)
			status_change_start(bl,sc[skillid-NPC_POISON],skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case NPC_SLEEPATTACK:
	case NPC_BLINDATTACK:
		if(rand()%100 < sc_def_int)
			status_change_start(bl,sc[skillid-NPC_POISON],skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case NPC_MENTALBREAKER:
		if(dstsd) {
			int sp = dstsd->status.max_sp*(10+skilllv)/100;
			if(sp < 1) sp = 1;
			pc_heal(dstsd,0,-sp);
		}
		break;

	case WZ_METEOR:
		if(rand()%100 < sc_def_vit)
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case WZ_VERMILION:
		if(rand()%100 < sc_def_int)
			status_change_start(bl,SC_BLIND,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case CH_TIGERFIST:
		if(rand()%100 < (10 + skilllv*10) * sc_def_vit / 100)
		{
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		}
		break;

	case LK_SPIRALPIERCE:
		if( rand()%100 < (15 + skilllv*5)*sc_def_vit/100 )
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case ST_REJECTSWORD:
		if (rand() % 100 < (skilllv * 15))
			status_change_start(bl, SC_AUTOCOUNTER, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		break;
	case PF_FOGWALL:
		if (src != bl && rand() % 100 < sc_def_int)
			status_change_start(bl, SC_BLIND, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		break;
	case LK_HEADCRUSH:
	  {
		int race = status_get_race(bl);
		int bleed_time = skill_get_time2(skillid, skilllv) - status_get_vit(bl) * 1000;
		if (bleed_time < 90000)
			bleed_time = 90000;
		if (!(battle_check_undead(race, status_get_elem_type(bl)) || race == 6) && rand() % 100 < 50 * sc_def_vit / 100)
			status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, bleed_time, 0);
	  }
		break;
	case LK_JOINTBEAT:
		if (rand() % 100 < (5 * skilllv + 5) * sc_def_vit / 100)
			status_change_start(bl, SC_JOINTBEAT, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		break;
	case PF_SPIDERWEB:
		{
			if(bl->type == BL_MOB || map[sd->bl.m].flag.pvp || map[sd->bl.m].flag.gvg)
			{
				int sec = skill_get_time2(skillid,skilllv);
				if(map[src->m].flag.pvp)
					sec = sec/2;
				battle_stopwalking(bl,1);
				status_change_start(bl,SC_SPIDERWEB,skilllv,0,0,0,sec,0);
			}
		}
		break;
	case ASC_METEORASSAULT:
		if (rand() % 100 < (5 + (5 * skilllv) * sc_def_int / 100))
			switch(rand() % 3) {
			case 0:
				status_change_start(bl, SC_STAN,     skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
				break;
			case 1:
				status_change_start(bl, SC_BLIND,    skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
				break;
			default:
				status_change_start(bl, SC_BLEEDING, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
				break;
			}
		break;
	case MO_EXTREMITYFIST:
		status_change_start(src,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;
	case HW_NAPALMVULCAN:
		if (rand()%10000 < 5*skilllv*sc_def_luk)
			status_change_start(bl,SC_CURSE,7,0,0,0,skill_get_time2(NPC_CURSEATTACK,7),0);
		break;
	}

	if (skillid != MC_CARTREVOLUTION && attack_type&BF_WEAPON){
		int i;
		int sc_def_card = 100;

		for(i = SC_STONE; i <= SC_BLIND; i++) {
			switch (i) {
			case SC_STONE:
			case SC_FREEZE:
				sc_def_card = sc_def_mdef;
				break;
			case SC_STAN:
			case SC_POISON:
			case SC_SILENCE:
				sc_def_card = sc_def_vit;
				break;
			case SC_SLEEP:
			case SC_CONFUSION:
			case SC_BLIND:
				sc_def_card = sc_def_int;
				break;
			case SC_CURSE:
				sc_def_card = sc_def_luk;
				break;
			}
			if (sd) {
				if (!sd->state.arrow_atk) {
					if (rand() % 10000 < (sd->addeff[i - SC_STONE]) * sc_def_card / 100) {
						if (battle_config.battle_log)
							printf("PC %d skill_addeff: card, effect %d %d\n", sd->bl.id, i, sd->addeff[i - SC_STONE]);
						status_change_start(bl, i, 7, 0, 0, 0, (i == SC_CONFUSION) ? 10000 + 7000 : skill_get_time2(sc2[i - SC_STONE], 7), 0);
					}
				} else {
					if (rand() % 10000 < (sd->addeff[i - SC_STONE] + sd->arrow_addeff[i - SC_STONE]) * sc_def_card / 100) {
						if (battle_config.battle_log)
							printf("PC %d skill_addeff: card,effect %d %d\n", sd->bl.id, i, sd->addeff[i - SC_STONE]);
						status_change_start(bl, i, 7, 0, 0, 0, (i == SC_CONFUSION) ? 10000 + 7000 : skill_get_time2(sc2[i - SC_STONE], 7), 0);
					}
				}
			}

			switch (i) {
			case SC_STONE:
			case SC_FREEZE:
				sc_def_card = sc_def_mdef2;
				break;
			case SC_STAN:
			case SC_POISON:
			case SC_SILENCE:
				sc_def_card = sc_def_vit2;
				break;
			case SC_SLEEP:
			case SC_CONFUSION:
			case SC_BLIND:
				sc_def_card = sc_def_int2;
				break;
			case SC_CURSE:
				sc_def_card = sc_def_luk2;
				break;
			}
			if (sd) {
				if (!sd->state.arrow_atk) {
					if (rand() % 10000 < (sd->addeff2[i - SC_STONE]) * sc_def_card / 100) {
						if (battle_config.battle_log)
							printf("PC %d skill_addeff: %d %d\n", src->id, i, sd->addeff2[i - SC_STONE]);
						status_change_start(src, i, 7, 0, 0, 0, (i == SC_CONFUSION) ? 10000 + 7000 : skill_get_time2(sc2[i - SC_STONE], 7), 0);
					}
				} else {
					if (rand() % 10000 < (sd->addeff2[i - SC_STONE] + sd->arrow_addeff2[i - SC_STONE]) * sc_def_card / 100) {
						if (battle_config.battle_log)
							printf("PC %d skill_addeff: %d %d\n", src->id, i, sd->addeff2[i - SC_STONE]);
						status_change_start(src, i, 7, 0, 0, 0, (i == SC_CONFUSION) ? 10000 + 7000 : skill_get_time2(sc2[i - SC_STONE], 7), 0);
					}
				}
			}

			if (dstsd &&
			    rand() % 10000 < dstsd->addeff3[i - SC_STONE] * sc_def_card / 100) {
				if (battle_config.battle_log)
					printf("PC %d skill_addeff: %d %d\n", src->id, i, dstsd->addeff3[i - SC_STONE]);
				status_change_start(src, i, 7, 0, 0, 0, skill_get_time2(sc2[i - SC_STONE], 7), 0);
			}
		}
	}

	return 0;
}

/*=========================================================================
 Blown Skill
-------------------------------------------------------------------------*/
int skill_blown(struct block_list *src, struct block_list *target, int count)
{
	int dx = 0, dy = 0, nx, ny;
	int x, y;
	int ret, prev_state = MS_IDLE;
	int moveblock;
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	struct pet_data *pd = NULL;
	struct skill_unit *su = NULL;

	nullpo_retr(0, src);
	nullpo_retr(0, target);

	if (map[src->m].flag.gvg)
		return 0;

	if (target->type == BL_PC) {
		sd = (struct map_session_data *)target;
	} else if (target->type == BL_MOB) {
		md = (struct mob_data *)target;
	} else if (target->type == BL_PET) {
		pd = (struct pet_data *)target;
	} else if (target->type == BL_SKILL) {
		su = (struct skill_unit *)target;
	} else
		return 0;

	x = target->x;
	y = target->y;

	if (!(count & 0x10000)) {
		dx = target->x - src->x;
		dx = (dx > 0) ? 1: ((dx < 0) ? -1 : 0);
		dy = target->y - src->y;
		dy = (dy > 0) ? 1: ((dy < 0) ? -1 : 0);
	}
	if (dx == 0 && dy == 0) {
		int dir = status_get_dir(target);
		if (dir >= 0 && dir < 8) {
			dx = -dirx[dir];
			dy = -diry[dir];
		}
	}

	ret = path_blownpos(target->m, x, y, dx, dy, count & 0xffff);
	nx = ret >> 16;
	ny = ret & 0xffff;

	if (md && (mob_db[md->class].mexp || md->class == 1288)) {
		nx = target->x;
		ny = target->y;
	}
	moveblock = (x / BLOCK_SIZE != nx / BLOCK_SIZE || y / BLOCK_SIZE != ny / BLOCK_SIZE);

	if (count & 0x20000) {
		battle_stopwalking(target, 1);
		if (sd) {
			sd->to_x = nx;
			sd->to_y = ny;
			clif_walkok(sd);
			clif_movechar(sd);
		} else if (md) {
			md->to_x = nx;
			md->to_y = ny;
			prev_state = md->state.state;
			md->state.state = MS_WALK;
			clif_fixmobpos(md);
		} else if (pd) {
			pd->to_x = nx;
			pd->to_y = ny;
			prev_state = pd->state.state;
			pd->state.state = MS_WALK;
			clif_fixpetpos(pd);
		}
	} else
		battle_stopwalking(target, 2);

	dx = nx - x;
	dy = ny - y;

	if (sd)
		map_foreachinmovearea(clif_pcoutsight, target->m, x - AREA_SIZE, y - AREA_SIZE, x + AREA_SIZE, y + AREA_SIZE, dx, dy, 0, sd);
	else if(md)
		map_foreachinmovearea(clif_moboutsight, target->m, x - AREA_SIZE, y - AREA_SIZE, x + AREA_SIZE, y + AREA_SIZE, dx, dy, BL_PC, md);
	else if(pd)
		map_foreachinmovearea(clif_petoutsight, target->m, x - AREA_SIZE, y - AREA_SIZE, x + AREA_SIZE, y + AREA_SIZE, dx, dy, BL_PC, pd);

	if (su) {
		skill_unit_move_unit_group(su->group, target->m, dx, dy);
	} else {
		skill_unit_move(target, gettick_cache, 0);
		if (moveblock) map_delblock(target);
		target->x = nx;
		target->y = ny;
		if (moveblock) map_addblock(target);
		skill_unit_move(target, gettick_cache, 1);
	}

	if (sd) {
		map_foreachinmovearea(clif_pcinsight, target->m, nx - AREA_SIZE, ny - AREA_SIZE, nx + AREA_SIZE, ny + AREA_SIZE, -dx, -dy, 0, sd);
		if (count & 0x20000)
			pc_stop_walking(sd, 1);
	} else if (md) {
		map_foreachinmovearea(clif_mobinsight, target->m, nx - AREA_SIZE, ny - AREA_SIZE, nx + AREA_SIZE, ny + AREA_SIZE, -dx, -dy, BL_PC, md);
		if (count & 0x20000)
			md->state.state = prev_state;
	} else if (pd) {
		map_foreachinmovearea(clif_petinsight, target->m, nx - AREA_SIZE, ny - AREA_SIZE, nx + AREA_SIZE, ny + AREA_SIZE, -dx, -dy, BL_PC, pd);
		if (count & 0x20000)
			pd->state.state = prev_state;
	}

	return 0;
}

// === SKILL ATTACK ===
// ====================
// RETURN - TOTAL SKILL DAMAGE
int skill_attack(int attack_type, struct block_list* src, struct block_list *dsrc, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct Damage dmg;
	struct status_change *sc_data;
	int type, lv, damage;
	static int tmpdmg = 0;

	if(skillid > 0 && skilllv <= 0)
		return 0;

	nullpo_retr(0, src);
	nullpo_retr(0, dsrc);
	nullpo_retr(0, bl);

	rdamage = 0;

	sc_data = status_get_sc_data(bl);

	if(dsrc->m != bl->m)
		return 0;
	if(src->prev == NULL || dsrc->prev == NULL || bl->prev == NULL)
		return 0;
	if(src->type == BL_PC && pc_isdead((struct map_session_data *)src))
		return 0;
	if(src != dsrc && dsrc->type == BL_PC && pc_isdead((struct map_session_data *)dsrc))
		return 0;
	if(bl->type == BL_PC && pc_isdead((struct map_session_data *)bl))
		return 0;
	if(src->type == BL_PC && skillnotok(skillid, (struct map_session_data *)src))
		return 0;

	if(sc_data)
	{
		if(sc_data[SC_TRICKDEAD].timer != -1)
			return 0;
		if(sc_data[SC_HIDING].timer != -1) {
			if(skill_get_pl(skillid) != 2)
				return 0;
		}
		if(skillid == WZ_STORMGUST && sc_data[SC_FREEZE].timer != -1)
			return 0;
	}

	if(skillid == WZ_FROSTNOVA && dsrc->x == bl->x && dsrc->y == bl->y)
		return 0;
	if(src->type == BL_PC && ((struct map_session_data *)src)->chatID)
		return 0;
	if(dsrc->type == BL_PC && ((struct map_session_data *)dsrc)->chatID)
		return 0;
	if(src->type == BL_PC && bl && mob_gvmobcheck(((struct map_session_data *)src),bl)==0)
		return 0;

	type = -1;
	lv = (flag>>20)&0xf;
	dmg = battle_calc_attack(attack_type,src,bl,skillid,skilllv,flag&0xff );

	if(attack_type&BF_MAGIC && sc_data && sc_data[SC_MAGICROD].timer != -1 && src == dsrc) { 
		dmg.damage = dmg.damage2 = 0;
		if(bl->type == BL_PC) {
			struct map_session_data *sd = (struct map_session_data *)bl;
			if (sd) {
				int sp = skill_get_sp(skillid, skilllv);
				sp = sp * sc_data[SC_MAGICROD].val2 / 100;
				if(skillid == WZ_WATERBALL && skilllv > 1)
					sp = sp / ((skilllv | 1) * (skilllv | 1));
				if (sp > 0x7fff) sp = 0x7fff;
				else if (sp < 1) sp = 1;
				if (sd->status.sp + sp > sd->status.max_sp) {
					sp = sd->status.max_sp - sd->status.sp;
					sd->status.sp = sd->status.max_sp;
				}
				else
					sd->status.sp += sp;
				clif_heal(sd->fd, SP_SP, sp);
				sd->canact_tick = tick + skill_delayfix(bl, skill_get_delay(SA_MAGICROD, sc_data[SC_MAGICROD].val1)); //
			}
		}
		clif_skill_nodamage(bl,bl,SA_MAGICROD,sc_data[SC_MAGICROD].val1,1);
	}

	if(src->type == BL_PET)
	{
		dmg.damage = battle_attr_fix(skilllv, skill_get_pl(skillid), status_get_element(bl));
		dmg.damage2 = 0;
	}

	damage = dmg.damage + dmg.damage2;

	if(lv == 15)
		lv = -1;

	if(flag&0xff00)
		type = (flag&0xff00)>>8;

	if(damage <= 0 || damage < dmg.div_)
		dmg.blewcount = 0;

	if (skillid == CR_GRANDCROSS || skillid == NPC_DARKGRANDCROSS) {
		if (battle_config.gx_disptype) dsrc = src;
		if (src == bl) type = 4;
	}

	if(src->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)src;
		nullpo_retr(0, sd);
		if(skillid == MO_CHAINCOMBO) {
			int delay = 1000 - 4 * status_get_agi(src) - 2 * status_get_dex(src); //��{�f�B���C�̌v�Z
			if(damage < status_get_hp(bl)) { //�_���[�W���Ώۂ�HP��菬�����ꍇ
				if(pc_checkskill(sd, MO_COMBOFINISH) > 0 && sd->spiritball > 0) //�җ���(MO_COMBOFINISH)�擾���C���ێ�����+300ms
					delay += 300 * battle_config.combo_delay_rate /100; //�ǉ��f�B���C��conf�ɂ�蒲��

					status_change_start(src,SC_COMBO,MO_CHAINCOMBO,skilllv,0,0,delay,0); //�R���{��Ԃ�
			}
			sd->attackabletime = sd->canmove_tick = tick + delay;
			clif_combo_delay(src,delay); //�R���{�f�B���C�p�P�b�g�̑��M
		} else if(skillid == MO_COMBOFINISH) {
			int delay = 700 - 4 * status_get_agi(src) - 2 * status_get_dex(src);
			if(damage < status_get_hp(bl))
			{
				if((pc_checkskill(sd, MO_EXTREMITYFIST) > 0) || (pc_checkskill(sd, CH_TIGERFIST) > 0 && sd->spiritball > 0) || (pc_checkskill(sd, CH_CHAINCRUSH) > 0 && sd->spiritball > 1))
					delay += 300 * battle_config.combo_delay_rate /100;

				status_change_start(src,SC_COMBO,MO_COMBOFINISH,skilllv,0,0,delay,0);
			}
			sd->attackabletime = sd->canmove_tick = tick + delay;
			clif_combo_delay(src,delay); //�R���{�f�B���C�p�P�b�g�̑��M
		}

		else if(skillid == CH_TIGERFIST) {
			int delay = 1000 - 4 * status_get_agi(src) - 2 * status_get_dex(src);
			if(damage < status_get_hp(bl)) {
				if(pc_checkskill(sd, CH_CHAINCRUSH) > 0) //�A������(CH_CHAINCRUSH)�擾����+300ms
					delay += 300 * battle_config.combo_delay_rate /100; //�ǉ��f�B���C��conf�ɂ�蒲��

				status_change_start(src,SC_COMBO,CH_TIGERFIST,skilllv,0,0,delay,0); //�R���{��Ԃ�
			}
			sd->attackabletime = sd->canmove_tick = tick + delay;
			clif_combo_delay(src,delay);
		} else if(skillid == CH_CHAINCRUSH) {
			int delay = 1000 - 4 * status_get_agi(src) - 2 * status_get_dex(src);
			if(damage < status_get_hp(bl))
			{
				if(pc_checkskill(sd, MO_EXTREMITYFIST) > 0)
					delay += 300 * battle_config.combo_delay_rate /100;

				status_change_start(src,SC_COMBO,CH_CHAINCRUSH,skilllv,0,0,delay,0);
			}
			sd->attackabletime = sd->canmove_tick = tick + delay;
			clif_combo_delay(src,delay);
		}

		if(damage > 0 && src != bl && (rdamage = battle_calc_rdamage(bl, damage, dmg.flag, attack_type, skillid)) > 0)	// calculate reflected damage
			clif_damage(src, src, tick, dmg.amotion, 0, rdamage, 1, 4, 0);												// send returned damage to player
	}

	switch(skillid)
	{
		case AS_SPLASHER:
			clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, -1, 5);
			break;
		case ASC_BREAKER:
			if(attack_type & BF_WEAPON)
			{
				tmpdmg = damage;
			} else {
				if(tmpdmg == 0 || damage == 0)
					clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, 0, dmg.div_, skillid, skilllv, type);
				damage += tmpdmg;
				tmpdmg = 0;
				if(damage > 0)
					clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, skilllv, type);
			}
			break;
		case SN_SHARPSHOOTING:
			clif_damage(src, bl, tick, dmg.amotion, dmg.dmotion, damage, 0, 0, 0);
			break;
		case PA_PRESSURE:
			clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, (lv != 0) ? lv : skilllv, (skillid == 0) ? 5 : type);
		case NPC_SELFDESTRUCTION:
			break;
		default:
			clif_skill_damage(dsrc, bl, tick, dmg.amotion, dmg.dmotion, damage, dmg.div_, skillid, (lv != 0) ? lv : skilllv, (skillid == 0) ? 5 : type);
	}

	if(dmg.blewcount > 0 && bl->type != BL_SKILL && !status_isdead(bl) && !map[src->m].flag.gvg)
	{
		skill_blown(dsrc, bl, dmg.blewcount);
		if(bl->type == BL_MOB)
			clif_fixmobpos((struct mob_data *)bl);
		else if(bl->type == BL_PET)
			clif_fixpetpos((struct pet_data *)bl);
		else
			clif_fixpos(bl);
	}

	map_freeblock_lock();
	if ((skillid || flag) && !(skillid == ASC_BREAKER && attack_type & BF_WEAPON)) {
		if (attack_type & BF_WEAPON) {
			battle_delay_damage(tick + dmg.amotion, src, bl, BF_WEAPON, skillid, skilllv, damage, dmg.dmg_lv, 0);
		} else {
			battle_damage(src, bl, damage, 0);
			if (!status_isdead(bl) && damage > 0)
				skill_additional_effect(src, bl, skillid, skilllv, attack_type, tick);
		}
	}
	if (skillid == RG_INTIMIDATE && damage > 0 && !(status_get_mode(bl) & 0x20) && !map[src->m].flag.gvg) {
		int s_lv = status_get_lv(src);
		int t_lv = status_get_lv(bl);
		int rate;
		rate = 50 + skilllv * 5 + (s_lv - t_lv);
		if (rand() % 100 < rate)
			skill_addtimerskill(src, tick + 800, bl->id, 0, 0, skillid, skilllv, 0, flag);
	}

	if (damage > 0 && dmg.flag & BF_SKILL && bl->type == BL_PC && pc_checkskill((struct map_session_data *)bl, RG_PLAGIARISM) && !pc_isdead((struct map_session_data *)bl) && sc_data[SC_PRESERVE].timer == -1)
	{
		struct map_session_data *tsd = (struct map_session_data *)bl;

		if(tsd == NULL)
		{
			map_freeblock_unlock();
			return 0;
		}

		// TODO: add option to allow copying advanced skills or not
		if((skillid >= LK_AURABLADE && skillid <= ASC_CDP) && (skillid >= ST_PRESERVE && skillid <= CR_CULTIVATION))
			return 0;

		if(skilllv > pc_checkskill(tsd, RG_PLAGIARISM))
			skilllv = pc_checkskill(tsd, RG_PLAGIARISM);

		if(!tsd->status.skill[skillid].id && !tsd->status.skill[skillid].lv && !(skillid > NPC_PIERCINGATT && skillid < NPC_SUMMONMONSTER) && !(skillid > NPC_SELFDESTRUCTION && skillid < NPC_UNDEADATTACK))
			skill_copy_skill(tsd, skillid, skilllv);
	}

	if (bl->prev != NULL) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		if (sd == NULL) {
			map_freeblock_unlock();
			return 0;
		}
		if (bl->type != BL_PC || (sd && !pc_isdead(sd))) {
			if (bl->type == BL_MOB && src != bl)
			{
				struct mob_data *md = (struct mob_data *)bl;
				if (md == NULL) {
					map_freeblock_unlock();
					return 0;
				}
				if (battle_config.mob_changetarget_byskill == 1)
				{
					int target;
					target = md->target_id;
					if (src->type == BL_PC)
						md->target_id = src->id;
					mobskill_use(md, tick, MSC_SKILLUSED | (skillid<<16));
					md->target_id = target;
				} else
					mobskill_use(md, tick, MSC_SKILLUSED | (skillid<<16));
			}
		}
	}

	if (src->type == BL_PC && dmg.flag&BF_WEAPON && src != bl && src == dsrc && damage > 0) {
		struct map_session_data *sd = (struct map_session_data *)src;
		int hp = 0,sp = 0;
		if (sd == NULL) {
			map_freeblock_unlock();
			return 0;
		}
		if (sd->hp_drain_rate && sd->hp_drain_per > 0 && dmg.damage > 0 && rand()%100 < sd->hp_drain_rate) {
			hp += (dmg.damage * sd->hp_drain_per)/100;
			if (sd->hp_drain_rate > 0 && hp < 1) hp = 1;
			else if (sd->hp_drain_rate < 0 && hp > -1) hp = -1;
		}
		if (sd->hp_drain_rate_ && sd->hp_drain_per_ > 0 && dmg.damage2 > 0 && rand()%100 < sd->hp_drain_rate_) {
			hp += (dmg.damage2 * sd->hp_drain_per_)/100;
			if (sd->hp_drain_rate_ > 0 && hp < 1) hp = 1;
			else if (sd->hp_drain_rate_ < 0 && hp > -1) hp = -1;
		}
		if (sd->sp_drain_rate > 0 && sd->sp_drain_per > 0 && dmg.damage > 0 && rand()%100 < sd->sp_drain_rate) {
			sp += (dmg.damage * sd->sp_drain_per)/100;
			if (sd->sp_drain_rate > 0 && sp < 1) sp = 1;
			else if (sd->sp_drain_rate < 0 && sp > -1) sp = -1;
		}
		if (sd->sp_drain_rate_ > 0 && sd->sp_drain_per_ > 0 && dmg.damage2 > 0 && rand()%100 < sd->sp_drain_rate_) {
			sp += (dmg.damage2 * sd->sp_drain_per_)/100;
			if (sd->sp_drain_rate_ > 0 && sp < 1) sp = 1;
			else if (sd->sp_drain_rate_ < 0 && sp > -1) sp = -1;
		}
		if (hp || sp)
			pc_heal(sd, hp, sp);
		if (sd->sp_drain_type && bl->type == BL_PC)
			battle_heal(NULL, bl, 0, -sp, 0);
	}

	if ((skillid || flag) && rdamage > 0) {
		if (attack_type & BF_WEAPON)
			battle_delay_damage(tick + dmg.amotion, bl, src, BF_WEAPON, skillid, skilllv, rdamage, dmg.dmg_lv, 0);
		else
			battle_damage(bl, src, rdamage, 0);
	}

	if(attack_type&BF_WEAPON && sc_data && sc_data[SC_AUTOCOUNTER].timer != -1 && sc_data[SC_AUTOCOUNTER].val4 > 0)
	{
		if(sc_data[SC_AUTOCOUNTER].val3 == dsrc->id)
			battle_weapon_attack(bl,dsrc,tick,0x8000|sc_data[SC_AUTOCOUNTER].val1);
		status_change_end(bl,SC_AUTOCOUNTER,-1);
	}

	if((skillid == MG_COLDBOLT || skillid == MG_FIREBOLT || skillid == MG_LIGHTNINGBOLT) && (sc_data = status_get_sc_data(src)) && sc_data[SC_DOUBLECASTING].timer != -1 && rand() % 100 < 30 + 10 * sc_data[SC_DOUBLECASTING].val1)
	{
		if(!(flag & 1))
		{
			skill_addtimerskill(src, tick + (skill_delayfix(src, skill_get_delay(skillid, skilllv)) / 2), bl->id, 0, 0, skillid, skilllv, BF_MAGIC, flag | 1);
		}
	}

	map_freeblock_unlock();

	return (dmg.damage+dmg.damage2);
}

/*==========================================
 *
 *------------------------------------------
 */
static int skill_area_temp[8];
typedef int (*SkillFunc)(struct block_list *,struct block_list *,int,int,unsigned int,int);
int skill_area_sub( struct block_list *bl,va_list ap )
{
	struct block_list *src;
	int skill_id, skill_lv, flag;
	unsigned int tick;
	SkillFunc func;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	if(bl->type!=BL_PC && bl->type!=BL_MOB && bl->type!=BL_SKILL)
		return 0;

	src=va_arg(ap,struct block_list *);
	skill_id=va_arg(ap,int);
	skill_lv=va_arg(ap,int);
	tick=va_arg(ap,unsigned int);
	flag=va_arg(ap,int);
	func=va_arg(ap,SkillFunc);

	if(battle_check_target(src,bl,flag) > 0)
		func(src,bl,skill_id,skill_lv,tick,flag);

	return 0;
}

static int skill_check_unit_range_sub(struct block_list *bl, va_list ap) {
	struct skill_unit *unit;
	int *c;
	int skillid, g_skillid, unit_id;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, unit = (struct skill_unit *)bl);
	nullpo_retr(0, c = va_arg(ap,int *));

	if(bl->prev == NULL || bl->type != BL_SKILL)
		return 0;

	if(!unit->alive)
		return 0;

	skillid = va_arg(ap,int);
	g_skillid = unit->group->skill_id;
	unit_id = unit->group->unit_id;

	if (skillid == MG_SAFETYWALL || skillid == AL_PNEUMA) {
		if (unit_id != 0x7e && unit_id != 0x85)
			return 0;
	} else if (skillid == AL_WARP) {
		if ((unit_id < 0x8f || unit_id > 0x99) && unit_id != 0x92)
			return 0;
	} else if ((skillid >= HT_SKIDTRAP && skillid <= HT_CLAYMORETRAP) || skillid == HT_TALKIEBOX) {
		if ((unit_id < 0x8f || unit_id > 0x99) && unit_id != 0x92)
			return 0;
	} else if (skillid == WZ_FIREPILLAR) {
		if (unit_id != 0x87)
			return 0;
	} else if (skillid == HP_BASILICA) {
		if ((unit_id < 0x8f || unit_id > 0x99) && unit_id != 0x92 && unit_id != 0x83)
			return 0;
		if (skillid != g_skillid)
			return 0;
	} else
		return 0;

	(*c)++;

	return 0;
}

int skill_check_unit_range(int m, int x, int y, int skillid, int skilllv) {
	int c = 0;
	int range = skill_get_unit_range(skillid);
	int layout_type = skill_get_unit_layout_type(skillid, skilllv);

	if (layout_type == -1 || layout_type > MAX_SQUARE_LAYOUT) {
		printf("skill_check_unit_range: unsupported layout type %d for skill %d\n", layout_type, skillid);
		return 0;
	}

	range += layout_type;
	map_foreachinarea(skill_check_unit_range_sub, m, x - range, y - range, x + range, y + range, BL_SKILL, &c, skillid);

	return c;
}

static int skill_check_unit_range2_sub(struct block_list *bl, va_list ap) {
	int *c;
	int skillid;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, c = va_arg(ap,int *));

	if (bl->prev == NULL || (bl->type != BL_PC && bl->type != BL_MOB))
		return 0;

	if (bl->type == BL_PC && pc_isdead((struct map_session_data *)bl))
		return 0;

	skillid = va_arg(ap, int);
	if (skillid == HP_BASILICA && bl->type == BL_PC)
		return 0;

	(*c)++;

	return 0;
}

int skill_check_unit_range2(int m, int x, int y, int skillid, int skilllv) {
	int c = 0, range;

	switch (skillid) { // fix by akrus (from freya's bug report)
	case WZ_ICEWALL:
		range = 2;
		break;
	default:
	  {
		int layout_type = skill_get_unit_layout_type(skillid, skilllv);
		if (layout_type == -1 || layout_type > MAX_SQUARE_LAYOUT) {
			printf("skill_check_unit_range2: unsupported layout type %d for skill %d.\n", layout_type, skillid);
			return 0;
		}
		range = skill_get_unit_range(skillid) + layout_type;
	  }
		break;
	}

	map_foreachinarea(skill_check_unit_range2_sub, m, x - range, y - range, x + range, y + range, 0, &c, skillid);

	return c;
}

int skill_guildaura_sub(struct block_list *bl, va_list ap) {
	struct map_session_data *sd;
	int gid, id;
	int flag;

	nullpo_retr(0, sd = (struct map_session_data *)bl); // player
	nullpo_retr(0, ap);

	id = va_arg(ap, int); // id of the guild master
	gid = va_arg(ap, int); // guild_id
	if (sd->status.guild_id != gid) // check if in same guild
		return 0;

	flag = va_arg(ap, int); // flag (checked before: > 0)

		if (sd->sc_count && sd->sc_data[SC_GUILDAURA].timer != -1) {
			if (sd->sc_data[SC_GUILDAURA].val4 != flag) {
				sd->sc_data[SC_GUILDAURA].val4 = flag;
				status_calc_pc(sd, 0);
			}
			return 0;
		}
		status_change_start(&sd->bl, SC_GUILDAURA, 1, id, 0, flag, 0, 0);

	return 0;
}

/*=========================================================================
 * Skill Area (Sub Count)
 */
int skill_area_sub_count(struct block_list *src,struct block_list *target,int skillid,int skilllv,unsigned int tick,int flag)
{
	if (skillid > 0 && skilllv <= 0) return 0;
	if (skill_area_temp[0] < 0xffff)
		skill_area_temp[0]++;

	return 0;
}

int skill_count_water(struct block_list *src, int range) {
	int i, x, y, cnt = 0,size = range * 2 + 1;
	struct skill_unit *unit;

	for (i = 0; i < size * size; i++) {
		x = src->x + (i % size - range);
		y = src->y + (i / size - range);
		if (map_getcell(src->m, x, y, CELL_CHKWATER)) {
			cnt++;
			continue;
		}
		unit = map_find_skill_unit_oncell(src, x, y, SA_DELUGE, NULL);
		if (unit) {
			cnt++;
			skill_delunit(unit);
		}
	}

	return cnt;
}

/*==========================================
 *
 *------------------------------------------
 */
static TIMER_FUNC(skill_timerskill) {
	struct map_session_data *sd = NULL;
	struct mob_data *md = NULL;
	struct pet_data *pd = NULL;
	struct block_list *src = map_id2bl(id),*target;
	struct skill_timerskill *skl = NULL;
	int range;

	nullpo_retr(0, src);

	if(src->prev == NULL)
		return 0;

	if (src->type == BL_PC) {
		nullpo_retr(0, sd = (struct map_session_data *)src);
		skl = &sd->skilltimerskill[data];
	}
	else if (src->type == BL_MOB) {
		nullpo_retr(0, md = (struct mob_data *)src);
		skl = &md->skilltimerskill[data];
	}
	else if (src->type == BL_PET) { // [Valaris]
		nullpo_retr(0, pd = (struct pet_data *)src);
		skl = &pd->skilltimerskill[data];
	}
	else
		return 0;

	nullpo_retr(0, skl);

	skl->timer = -1;
	if(skl->target_id) {
		struct block_list tbl;
		target = map_id2bl(skl->target_id);
		if(skl->skill_id == RG_INTIMIDATE) {
			if(target == NULL) {
				target = &tbl; //���������ĂȂ��̂ɃA�h���X�˂�����ł����̂��ȁH
				target->type = BL_NUL;
				target->m = src->m;
				target->prev = target->next = NULL;
			}
		}
		if(target == NULL)
			return 0;
		if(target->prev == NULL && skl->skill_id != RG_INTIMIDATE)
			return 0;
		if(src->m != target->m)
			return 0;
		if(sd && pc_isdead(sd))
			return 0;
		if(target->type == BL_PC && pc_isdead((struct map_session_data *)target) && skl->skill_id != RG_INTIMIDATE)
			return 0;

		switch(skl->skill_id) {
			case TF_BACKSLIDING:
				clif_skill_nodamage(src,src,skl->skill_id,skl->skill_lv,1);
				break;
			case RG_INTIMIDATE:
				if (sd && !map[src->m].flag.noteleport) {
					int x, y, i, j;
					pc_randomwarp(sd);
					for(i=0;i<16;i++) {
						j = rand()%8;
						x = sd->bl.x + dirx[j];
						y = sd->bl.y + diry[j];
						if (map_getcell(sd->bl.m, x, y, CELL_CHKPASS))
							break;
					}
					if(i >= 16) {
						x = sd->bl.x;
						y = sd->bl.y;
					}
					if(target->prev != NULL) {
						if(target->type == BL_PC && !pc_isdead((struct map_session_data *)target))
							pc_setpos((struct map_session_data *)target,map[sd->bl.m].name,x,y,3);
						else if(target->type == BL_MOB)
							mob_warp((struct mob_data *)target,-1,x,y,3);
					}
				}
				else if(md && !map[src->m].flag.monster_noteleport) {
					int x, y, i, j;
					mob_warp(md,-1,-1,-1,3);
					for(i=0;i<16;i++) {
						j = rand()%8;
						x = md->bl.x + dirx[j];
						y = md->bl.y + diry[j];
						if (map_getcell(md->bl.m, x, y, CELL_CHKPASS))
							break;
					}
					if(i >= 16) {
						x = md->bl.x;
						y = md->bl.y;
					}
					if(target->prev != NULL) {
						if(target->type == BL_PC && !pc_isdead((struct map_session_data *)target))
							pc_setpos((struct map_session_data *)target,map[md->bl.m].name,x,y,3);
						else if(target->type == BL_MOB)
							mob_warp((struct mob_data *)target,-1,x,y,3);
					}
				}
				break;

			case BA_FROSTJOKE:
			case DC_SCREAM:
				range = 15;
				map_foreachinarea(skill_frostjoke_scream, src->m, src->x - range, src->y - range,
				                  src->x + range, src->y + range, 0, src, skl->skill_id, skl->skill_lv, tick);
				break;

			case WZ_WATERBALL:
				if (skl->type > 1) {
					skl->timer = 0;
					skill_addtimerskill(src, tick + 150, target->id, 0, 0, skl->skill_id, skl->skill_lv, skl->type - 1, skl->flag);
					skl->timer = -1;
				}
				skill_attack(BF_MAGIC, src, src, target, skl->skill_id, skl->skill_lv, tick, skl->flag);
				break;

			default:
				skill_attack(skl->type,src,src,target,skl->skill_id,skl->skill_lv,tick,skl->flag);
				break;
		}
	} else {
		if(src->m != skl->map)
			return 0;
		switch(skl->skill_id) {
			case WZ_METEOR:
				if (skl->type >= 0) {
					skill_unitsetting(src, skl->skill_id, skl->skill_lv, skl->type >> 16, skl->type & 0xFFFF, 0);
					clif_skill_poseffect(src, skl->skill_id, skl->skill_lv, skl->x, skl->y, tick);
				}
				else
					skill_unitsetting(src,skl->skill_id,skl->skill_lv,skl->x,skl->y,0);
				break;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_addtimerskill(struct block_list *src, unsigned int tick, int target, int x, int y, int skill_id, int skill_lv, int type, int flag) {
	int i;

	nullpo_retr(1, src);

	if (src->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)src;
		nullpo_retr(1, sd);
		for(i = 0; i < MAX_SKILLTIMERSKILL; i++) {
			if (sd->skilltimerskill[i].timer == -1) {
				sd->skilltimerskill[i].timer = add_timer(tick, skill_timerskill, src->id, i);
				sd->skilltimerskill[i].src_id = src->id;
				sd->skilltimerskill[i].target_id = target;
				sd->skilltimerskill[i].skill_id = skill_id;
				sd->skilltimerskill[i].skill_lv = skill_lv;
				sd->skilltimerskill[i].map = src->m;
				sd->skilltimerskill[i].x = x;
				sd->skilltimerskill[i].y = y;
				sd->skilltimerskill[i].type = type;
				sd->skilltimerskill[i].flag = flag;
				return 0;
			}
		}
		return 1;
	} else if (src->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)src;
		nullpo_retr(1, md);
		for(i = 0; i < MAX_MOBSKILLTIMERSKILL; i++) {
			if (md->skilltimerskill[i].timer == -1) {
				md->skilltimerskill[i].timer = add_timer(tick, skill_timerskill, src->id, i);
				md->skilltimerskill[i].src_id = src->id;
				md->skilltimerskill[i].target_id = target;
				md->skilltimerskill[i].skill_id = skill_id;
				md->skilltimerskill[i].skill_lv = skill_lv;
				md->skilltimerskill[i].map = src->m;
				md->skilltimerskill[i].x = x;
				md->skilltimerskill[i].y = y;
				md->skilltimerskill[i].type = type;
				md->skilltimerskill[i].flag = flag;
				return 0;
			}
		}
		return 1;
	} else if (src->type == BL_PET) { // [Valaris]
		struct pet_data *pd = (struct pet_data *)src;
		nullpo_retr(1, pd);
		for(i = 0; i < MAX_MOBSKILLTIMERSKILL; i++) {
			if (pd->skilltimerskill[i].timer == -1) {
				pd->skilltimerskill[i].timer = add_timer(tick, skill_timerskill, src->id, i);
				pd->skilltimerskill[i].src_id = src->id;
				pd->skilltimerskill[i].target_id = target;
				pd->skilltimerskill[i].skill_id = skill_id;
				pd->skilltimerskill[i].skill_lv = skill_lv;
				pd->skilltimerskill[i].map = src->m;
				pd->skilltimerskill[i].x = x;
				pd->skilltimerskill[i].y = y;
				pd->skilltimerskill[i].type = type;
				pd->skilltimerskill[i].flag = flag;
				return 0;
			}
		}
		return 1;
	}

	return 1;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_cleartimerskill(struct block_list *src)
{
	int i;

	nullpo_retr(0, src);

	if (src->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)src;
		nullpo_retr(0, sd);
		for(i = 0; i < MAX_SKILLTIMERSKILL; i++) {
			if (sd->skilltimerskill[i].timer != -1) {
				delete_timer(sd->skilltimerskill[i].timer, skill_timerskill);
				sd->skilltimerskill[i].timer = -1;
			}
		}
	} else if (src->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)src;
		nullpo_retr(0, md);
		for(i = 0; i < MAX_MOBSKILLTIMERSKILL; i++) {
			if (md->skilltimerskill[i].timer != -1) {
				delete_timer(md->skilltimerskill[i].timer, skill_timerskill);
				md->skilltimerskill[i].timer = -1;
			}
		}
	} else if (src->type == BL_PET) {
		struct pet_data *pd = (struct pet_data *)src;
		nullpo_retr(0, pd);
		for(i = 0; i < MAX_MOBSKILLTIMERSKILL; i++) {
			if (pd->skilltimerskill[i].timer != -1) {
				delete_timer(pd->skilltimerskill[i].timer, skill_timerskill);
				pd->skilltimerskill[i].timer = -1;
			}
		}
	}

	return 0;
}


/*==========================================
 * Skill Cast End Damage ID
 *------------------------------------------
 */
int skill_castend_damage_id(struct block_list* src, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data *sd = NULL;
	struct status_change *sc_data;
	int i;

	if(skillid < 0)
		return 0;
	if(skillid > 0 && skilllv <= 0)
		return 0;

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	sc_data = status_get_sc_data(src);

	if(src->type==BL_PC)
		sd=(struct map_session_data *)src;
	if(sd && pc_isdead(sd))
		return 1;

	if ((skillid == CR_GRANDCROSS || skillid == NPC_DARKGRANDCROSS) && src != bl)
		bl = src;
	if (bl->prev == NULL)
		return 1;
	if (bl->type == BL_PC && pc_isdead((struct map_session_data *)bl))
		return 1;

	map_freeblock_lock();
	switch(skillid)
	{
	// BF_WEAPON attacks
	case SM_BASH:
	case MC_MAMMONITE:
	case AC_DOUBLE:
	case AS_SONICBLOW:
	case KN_PIERCE:
	case KN_SPEARBOOMERANG:
	case TF_POISON:
	case TF_SPRINKLESAND:
	case AC_CHARGEARROW:
	case KN_SPEARSTAB:
	case RG_RAID:
	case RG_INTIMIDATE:
	case BA_MUSICALSTRIKE:
	case DC_THROWARROW:
	case BA_DISSONANCE:
	case CR_HOLYCROSS:
	case CR_SHIELDCHARGE:
	case CR_SHIELDBOOMERANG:
	case HT_PHANTASMIC:
	case NPC_PIERCINGATT:
	case NPC_MENTALBREAKER:
	case NPC_RANGEATTACK:
	case NPC_CRITICALSLASH:
	case NPC_COMBOATTACK:
	case NPC_GUIDEDATTACK:
	case NPC_POISON:
	case NPC_BLINDATTACK:
	case NPC_SILENCEATTACK:
	case NPC_STUNATTACK:
	case NPC_PETRIFYATTACK:
	case NPC_CURSEATTACK:
	case NPC_SLEEPATTACK:
	case NPC_RANDOMATTACK:
	case NPC_WATERATTACK:
	case NPC_GROUNDATTACK:
	case NPC_FIREATTACK:
	case NPC_WINDATTACK:
	case NPC_POISONATTACK:
	case NPC_HOLYATTACK:
	case NPC_DARKNESSATTACK:
	case NPC_TELEKINESISATTACK:
	case NPC_UNDEADATTACK:
	case LK_AURABLADE:
	case LK_SPIRALPIERCE:
	case LK_HEADCRUSH:
	case LK_JOINTBEAT:
	case CG_ARROWVULCAN:
	case HW_MAGICCRASHER:
	case ITM_TOMAHAWK:
	case PA_SHIELDCHAIN:
	case ASC_METEORASSAULT:
		skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		break;

	case WS_CARTTERMINATION:
		{
			struct status_change *sc_data = status_get_sc_data(src);
			// cart termination fails if cartboost is disabled or meltdown is enabled
			if(sc_data[SC_CARTBOOST].timer == -1 || sc_data[SC_MELTDOWN].timer != -1)
			{
				clif_skill_fail(sd, skillid, 0, 0);
				break;
			}
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		}
		break;

	case ASC_BREAKER:
		skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick, flag);
		break;

	case SN_SHARPSHOOTING:			/* �V���[�v�V���[�e�B���O */
		// Does it stop if touch an obstacle? it shouldn't shoot trough walls
		map_foreachinpath(skill_attack_area, src->m, src->x, src->y, bl->x, bl->y, 2, 0, // function, map, source xy, target xy, range, type
		                  BF_WEAPON, src, src, skillid, skilllv, tick, flag, BCT_ENEMY); // varargs
		break;

	case PA_PRESSURE:
		skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		if(rand()%100 < 50)
			status_change_start(bl, SC_STAN, skilllv, 0, 0, 0, skill_get_time2(PA_PRESSURE, skilllv), 0);
		else
			if(rand()%100 < (50 - (status_get_vit(bl) / 10)))
				status_change_start(bl, SC_BLEEDING, skilllv, 0, 0, 0, skill_get_time2(PA_PRESSURE, skilllv), 0);
		if(bl->type == BL_PC)
		{
			struct map_session_data *tsd = (struct map_session_data *)bl;
			tsd->status.sp -= (tsd->status.sp * (15 + 5 * skilllv) / 100);
			clif_updatestatus(tsd, SP_SP);
		}
		break;

	case NPC_DARKBREATH:
		clif_emotion(src,7);
		skill_attack(BF_MISC, src, src, bl, skillid, skilllv, tick, flag);
		break;

	case MO_INVESTIGATE:	/* ���� */
		{
			struct status_change *sc_data = status_get_sc_data(src);
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			if(sc_data && sc_data[SC_BLADESTOP].timer != -1)
				status_change_end(src,SC_BLADESTOP,-1);
		}
		break;
	case SN_FALCONASSAULT:
	case CR_ACIDDEMONSTRATION:
		skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,flag);
		break;
	case KN_BRANDISHSPEAR:
		{
			struct mob_data *md = (struct mob_data *)bl;
			if (md == NULL) {
				map_freeblock_unlock();
				return 1;
			}
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			if (md->hp > 0){
				skill_blown(src,bl,skill_get_blewcount(skillid,skilllv));
				if (bl->type == BL_MOB)
					clif_fixmobpos((struct mob_data *)bl);
				else if (bl->type == BL_PET)
					clif_fixpetpos((struct pet_data *)bl);
				else
					clif_fixpos(bl);
			}
		}
		break;
	case RG_BACKSTAP:		/* �o�b�N�X�^�u */
		{
			int dir = map_calc_dir(src,bl->x,bl->y);
			int t_dir = status_get_dir(bl);
			int dist = distance(src->x,src->y,bl->x,bl->y);
			if ((dist > 0 && !map_check_dir(dir,t_dir)) || bl->type == BL_SKILL) {
				struct status_change *sc_data = status_get_sc_data(src);
				if(sc_data && sc_data[SC_HIDING].timer != -1)
					status_change_end(src, SC_HIDING, -1);	// �n�C�f�B���O����
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
				dir = dir < 4 ? dir+4 : dir-4; // change direction [Celest]
				if (bl->type == BL_PC)
					((struct map_session_data *)bl)->dir = dir;
				else if (bl->type == BL_MOB)
					((struct mob_data *)bl)->dir = dir;
				clif_changed_dir(bl);
			}
			else if (src->type == BL_PC)
				clif_skill_fail(sd, sd->skillid, 0, 0);
		}
		break;

	case AM_ACIDTERROR:		/* �A�V�b�h�e���[ */
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		if (bl->type == BL_PC && rand()%100 < skill_get_time(skillid,skilllv) && battle_config.equipment_breaking) {
			pc_breakarmor((struct map_session_data *)bl);
			clif_emotion(bl, 23);
		}
		break;
	case MO_FINGEROFFENSIVE:	/* �w�e */
	  {
		struct status_change *sc_data = status_get_sc_data(src);
		if (!battle_config.finger_offensive_type)
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
		else {
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
			if (sd) {
				for(i = 1; i < sd->spiritball_old; i++)
					skill_addtimerskill(src, tick + i * 200, bl->id, 0, 0, skillid, skilllv, BF_WEAPON, flag);
				sd->canmove_tick = tick + (sd->spiritball_old - 1) * 200;
			}
		}
		if (sc_data && sc_data[SC_BLADESTOP].timer != -1)
			status_change_end(src, SC_BLADESTOP, -1);
	  }
		break;
	case MO_CHAINCOMBO:		/* �A�ŏ� */
	  {
		struct status_change *sc_data = status_get_sc_data(src);
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		if(sc_data && sc_data[SC_BLADESTOP].timer != -1)
			status_change_end(src,SC_BLADESTOP,-1);
	  }
		break;
	case MO_COMBOFINISH:
	case CH_TIGERFIST:
	case CH_CHAINCRUSH:
	case CH_PALMSTRIKE:
		skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
		break;
	case KN_CHARGEATK:
	case MO_EXTREMITYFIST:
	  {
		struct status_change *sc_data = status_get_sc_data(src);

		if(sd)
		{
			struct walkpath_data wpd;
			int dx,dy;

			dx = bl->x - sd->bl.x;
			dy = bl->y - sd->bl.y;
			if(dx > 0) dx++;
			else if(dx < 0) dx--;
			if(dy > 0) dy++;
			else if(dy < 0) dy--;
			if(dx == 0 && dy == 0) dx++;
			if(path_search(&wpd,src->m,sd->bl.x,sd->bl.y,sd->bl.x+dx,sd->bl.y+dy,1) == -1) {
				dx = bl->x - sd->bl.x;
				dy = bl->y - sd->bl.y;
				if(path_search(&wpd,src->m,sd->bl.x,sd->bl.y,sd->bl.x+dx,sd->bl.y+dy,1) == -1) {
					clif_skill_fail(sd, sd->skillid, 0, 0);
					break;
				}
			}
			if (skillid == KN_CHARGEATK)
				flag = wpd.path_len;
			if (skillid != MO_EXTREMITYFIST || battle_check_target(src, bl, BCT_ENEMY) > 0)
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, flag);
			else
				clif_skill_fail(sd, skillid, 0, 0);
			sd->to_x = sd->bl.x + dx;
			sd->to_y = sd->bl.y + dy;
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,flag);
			int speed = sd->speed;
			sd->speed = 20;
			clif_updatestatus(sd, SP_SPEED);
			clif_walkok(sd);
			clif_movechar(sd);
			if(dx < 0) dx = -dx;
			if(dy < 0) dy = -dy;
			sd->attackabletime = sd->canmove_tick = tick + 100 + sd->speed * ((dx > dy)? dx:dy);
			if(sd->canact_tick < sd->canmove_tick)
				sd->canact_tick = sd->canmove_tick;
			sd->speed = speed;
			clif_updatestatus(sd, SP_SPEED);
			pc_movepos(sd,sd->to_x,sd->to_y);
			status_change_end(&sd->bl,SC_COMBO,-1);
		}
		else
			skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, skillid == KN_CHARGEATK?1:flag);
			if (skillid == MO_EXTREMITYFIST && sc_data && sd->sc_count) {
				status_change_end(src, SC_EXPLOSIONSPIRITS, -1);
				if (sc_data[SC_BLADESTOP].timer != -1)
					status_change_end(src,SC_BLADESTOP,-1);
				if (sc_data[SC_COMBO].timer != -1)
					status_change_end(src,SC_COMBO,-1);
			}
	  }
		break;
	case AC_SHOWER:
	case AS_GRIMTOOTH:
	case MC_CARTREVOLUTION:
	case NPC_SPLASHATTACK:
	case AS_SPLASHER:
		if(flag & 1)
		{
			if(bl->id != skill_area_temp[1])
			{
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, 0x0500);
				if(bl->type == BL_MOB && skillid == AS_GRIMTOOTH)
				{
					struct status_change *sc_data = status_get_sc_data(bl);
					if(sc_data && sc_data[SC_SLOWDOWN].timer == -1)
						status_change_start(bl, SC_SLOWDOWN, 0, 0, 0, 0, 1000, 0);
				}
			}
		} else {
			int ar = 1;
			int x = bl->x, y = bl->y;
			if(skillid == AC_SHOWER)
				ar = 2;
			else if(skillid == AS_SPLASHER)
				ar = 1;
			else if(skillid == NPC_SPLASHATTACK)
				ar = 3;

			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = x;
			skill_area_temp[3] = y;
			map_foreachinarea(skill_area_sub,
				bl->m,x-ar,y-ar,x+ar,y+ar,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
			skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
		}
		break;

	case SM_MAGNUM:
		if (flag & 1) {
			if (bl->id != skill_area_temp[1]) {
				int dist = distance(bl->x, bl->y, skill_area_temp[2], skill_area_temp[3]);
				skill_attack(BF_WEAPON, src, src, bl, skillid, skilllv, tick, 0x0500 | dist);
			}
		} else {
			skill_area_temp[1] = src->id;
			skill_area_temp[2] = src->x;
			skill_area_temp[3] = src->y;
			map_foreachinarea(skill_area_sub, src->m, src->x - 2, src->y - 2, src->x + 2, src->y + 2, 0,
			                  src, skillid, skilllv, tick, flag | BCT_ENEMY | 1,
			                  skill_castend_damage_id);
			status_change_start(src, SC_FLAMELAUNCHER, 0, 0, 0, 0, 10000, 0);
			clif_skill_nodamage(src, src, skillid, skilllv, 1);
		}
		break;

	case KN_BOWLINGBASH:	/* �{�E�����O�o�b�V�� */
		if(flag&1){
			if(bl->id!=skill_area_temp[1])
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0x0500);
		} else {
				int i,c;
				c = skill_get_blewcount(skillid,skilllv);
				if (map[bl->m].flag.gvg)
					c = 0;
				for(i=0;i<c;i++){
					skill_blown(src,bl,1);
					if(bl->type == BL_MOB)
						clif_fixmobpos((struct mob_data *)bl);
					else if(bl->type == BL_PET)
						clif_fixpetpos((struct pet_data *)bl);
					else
						clif_fixpos(bl);
					skill_area_temp[0]=0;
					map_foreachinarea(skill_area_sub,
						bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
						src,skillid,skilllv,tick, flag|BCT_ENEMY ,
						skill_area_sub_count);
					if (skill_area_temp[0] > 1) break;
				}
				skill_area_temp[1]=bl->id;
				skill_area_temp[2]=bl->x;
				skill_area_temp[3]=bl->y;
				skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0);
				map_foreachinarea(skill_area_sub,
					bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
					src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
					skill_castend_damage_id);
		}
		break;

	case ALL_RESURRECTION:		/* ���U���N�V���� */
	case PR_TURNUNDEAD:			/* �^�[���A���f�b�h */
		if(bl->type != BL_PC && battle_check_undead(status_get_race(bl),status_get_elem_type(bl)))
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		else {
			map_freeblock_unlock();
			return 1;
		}
		break;

	case MG_SOULSTRIKE:			/* �\�E���X�g���C�N */
	case NPC_DARKSOULSTRIKE:		/*�Ń\�E���X�g���C�N*/
	case MG_COLDBOLT:			/* �R�[���h�{���g */
	case MG_FIREBOLT:			/* �t�@�C�A�[�{���g */
	case MG_LIGHTNINGBOLT:		/* ���C�g�j���O�{���g */
	case WZ_EARTHSPIKE:			/* �A�[�X�X�p�C�N */
	case AL_HEAL:				/* �q�[�� */
	case AL_HOLYLIGHT:			/* �z�[���[���C�g */
	case WZ_JUPITEL:			/* ���s�e���T���_�[ */
	case NPC_DARKJUPITEL:			/*�Ń��s�e��*/
	case NPC_MAGICALATTACK:		/* MOB:���@��?�U? */
	case PR_ASPERSIO:			/* �A�X�y���V�I */
		skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case MG_FROSTDIVER:			/* �t���X�g�_�C�o�[ */
	{
		struct status_change *sc_data = status_get_sc_data(bl);
		int sc_def_mdef, rate, damage;
		sc_def_mdef = status_get_sc_def_mdef(bl);
		rate = (skilllv * 3 + 35) * sc_def_mdef / 100 - (status_get_int(bl) + status_get_luk(bl)) / 15;
		rate = rate <= 5 ? 5 : rate;
		if (sc_data && sc_data[SC_FREEZE].timer != -1) {
			skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick, flag);
			if (sd)
				clif_skill_fail(sd, skillid, 0, 0);
			break;
		}
		damage = skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		if (status_get_hp(bl) > 0 && damage > 0 && rand() % 100 < rate) {
			status_change_start(bl, SC_FREEZE, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		} else if (sd) {
			clif_skill_fail(sd, skillid, 0, 0);
		}
		break;
	}

	case WZ_WATERBALL:			/* �E�H�[�^�[�{�[�� */
		skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick, flag);
		if (skilllv > 1) {
			int cnt,range;
			range = skilllv > 5 ? 2 : skilllv / 2;
			if (sd)
				cnt = skill_count_water(src, range) - 1;
			else
				cnt = skill_get_num(skillid, skilllv) - 1;
			if (cnt > 0)
				skill_addtimerskill(src, tick + 150, bl->id, 0, 0, skillid, skilllv, cnt, flag);
		}
		break;

	case PR_BENEDICTIO:			/* ���̍~�� */
		if(status_get_race(bl)==1 || status_get_race(bl)==6)
			skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
		break;

	case MG_NAPALMBEAT:			/* �i�p�[���r�[�g */
	case MG_FIREBALL:			/* �t�@�C���[�{�[�� */
	case WZ_SIGHTRASHER:		/* �T�C�g���b�V���[ */
		if(flag&1){
			if(bl->id!=skill_area_temp[1]){
				if(skillid==MG_FIREBALL){	/* �t�@�C���[�{�[���Ȃ璆�S����̋������v�Z */
					int dx=abs( bl->x - skill_area_temp[2] );
					int dy=abs( bl->y - skill_area_temp[3] );
					skill_area_temp[0]=((dx>dy)?dx:dy);
				}
				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,
					skill_area_temp[0]| 0x0500);
			}
		} else {
			int ar;
			skill_area_temp[0] = 0;
			skill_area_temp[1] = bl->id;
			switch (skillid) {
			case MG_NAPALMBEAT:
				ar = 1;
				map_foreachinarea(skill_area_sub,
						bl->m, bl->x - ar, bl->y - ar, bl->x + ar, bl->y + ar, 0,
						src, skillid, skilllv, tick, flag | BCT_ENEMY,
						skill_area_sub_count);
				break;
			case MG_FIREBALL:
				ar = 2;
				skill_area_temp[2] = bl->x;
				skill_area_temp[3] = bl->y;
				skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick,
						skill_area_temp[0]);
				break;
			case WZ_SIGHTRASHER:
			default:
				ar = 3;
				bl = src;
				status_change_end(src, SC_SIGHT, -1);
				break;
			}
			if (skillid == WZ_SIGHTRASHER) {
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			} else {
				skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick,
						skill_area_temp[0]);
			}
			map_foreachinarea(skill_area_sub,
					bl->m, bl->x - ar, bl->y - ar, bl->x + ar, bl->y + ar, 0,
					src, skillid, skilllv, tick, flag | BCT_ENEMY | 1,
					skill_castend_damage_id);
		}
		break;

		case HW_NAPALMVULCAN: // Fixed By SteelViruZ
			if(flag&1){
				if(bl->id!=skill_area_temp[1]){
					skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,
						skill_area_temp[0]);
				}
			}else{
				int ar=(skillid==HW_NAPALMVULCAN)?1:2;
				skill_area_temp[1]=bl->id;
				if(skillid==HW_NAPALMVULCAN){
					skill_area_temp[0]=0;
					map_foreachinarea(skill_area_sub,
						bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
						src,skillid,skilllv,tick, flag|BCT_ENEMY ,
						skill_area_sub_count);
				}else{
					skill_area_temp[0]=0;
					skill_area_temp[2]=bl->x;
					skill_area_temp[3]=bl->y;
				}
				skill_attack(BF_MAGIC,src,src,bl,skillid,skilllv,tick,
					skill_area_temp[0] );
				map_foreachinarea(skill_area_sub,
					bl->m,bl->x-ar,bl->y-ar,bl->x+ar,bl->y+ar,0,
					src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
					skill_castend_damage_id);
			}
			break;

	case WZ_FROSTNOVA:			/* �t���X�g�m���@ */
		map_foreachinarea(skill_attack_area, src->m, src->x-5, bl->y-5, bl->x+5, bl->y+5, 0, BF_MAGIC, src, src, skillid, skilllv, tick, flag, BCT_ENEMY);
		break;

	case HT_BLITZBEAT:			/* �u���b�c�r�[�g */
		if(flag&1){
			if(bl->id!=skill_area_temp[1])
				skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,skill_area_temp[0]|(flag&0xf00000));
		}else{
			skill_area_temp[0]=0;
			skill_area_temp[1]=bl->id;
			if(flag&0xf00000)
				map_foreachinarea(skill_area_sub,bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
					src,skillid,skilllv,tick, flag|BCT_ENEMY ,skill_area_sub_count);
			skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,skill_area_temp[0]|(flag&0xf00000));
			map_foreachinarea(skill_area_sub,
				bl->m,bl->x-1,bl->y-1,bl->x+1,bl->y+1,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_damage_id);
		}
		break;

	case CR_GRANDCROSS:			/* �O�����h�N���X */
	case NPC_DARKGRANDCROSS:		/*�ŃO�����h�N���X*/
		skill_castend_pos2(src,bl->x,bl->y,skillid,skilllv,tick,0);
		if(sd)
			sd->canmove_tick = tick + 1000;
		else if (src->type == BL_MOB)
			mob_changestate((struct mob_data *)src, MS_DELAY, 1000);
		break;

	case TF_THROWSTONE:			/* �Γ��� */
	case NPC_SMOKING:			/* �X���[�L���O */
		skill_attack(BF_MISC,src,src,bl,skillid,skilllv,tick,0 );
		break;

	case PF_SOULBURN:
	  {
		int per = skilllv < 5 ? 30 + skilllv * 10 : 70;
		if (rand() % 100 < per) {
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			if (skilllv == 5)
				skill_attack(BF_MAGIC, src, src, bl, skillid, skilllv, tick, 0);
			if (bl->type == BL_PC) {
				struct map_session_data *tsd = (struct map_session_data *)bl;
				if (tsd) {
					tsd->status.sp = 0;
					clif_updatestatus(tsd, SP_SP);
				}
			}
		} else {
			clif_skill_nodamage(src, src, skillid, skilllv, 1);
			if (skilllv == 5)
				skill_attack(BF_MAGIC, src, src, src, skillid, skilllv, tick, 0);
			if (sd) {
				sd->status.sp = 0;
				clif_updatestatus(sd, SP_SP);
			}
		}
		if (sd)
			pc_blockskill_start(sd, skillid, (skilllv < 5 ? 10000: 15000));
	  }
		break;

	case NPC_SELFDESTRUCTION:
		if (flag & 1) {
			if (bl->id != skill_area_temp[1])
				skill_attack(BF_MISC, src, src, bl, NPC_SELFDESTRUCTION, skilllv, tick, flag);
		} else {
			skill_area_temp[1] = bl->id;
			skill_area_temp[2] = status_get_hp(src);
			clif_skill_nodamage(src, src, NPC_SELFDESTRUCTION, -1, 1);
			map_foreachinarea(skill_area_sub, bl->m, bl->x-5, bl->y-5, bl->x+5, bl->y+5, 0,
					src, skillid, skilllv, tick, flag | BCT_ENEMY | 1, skill_castend_damage_id);
			battle_damage(src, src, skill_area_temp[2], 0);
		}
		break;

	case NPC_BLOODDRAIN:
	case NPC_ENERGYDRAIN:
		{
			int heal;
			heal = skill_attack((skillid==NPC_BLOODDRAIN)?BF_WEAPON:BF_MAGIC,src,src,bl,skillid,skilllv,tick,flag);
			if( heal > 0 ){
				struct block_list tbl;
				tbl.id = 0;
				tbl.m = src->m;
				tbl.x = src->x;
				tbl.y = src->y;
				clif_skill_nodamage(&tbl,src,AL_HEAL,heal,1);
				battle_heal(NULL,src,heal,0,0);
			}
		}
		break;

	case NPC_BIND:
	case NPC_EXPLOSIONSPIRITS:
	case NPC_INCAGI:
        clif_skill_nodamage(src,bl,skillid,skilllv,1);
        break;

    case NPC_BREAKARMOR:
    case NPC_BREAKWEAPON:
    case NPC_BREAKHELM:
    case NPC_BREAKSHIELD:
        skill_castend_nodamage_id(src,bl, skillid, skilllv, 0, 0);
        break;

	case 0:
		if(sd) {
			if(flag&3){
				if(bl->id!=skill_area_temp[1])
					skill_attack(BF_WEAPON,src,src,bl,skillid,skilllv,tick,0x0500);
			}
			else{
				int ar=sd->splash_range;
				skill_area_temp[1]=bl->id;
				map_foreachinarea(skill_area_sub,
					bl->m, bl->x - ar, bl->y - ar, bl->x + ar, bl->y + ar, 0,
					src, skillid, skilllv, tick, flag | BCT_ENEMY | 1,
					skill_castend_damage_id);
			}
		}
		break;
	default:
		printf("Unknown skill used:%d\n",skillid);
		map_freeblock_unlock();
		return 1;
	}
	if(sc_data) {
		if (sc_data[SC_MAGICPOWER].timer != -1 && skillid != HW_MAGICPOWER)	//�}�W�b�N�p��?��?�ʏI��
			status_change_end(src,SC_MAGICPOWER,-1);
	}
	map_freeblock_unlock();

	return 0;
}

int skill_castend_nodamage_id(struct block_list *src, struct block_list *bl, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data *sd = NULL;
	struct map_session_data *dstsd = NULL;
	struct mob_data *md = NULL;
	struct mob_data *dstmd = NULL;
	struct status_change *sc_data = status_get_sc_data(src);
	struct status_change *tsc_data = status_get_sc_data(bl);
	int i = 0, abra_skillid = 0, abra_skilllv;
	int sc_def_vit, sc_def_mdef;
	int sc_dex, sc_luk;

	int changeclass[]= {1038,1039,1046,1059,1086,1087,1112,1115,
	                    1157,1159,1190,1272,1312,1373,1492};
	int poringclass[] = {1002};

	if (skillid > 0 && skilllv <= 0)
		return 0;

	nullpo_retr(1, src);
	nullpo_retr(1, bl);

	if (src->type == BL_PC)
		sd = (struct map_session_data *)src;
	else if (src->type == BL_MOB)
		md = (struct mob_data *)src;

	sc_dex = status_get_mdef(bl);
	sc_luk = status_get_luk(bl);
	sc_def_vit = status_get_sc_def_vit(bl);
	sc_def_mdef = status_get_sc_def_mdef(bl);

	if (bl->type == BL_PC) {
		nullpo_retr(1, dstsd = (struct map_session_data *)bl);
	} else if (bl->type == BL_MOB) {
		nullpo_retr(1, dstmd = (struct mob_data *)bl);
	}

	if (bl == NULL || bl->prev == NULL)
		return 1;
	if (sd && pc_isdead(sd))
		return 1;
	if (dstsd && pc_isdead(dstsd) && skillid != ALL_RESURRECTION && skillid != PR_REDEMPTIO)
		return 1;
	if (status_get_class(bl) == 1288)
		return 1;
	if (sd && skillnotok(skillid, sd)) // [MouseJstr]
		return 0;

	map_freeblock_lock();
	switch(skillid)
	{
	case AL_HEAL:				/* �q�[�� */
	  {
		int heal=skill_calc_heal( src, skilllv );
		int heal_get_jobexp;
		int skill;
		struct pc_base_job s_class;

		if( dstsd && dstsd->special_state.no_magic_damage )
			heal=0;	/* ����峃J�[�h�i�q�[���ʂO�j */
		if (sd){
			s_class = pc_calc_base_job(sd->status.class);
		if((skill=pc_checkskill(sd,HP_MEDITATIO))>0) // ���f�B�e�C�e�B�I
			heal += heal*skill*2/100; // calculation fixed by Yor
			if(sd && dstsd && sd->status.partner_id == dstsd->status.char_id && s_class.job == 23 && sd->status.sex == 0) //�������Ώۂ�PC�A�Ώۂ������̃p�[�g�i�[�A�������X�p�m�r�A���������Ȃ�
				heal = heal*2;	//�X�p�m�r�̉ł��U�߂Ƀq�[�������2�{�ɂȂ�
		}

		clif_skill_nodamage(src,bl,skillid,heal,1);
		heal_get_jobexp = battle_heal(NULL,bl,heal,0,0);

		if(src->type == BL_PC && bl->type==BL_PC && heal > 0 && src != bl && battle_config.heal_exp > 0){
			heal_get_jobexp = heal_get_jobexp * battle_config.heal_exp / 100;
			if(heal_get_jobexp <= 0)
				heal_get_jobexp = 1;
			pc_gainexp((struct map_session_data *)src,0,heal_get_jobexp);
		}
	  }
		break;


	case PR_REDEMPTIO:
		if (sd && !(flag&1)) {
			if (sd->status.party_id == 0) {
				clif_skill_fail(sd, skillid, 0, 0);
				break;
			}
			skill_area_temp[0] = 0;
			party_foreachsamemap(skill_area_sub, sd, skill_get_range(skillid, skilllv), src, skillid, skilllv,
				tick, flag|BCT_PARTY|1,	skill_castend_nodamage_id);

			if (skill_area_temp[0] == 0) {
				clif_skill_fail(sd, skillid, 0, 0);
				break;
			}
			skill_area_temp[0] = 5 - skill_area_temp[0];
			if (skill_area_temp[0] > 0 && !map[src->m].flag.nopenalty) {
				sd->status.base_exp -= pc_nextbaseexp(sd) * skill_area_temp[0] * 2/1000;
				sd->status.job_exp -= pc_nextjobexp(sd) * skill_area_temp[0] * 2/1000;
				clif_updatestatus(sd, SP_BASEEXP);
				clif_updatestatus(sd, SP_JOBEXP);
			}
			pc_heal(sd, 1-sd->status.hp, 1-sd->status.sp);
			break;
		} else if (dstsd && pc_isdead(dstsd) && flag&1) {
			skill_area_temp[0]++;
			skilllv = 3;
		} else
			break;

	case ALL_RESURRECTION:
		if (bl->type == BL_PC) {
			int per = 0;
			struct map_session_data *tsd = (struct map_session_data*)bl;
			if (tsd == NULL) {
				map_freeblock_unlock();
				return 1;
			}
			if ((map[bl->m].flag.pvp) && tsd->pvp_point < 0)
				break;			/* PVP�ŕ����s�\��� */

			if(pc_isdead(tsd)){	/* ���S���� */
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				switch(skilllv){
				case 1: per=10; break;
				case 2: per=30; break;
				case 3: per=50; break;
				case 4: per=80; break;
				}
				tsd->status.hp=tsd->status.max_hp*per/100;
				if(tsd->status.hp<=0) tsd->status.hp=1;
				if(tsd->special_state.restart_full_recover ){	/* �I�V���X�J�[�h */
					tsd->status.hp=tsd->status.max_hp;
					tsd->status.sp=tsd->status.max_sp;
				}
				pc_setstand(tsd);
				if(battle_config.pc_invincible_time > 0)
					pc_setinvincibletimer(tsd,battle_config.pc_invincible_time);
				clif_updatestatus(tsd,SP_HP);
				clif_resurrection(&tsd->bl,1);
				if(src != bl && sd && battle_config.resurrection_exp > 0) {
					int exp = 0,jexp = 0;
					int lv = tsd->status.base_level - sd->status.base_level, jlv = tsd->status.job_level - sd->status.job_level;
					if(lv > 0) {
						exp = (int)((double)tsd->status.base_exp * (double)lv * (double)battle_config.resurrection_exp / 1000000.);
						if(exp < 1) exp = 1;
					}
					if(jlv > 0) {
						jexp = (int)((double)tsd->status.job_exp * (double)lv * (double)battle_config.resurrection_exp / 1000000.);
						if(jexp < 1) jexp = 1;
					}
					if(exp > 0 || jexp > 0)
						pc_gainexp(sd,exp,jexp);
				}
			}
		}
		break;

	case AL_DECAGI:
		if(bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
			break;
		if(rand()%100 < (50 + skilllv * 3 + (status_get_lv(src) + status_get_int(src) / 5) - sc_def_mdef))
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if(bl->type == BL_PC)
			{
				// cancel cart boost status of target
				if(tsc_data[SC_CARTBOOST].timer != -1)
					status_change_end(bl, SC_CARTBOOST, -1);
				// half skill duration when casted on player
				i = (skill_get_time(skillid, skilllv) / 2);
				status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, i, 0);
				break;
			}
			status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, i, 0);
		}
		break;

	case AL_CRUCIS:
		if(flag&1) {
			int race = status_get_race(bl);
			int ele = status_get_elem_type(bl);
			if (battle_check_target(src,bl,BCT_ENEMY) && (race == 6 || battle_check_undead(race,ele))) {
				if (rand() % 100 < 25 + skilllv * 2 + status_get_lv(src) - status_get_lv(bl))
					status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, 0, 0);
			}
		}
		else {
			int range = 15;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			map_foreachinarea(skill_area_sub,
				src->m,src->x-range,src->y-range,src->x+range,src->y+range,0,
				src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
				skill_castend_nodamage_id);
		}
		break;

	case PR_LEXDIVINA:
		{
			struct status_change *sc_data = status_get_sc_data(bl);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if(bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
				break;
			if(sc_data && sc_data[SC_SILENCE].timer != -1)
				status_change_end(bl,SC_SILENCE,-1);
			else if(rand()%100 < sc_def_vit) {
				status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0);
			}
		}
		break;
	case SA_ABRACADABRA:
		if(battle_config.gm_skilluncond != 1)				// No need to check if GM_SKILL_UNCONDITIONAL is set to 1
		{
			if ((i = pc_search_inventory(sd, 715)) < 0)		// Check for gemstones
			{
				clif_skill_fail(sd, sd->skillid, 0, 0);		// Not enough gemstones
				break;
			}
			pc_delitem(sd, i, 1, 0);						// Consume gemstones
		}
		do{
			abra_skillid=skill_abra_dataset(skilllv);
		}while(abra_skillid == 0);
		abra_skilllv=skill_get_max(abra_skillid)>pc_checkskill(sd,SA_ABRACADABRA)?pc_checkskill(sd,SA_ABRACADABRA):skill_get_max(abra_skillid);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		sd->skillitem=abra_skillid;
		sd->skillitemlv=abra_skilllv;
		clif_item_skill(sd,abra_skillid,abra_skilllv,"�A�u���J�_�u��");
		break;
	case SA_COMA:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		if(dstsd){
			dstsd->status.hp=1;
			dstsd->status.sp=1;
			clif_updatestatus(dstsd,SP_HP);
			clif_updatestatus(dstsd,SP_SP);
		}
		if(dstmd) dstmd->hp=1;
		break;
	case SA_FULLRECOVERY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		if(dstsd) pc_heal(dstsd,dstsd->status.max_hp,dstsd->status.max_sp);
		if(dstmd) dstmd->hp=status_get_max_hp(&dstmd->bl);
		break;
	case SA_SUMMONMONSTER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (sd) mob_once_spawn(sd,map[sd->bl.m].name,sd->bl.x,sd->bl.y,"--ja--",-1,1,"");
		break;
	case SA_LEVELUP:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (sd && pc_nextbaseexp(sd)) pc_gainexp(sd,pc_nextbaseexp(sd)*10/100,0);
		break;

	case SA_INSTANTDEATH:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (sd)
			pc_damage(NULL, sd, sd->status.max_hp);
		break;

	case SA_QUESTION:
	case SA_GRAVITY:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case SA_CLASSCHANGE:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(dstmd) mob_class_change(dstmd,changeclass, sizeof(changeclass) / sizeof(int));
		break;
	case SA_MONOCELL:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(dstmd) mob_class_change(dstmd,poringclass, sizeof(poringclass) / sizeof(int));
		break;
	case SA_DEATH:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (dstsd)
			pc_damage(NULL, dstsd, dstsd->status.max_hp);
		if (dstmd)
			mob_damage(NULL, dstmd, dstmd->hp, 1);
		break;
	case SA_REVERSEORCISH:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (dstsd) pc_setoption(dstsd,dstsd->status.option|0x0800);
		break;
	case SA_FORTUNE:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (sd)
			pc_getzeny(sd, status_get_lv(bl) * 100);
		break;
	case SA_TAMINGMONSTER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if (dstmd){
			for(i=0;i<MAX_PET_DB;i++){
				if(dstmd->class == pet_db[i].class){
					pet_catch_process1(sd,dstmd->class);
					break;
				}
			}
		}
		break;
	case AL_INCAGI:			/* ���x���� */
	case AL_BLESSING:		/* �u���b�V���O */
	case PR_SLOWPOISON:
	case PR_IMPOSITIO:		/* �C���|�V�e�B�I�}�k�X */
	case PR_LEXAETERNA:		/* ���b�N�X�G�[�e���i */
	case PR_SUFFRAGIUM:		/* �T�t���M�E�� */
	case PR_BENEDICTIO:		/* ���̍~�� */
	case CR_PROVIDENCE:		/* �v�����B�f���X */
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage ){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}else{
			status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case CG_MARIONETTE: /* �}���I�l�b�g�R���g��?�� */
		if (sd && dstsd) {
			struct status_change *sc_data = status_get_sc_data(src);
			struct status_change *tsc_data = status_get_sc_data(bl);
			int sc = SkillStatusChangeTable[skillid];
			int sc2 = SC_MARIONETTE2;

			if((dstsd->bl.type != BL_PC) ||
			   (sd->bl.id == dstsd->bl.id) ||
			   (!sd->status.party_id) ||
			   (sd->status.party_id != dstsd->status.party_id)) {
				clif_skill_fail(sd, skillid, 0, 0);
				map_freeblock_unlock();
				return 1;
			}
			if (sc_data && tsc_data){
				if(sc_data[sc].timer == -1 && tsc_data[sc2].timer == -1) {
					status_change_start (src,sc,skilllv,0,bl->id,0,skill_get_time(skillid,skilllv),0);
					status_change_start (bl,sc2,skilllv,0,src->id,0,skill_get_time(skillid,skilllv),0);
				}
				else if (sc_data[sc].timer != -1 && tsc_data[sc2].timer != -1 &&
				sc_data[sc].val3 == bl->id && tsc_data[sc2].val3 == src->id) {
					status_change_end(src, sc, -1);
					status_change_end(bl, sc2, -1);
				}
				else {
					clif_skill_fail(sd, skillid, 0, 0);
					map_freeblock_unlock();
					return 1;
				}
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
			}
		}
		break;

	case SA_FLAMELAUNCHER:	// added failure chance and chance to break weapon if turned on [Valaris]
	case SA_FROSTWEAPON:
	case SA_LIGHTNINGLOADER:
	case SA_SEISMICWEAPON:
		if(bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage ){
			clif_skill_nodamage(src,bl,skillid,skilllv,0);
			break;
		}
		if(bl->type==BL_PC) {
			struct map_session_data *sd2=(struct map_session_data *)bl;
			if(sd2->status.weapon==0 || sd2->sc_data[SC_FLAMELAUNCHER].timer!=-1 || sd2->sc_data[SC_FROSTWEAPON].timer!=-1 || sd2->sc_data[SC_LIGHTNINGLOADER].timer!=-1 || sd2->sc_data[SC_SEISMICWEAPON].timer!=-1 || sd2->sc_data[SC_ENCPOISON].timer!=-1)
			{
				clif_skill_fail(sd, skillid, 0, 0);
				clif_skill_nodamage(src, bl, skillid, skilllv, 0);
				break;
			}
		}
		if (skilllv < 5 && rand() % 100 > (60 + skilllv * 10)) {
			clif_skill_fail(sd, skillid, 0, 0);
			clif_skill_nodamage(src,bl,skillid,skilllv,0);
			if(bl->type==BL_PC && battle_config.equipment_breaking) {
				struct map_session_data *sd2=(struct map_session_data *)bl;
				if(sd!=sd2) clif_displaymessage(sd->fd, msg_txt(537)); // You broke target's weapon.
				pc_breakweapon(sd2);
			}
			break;
		}
		else {
			status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case PR_ASPERSIO:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		if(bl->type==BL_MOB)
			break;
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
	case PR_KYRIE:
		clif_skill_nodamage(bl,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
    case AL_RUWACH:
	case AS_POISONREACT:
	case ASC_EDP:
	case CG_MOONLIT:
	case CR_SPEARQUICKEN:
	case CR_REFLECTSHIELD:
	case HP_ASSUMPTIO:
	case HW_MAGICPOWER:
    case KN_AUTOCOUNTER:
	case KN_TWOHANDQUICKEN:
    case LK_AURABLADE:
	case LK_CONCENTRATION:
    case LK_PARRYING:
	case MC_LOUD:
	case MG_ENERGYCOAT:
	case MG_SIGHT:
	case MO_EXPLOSIONSPIRITS:
	case MO_STEELBODY:
	case PA_SACRIFICE:
	case PF_DOUBLECASTING:
    case PF_MEMORIZE:
	case SN_SIGHT:
	case ST_REJECTSWORD:
    case WS_MELTDOWN:
	case WS_MAXOVERTHRUST:
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		break;
	case SM_ENDURE:
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (sd)
			pc_blockskill_start(sd, skillid, 10000);
		break;

	case SM_AUTOBERSERK:
	  {
		struct status_change *tsc_data = status_get_sc_data(bl);
		int sc=SkillStatusChangeTable[skillid];
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( tsc_data ){
			if( tsc_data[sc].timer == -1 )
				status_change_start(bl,sc,skilllv,0,0,0,0,0);
			else
				status_change_end(bl, sc, -1);
		}
	  }
		break;

	case AS_ENCHANTPOISON: // Prevent spamming [Valaris]
		if (bl->type == BL_PC) {
			struct map_session_data *sd2 = (struct map_session_data *)bl;
			if(sd2->sc_data[SC_FLAMELAUNCHER].timer != -1 || sd2->sc_data[SC_FROSTWEAPON].timer != -1 || sd2->sc_data[SC_LIGHTNINGLOADER].timer != -1 || sd2->sc_data[SC_SEISMICWEAPON].timer != -1 || sd2->sc_data[SC_ENCPOISON].timer != -1)
			{
				clif_skill_nodamage(src, bl, skillid, skilllv, 0);
				clif_skill_fail(sd, skillid, 0, 0);
				break;
			}
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
	case LK_TENSIONRELAX:	/* �e���V���������b�N�X */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		pc_setsit(sd);
		clif_sitting(sd);
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
	case LK_BERSERK:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
		break;
	case MC_CHANGECART:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;
	case AC_CONCENTRATION:	/* �W���͌��� */
	  {
		int range = 1;
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		map_foreachinarea( status_change_timer_sub,
			src->m, src->x-range, src->y-range, src->x+range,src->y+range,0,
			src,SkillStatusChangeTable[skillid],tick);
	  }
		break;
	case SM_PROVOKE:
		{
			if((bl->type==BL_MOB && status_get_mode(bl) & 0x20) || battle_check_undead(status_get_race(bl),status_get_elem_type(bl))) //�s���ɂ͌����Ȃ�
			{
				map_freeblock_unlock();
				return 1;
			}

			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );

			if(dstmd && dstmd->skilltimer!=-1 && dstmd->state.skillcastcancel)	// �r���W�Q
				skill_castcancel(bl,0);
			if(dstsd && dstsd->skilltimer!=-1 && (!dstsd->special_state.no_castcancel || map[bl->m].flag.gvg)
				&& dstsd->state.skillcastcancel && !dstsd->special_state.no_castcancel2)
				skill_castcancel(bl,0);

			if(sc_data){
				if(sc_data[SC_FREEZE].timer!=-1)
					status_change_end(bl,SC_FREEZE,-1);
				if(sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
					status_change_end(bl,SC_STONE,-1);
				if(sc_data[SC_SLEEP].timer!=-1)
					status_change_end(bl,SC_SLEEP,-1);
			}

			if (dstmd) {
				int range = skill_get_range(skillid, skilllv);
				if (range < 0)
					range = status_get_range(src) - (range + 1);
				dstmd->state.provoke_flag = src->id;
				mob_target(dstmd, src, range);
			}
		}
		break;

	case CR_DEVOTION:		/* �f�B�{�[�V���� */
		if (sd && dstsd) {
			//�]����{�q�̏ꍇ�̌��̐E�Ƃ��Z�o����
			struct pc_base_job dst_s_class = pc_calc_base_job(dstsd->status.class);

			int lv = sd->status.base_level-dstsd->status.base_level;
			lv = (lv < 0) ? -lv : lv;
			if ((dstsd->bl.type != BL_PC)	// �����PC����Ȃ��Ƃ���
			    || (sd->bl.id == dstsd->bl.id)	// ���肪�����͂���
			    || (lv > battle_config.devotion_level_difference)	// ���x�����}10�܂�
			    || (!sd->status.party_id && !sd->status.guild_id)	// PT�ɂ��M���h�ɂ����������͂���
			    || ((sd->status.party_id != dstsd->status.party_id)	// �����p�[�e�B�[���A
			     && (sd->status.guild_id != dstsd->status.guild_id))	// �����M���h����Ȃ��Ƃ���
			    || (dst_s_class.job == 14 || dst_s_class.job == 21)) {	// �N���Z����
				clif_skill_fail(sd, skillid, 0, 0);
				map_freeblock_unlock();
				return 1;
			}
			for(i=0;i<skilllv;i++){
				if (!sd->dev.val1[i]) {		// �󂫂�������������
					sd->dev.val1[i] = bl->id;
					sd->dev.val2[i] = bl->id;
					break;
				} else if (i == skilllv-1) {		// �󂫂��Ȃ�����
					clif_skill_fail(sd, skillid, 0, 0);
					map_freeblock_unlock();
					return 1;
				}
			}
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			clif_devotion(sd,bl->id);
			status_change_start(bl,SkillStatusChangeTable[skillid],src->id,1,0,0,1000*(15+15*skilllv),0 );
		} else
			clif_skill_fail(sd, skillid, 0, 0);
		break;
	case MO_CALLSPIRITS:	// �C��
		if(sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			pc_addspiritball(sd,skill_get_time(skillid,skilllv),skilllv);
		}
		break;
	case CH_SOULCOLLECT:	// ���C��
		if(sd) {
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			for(i=0;i<5;i++)
				pc_addspiritball(sd,skill_get_time(skillid,skilllv),5);
		}
		break;
	case MO_BLADESTOP:
		if(skilllv == 5 && (sd->spiritball < 4 || sc_data[SC_EXPLOSIONSPIRITS].timer == -1))
		{
			clif_skill_fail(sd, skillid, 0, 0);
			break;
		}
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_start(src,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
	case MO_ABSORBSPIRITS:	// �C�D
		i = 0;
		if (dstsd) {
			if ((sd && sd == dstsd) || map[src->m].flag.pvp || map[src->m].flag.gvg) {
				if (dstsd->spiritball > 0) {
					clif_skill_nodamage(src, bl, skillid, skilllv, 1);
					i = dstsd->spiritball * 7;
					pc_delspiritball(dstsd, dstsd->spiritball, 0);
					if (i > 0x7FFF)
						i = 0x7FFF;
					if (sd && sd->status.sp + i > sd->status.max_sp)
						i = sd->status.max_sp - sd->status.sp;
					}
			}
		} else if (dstmd) { //�Ώۂ������X�^�[�̏ꍇ
			//20%�̊m���őΏۂ�Lv*2��SP���񕜂���B���������Ƃ��̓^�[�Q�b�g(��߄D�)�йޯ�!!
			if (rand() % 100 < 20){
				i = 2 * mob_db[dstmd->class].lv;
				mob_target(dstmd, src, 0);
			}
		}
		if (i && sd) {
			sd->status.sp += i;
			clif_heal(sd->fd, SP_SP, i);
		} else
			clif_skill_nodamage(src, bl, skillid, skilllv, 0);
		break;

	case AC_MAKINGARROW:			/* ��쐬 */
		if (sd) {
			clif_arrow_create_list(sd);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case AM_PHARMACY:
	case CR_ALCHEMY:
		if (sd) {
			clif_skill_produce_mix_list(sd, 32);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;
	case WS_CREATECOIN:
		if(sd)
		{
			clif_skill_produce_mix_list(sd,64);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	case WS_CREATENUGGET:
		if(sd)
		{
			clif_skill_produce_mix_list(sd,128);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	case WS_CARTBOOST:
		// player can't cast cart boost shile agility is decreased
		if(sc_data[SC_DECREASEAGI].timer != -1)
		{
			clif_skill_fail(sd, skillid, 0, 0);
			break;
		}
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		break;

	case ASC_CDP: // [DracoRPG]
		// notes: success rate (from emperium.org) = 20 + [(20*Dex)/50] + [(20*Luk)/100]
		if (sd) {
			clif_skill_produce_mix_list(sd,256);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;
	case BS_HAMMERFALL:		/* �n���}�[�t�H�[�� */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_weapon_damage )
			break;
		if( rand()%100 < (20+ 10*skilllv)*sc_def_vit/100 ) {
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		}
		break;

	case RG_RAID:			/* �T�v���C�Y�A�^�b�N */
	case ASC_METEORASSAULT:	/* ���e�I�A�T���g */ // Meteor Assault skill fix (thanks to [Mikey] from freya's bug report)
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
	  {
		int x = bl->x, y = bl->y;
		int ar = 1;
		if (skillid == ASC_METEORASSAULT) // Meteor Assault skill fix (thanks to [Mikey] from freya's bug report)
			ar = 2;
		skill_area_temp[1] = bl->id;
		skill_area_temp[2] = x;
		skill_area_temp[3] = y;
		map_foreachinarea(skill_area_sub,
			bl->m, x - ar, y - ar, x + ar, y + ar, 0,
			src, skillid, skilllv, tick, flag | BCT_ENEMY | 1,
			skill_castend_damage_id);
	  }
		if (skillid == RG_RAID)
			status_change_end(src, SC_HIDING, -1);	// �n�C�f�B���O����
		break;

	case KN_BRANDISHSPEAR:	/*�u�����f�B�b�V���X�s�A*/
		{
			int c,n=4,ar;
			int dir = map_calc_dir(src,bl->x,bl->y);
			struct square tc;
			int x=bl->x,y=bl->y;
			ar=skilllv/3;
			skill_brandishspear_first(&tc,dir,x,y);
			skill_brandishspear_dir(&tc,dir,4);
			/* �͈͇C */
			if(skilllv == 10){
				for(c=1;c<4;c++){
					map_foreachinarea(skill_area_sub,
						bl->m,tc.val1[c],tc.val2[c],tc.val1[c],tc.val2[c],0,
						src,skillid,skilllv,tick, flag|BCT_ENEMY|n,
						skill_castend_damage_id);
				}
			}
			/* �͈͇B�A */
			if(skilllv > 6){
				skill_brandishspear_dir(&tc,dir,-1);
				n--;
			}else{
				skill_brandishspear_dir(&tc,dir,-2);
				n-=2;
			}

			if(skilllv > 3){
				for(c=0;c<5;c++){
					map_foreachinarea(skill_area_sub,
						bl->m,tc.val1[c],tc.val2[c],tc.val1[c],tc.val2[c],0,
						src,skillid,skilllv,tick, flag|BCT_ENEMY|n,
						skill_castend_damage_id);
					if(skilllv > 6 && n==3 && c==4){
						skill_brandishspear_dir(&tc,dir,-1);
						n--;c=-1;
					}
				}
			}
			/* �͈͇@ */
			for(c=0;c<10;c++){
				if(c==0||c==5) skill_brandishspear_dir(&tc,dir,-1);
				map_foreachinarea(skill_area_sub,
					bl->m,tc.val1[c%5],tc.val2[c%5],tc.val1[c%5],tc.val2[c%5],0,
					src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
					skill_castend_damage_id);
			}
		}
		break;

	case AL_ANGELUS:		/* �G���W�F���X */
	case PR_MAGNIFICAT:		/* �}�O�j�t�B�J�[�g */
	case PR_GLORIA:			/* �O�����A */
	case SN_WINDWALK:		/* �E�C���h�E�H�[�N */
		if(sd == NULL || sd->status.party_id==0 || (flag&1) ){
			/* �ʂ̏��� */
			clif_skill_nodamage(bl,bl,skillid,skilllv,1);
			if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
				break;
			status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0);
		}
		else{
			/* �p�[�e�B�S�̂ւ̏��� */
			party_foreachsamemap(skill_area_sub,
				sd,1,
				src,skillid,skilllv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
		}
		break;
	case BS_ADRENALINE:		/* �A�h���i�������b�V�� */
	case BS_WEAPONPERFECT:	/* �E�F�|���p�[�t�F�N�V���� */
	case BS_OVERTHRUST:		/* �I�[�o�[�g���X�g */
		if(sd == NULL || sd->status.party_id==0 || (flag&1) ){
			clif_skill_nodamage(bl,bl,skillid,skilllv,1);
			status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,(src == bl)? 1:0,0,0,skill_get_time(skillid,skilllv),0);
		}
		else{
			party_foreachsamemap(skill_area_sub,
				sd,1,
				src,skillid,skilllv,tick, flag|BCT_PARTY|1,
				skill_castend_nodamage_id);
		}
		break;

	case BS_MAXIMIZE:		/* �}�L�V�}�C�Y�p���[ */
	case NV_TRICKDEAD:		/* ���񂾂ӂ� */
	case CR_DEFENDER:		/* �f�B�t�F���_�[ */
	case CR_AUTOGUARD:		/* �I�[�g�K�[�h */
		{
			struct status_change *tsc_data = status_get_sc_data(bl);
			int sc=SkillStatusChangeTable[skillid];
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if(tsc_data){
				if( tsc_data[sc].timer == -1 )
					status_change_start(bl,sc,skilllv,0,0,0,skill_get_time(skillid,skilllv),0);
				else
					status_change_end(bl, sc, -1);
			}
		}
		break;

	case TF_HIDING:			/* �n�C�f�B���O */
	  {
		struct status_change *tsc_data = status_get_sc_data(bl);
		int sc = SkillStatusChangeTable[skillid];
		clif_skill_nodamage(src,bl,skillid,-1,1);
		if (tsc_data && tsc_data[sc].timer != -1) {
			status_change_end(bl, sc, -1);
		} else {
			status_change_start(bl, sc, skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
		}
	  }
		break;

	case AS_CLOAKING:		/* �N���[�L���O */
		if (skilllv >= 3 || !skill_check_cloaking(bl)) // <--- correction of cloacking and walls found on freya's bug report (thanks to [BeoWulf])
		{
			struct status_change *tsc_data = status_get_sc_data(bl);
			int sc = SkillStatusChangeTable[skillid];
			if (battle_config.no_caption_cloaking)
				clif_skill_nodamage(src, bl, skillid, -1, 1);
			else
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			if (tsc_data && tsc_data[sc].timer != -1) {
				status_change_end(bl, sc, -1);
			} else {
				status_change_start(bl, sc, skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
			}
		}
		break;

	case ST_CHASEWALK:			/* �n�C�f�B���O */
	  {
		struct status_change *tsc_data = status_get_sc_data(bl);
		int sc=SkillStatusChangeTable[skillid];
		clif_skill_nodamage(src,bl,skillid,-1,1);
		if (tsc_data && tsc_data[sc].timer != -1) {
			status_change_end(bl, sc, -1);
		} else {
			status_change_start(bl, sc, skilllv, 0, 0, 0, skill_get_time(skillid,skilllv), 0);
		}
	  }
		break;

	case BD_LULLABY:			/* �q��S */
	case BD_RICHMANKIM:			/* �j�����h�̉� */
	case BD_ETERNALCHAOS:		/* �i���̍��� */
	case BD_DRUMBATTLEFIELD:	/* �푾�ۂ̋��� */
	case BD_RINGNIBELUNGEN:		/* �j�[�x�����O�̎w�� */
	case BD_ROKISWEIL:			/* ���L�̋��� */
	case BD_INTOABYSS:			/* �[���̒��� */
	case BD_SIEGFRIED:			/* �s���g�̃W�[�N�t���[�h */
	case BA_DISSONANCE:			/* �s���a�� */
	case BA_POEMBRAGI:			/* �u���M�̎� */
	case BA_WHISTLE:			/* ���J */
	case BA_ASSASSINCROSS:		/* �[�z�̃A�T�V���N���X */
	case BA_APPLEIDUN:			/* �C�h�D���̗ь� */
	case DC_UGLYDANCE:			/* ��������ȃ_���X */
	case DC_HUMMING:			/* �n�~���O */
	case DC_DONTFORGETME:		/* ����Y��Ȃ��Łc */
	case DC_FORTUNEKISS:		/* �K�^�̃L�X */
	case DC_SERVICEFORYOU:		/* �T�[�r�X�t�H�[���[ */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skill_unitsetting(src,skillid,skilllv,src->x,src->y,0);
		break;

	case HP_BASILICA:
	case CG_HERMODE:
		{

			struct skill_unit_group *sg;
			battle_stopwalking(src, 1);
			skill_clear_unitgroup(src);
			sg = skill_unitsetting(src, skillid, skilllv, src->x, src->y, 0);

			if(skillid == CG_HERMODE)
				status_change_start(src, SC_DANCING, skillid, 0, 0, sg->group_id, skill_get_time(skillid,skilllv),0);
			else
				status_change_start(src, SkillStatusChangeTable[skillid], skilllv, 0, BCT_SELF, sg->group_id, skill_get_time(skillid, skilllv), 0);
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		}
		break;

	case PA_GOSPEL:				/* �S�X�y�� */
		skill_clear_unitgroup(src);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		skill_unitsetting(src, skillid, skilllv, src->x, src->y, 0);
		status_change_start(src, SkillStatusChangeTable[skillid], skilllv, 0, 0, BCT_SELF, skill_get_time(skillid, skilllv), 0);
		break;

	case BD_ADAPTATION:
		{
			struct status_change *sc_data = status_get_sc_data(src);
			if(sc_data && sc_data[SC_DANCING].timer != -1) {
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				skill_stop_dancing(src,0);
			}
		}
		break;

	case BA_FROSTJOKE:			/* �����W���[�N */
	case DC_SCREAM:				/* �X�N���[�� */
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		skill_addtimerskill(src, tick + 3000, bl->id, 0, 0, skillid, skilllv, 0, flag);
		break;

	case TF_STEAL:			// �X�e�B�[��
		if (sd) {
			if (pc_steal_item(sd, bl))
				clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			else
				clif_skill_fail(sd, skillid, 0x0a, 0);
		}
		break;

	case RG_STEALCOIN:		// �X�e�B�[���R�C��
		if (sd) {
			if (pc_steal_coin(sd,bl)) {
				int range = skill_get_range(skillid,skilllv);
				if (range < 0)
					range = status_get_range(src) - (range + 1);
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				mob_target((struct mob_data *)bl,src,range);
			} else
				clif_skill_fail(sd, skillid, 0, 0);
		}
		break;

	case MG_STONECURSE: /* �X�g�[���J�[�X */
	  {
		// Level 6-10 doesn't consume a red gem if it fails [celest]
		int i, gem_flag = 1;
		if (bl->type == BL_MOB && status_get_mode(bl) & 0x20) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			break;
		}
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
			break;
	  {
		// To avoid Stone Curse stacking [Aalye]
		struct status_change *sc_data = status_get_sc_data(bl);
		if (sc_data && sc_data[SC_STONE].timer != -1) {
			status_change_end(bl,SC_STONE,-1);
			if (sd)
				clif_skill_fail(sd, skillid, 0, 0);
		}
		else if (rand() % 100 < skilllv * 4 + 20 && !battle_check_undead(status_get_race(bl),status_get_elem_type(bl)))
			status_change_start(bl, SC_STONE, skilllv, 0, 0, 0, skill_get_time2(skillid, skilllv), 0);
		else if (sd) {
			if (skilllv > 5)
				gem_flag = 0;
			clif_skill_fail(sd, skillid, 0, 0);
		}
	  }
		if (dstmd)
			mob_target(dstmd, src, skill_get_range(skillid, skilllv));
		if (sd && gem_flag) {
			if ((i = pc_search_inventory(sd, skill_db[skillid].itemid[0])) < 0) {
				clif_skill_fail(sd, sd->skillid, 0, 0);
				break;
			}
			pc_delitem(sd, i, skill_db[skillid].amount[0], 0);
		}
	  }
		break;

	case NV_FIRSTAID:			/* ���}�蓖 */
		clif_skill_nodamage(src,bl,skillid,5,1);
		battle_heal(NULL,bl,5,0,0);
		break;

	case AL_CURE:				/* �L���A�[ */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		status_change_end(bl, SC_SILENCE	, -1 );
		status_change_end(bl, SC_BLIND	, -1 );
		status_change_end(bl, SC_CONFUSION, -1 );
		if( battle_check_undead(status_get_race(bl),status_get_elem_type(bl)) ){//�A���f�b�h�Ȃ�ÈŌ���
			status_change_start(bl, SC_CONFUSION,1,0,0,0,6000,0);
		}
		break;

	case TF_DETOXIFY:			/* ��� */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_end(bl, SC_POISON	, -1 );
		status_change_end(bl, SC_DPOISON	, -1 );
		break;

	case PR_STRECOVERY:			/* ���J�o���[ */
	  {
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
			break;
		status_change_end(bl, SC_FREEZE	, -1 );
		status_change_end(bl, SC_STONE	, -1 );
		status_change_end(bl, SC_SLEEP	, -1 );
		status_change_end(bl, SC_STAN		, -1 );
		if( battle_check_undead(status_get_race(bl),status_get_elem_type(bl)) ){
			int blind_time;
			blind_time=30*(100-(status_get_int(bl)+status_get_vit(bl))/2)/100;
			if(rand()%100 < (100-(status_get_int(bl)/2+status_get_vit(bl)/3+status_get_luk(bl)/10)))
				status_change_start(bl, SC_BLIND,1,0,0,0,blind_time,0);
		}
		if(dstmd){
			dstmd->attacked_id=0;
			dstmd->target_id=0;
			dstmd->state.targettype = NONE_ATTACKABLE;
			dstmd->state.skillstate=MSS_IDLE;
			dstmd->next_walktime=tick+rand()%3000+3000;
		}
	  }
		break;

	case WZ_ESTIMATION:			/* �����X�^�[��� */
		if(src->type==BL_PC){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			clif_skill_estimation((struct map_session_data *)src,bl);
		}
		break;

	case MC_IDENTIFY:			/* �A�C�e���Ӓ� */
		if(sd)
			clif_item_identify_list(sd);
		break;

	case BS_REPAIRWEAPON:			/* ����C�� */
		if(sd && dstsd)
			clif_item_repair_list(sd, dstsd);
		break;

	case MC_VENDING:			/* �I�X�J�� */
		if(sd)
			clif_openvendingreq(sd,2+sd->skilllv);
		break;

	case AL_TELEPORT:			/* �e���|�[�g */
		if (sd) {
			if (map[sd->bl.m].flag.noteleport){	/* �e���|�֎~ */
				clif_skill_teleportmessage(sd,0);
				break;
			}
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			if (sd->skilllv == 1)
				clif_skill_warppoint(sd, sd->skillid, "Random", "", "", "");
			else {
				clif_skill_warppoint(sd, sd->skillid, "Random", sd->status.save_point.map, "", "");
			}
		}else if (bl->type == BL_MOB)
			mob_warp((struct mob_data *)bl, -1, -1, -1, 3);
		break;

	case AL_HOLYWATER:
		if(sd) {
			if(!skill_produce_mix(sd, 523, 0, 0, 0)) //523: Holy Water
				clif_skill_fail(sd, skillid, 0, 0);
		}
		break;

	case TF_PICKSTONE:
		if (sd) {
			int eflag;
			struct item item_tmp;
			struct block_list tbl;
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			memset(&item_tmp,0,sizeof(item_tmp));
			memset(&tbl,0,sizeof(tbl)); // [MouseJstr]
			item_tmp.nameid = 7049;
			item_tmp.identify = 1;
			tbl.id = 0;
			clif_takeitem(&sd->bl, &tbl);
			eflag = pc_additem(sd, &item_tmp, 1);
			if (eflag) {
				clif_additem(sd, 0, 0, eflag);
				map_addflooritem(&item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, NULL, NULL, NULL, sd->bl.id, 0);
			}
		}
		break;

	case RG_STRIPWEAPON:		/* �X�g���b�v�E�F�|�� */
	case RG_STRIPSHIELD:		/* �X�g���b�v�V�[���h */
	case RG_STRIPARMOR:			/* �X�g���b�v�A�[�}�[ */
	case RG_STRIPHELM:			/* �X�g���b�v�w���� */
	case ST_FULLSTRIP:
	  {
		struct status_change *tsc_data;
		int equip, strip_fix;
		int sclist[4] = {0, 0, 0, 0};

		switch (skillid) {
		case RG_STRIPWEAPON:
			equip = EQP_WEAPON;
			break;
		case RG_STRIPSHIELD:
			equip = EQP_SHIELD;
			break;
		case RG_STRIPARMOR:
			equip = EQP_ARMOR;
			break;
		case RG_STRIPHELM:
			equip = EQP_HELM;
			break;
		case ST_FULLSTRIP:
			equip = EQP_WEAPON | EQP_SHIELD | EQP_ARMOR | EQP_HELM;
			break;
		default:
			map_freeblock_unlock();
			return 1;
		}

		strip_fix = status_get_dex(src) - status_get_dex(bl);
		if (strip_fix < 0)
			strip_fix = 0;
		if (rand() % 100 >= 5 + 2 * skilllv + strip_fix / 5)
			break;
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);

		if (dstsd) {
			tsc_data = status_get_sc_data(bl);
			for (i = 0; i < 11; i++) {
				if (equip & EQP_WEAPON && (i == 9 ||
					( i == 8 && dstsd->equip_index[i] >= 0 && dstsd->inventory_data[dstsd->equip_index[8]]->type == 4)) &&
					!(dstsd->unstripable_equip & EQP_WEAPON) && !(tsc_data && tsc_data[SC_CP_WEAPON].timer != -1)) {
						sclist[0] = SC_STRIPWEAPON;
						if (dstsd->equip_index[i] >= 0 && dstsd->inventory_data[dstsd->equip_index[i]])
							pc_unequipitem(dstsd, dstsd->equip_index[i], 3);
				} else if (equip & EQP_SHIELD && (i == 8) && !(tsc_data && tsc_data[SC_CP_SHIELD].timer != -1) &&
					!(dstsd->unstripable_equip & EQP_SHIELD)) {
						sclist[1] = SC_STRIPSHIELD;
						if (dstsd->equip_index[i] >= 0 && dstsd->inventory_data[dstsd->equip_index[i]] 
						&& dstsd->inventory_data[dstsd->equip_index[i]]->type == 5)
							pc_unequipitem(dstsd, dstsd->equip_index[i], 3);
				} else if (equip & EQP_ARMOR && i == 7 && !(dstsd->unstripable_equip & EQP_ARMOR) && 
					!(tsc_data && tsc_data[SC_CP_ARMOR].timer != -1)) {
						sclist[2] = SC_STRIPARMOR;
						if (dstsd->equip_index[i] >= 0 && dstsd->inventory_data[dstsd->equip_index[i]] )
							pc_unequipitem(dstsd, dstsd->equip_index[i], 3);
				} else if (equip & EQP_HELM && i == 6 && !(dstsd->unstripable_equip & EQP_HELM) && 
					!(tsc_data && tsc_data[SC_CP_HELM].timer != -1)) {
						sclist[3] = SC_STRIPHELM;
						if (dstsd->equip_index[i] >= 0 && dstsd->inventory_data[dstsd->equip_index[i]] )
							pc_unequipitem(dstsd, dstsd->equip_index[i], 3);
				}
			}
		} else if (dstmd) {
			if (equip & EQP_WEAPON)
				sclist[0] = SC_STRIPWEAPON;
			if (equip & EQP_SHIELD)
				sclist[1] = SC_STRIPSHIELD;
			if (equip & EQP_ARMOR)
				sclist[2] = SC_STRIPARMOR;
			if (equip & EQP_HELM)
				sclist[3] = SC_STRIPHELM;
		}

		for (i = 0; i < 4; i++) {
			if (sclist[i] > 0) // Start the SC only if an equipment was stripped from this location
				status_change_start(bl, sclist[i], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv) + strip_fix / 2, 0);
		}
	  }
		break;

	case AM_POTIONPITCHER: /* �|�[�V�����s�b�`���[ */
	  {
		struct block_list tbl;
		int i, x, hp = 0,sp = 0;
		if (sd) {
			/* On kRO/iRO, you can use on oneself. [Aalye]
			if (sd == dstsd) { // cancel use on oneself
				map_freeblock_unlock();
				return 1;
			}*/
			x = skilllv % 11 - 1;
			i = pc_search_inventory(sd,skill_db[skillid].itemid[x]);
			if (i < 0 || skill_db[skillid].itemid[x] <= 0) {
				clif_skill_fail(sd, skillid, 0, 0);
				map_freeblock_unlock();
				return 1;
			}
			if (sd->inventory_data[i] == NULL || sd->status.inventory[i].amount < skill_db[skillid].amount[x]) {
				clif_skill_fail(sd, skillid, 0, 0);
				map_freeblock_unlock();
				return 1;
			}
			sd->state.potionpitcher_flag = 1;
			sd->potion_hp = sd->potion_sp = sd->potion_per_hp = sd->potion_per_sp = 0;
			sd->skilltarget = bl->id;
			run_script(sd->inventory_data[i]->script, 0, sd->bl.id, 0);
			pc_delitem(sd, i, skill_db[skillid].amount[x], 0);
			sd->state.potionpitcher_flag = 0;
			if (sd->potion_per_hp > 0 || sd->potion_per_sp > 0) {
				hp = status_get_max_hp(bl) * sd->potion_per_hp / 100;
				hp = hp * (100 + pc_checkskill(sd, AM_POTIONPITCHER) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5) / 100;
				if(dstsd) {
					sp = dstsd->status.max_sp * sd->potion_per_sp / 100;
					sp = sp * (100 + pc_checkskill(sd, AM_POTIONPITCHER) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5) / 100;
				}
			}
			else {
				if (sd->potion_hp > 0) {
					hp = sd->potion_hp * (100 + pc_checkskill(sd, AM_POTIONPITCHER) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5) / 100;
					hp = hp * (100 + (status_get_vit(bl)<<1)) / 100;
					if (dstsd)
						hp = hp * (100 + pc_checkskill(dstsd, SM_RECOVERY) * 10) / 100;
				}
				if (sd->potion_sp > 0) {
					sp = sd->potion_sp * (100 + pc_checkskill(sd, AM_POTIONPITCHER) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5) / 100;
					sp = sp * (100 + (status_get_int(bl)<<1)) / 100;
					if (dstsd)
						sp = sp * (100 + pc_checkskill(dstsd, MG_SRECOVERY) * 10) / 100;
				}
			}
		}
		else {
			hp = (1 + rand() % 400) * (100 + skilllv * 10) / 100;
			hp = hp * (100 + (status_get_vit(bl)<<1)) / 100;
			if (dstsd)
				hp = hp * (100 + pc_checkskill(dstsd, SM_RECOVERY) * 10) / 100;
		}
		tbl.id = 0;
		tbl.m = src->m;
		tbl.x = src->x;
		tbl.y = src->y;
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (hp > 0 || (hp <= 0 && sp <= 0))
			clif_skill_nodamage(&tbl, bl, AL_HEAL, hp, 1);
		if (sp > 0)
			clif_skill_nodamage(&tbl, bl, MG_SRECOVERY, sp, 1);
		battle_heal(src, bl, hp, sp, 0);
	  }
		break;
	case AM_CP_WEAPON:
	case AM_CP_SHIELD:
	case AM_CP_ARMOR:
	case AM_CP_HELM:
	  {
		int scid = SC_STRIPWEAPON + (skillid - AM_CP_WEAPON);
		struct status_change *tsc_data = status_get_sc_data(bl);
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (tsc_data && tsc_data[scid].timer != -1)
			status_change_end(bl, scid, -1 );
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
	  }
		break;
	case SA_DISPELL:
	  {
		int i;
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
			break;
		for(i = 0; i < SC_MAX; i++) {
			if(i == SC_WEIGHT50 || i == SC_WEIGHT90 || i == SC_RIDING || i == SC_FALCON || i == SC_HALLUCINATION || i == SC_WEIGHT50 || i == SC_WEIGHT90 || i == SC_STRIPWEAPON || i == SC_STRIPSHIELD || i == SC_STRIPARMOR || i == SC_STRIPHELM || i == SC_CP_WEAPON || i == SC_CP_SHIELD || i == SC_CP_ARMOR || i == SC_CP_HELM || i == SC_COMBO || i == SC_MELTDOWN || i == SC_CARTBOOST || i == SC_DANCING || i == SC_EDP || i == SC_CARTBOOST || i == SC_MELTDOWN || i == SC_MOONLIT)
				continue;
			if(rand()%10000 >= (10000 - i) * (50 + 10 * skilllv) / 100)
				break;
			status_change_end(bl, i, -1);
		}
	  }
		break;

	case TF_BACKSLIDING:
		battle_stopwalking(src,1);
		skill_blown(src,bl,skill_get_blewcount(skillid,skilllv)|0x10000);
		if(src->type == BL_MOB)
			clif_fixmobpos((struct mob_data *)src);
		else if(src->type == BL_PET)
			clif_fixpetpos((struct pet_data *)src);
		else if(src->type == BL_PC)
			clif_fixpos(src);
		skill_addtimerskill(src, tick + 200, src->id, 0, 0, skillid, skilllv, 0, flag);
		break;

	case SA_CASTCANCEL:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		skill_castcancel(src, 1);
		if (sd) {
			int sp = skill_get_sp(sd->skillid_old, sd->skilllv_old);
			sp = sp * (90 - (skilllv-1) * 20) / 100;
			if (sp < 0) sp = 0;
			pc_heal(sd, 0, -sp);
		}
		break;
	case SA_SPELLBREAKER:
	  {
		struct status_change *sc_data = status_get_sc_data(bl);
		int sp;
		if(sc_data && sc_data[SC_MAGICROD].timer != -1) {
			if(dstsd) {
				sp = skill_get_sp(skillid,skilllv);
				sp = sp * sc_data[SC_MAGICROD].val2 / 100;
				if(sp > 0x7fff) sp = 0x7fff;
				else if(sp < 1) sp = 1;
				if(dstsd->status.sp + sp > dstsd->status.max_sp) {
					sp = dstsd->status.max_sp - dstsd->status.sp;
					dstsd->status.sp = dstsd->status.max_sp;
				}
				else
					dstsd->status.sp += sp;
				clif_heal(dstsd->fd,SP_SP,sp);
			}
			clif_skill_nodamage(bl,bl,SA_MAGICROD,sc_data[SC_MAGICROD].val1,1);
			if(sd) {
				sp = sd->status.max_sp/5;
				if(sp < 1) sp = 1;
				pc_heal(sd,0,-sp);
			}
		}
		else {
			int bl_skillid = 0, bl_skilllv = 0;
			if (bl->type == BL_PC) {
				if (dstsd && dstsd->skilltimer != -1) {
					bl_skillid = dstsd->skillid;
					bl_skilllv = dstsd->skilllv;
				}
			}
			else if (bl->type == BL_MOB) {
				if (dstmd && dstmd->skilltimer != -1) {
					bl_skillid = dstmd->skillid;
					bl_skilllv = dstmd->skilllv;
				}
			}
			if (bl_skillid > 0 && skill_db[bl_skillid].skill_type == BF_MAGIC) {
				clif_skill_nodamage(src,bl,skillid,skilllv,1);
				skill_castcancel(bl,0);
				sp = skill_get_sp(bl_skillid,bl_skilllv);
				if(dstsd)
					pc_heal(dstsd,0,-sp);
				if(sd) {
					sp = sp*(25*(skilllv-1))/100;
					if (skilllv > 1 && sp < 1) sp = 1;
					if (sp > 0x7fff) sp = 0x7fff;
					else if (sp < 1) sp = 1;
					if (sd->status.sp + sp > sd->status.max_sp) {
						sp = sd->status.max_sp - sd->status.sp;
						sd->status.sp = sd->status.max_sp;
					}
					else
						sd->status.sp += sp;
					clif_heal(sd->fd, SP_SP, sp);
				}
			} else if (sd)
				clif_skill_fail(sd, skillid, 0, 0);
		}
	  }
		break;
	case SA_MAGICROD:
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;
	case SA_AUTOSPELL:			/* �I�[�g�X�y�� */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(sd)
			clif_autospell(sd,skilllv);
		else {
			int maxlv=1,spellid=0;
			static const int spellarray[3] = { MG_COLDBOLT,MG_FIREBOLT,MG_LIGHTNINGBOLT };
			if(skilllv >= 10) {
				spellid = MG_FROSTDIVER;
				maxlv = skilllv - 9;
			}
			else if(skilllv >=8) {
				spellid = MG_FIREBALL;
				maxlv = skilllv - 7;
			}
			else if(skilllv >=5) {
				spellid = MG_SOULSTRIKE;
				maxlv = skilllv - 4;
			}
			else if(skilllv >=2) {
				int i = rand()%3;
				spellid = spellarray[i];
				maxlv = skilllv - 1;
			}
			else if(skilllv > 0) {
				spellid = MG_NAPALMBEAT;
				maxlv = 3;
			}
			if(spellid > 0)
				status_change_start(src,SC_AUTOSPELL,skilllv,spellid,maxlv,0,
					skill_get_time(SA_AUTOSPELL,skilllv),0);
		}
		break;

	case NPC_ATTRICHANGE:
	case NPC_CHANGEWATER:
	case NPC_CHANGEGROUND:
	case NPC_CHANGEFIRE:
	case NPC_CHANGEWIND:
	case NPC_CHANGEPOISON:
	case NPC_CHANGEHOLY:
	case NPC_CHANGEDARKNESS:
	case NPC_CHANGETELEKINESIS:
		if(md){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			md->def_ele=skill_get_pl(skillid);
			if(md->def_ele==0)			/* �����_���ω��A�������A*/
				md->def_ele=rand()%10;	/* �s�������͏��� */
			md->def_ele+=(1+rand()%4)*20;	/* �������x���̓����_�� */
		}
		break;

	case NPC_EXPLOSIONSPIRITS:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		break;

	case NPC_PROVOCATION:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(md)
			clif_pet_performance(src,mob_db[md->class].skill[md->skillidx].val[0]);
		break;

	case NPC_HALLUCINATION:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );
		break;

	case NPC_KEEPING:
	case NPC_BARRIER:
	  {
		int skill_time = skill_get_time(skillid,skilllv);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		status_change_start(bl,SkillStatusChangeTable[skillid],skilllv,0,0,0,skill_time,0 );
		if (src->type == BL_MOB)
			mob_changestate((struct mob_data *)src, MS_DELAY, skill_time);
		else if (src->type == BL_PC)
			sd->attackabletime = sd->canmove_tick = tick + skill_time;
	  }
		break;

	case NPC_DARKBLESSING:
	  {
		int sc_def = 100 - status_get_mdef(bl);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage )
			break;
		if(status_get_elem_type(bl) == 7 || status_get_race(bl) == 6)
			break;
		if(rand()%100 < sc_def*(50+skilllv*5)/100) {
			if(dstsd) {
				int hp = status_get_hp(bl)-1;
				pc_heal(dstsd,-hp,0);
			}
			else if(dstmd)
				dstmd->hp = 1;
		}
	  }
		break;

	case NPC_LICK:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if( bl->type==BL_PC && ((struct map_session_data *)bl)->special_state.no_weapon_damage )
			break;
		if(dstsd)
			pc_heal(dstsd,0,-100);
		if(rand()%100 < (skilllv*5)*sc_def_vit/100)
			status_change_start(bl,SC_STAN,skilllv,0,0,0,skill_get_time2(skillid,skilllv),0);
		break;

	case NPC_SUICIDE:			/* ���� */
		if(src && bl){
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			if (md)
				mob_damage(NULL,md,md->hp,0);
			else if (sd)
				pc_damage(NULL,sd,sd->status.hp);
		}
		break;

	case NPC_SUMMONSLAVE:		/* �艺���� */
	case NPC_SUMMONMONSTER:		/* MOB���� */
		if (md) {
			mob_summonslave(md, mob_db[md->class].skill[md->skillidx].val, skilllv, (skillid == NPC_SUMMONSLAVE) ? 1 : 0);
		}
		break;
	case NPC_CALLSLAVE:
		mob_warpslave((struct mob_data *) src, src->x, src->y);
		break;

	case NPC_TRANSFORMATION:
	case NPC_METAMORPHOSIS:
		if(md)
			mob_class_change(md,mob_db[md->class].skill[md->skillidx].val, 1);
		break;

	case NPC_EMOTION:			/* �G���[�V���� */
		if(md)
			clif_emotion(&md->bl,mob_db[md->class].skill[md->skillidx].val[0]);
		break;

	case NPC_DEFENDER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		break;

	case NPC_BREAKWEAPON:
    case NPC_BREAKARMOR:
    case NPC_BREAKHELM:
    case NPC_BREAKSHIELD:
		if(bl->type == BL_PC && rand()%100 < 1.5*skilllv && battle_config.equipment_breaking)
		{
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
			switch (skillid)
			{
				case NPC_BREAKWEAPON:
					pc_breakweapon((struct map_session_data *)bl);
					break;
				case NPC_BREAKARMOR:
					pc_breakarmor((struct map_session_data *)bl);
					break;
				case NPC_BREAKHELM:
					pc_breakhelm((struct map_session_data *)bl);
					break;
				case NPC_BREAKSHIELD:
					pc_breakshield((struct map_session_data *)bl);
					break;
			}
		}
		break;


	case WE_MALE:
		if(sd && dstsd){
			int hp_rate = (skilllv <= 0)? 0:skill_db[skillid].hp_rate[skilllv-1];
			int gain_hp = dstsd->status.max_hp*abs(hp_rate)/100;
			gain_hp = battle_heal(NULL,bl,gain_hp,0,0);
			clif_skill_nodamage(src,bl,skillid,gain_hp,1);
		}
		break;
	case WE_FEMALE:
		if(sd && dstsd){
			int sp_rate = (skilllv <= 0)? 0:skill_db[skillid].sp_rate[skilllv-1];
			int gain_sp = dstsd->status.max_sp*abs(sp_rate) / 100;
			gain_sp = battle_heal(NULL,bl,0,gain_sp,0);
			clif_skill_nodamage(src,bl,skillid,gain_sp,1);
		}
		break;

	case WE_CALLPARTNER:
		if (sd && dstsd) {
			if ((dstsd = pc_get_partner(sd)) == NULL) {
				clif_skill_fail(sd, skillid, 0, 0);
				map_freeblock_unlock();
				return 0;
			}
			if(map[sd->bl.m].flag.nomemo || map[sd->bl.m].flag.nowarpto || map[dstsd->bl.m].flag.nowarp){
				clif_skill_teleportmessage(sd,1);
				map_freeblock_unlock();
				return 0;
			}
			skill_unitsetting(src,skillid,skilllv,sd->bl.x,sd->bl.y,0);
		}
		break;

	case PF_HPCONVERSION:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		if(sd){
			int hp = 0, sp = 0;
			hp = sd->status.max_hp / 10;
			sp = hp * 10 * skilllv / 100;
			if (sd->status.sp + sp > sd->status.max_sp)
				sp = sd->status.max_sp - sd->status.sp;
			if (sd->status.sp + skill_get_sp(skillid, skilllv) >= sd->status.max_sp)
				hp = sp = 0;
			pc_heal(sd,-hp,sp);
			clif_heal(sd->fd,SP_SP,sp);
		}
		break;
	case HT_REMOVETRAP:				/* �����[�u�g���b�v */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
	  {
		struct skill_unit *su=NULL;
		struct item item_tmp;
		int flag;
		if((bl->type==BL_SKILL) &&
		   (su=(struct skill_unit *)bl) &&
		   (su->group->src_id == src->id || map[bl->m].flag.pvp || map[bl->m].flag.gvg) &&
		   (su->group->unit_id >= 0x8f && su->group->unit_id <= 0x99) &&
		   (su->group->unit_id != 0x92))
		{
			if (sd && sd->sc_data[SC_INTOABYSS].timer == -1)	// Prevent free trap exploit [Harbin]
			{
				if(battle_config.skill_removetrap_type == 1)
				{
					for(i=0;i<10;i++) {
						if(skill_db[su->group->skill_id].itemid[i] > 0)
						{
							memset(&item_tmp,0,sizeof(item_tmp));
							item_tmp.nameid = skill_db[su->group->skill_id].itemid[i];
							item_tmp.identify = 1;
							if (item_tmp.nameid && (flag = pc_additem(sd, &item_tmp, skill_db[su->group->skill_id].amount[i])))
							{
								clif_additem(sd, 0, 0, flag);
								map_addflooritem(&item_tmp, skill_db[su->group->skill_id].amount[i], sd->bl.m, sd->bl.x, sd->bl.y, NULL, NULL, NULL, sd->bl.id, 0);
							}
						}
					}
				} else {
					memset(&item_tmp,0,sizeof(item_tmp));
					item_tmp.nameid = 1065;
					item_tmp.identify = 1;
					if (item_tmp.nameid && (flag = pc_additem(sd, &item_tmp, 1))){
						clif_additem(sd, 0, 0, flag);
						map_addflooritem(&item_tmp, 1, sd->bl.m, sd->bl.x, sd->bl.y, NULL, NULL, NULL, sd->bl.id, 0);
					}
				}
			}
			if(su->group->unit_id == 0x91 && su->group->val2){
				struct block_list *target=map_id2bl(su->group->val2);
				if(target && (target->type == BL_PC || target->type == BL_MOB))
					status_change_end(target,SC_ANKLE,-1);
			}
			skill_delunit(su);
		}
	  }
		break;
	case HT_SPRINGTRAP:				/* �X�v�����O�g���b�v */
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
	  {
		struct skill_unit *su=NULL;
		if((bl->type==BL_SKILL) && (su=(struct skill_unit *)bl) && (su->group) ){
			switch(su->group->unit_id){
				case 0x91: // ankle snare
					if (su->group->val2 != 0)
						// if it is already trapping something don't spring it,
						// remove trap should be used instead
						break;
					// otherwise fallthrough to below
				case 0x8f:	/* �u���X�g�}�C�� */
				case 0x90:	/* �X�L�b�h�g���b�v */
				case 0x93:	/* �����h�}�C�� */
				case 0x94:	/* �V���b�N�E�F�[�u�g���b�v */
				case 0x95:	/* �T���h�}�� */
				case 0x96:	/* �t���b�V���[ */
				case 0x97:	/* �t���[�W���O�g���b�v */
				case 0x98:	/* �N���C���A�[�g���b�v */
				case 0x99:	/* �g�[�L�[�{�b�N�X */
					su->group->unit_id = 0x8c;
					clif_changelook(bl,LOOK_BASE,su->group->unit_id);
					su->group->limit=DIFF_TICK(tick+1500,su->group->tick);
					su->limit=DIFF_TICK(tick+1500,su->group->tick);
			}
		}
	  }
		break;

	case CG_LONGINGFREEDOM:
		if(sc_data[SC_LONGING].timer == -1 && sc_data[SC_DANCING].timer != -1 && sc_data[SC_DANCING].val1 != CG_MOONLIT)
		{
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			status_change_start(bl, SC_LONGING, 100, 0, 0, 0, skill_get_time(skillid, skilllv), 0);		// TODO: check this out [Harbin]
		} else {
			clif_skill_fail(sd, skillid, 0, 0);
		}
		break;

	case CG_TAROTCARD:
		{
			int eff, count = -1;
			if (rand() % 100 > skilllv * 8) {
				if (sd) clif_skill_fail(sd,skillid,0,0);
				map_freeblock_unlock();
				return 0;
			}
			do {
				eff = rand() % 14;
				clif_specialeffect(bl, 523 + eff, 0);
				switch (eff)
				{
				case 0:	// heals SP to 0
					if (dstsd)
						pc_heal(dstsd,0,-dstsd->status.sp);
					break;
				case 1:	// matk halved
					status_change_start(bl,SC_INCMATKRATE,-50,0,0,0,30000,0);
					break;
				case 2:	// all buffs removed
					status_change_clear_buffs(bl);
					break;
				case 3:	// 1000 damage, random armor destroyed
					{
						int where[] = { EQP_ARMOR, EQP_SHIELD, EQP_HELM };
						battle_damage(src, bl, 1000, 0);
						clif_damage(src,bl,tick,0,0,1000,0,0,0);
						if (dstsd)
							pc_break_equip(dstsd, where[rand() % 3]);
					}
					break;
				case 4:	// atk halved
					status_change_start(bl,SC_INCATKRATE,-50,0,0,0,30000,0);
					break;
				case 5:	// 2000HP heal, random teleported
					battle_heal(src, src, 2000, 0, 0);
					if(sd && !map[src->m].flag.noteleport)
						pc_randomwarp(sd);
					else if(md)
						mob_warp(md,-1,-1,-1,3);
					break;
				case 6:	// random 2 other effects
					if (count == -1)
						count = 3;
					else
						count++; //Should not retrigger this one.
					break;
				case 7:	// stop freeze or stoned
					{
						int sc[] = { SC_STOP, SC_FREEZE, SC_STONE };
						status_change_start(bl,sc[rand()%3],skilllv,0,0,0,30000,0);
					}
					break;
				case 8:	// curse coma and poison
					status_change_start(bl,SC_COMA,skilllv,0,0,0,30000,0);
					status_change_start(bl,SC_CURSE,skilllv,0,0,0,30000,0);
					status_change_start(bl,SC_POISON,skilllv,0,0,0,30000,0);
					break;
				case 9:	// chaos
					status_change_start(bl,SC_CONFUSION,skilllv,0,0,0,30000,0);
					break;
				case 10:	// 6666 damage, atk matk halved, cursed
					battle_damage(src, bl, 6666, 0);
					clif_damage(src,bl,tick,0,0,6666,0,0,0);
					status_change_start(bl,SC_INCATKRATE,-50,0,0,0,30000,0);
					status_change_start(bl,SC_INCMATKRATE,-50,0,0,0,30000,0);
					status_change_start(bl,SC_CURSE,skilllv,0,0,0,30000,0);
					break;
				case 11:	// 4444 damage
					battle_damage(src, bl, 4444, 0);
					clif_damage(src,bl,tick,0,0,4444,0,0,0);
					break;
				case 12:	// stun
					status_change_start(bl,SC_STAN,skilllv,0,0,0,5000,0);
					break;
				case 13:	// atk,matk,hit,flee,def reduced
					status_change_start(bl,SC_INCATKRATE,-20,0,0,0,30000,0);
					status_change_start(bl,SC_INCMATKRATE,-20,0,0,0,30000,0);
					status_change_start(bl,SC_INCHITRATE,-20,0,0,0,30000,0);
					status_change_start(bl,SC_INCFLEERATE,-20,0,0,0,30000,0);
					status_change_start(bl,SC_INCDEFRATE,-20,0,0,0,30000,0);
					break;
				default:
					break;			
				}
			} while ((--count) > 0);
			clif_skill_nodamage(src,bl,skillid,skilllv,1);
		}
		break;

	case BD_ENCORE:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		if (sd)
			skill_use_id(sd, src->id, sd->skillid_dance, sd->skilllv_dance);
		break;

	case AS_SPLASHER:
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		status_change_start(bl, SkillStatusChangeTable[skillid], skilllv, skillid, src->id, skill_get_time(skillid, skilllv), 1000, 0);
		break;

	case PF_MINDBREAKER:
	  {
		struct status_change *sc_data = status_get_sc_data(bl);

		if((bl->type==BL_MOB && status_get_mode(bl) & 0x20) || battle_check_undead(status_get_race(bl), status_get_elem_type(bl)))
		{
			map_freeblock_unlock();
			return 1;
		}

		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
		status_change_start(bl,SC_MINDBREAKER,skilllv,0,0,0,skill_get_time(skillid,skilllv),0 );

		if (dstmd && dstmd->skilltimer != -1 && dstmd->state.skillcastcancel)
			skill_castcancel(bl,0);
		if (dstsd && dstsd->skilltimer != -1 && (!dstsd->special_state.no_castcancel || map[bl->m].flag.gvg) && dstsd->state.skillcastcancel && !dstsd->special_state.no_castcancel2)
			skill_castcancel(bl,0);

		if(sc_data)
		{
			if(sc_data[SC_FREEZE].timer!=-1)
				status_change_end(bl,SC_FREEZE,-1);
			if(sc_data[SC_STONE].timer!=-1 && sc_data[SC_STONE].val2==0)
				status_change_end(bl,SC_STONE,-1);
			if(sc_data[SC_SLEEP].timer!=-1)
				status_change_end(bl,SC_SLEEP,-1);
		}

		if(bl->type == BL_MOB)
		{
			int range = skill_get_range(skillid,skilllv);
			if(range < 0)
				range = status_get_range(src) - (range + 1);
			mob_target((struct mob_data *)bl,src,range);
		}
	  }
		break;

	case PF_SOULCHANGE:
	  {
		int sp1 = 0, sp2 = 0;
		if (sd) {
			if (dstsd) {
				sp1 = sd->status.sp > dstsd->status.max_sp ? dstsd->status.max_sp : sd->status.sp;
				sp2 = dstsd->status.sp > sd->status.max_sp ? sd->status.max_sp : dstsd->status.sp;
				sd->status.sp = sp2;
				dstsd->status.sp = sp1;
				clif_heal(sd->fd, SP_SP, sp2);
				clif_updatestatus(sd, SP_SP);
				clif_heal(dstsd->fd, SP_SP, sp1);
				clif_updatestatus(dstsd, SP_SP);
			} else if (dstmd) {
				if (dstmd->state.soul_change_flag) {
					clif_skill_fail(sd, skillid, 0, 0);
					map_freeblock_unlock();
					return 0;
				}
				sp2 = sd->status.max_sp * 3 /100;
				if (sd->status.sp + sp2 > sd->status.max_sp)
					sp2 = sd->status.max_sp - sd->status.sp;
				sd->status.sp += sp2;
				clif_heal(sd->fd, SP_SP, sp2);
				clif_updatestatus(sd, SP_SP);
				dstmd->state.soul_change_flag = 1;
			}
		}
		clif_skill_nodamage(src, bl, skillid, skilllv, 1);
	  }
		break;

	// Weapon Refining [Celest]
	case WS_WEAPONREFINE:
		if(sd)
			clif_item_refine_list(sd);
		break;

	// Slim Pitcher
	case CR_SLIMPITCHER:
	  {
		if (sd && flag & 1) {
			struct block_list tbl;
			int hp = sd->potion_hp * (100 + pc_checkskill(sd, CR_SLIMPITCHER) * 10 + pc_checkskill(sd, AM_POTIONPITCHER) * 10 + pc_checkskill(sd, AM_LEARNINGPOTION) * 5) / 100;
			hp = hp * (100 + (status_get_vit(bl) << 1)) / 100;
			if (dstsd) {
				hp = hp * (100 + pc_checkskill(dstsd, SM_RECOVERY) * 10) / 100;
			}
			tbl.id = 0;
			tbl.m = src->m;
			tbl.x = src->x;
			tbl.y = src->y;
			clif_skill_nodamage(&tbl, bl, AL_HEAL, hp, 1);
			battle_heal(NULL, bl, hp, 0, 0);
		}
	  }
		break;

	// Full Chemical Protection
	case CR_FULLPROTECTION:
	  {
		int i, skilltime;
		struct status_change *tsc_data = status_get_sc_data(bl);
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
		skilltime = skill_get_time(skillid,skilllv);
		for (i=0; i<4; i++) {
			if(tsc_data && tsc_data[SC_STRIPWEAPON + i].timer != -1)
				status_change_end(bl, SC_STRIPWEAPON + i, -1 );
			status_change_start(bl,SC_CP_WEAPON + i,skilllv,0,0,0,skilltime,0 );
		}
	  }
		break;

	case RG_CLEANER:
		clif_skill_nodamage(src,bl,skillid,skilllv,1);
	  {
		struct skill_unit *su;
		if((bl->type==BL_SKILL) &&
		   (su=(struct skill_unit *)bl) &&
		   (su->group->src_id == src->id || map[bl->m].flag.pvp || map[bl->m].flag.gvg) &&
		   (su->group->unit_id == 0xb0)){ //㩂����Ԃ�
			if(sd)
			skill_delunit(su);
		}
	  }
		break;

	case ST_PRESERVE:
		if (sd) {
			if (sd->sc_count && sd->sc_data[SC_PRESERVE].timer != -1)
				status_change_end(src, SC_PRESERVE, -1);
			else
				status_change_start(src, SC_PRESERVE, skilllv, 0, 0, 0, skill_get_time(skillid, skilllv), 0);
			clif_skill_nodamage(src, src, skillid, skilllv, 1);
		}
		break;

	// New guild skills [Celest]
	case GD_BATTLEORDER:
		// Only usable during WoE
		if (!agit_flag) { // 0: WoE not starting, Woe is running
			clif_skill_fail(sd, skillid, 0, 0);
			map_freeblock_unlock();
			return 0;
		}
	  {
		struct guild *g = NULL;
		if (flag & 1) {
			if (dstsd && dstsd->status.guild_id == sd->status.guild_id) {
				status_change_start(&dstsd->bl, SC_BATTLEORDERS, skilllv, 0, 0, 0, 0, 0);
			}
		} else if (sd && sd->status.guild_id > 0 && (g = guild_search(sd->status.guild_id)) &&
		           strcmp(sd->status.name, g->master) == 0) {
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			map_foreachinarea(skill_area_sub,
			                  src->m, src->x-15, src->y-15, src->x+15, src->y+15, 0,
			                  src, skillid, skilllv, tick, flag|BCT_ALL|1,
			                  skill_castend_nodamage_id);
			pc_blockskill_start(sd, skillid, 300000);
		}
	  }
		break;
	case GD_REGENERATION:
		// Only usable during WoE
		if (!agit_flag) { // 0: WoE not starting, Woe is running
			clif_skill_fail(sd, skillid, 0, 0);
			map_freeblock_unlock();
			return 0;
		}
	  {
		struct guild *g = NULL;
		if (flag & 1) {
			if (dstsd && dstsd->status.guild_id == sd->status.guild_id) {
				status_change_start(&dstsd->bl, SC_REGENERATION, skilllv, 0, 0, 0, 0, 0);
			}
		} else if (sd && sd->status.guild_id > 0 && (g = guild_search(sd->status.guild_id)) &&
		           strcmp(sd->status.name, g->master) == 0) {
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			map_foreachinarea(skill_area_sub,
			                  src->m, src->x-15, src->y-15, src->x+15, src->y+15, 0,
			                  src, skillid, skilllv, tick, flag|BCT_ALL|1,
			                  skill_castend_nodamage_id);
			pc_blockskill_start(sd, skillid, 300000);
		}
	  }
		break;
	case GD_RESTORE:
		// Only usable during WoE
		if (!agit_flag) { // 0: WoE not starting, Woe is running
			clif_skill_fail(sd, skillid, 0, 0);
			map_freeblock_unlock();
			return 0;
		}
	  {
		struct guild *g = NULL;
		if (flag & 1) {
			if (dstsd && dstsd->status.guild_id == sd->status.guild_id) {
				int hp, sp;
				hp = dstsd->status.max_hp * 9 / 10;
				sp = dstsd->status.max_sp * 9 / 10;
				sp = (dstsd->status.sp + sp <= dstsd->status.max_sp) ? sp : dstsd->status.max_sp - dstsd->status.sp;
				clif_skill_nodamage(src, bl, AL_HEAL, hp, 1);
				battle_heal(NULL, bl, hp, sp, 0);
			}
		} else if (sd && sd->status.guild_id > 0 && (g = guild_search(sd->status.guild_id)) &&
		           strcmp(sd->status.name, g->master) == 0) {
			clif_skill_nodamage(src, bl, skillid, skilllv, 1);
			map_foreachinarea(skill_area_sub,
			                  src->m, src->x-15, src->y-15, src->x+15, src->y+15, 0,
			                  src, skillid, skilllv, tick, flag|BCT_ALL|1,
			                  skill_castend_nodamage_id);
			pc_blockskill_start (sd, skillid, 300000);
		}
	  }
		break;
	case GD_EMERGENCYCALL:
		// Only usable during WoE
		if (!agit_flag || // 0: WoE not starting, Woe is running
		    (sd && map[sd->bl.m].flag.nowarpto && // if not allowed to warp to the map
		     guild_mapname2gc(sd->mapname) == NULL)) { // and it's not a castle...
			clif_skill_fail(sd, skillid, 0, 0);
			map_freeblock_unlock();
			return 0;
		}
	  {
		int dx[9] = {-1, 1, 0, 0,-1, 1,-1, 1, 0};
		int dy[9] = { 0, 0, 1,-1, 1,-1,-1, 1, 0};
		int j = 0;
		struct guild *g = NULL;
		// i don't know if it actually summons in a circle, but oh well. ;P
		if (sd && sd->status.guild_id > 0 && (g = guild_search(sd->status.guild_id)) &&
		    strcmp(sd->status.name, g->master) == 0) {
			for(i = 0; i < g->max_member; i++, j++) {
				if (j > 8) j = 0;
				if ((dstsd = g->member[i].sd) != NULL && sd != dstsd) {
					if (map[dstsd->bl.m].flag.nowarp &&
					    guild_mapname2gc(sd->mapname) == NULL)
						continue;
					clif_skill_nodamage(src, bl, skillid, skilllv, 1);
					if (map_getcell(sd->bl.m, sd->bl.x + dx[j], sd->bl.y + dy[j], CELL_CHKNOPASS))
						dx[j] = dy[j] = 0;
					pc_setpos(dstsd, sd->mapname, sd->bl.x + dx[j], sd->bl.y + dy[j], 2);
				}
			}
			pc_blockskill_start(sd, skillid, 300000);
		}
	  }
		break;

	default:
		printf("Unknown skill used:%d\n",skillid);
		map_freeblock_unlock();
		return 1;
	}

	map_freeblock_unlock();

	return 0;
}

/*==========================================
 * Skill Cast End ID
 *------------------------------------------
 */
TIMER_FUNC(skill_castend_id) {
	struct map_session_data* sd = map_id2sd(id);
	struct block_list *bl;
	int range,inf2;

	nullpo_retr(0, sd);

	if (sd->bl.prev == NULL)
		return 0;

	if (sd->skillid != SA_CASTCANCEL) {
		if (sd->skilltimer != tid)
			return 0;
		if (sd->skilltimer != -1 && pc_checkskill(sd, SA_FREECAST) > 0) {
			sd->speed = sd->prev_speed;
			clif_updatestatus(sd, SP_SPEED);
		}
		sd->skilltimer = -1;
	}

	if ((bl = map_id2bl(sd->skilltarget)) == NULL || bl->prev == NULL) {
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}
	if (sd->bl.m != bl->m || pc_isdead(sd)) {
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}

	if (sd->skillid == PR_LEXAETERNA) {
		struct status_change *sc_data = status_get_sc_data(bl);
		if (sc_data && (sc_data[SC_FREEZE].timer != -1 || (sc_data[SC_STONE].timer != -1 && sc_data[SC_STONE].val2 == 0))) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}
	else if (sd->skillid == RG_BACKSTAP) {
		int dir = map_calc_dir(&sd->bl,bl->x,bl->y);
		int t_dir = status_get_dir(bl);
		int dist = distance(sd->bl.x,sd->bl.y,bl->x,bl->y);
		if (bl->type != BL_SKILL && (dist == 0 || map_check_dir(dir,t_dir))) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}

	inf2 = skill_get_inf2(sd->skillid);
	if (((skill_get_inf(sd->skillid) & 1) || inf2 & 4) &&
	    battle_check_target(&sd->bl, bl, BCT_ENEMY) <= 0) {
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}
	if (inf2 & 0xC00 && sd->bl.id != bl->id) {
		int fail_flag = 1;
		if (inf2 & 0x400 && battle_check_target(&sd->bl, bl, BCT_PARTY) > 0)
			fail_flag = 0;
		if (inf2 & 0x800 && sd->status.guild_id > 0 && sd->status.guild_id == status_get_guild_id(bl))
			fail_flag = 0;
		if (fail_flag) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}

	range = skill_get_range(sd->skillid,sd->skilllv);
	if (range < 0)
		range = status_get_range(&sd->bl) - (range + 1);
	range += battle_config.pc_skill_add_range;
	if ((sd->skillid == MO_EXTREMITYFIST && sd->sc_data[SC_COMBO].timer != -1 && sd->sc_data[SC_COMBO].val1 == MO_COMBOFINISH) ||
	    (sd->skillid == CH_TIGERFIST && sd->sc_data[SC_COMBO].timer != -1 && sd->sc_data[SC_COMBO].val1 == MO_COMBOFINISH) ||
	    (sd->skillid == CH_CHAINCRUSH && sd->sc_data[SC_COMBO].timer != -1 && sd->sc_data[SC_COMBO].val1 == MO_COMBOFINISH) ||
	    (sd->skillid == CH_CHAINCRUSH && sd->sc_data[SC_COMBO].timer != -1 && sd->sc_data[SC_COMBO].val1 == CH_TIGERFIST))
		range += skill_get_blewcount(MO_COMBOFINISH,sd->sc_data[SC_COMBO].val2);
	if (battle_config.skill_out_range_consume) { // changed to allow casting when target walks out of range [Valaris]
		if (range < distance(sd->bl.x, sd->bl.y, bl->x, bl->y)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}
	if (!skill_check_condition(sd,1)) {
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}
	sd->skillitem = sd->skillitemlv = -1;
	if (battle_config.skill_out_range_consume) {
		if (range < distance(sd->bl.x, sd->bl.y, bl->x, bl->y)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}

	if (battle_config.pc_skill_log)
		printf("PC %d skill castend skill=%d\n", sd->bl.id, sd->skillid);
	pc_stop_walking(sd, 0);

	switch(skill_get_nk(sd->skillid))
	{
	case 0:
	case 2:
		skill_castend_damage_id(&sd->bl, bl, sd->skillid, sd->skilllv, tick, 0);
		break;
	case 1:/* �x���n */
		if ((sd->skillid == AL_HEAL || (sd->skillid == ALL_RESURRECTION && bl->type != BL_PC) || sd->skillid == PR_ASPERSIO) && battle_check_undead(status_get_race(bl), status_get_elem_type(bl)))
			skill_castend_damage_id(&sd->bl, bl, sd->skillid, sd->skilllv, tick, 0);
		else
			skill_castend_nodamage_id(&sd->bl, bl, sd->skillid, sd->skilllv, tick, 0);
		break;
	}

	return 0;
}

/*==========================================
 * Skill Cast End Pos2
 *------------------------------------------
 */
int skill_castend_pos2(struct block_list *src, int x, int y, int skillid, int skilllv, unsigned int tick, int flag)
{
	struct map_session_data *sd = NULL;
	int i, tmpx = 0, tmpy = 0, x1 = 0, y_1 = 0;

	if (skillid > 0 && skilllv <= 0) return 0; // celest

	nullpo_retr(0, src);

	if (src->type == BL_PC) {
		nullpo_retr(0, sd = (struct map_session_data *)src);
	}
	if (skillid != WZ_METEOR &&
	    skillid != AM_CANNIBALIZE &&
	    skillid != AM_SPHEREMINE)
		clif_skill_poseffect(src, skillid, skilllv, x, y, tick);

	if (sd && skillnotok(skillid, sd))
		return 0;

	switch(skillid)
	{
	case PR_BENEDICTIO:			/* ���̍~�� */
		skill_area_temp[1]=src->id;
		map_foreachinarea(skill_area_sub,
			src->m,x-1,y-1,x+1,y+1,0,
			src,skillid,skilllv,tick, flag|BCT_NOENEMY|1,
			skill_castend_nodamage_id);
		map_foreachinarea(skill_area_sub,
			src->m,x-1,y-1,x+1,y+1,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|1,
			skill_castend_damage_id);
		break;

	case BS_HAMMERFALL:			/* �n���}�[�t�H�[�� */
		skill_area_temp[1]=src->id;
		skill_area_temp[2]=x;
		skill_area_temp[3]=y;
		map_foreachinarea(skill_area_sub,
			src->m,x-2,y-2,x+2,y+2,0,
			src,skillid,skilllv,tick, flag|BCT_ENEMY|2,
			skill_castend_nodamage_id);
		break;

	case HT_DETECTING:				/* �f�B�e�N�e�B���O */
		{
			const int range = 7;
			if (src->x != x)
				x += (src->x - x > 0) ? -range : range;
			if (src->y != y)
				y += (src->y - y > 0) ? -range: range;
			map_foreachinarea(status_change_timer_sub,
				src->m, x - range, y - range, x + range, y + range, 0,
				src, SC_SIGHT, tick);
		}
		break;

	case MG_SAFETYWALL:			/* �Z�C�t�e�B�E�H�[�� */
	case MG_FIREWALL:			/* �t�@�C���[�E�H�[�� */
	case MG_THUNDERSTORM:		/* �T���_�[�X�g�[�� */
	case AL_PNEUMA:				/* �j���[�} */
	case WZ_ICEWALL:			/* �A�C�X�E�H�[�� */
	case WZ_FIREPILLAR:			/* �t�@�C�A�s���[ */
	case WZ_QUAGMIRE:			/* �N�@�O�}�C�A */
	case WZ_VERMILION:			/* ���[�h�I�u���@�[�~���I�� */
	case WZ_STORMGUST:			/* �X�g�[���K�X�g */
	case WZ_HEAVENDRIVE:		/* �w�����Y�h���C�u */
	case PR_SANCTUARY:			/* �T���N�`���A�� */
	case PR_MAGNUS:				/* �}�O�k�X�G�N�\�V�Y�� */
	case CR_GRANDCROSS:			/* �O�����h�N���X */
	case NPC_DARKGRANDCROSS:	/*�ŃO�����h�N���X*/
	case HT_SKIDTRAP:			/* �X�L�b�h�g���b�v */
	case HT_LANDMINE:			/* �����h�}�C�� */
	case HT_ANKLESNARE:			/* �A���N���X�l�A */
	case HT_SHOCKWAVE:			/* �V���b�N�E�F�[�u�g���b�v */
	case HT_SANDMAN:			/* �T���h�}�� */
	case HT_FLASHER:			/* �t���b�V���[ */
	case HT_FREEZINGTRAP:		/* �t���[�W���O�g���b�v */
	case HT_BLASTMINE:			/* �u���X�g�}�C�� */
	case HT_CLAYMORETRAP:		/* �N���C���A�[�g���b�v */
	case AS_VENOMDUST:			/* �x�m���_�X�g */
	case AM_DEMONSTRATION:			/* �f�����X�g���[�V���� */
	case PF_SPIDERWEB:			/* �X�p�C�_�[�E�F�b�u */
	case PF_FOGWALL:			/* �t�H�O�E�H�[�� */
	case HT_TALKIEBOX:			/* �g�[�L�[�{�b�N�X */
	case AC_SHOWER:
		skill_unitsetting(src,skillid,skilllv,x,y,0);
			break;

	case RG_GRAFFITI:			/* Graffiti [Valaris] */
		skill_clear_unitgroup(src);
		skill_unitsetting(src,skillid,skilllv,x,y,0);
			break;

	case SA_VOLCANO:		/* �{���P�[�m */
	case SA_DELUGE:			/* �f�����[�W */
	case SA_VIOLENTGALE:	/* �o�C�I�����g�Q�C�� */
	case SA_LANDPROTECTOR:	/* �����h�v���e�N�^�[ */
		skill_clear_element_field(src);
		skill_unitsetting(src,skillid,skilllv,x,y,0);
		break;

	case WZ_METEOR:				//���e�I�X�g�[��
		{
			int flag = 0;
			for(i = 0; i < 2 + (skilllv >> 1); i++) {
				int j = 0;
				do {
					tmpx = x + (rand()%7 - 3);
					tmpy = y + (rand()%7 - 3);
					if(tmpx < 0)
						tmpx = 0;
					else if(tmpx >= map[src->m].xs)
						tmpx = map[src->m].xs - 1;
					if(tmpy < 0)
						tmpy = 0;
					else if(tmpy >= map[src->m].ys)
						tmpy = map[src->m].ys - 1;
					j++;
				} while(map_getcell(src->m, tmpx, tmpy, CELL_CHKNOPASS) && j < 100);
				if(j >= 100)
					continue;
				if (flag == 0) {
					clif_skill_poseffect(src,skillid,skilllv,tmpx,tmpy,tick);
					flag=1;
				}
				if (i > 0)
					skill_addtimerskill(src, tick + i * 1000, 0, tmpx, tmpy, skillid, skilllv, (x1<<16)|y_1, flag);
				x1  = tmpx;
				y_1 = tmpy;
			}
			skill_addtimerskill(src, tick + i * 1000, 0, tmpx, tmpy, skillid, skilllv, -1, flag);
		}
		break;

	case AL_WARP:				/* ���[�v�|�[�^�� */
		if (sd) {
			if (map[sd->bl.m].flag.noteleport) /* �e���|�֎~ */
				break;
			clif_skill_warppoint(sd, sd->skillid, sd->status.save_point.map,
			                     (sd->skilllv > 1) ? sd->status.memo_point[0].map : "",
			                     (sd->skilllv > 2) ? sd->status.memo_point[1].map : "",
			                     (sd->skilllv > 3) ? sd->status.memo_point[2].map : ""); // MAX_PORTAL_MEMO
		}
		break;
	case MO_BODYRELOCATION:
		if (sd) {
			pc_movepos(sd, x, y);
			pc_blockskill_start(sd, MO_EXTREMITYFIST, 2000);
		} else if (src->type == BL_MOB)
			mob_warp((struct mob_data *)src, -1, x, y, 0);
		break;
	case AM_CANNIBALIZE: // �o�C�I�v�����g
		if (sd) {
			int mx, my, id = 0;
			struct mob_data *md;
			int summons[5] = { 1020, 1068, 1118, 1500, 1368 };

			mx = x; // + (rand() % 10 - 5);
			my = y; // + (rand() % 10 - 5);
			id = mob_once_spawn(sd, "this", mx, my, "--ja--", ((skilllv < 6) ? summons[skilllv-1] : 1368), 1, "");
			if ((md = (struct mob_data *)map_id2bl(id)) !=NULL) {
				md->master_id = sd->bl.id;
				md->hp = 2210 + skilllv * 200;
				md->state.special_mob_ai = 1; // 0: nothing, 1: cannibalize, 2-3: spheremine
				md->deletetimer = add_timer(gettick_cache + skill_get_time(skillid, skilllv), mob_timer_delete, id, 0);
			}
			clif_skill_poseffect(src, skillid, skilllv, x, y, tick);
		}
		break;
	case AM_SPHEREMINE:	// �X�t�B�A�[�}�C��
		if (sd) {
			int mx, my, id = 0;
			struct mob_data *md;

			mx = x; // + (rand()%10 - 5);
			my = y; // + (rand()%10 - 5);
			id = mob_once_spawn(sd, "this", mx, my, "--ja--", 1142, 1, "");
			if ((md = (struct mob_data *)map_id2bl(id)) != NULL) {
				md->master_id = sd->bl.id;
				md->hp = 2000 + skilllv * 400;
				md->state.special_mob_ai = 2; // 0: nothing, 1: cannibalize, 2-3: spheremine
				md->deletetimer = add_timer(gettick_cache + skill_get_time(skillid, skilllv), mob_timer_delete, id, 0);
			}
			clif_skill_poseffect(src, skillid, skilllv, x, y, tick);
		}
		break;

	case CR_SLIMPITCHER:
	  {
		if (sd) {
			int i = skilllv % 11 - 1;
			int j = pc_search_inventory(sd,skill_db[skillid].itemid[i]);
			if (j < 0 || skill_db[skillid].itemid[i] <= 0 || sd->inventory_data[j] == NULL ||
				sd->status.inventory[j].amount < skill_db[skillid].amount[i]) {
				clif_skill_fail(sd, skillid, 0, 0);
				return 1;
			}
			sd->state.potionpitcher_flag = 1;
			sd->potion_hp = 0;
			run_script(sd->inventory_data[j]->script, 0, sd->bl.id, 0);
			pc_delitem(sd, j, skill_db[skillid].amount[i], 0);
			sd->state.potionpitcher_flag = 0;
			clif_skill_poseffect(src, skillid, skilllv, x, y, tick);
			if (sd->potion_hp > 0) {
				map_foreachinarea(skill_area_sub,
				                  src->m, x-3, y-3, x+3, y+3, 0,
				                  src, skillid, skilllv, tick, flag|BCT_PARTY|1,
				                  skill_castend_nodamage_id);
			}
		}
	  }
		break;

	case HW_GANBANTEIN:
		if (rand()%100 < 80) {
			clif_skill_poseffect(src,skillid,skilllv,x,y,tick);
			map_foreachinarea(skill_ganbatein, src->m, x - 1, y - 1, x + 1, y + 1, BL_SKILL);
		} else {
			clif_skill_fail(sd,skillid,0,0);
			return 1;
		}
		break;

	case HW_GRAVITATION:
		{
			struct skill_unit_group *sg;
			clif_skill_poseffect(src,skillid,skilllv,x,y,tick);
			sg = skill_unitsetting(src,skillid,skilllv,x,y,0);	
			status_change_start(src, SkillStatusChangeTable[skillid], skilllv, 0, BCT_SELF, (intptr_t)sg, skill_get_time(skillid, skilllv), 0);
		}
		break;

	}

	return 0;
}

/*==========================================
 * Skill Cast End Map
 *------------------------------------------
 */
void skill_castend_map(struct map_session_data *sd, int skill_num, const char *mapname) {
	int x = 0, y = 0;

//	nullpo_retv(sd); // checked before to call function

	if (pc_isdead(sd))
		return;

	if (skillnotok(skill_num, sd))
		return;

	if (sd->opt1 > 0 || sd->status.option & 2)
		return;
	if (sd->sc_count) {
		if (sd->sc_data[SC_SILENCE].timer!=-1 ||
		    sd->sc_data[SC_ROKISWEIL].timer!=-1 ||
		    sd->sc_data[SC_AUTOCOUNTER].timer != -1 ||
		    sd->sc_data[SC_STEELBODY].timer != -1 ||
		    sd->sc_data[SC_DANCING].timer!=-1 ||
		    sd->sc_data[SC_MARIONETTE].timer != -1)
			return;
	}

	if (skill_num != sd->skillid)
		return;

	pc_stopattack(sd);

	if (battle_config.pc_skill_log)
		printf("PC %d skill castend skill =%d map=%s\n", sd->bl.id, skill_num, mapname);
	pc_stop_walking(sd, 0);

	if (strcmp(mapname, "cancel") == 0)
		return;

	switch(skill_num) {
	case AL_TELEPORT:
		if (strcmp(mapname, "Random") == 0)
			pc_randomwarp(sd);
		else
			pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 3);
		break;

	case AL_WARP:
	  {
		const struct point *p[4];
		struct skill_unit_group *group;
		int i;
		int maxcount = 0;
		p[0] = &sd->status.save_point;
		p[1] = &sd->status.memo_point[0];
		p[2] = &sd->status.memo_point[1];
		p[3] = &sd->status.memo_point[2]; // MAX_PORTAL_MEMO

		if ((maxcount = skill_get_maxcount(sd->skillid)) > 0) {
			int c;
			c = 0;
			for(i = 0; i < MAX_SKILLUNITGROUP; i++) {
				if (sd->skillunit[i].alive_count > 0 && sd->skillunit[i].skill_id == sd->skillid)
					c++;
			}
			if (c >= maxcount) {
				clif_skill_fail(sd, sd->skillid, 0, 0);
				sd->canact_tick = gettick_cache;
				sd->canmove_tick = gettick_cache;
				return;
			}
		}

		if (sd->skilllv <= 0)
			return;
		for(i = 0; i < sd->skilllv; i++) {
			if (strcmp(mapname, p[i]->map) == 0) {
				x = p[i]->x;
				y = p[i]->y;
				break;
			}
		}
		if (x == 0 || y == 0)
			return;

		if (!skill_check_condition(sd, 3))
			return;
		if ((group = skill_unitsetting(&sd->bl, sd->skillid, sd->skilllv, sd->skillx, sd->skilly, 0)) == NULL)
			return;
		CALLOC(group->valstr, char, 17); // 16 + NULL
		strncpy(group->valstr, mapname, 16);
		group->val2 = (x << 16) | y;
	  }
		break;
	}

	return;
}

/*==========================================
 * Skill Unit Group
 *------------------------------------------
 */
struct skill_unit_group *skill_unitsetting( struct block_list *src, int skillid,int skilllv,int x,int y,int flag)
{
	struct skill_unit_group *group;
	int i,limit, val1 = 0, val2 = 0, val3 = 0;
	int count = 0;
	int target, interval, range, unit_flag;
	struct skill_unit_layout *layout;
	struct status_change *sc_data;
	int active_flag = 1;

	nullpo_retr(0, src);

	limit = skill_get_time(skillid, skilllv);
	range = skill_get_unit_range(skillid);
	interval = skill_get_unit_interval(skillid);
	target = skill_get_unit_target(skillid);
	unit_flag = skill_get_unit_flag(skillid);
	layout = skill_get_unit_layout(skillid, skilllv, src, x, y);

	if (unit_flag & UF_DEFNOTENEMY && battle_config.defnotenemy)
		target = BCT_NOENEMY;

	sc_data = status_get_sc_data(src); // for firewall and fogwall - celest

	switch(skillid) {

	case MG_SAFETYWALL:			/* �Z�C�t�e�B�E�H�[�� */
		val2 = skilllv + 1;
		break;

	case MG_FIREWALL:			/* �t�@�C���[�E�H�[�� */
		if (sc_data && sc_data[SC_VIOLENTGALE].timer != -1)
			limit = limit * 3 / 2;
		val2 = 4 + skilllv;
		break;

	case AL_WARP:				/* ���[�v�|�[�^�� */
		val1 = skilllv + 6;
		if (flag == 0)
			limit = 2000;
		active_flag = 0;
		break;

	case PR_SANCTUARY:			/* �T���N�`���A�� */
		val1 = (skilllv + 3) * 2;
		val2 = (skilllv > 6) ? 777 : skilllv * 100;
		interval += 500;
		break;

	case WZ_FIREPILLAR:			/* �t�@�C�A�[�s���[ */
		if (flag != 0)
			limit = 1000;
		val1 = skilllv + 2;
		if (skilllv >= 6)
			range=2;
		break;

	case HT_SANDMAN:			/* �T���h�}�� */
	case HT_CLAYMORETRAP:		/* �N���C���A�[�g���b�v */
	case HT_SKIDTRAP:			/* �X�L�b�h�g���b�v */
	case HT_LANDMINE:			/* �����h�}�C�� */
	case HT_ANKLESNARE:			/* �A���N���X�l�A */
	case HT_FLASHER:			/* �t���b�V���[ */
	case HT_FREEZINGTRAP:		/* �t���[�W���O�g���b�v */
	case HT_BLASTMINE:			/* �u���X�g�}�C�� */
		if (map[src->m].flag.gvg) limit *= 4;
		break;

	case HT_SHOCKWAVE:			/* �V���b�N�E�F�[�u�g���b�v */
		val1 = skilllv * 15 + 10;
		break;

	case SA_LANDPROTECTOR:	/* �O�����h�N���X */
	  {
		int aoe_diameter = 0;
		val1 = skilllv * 15 + 10;
		if(skilllv == 1 || skilllv == 2)
			aoe_diameter = 7;
		if(skilllv == 3 || skilllv == 4)
			aoe_diameter = 9;
		if(skilllv == 5)
			aoe_diameter = 11;
		count = aoe_diameter * aoe_diameter;
	  }
		break;

	case BA_WHISTLE:			/* ���J */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, BA_MUSICALLESSON) + 1) >> 1;
		val2 = ((status_get_agi(src) / 10) & 0xffff) << 16;
		val2 |= (status_get_luk(src) / 10) & 0xffff;
		break;

	case DC_HUMMING:			/* �n�~���O */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, DC_DANCINGLESSON) + 1) >> 1;
		val2 = status_get_dex(src) / 10;
		break;

	case DC_DONTFORGETME:		/* ����Y��Ȃ��Łc */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, DC_DANCINGLESSON) + 1) >> 1;
		val2 = ((status_get_str(src) / 20) & 0xffff) << 16;
		val2 |= (status_get_agi(src) / 10) & 0xffff;
		break;

	case BA_POEMBRAGI:			/* �u���M�̎� */
		if (src->type == BL_PC)
			val1 = pc_checkskill((struct map_session_data *)src, BA_MUSICALLESSON);
		val2 = ((status_get_dex(src) / 10) & 0xffff) << 16;
		val2 |= (status_get_int(src) / 5) & 0xffff;
		break;

	case BA_APPLEIDUN:			/* �C�h�D���̗ь� */
		if (src->type == BL_PC)
			val1 = pc_checkskill((struct map_session_data *)src, BA_MUSICALLESSON) & 0xffff;
		val2 |= (status_get_vit(src)) & 0xffff;
		val3 = 0;//�񕜗p�^�C���J�E���^(6�b?��1?��)
		break;

	case DC_SERVICEFORYOU:		/* �T�[�r�X�t�H�[���[ */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, DC_DANCINGLESSON) + 1) >> 1;
		val2 = status_get_int(src) / 10;
		break;

	case BA_ASSASSINCROSS:		/* �[�z�̃A�T�V���N���X */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, BA_MUSICALLESSON) + 1) >> 1;
		val2 = status_get_agi(src) / 20;
		break;

	case DC_FORTUNEKISS:		/* �K�^�̃L�X */
		if (src->type == BL_PC)
			val1 = (pc_checkskill((struct map_session_data *)src, DC_DANCINGLESSON) + 1) >> 1;
		val2 = status_get_luk(src) / 10;
		break;

	case PF_FOGWALL:	/* �t�H�O�E�H�[�� */
		if (sc_data && sc_data[SC_DELUGE].timer != -1) limit *= 2;
		break;

	case RG_GRAFFITI: /* Graffiti */
		count = 1; // Leave this at 1 [Valaris]
		break;
	}

	nullpo_retr(NULL, group = skill_initunitgroup(src, (count > 0 ? count : layout->count),
	            skillid, skilllv, skill_get_unit_id(skillid, flag & 1)));
	group->limit = limit;
	group->val1 = val1;
	group->val2 = val2;
	group->val3 = val3;
	group->target_flag = target;
	group->interval = interval;
	if (skillid == HT_TALKIEBOX || skillid == RG_GRAFFITI) {
		CALLOC(group->valstr, char, 81); // 80 + NULL
		strncpy(group->valstr, talkie_mes, 80); // 80 + NULL
	}
	for(i = 0; i < layout->count; i++) {
		struct skill_unit *unit;
		int ux, uy, val1 = skilllv, val2 = 0, limit = group->limit, alive = 1;
		ux = x + layout->dx[i];
		uy = y + layout->dy[i];

		switch (skillid) {
		case MG_FIREWALL:		/* �t�@�C���[�E�H�[�� */
			val2 = group->val2;
			break;

		case WZ_ICEWALL:		/* �A�C�X�E�H�[�� */
			if (skilllv <= 1)
				val1 = 500;
			else
				val1 = 200 + 200 * skilllv;
			break;

		case RG_GRAFFITI:	/* Graffiti [Valaris] */
			ux += (i % 5 - 2);
			uy += (i / 5 - 2);
			break;
		}
		//����X�L���̏ꍇ�ݒu���W��Ƀ����h�v���e�N�^�[���Ȃ����`�F�b�N
		if(range<=0)
			map_foreachinarea(skill_landprotector,src->m,ux,uy,ux,uy,BL_SKILL,skillid,&alive);

		if (skillid == WZ_ICEWALL && alive) {
			val2 = map_getcell(src->m, ux, uy, CELL_GETTYPE);
			if (val2 == 5 || val2 == 1)
				alive = 0;
			else {
				map_setcell(src->m, ux, uy, 5);
				clif_changemapcell(src->m, ux, uy, 5, 0);
			}
		}

		if (alive) {
			nullpo_retr(NULL, unit=skill_initunit(group, i, ux, uy));
			unit->val1 = val1;
			unit->val2 = val2;
			unit->limit = limit;
			unit->range = range;

			if (range == 0 && active_flag)
				map_foreachinarea(skill_unit_effect, unit->bl.m
				                 , unit->bl.x, unit->bl.y, unit->bl.x, unit->bl.y
				                 , 0, &unit->bl, gettick_cache, 1);
		}
	}

	return group;
}

/*==========================================
 * Skill Unit On Place
 *------------------------------------------
 */
int skill_unit_onplace(struct skill_unit *src, struct block_list *bl, unsigned int tick)
{
	struct skill_unit_group *sg;
	struct block_list *ss;
	struct skill_unit *unit2;
	struct status_change *sc_data;
	int type;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);

	if (bl->prev == NULL || !src->alive || (bl->type == BL_PC && pc_isdead((struct map_session_data *)bl)))
		return 0;

	nullpo_retr(0, sg = src->group);
	nullpo_retr(0, ss = map_id2bl(sg->src_id));

	sc_data = status_get_sc_data(bl);
	type = SkillStatusChangeTable[sg->skill_id];

	if (battle_check_target(&src->bl, bl, sg->target_flag) <= 0)
		return 0;

	if (map_find_skill_unit_oncell(bl, bl->x, bl->y, SA_LANDPROTECTOR, NULL))
		return 0;

	switch (sg->unit_id) {
	case 0x85:	/* �j���[�} */
	case 0x7e:	/* �Z�C�t�e�B�E�H�[�� */
		if (sc_data && sc_data[type].timer == -1)
			status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0, 0, 0);
		break;

	case 0x80:	/* ���[�v�|�[�^��(������) */
		if (bl->type == BL_PC) {
			struct map_session_data *sd = (struct map_session_data *)bl;
			if (sd && src->bl.m == bl->m && src->bl.x == bl->x && src->bl.y == bl->y &&
			    src->bl.x == sd->to_x && src->bl.y == sd->to_y) {
				if (battle_config.chat_warpportal || !sd->chatID) {
					pc_setpos(sd, sg->valstr, sg->val2 >> 16, sg->val2 & 0xffff, 3);
					if (sg->src_id == bl->id || (strcmp(map[src->bl.m].name, sg->valstr) == 0 &&
						src->bl.x == (sg->val2 >> 16) && src->bl.y == (sg->val2 & 0xffff)))
						skill_delunitgroup(sg);
					if (--sg->val1 <= 0)
						skill_delunitgroup(sg);
				}
			}
		} else if (bl->type == BL_MOB && battle_config.mob_warpportal) {
			int m = map_mapname2mapid(sg->valstr); // map id on this server (m == -1 if not in actual map-server)
			// what's about map on other map-servers?
			mob_warp((struct mob_data *)bl, m, sg->val2 >> 16, sg->val2 & 0xffff, 3);
		}
		break;

	case 0x8e:	/* �N�@�O�}�C�A */
		if (bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
			break;
		if (sc_data && sc_data[type].timer == -1)
			status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0,
			                    skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0x9a:	/* �{���P?�m */
	case 0x9b:	/* �f����?�W */
	case 0x9c:	/* �o�C�I�����g�Q�C�� */
		if (sc_data && sc_data[type].timer!=-1) {
			unit2 = (struct skill_unit *)sc_data[type].val2; // correction by akrus (val4 -> val2)
			if (unit2 && unit2->group &&
			    (unit2 == src || DIFF_TICK(sg->tick, unit2->group->tick) <= 0))
				break;
		}
		status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0,
		                    skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0x9e:	/* �q��S */
	case 0x9f:	/* �j�����h�̉� */
	case 0xa0:	/* �i���̍��� */
	case 0xa1:	/* ?���ۂ̋��� */
	case 0xa2:	/* �j?�x�����O�̎w�� */
	case 0xa3:	/* ���L�̋��� */
	case 0xa4:	/* �[���̒��� */
	case 0xa5:	/* �s���g�̃W?�N�t��?�h */
	case 0xa6:	/* �s���a�� */
	case 0xa7:	/* ���J */
	case 0xa8:	/* �[�z�̃A�T�V���N���X */
	case 0xa9:	/* �u���M�̎� */
	case 0xaa:	/* �C�h�D���̗ь� */
	case 0xab:	/* ��������ȃ_���X */
	case 0xac:	/* �n�~���O */
	case 0xad:	/* ����Y��Ȃ��Łc */
	case 0xae:	/* �K�^�̃L�X */
	case 0xaf:	/* �T?�r�X�t�H?��? */
	case 0xb9:
		if (sg->src_id == bl->id)
			break;
		sc_data = status_get_sc_data(bl);
		type = SkillStatusChangeTable[sg->skill_id];
		if (sc_data && sc_data[type].timer == -1)
			status_change_start(bl, type, sg->skill_lv, sg->val1, sg->val2, sg->group_id, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0xb4:
		if (battle_check_target(&src->bl, bl, BCT_NOENEMY) > 0) {
			if (sc_data && sc_data[type].timer != -1) {
				struct skill_unit_group *sg2 = (struct skill_unit_group *)sc_data[type].val4;
				if (sg2 && (sg2 == src->group || DIFF_TICK(sg->tick, sg2->tick) <= 0))
					break;
			} else
				status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		} else if (!status_get_mode(bl) & 0x20)
			skill_blown(&src->bl, bl, 1);
		break;

	case 0xb6:
		if (sc_data && sc_data[type].timer != -1) {
			unit2 = (struct skill_unit *)sc_data[type].val4;
			if (unit2 && unit2->group &&
			    (unit2 == src || DIFF_TICK(sg->tick, unit2->group->tick) <= 0))
				break;
		}
		status_change_start(bl, type, sg->skill_lv, sg->val1, sg->val2,
		                    (intptr_t)src, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		skill_additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_MISC, tick);
		break;

	case 0xb8:
		sc_data = status_get_sc_data(bl);
		type = SkillStatusChangeTable[sg->skill_id];
		if (sc_data && sc_data[type].timer == -1 && (!status_get_mode(bl) & 0x20))
			status_change_start(bl, type, sg->skill_lv, 5 * sg->skill_lv, BCT_ENEMY, sg->group_id, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0xb2:
	case 0xb3:
		break;
	}

	return 0;
}

/*==========================================
 * Skill Unit On Place Timer
 *------------------------------------------
 */
int skill_unit_onplace_timer(struct skill_unit *src, struct block_list *bl, unsigned int tick)
{
	struct skill_unit_group *sg;
	struct block_list *ss;
	int splash_count = 0;
	struct status_change *sc_data;
	struct skill_unit_group_tickset *ts;
	int type;
	int diff = 0;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);

	if (bl->type != BL_PC && bl->type != BL_MOB)
		return 0;

	if (bl->prev == NULL || !src->alive ||
	    (bl->type == BL_PC && pc_isdead((struct map_session_data *)bl)))
		return 0;

	nullpo_retr(0, sg = src->group);
	nullpo_retr(0, ss = map_id2bl(sg->src_id));
	sc_data = status_get_sc_data(bl);
	type = SkillStatusChangeTable[sg->skill_id];

	if (map_find_skill_unit_oncell(bl, bl->x, bl->y, SA_LANDPROTECTOR, NULL))
		return 0;

	nullpo_retr(0, ts = skill_unitgrouptickset_search(bl, sg, tick));
	diff = DIFF_TICK(tick, ts->tick);
	if (sg->skill_id == PR_SANCTUARY)
		diff += 500;
	if (diff < 0)
		return 0;
	ts->tick = tick+sg->interval;
	if (sg->skill_id == CR_GRANDCROSS && !battle_config.gx_allhit)
		ts->tick += sg->interval * (map_count_oncell(bl->m, bl->x, bl->y) - 1);

	switch (sg->unit_id) {
	case 0x83:	/* �T���N�`���A�� */
		{
			int race = status_get_race(bl);

			if (battle_check_undead(race, status_get_elem_type(bl)) || race == 6) {
				if(skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0))
				{
					sg->val1--;
				}
			} else {
				int heal = sg->val2;
				if (status_get_hp(bl) >= status_get_max_hp(bl))
					break;
				if (bl->type == BL_PC && ((struct map_session_data *)bl)->special_state.no_magic_damage)
					heal = 0;
				clif_skill_nodamage(&src->bl, bl, AL_HEAL, heal, 1);
				battle_heal(NULL, bl, heal, 0, 0);
				if (diff >= 500)
					sg->val1--;
			}
			if (sg->val1 <= 0)
				skill_delunitgroup(sg);
			break;
		}

	case 0x84:	/* �}�O�k�X�G�N�\�V�Y�� */
		{
			int race = status_get_race(bl);
			if (!battle_check_undead(race, status_get_elem_type(bl)) && race != 6)
				return 0;
			skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
			src->val2++;
			break;
		}

	case 0x7f:	/* �t�@�C���[�E�H�[�� */
		skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
		if (--src->val2 <= 0)
			skill_delunit(src);
		break;
	case 0x86:	/* ���[�h�I�u���@�[�~���I��(TS,MS,FN,SG,HD,GX,��GX) */
		skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
		break;
	case 0x87:	/* �t�@�C�A�[�s���[(�����O) */
		skill_delunit(src);
		skill_unitsetting(ss, sg->skill_id, sg->skill_lv, src->bl.x, src->bl.y, 1);
		break;

	case 0x88:	/* �t�@�C�A�[�s���[(������) */
		map_foreachinarea(skill_attack_area, bl->m, bl->x - 1, bl->y - 1, bl->x + 1, bl->y + 1, 0,
		                  BF_MAGIC, ss, &src->bl, sg->skill_id, sg->skill_lv, tick, 0, BCT_ENEMY);
		break;

	case 0x90:	/* �X�L�b�h�g���b�v */
		{
			int i, c = skill_get_blewcount(sg->skill_id, sg->skill_lv);
			if (map[bl->m].flag.gvg) c = 0;
			for(i = 0; i < c; i++)
				skill_blown(&src->bl, bl, 1 | 0x30000);
			sg->unit_id = 0x8c;
			clif_changelook(&src->bl, LOOK_BASE, sg->unit_id);
			sg->limit = DIFF_TICK(tick, sg->tick) + 1500;
		}
		break;

	case 0x93:	/* �����h�}�C�� */
		skill_attack(BF_MISC,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
		sg->unit_id = 0x8c;
		clif_changelook(&src->bl,LOOK_BASE,0x88);
		sg->limit=DIFF_TICK(tick,sg->tick)+1500;
		break;

	case 0x8f:	/* �u���X�g�}�C�� */
	case 0x94:	/* �V���b�N�E�F�[�u�g���b�v */
	case 0x95:	/* �T���h�}�� */
	case 0x96:	/* �t���b�V���[ */
	case 0x97:	/* �t���[�W���O�g���b�v */
	case 0x98:	/* �N���C���A�[�g���b�v */
		map_foreachinarea(skill_count_target,src->bl.m
					,src->bl.x-src->range,src->bl.y-src->range
					,src->bl.x+src->range,src->bl.y+src->range
					,0,&src->bl,&splash_count);
		map_foreachinarea(skill_trap_splash,src->bl.m
					,src->bl.x-src->range,src->bl.y-src->range
					,src->bl.x+src->range,src->bl.y+src->range
					,0,&src->bl,tick,splash_count);
		sg->unit_id = 0x8c;
		clif_changelook(&src->bl,LOOK_BASE,sg->unit_id);
		sg->limit=DIFF_TICK(tick,sg->tick)+1500;
		break;

	case 0x91:	/* �A���N���X�l�A */
		if (sg->val2 == 0 && sc_data && sc_data[SC_ANKLE].timer == -1) {
			int moveblock = (bl->x / BLOCK_SIZE != src->bl.x / BLOCK_SIZE || bl->y / BLOCK_SIZE != src->bl.y / BLOCK_SIZE);
			int sec = skill_get_time2(sg->skill_id, sg->skill_lv) - status_get_agi(bl) * 100;
			if (status_get_mode(bl) & 0x20) // Lasts 5 times less on bosses
				sec = sec / 5;
			if (sec < 3000) // Minimum trap time of 3 seconds [celest]
				sec = 3000;
			battle_stopwalking(bl, 1);
			status_change_start(bl, SC_ANKLE, sg->skill_lv, 0, 0, 0, sec, 0);
			skill_unit_move(bl, tick, 0);
			if (moveblock) map_delblock(bl);
			bl->x = src->bl.x;
			bl->y = src->bl.y;
			if (moveblock) map_addblock(bl);
			skill_unit_move(bl, tick, 1);
			if (bl->type == BL_MOB)
				clif_fixmobpos((struct mob_data *)bl);
			else if (bl->type == BL_PET)
				clif_fixpetpos((struct pet_data *)bl);
			else
				clif_fixpos(bl);
			clif_01ac(&src->bl);
			sg->limit = DIFF_TICK(tick, sg->tick) + sec;
			sg->val2 = bl->id;
			sg->interval = -1;
			src->range = 0;
		}
		break;

	case 0x92:	/* �x�m���_�X�g */
		if (sc_data && sc_data[type].timer == -1)
			status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0xb1:	/* �f�����X�g���[�V���� */
		skill_attack(BF_WEAPON,ss,&src->bl,bl,sg->skill_id,sg->skill_lv,tick,0);
		if(bl->type == BL_PC && rand()%100 < sg->skill_lv && battle_config.equipment_breaking)
			pc_breakweapon((struct map_session_data *)bl);
		break;

	case 0x99:				/* �g�[�L�[�{�b�N�X */
		if(sg->src_id == bl->id) //����������ł��������Ȃ�
			break;
		if(sg->val2==0){
			clif_talkiebox(&src->bl,sg->valstr);
			sg->unit_id = 0x8c;
			clif_changelook(&src->bl,LOOK_BASE,sg->unit_id);
			sg->limit=DIFF_TICK(tick,sg->tick)+5000;
			sg->val2=-1; //����
		}
		break;

	case 0xa6:	/* BA_DISSONANCE */
		skill_attack(BF_MISC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0);
		break;

	// Basilica
	case 0xb4:				/* �o�W���J */
		if (battle_check_target(&src->bl, bl, BCT_ENEMY) > 0 && !(status_get_mode(bl) & 0x20))
			skill_blown(&src->bl, bl, 1);
		if (sg->src_id == bl->id)
			break;
		if (battle_check_target(&src->bl, bl, BCT_NOENEMY) > 0 && sc_data && sc_data[type].timer == -1)
			status_change_start(bl, type, sg->skill_lv, (intptr_t)src, 0, 0, skill_get_time2(sg->skill_id, sg->skill_lv), 0);
		break;

	case 0xb7:
		if(sg->val2==0){
			int moveblock = ( bl->x/BLOCK_SIZE != src->bl.x/BLOCK_SIZE || bl->y/BLOCK_SIZE != src->bl.y/BLOCK_SIZE);
			skill_additional_effect(ss,bl,sg->skill_id,sg->skill_lv,BF_MISC,tick);
			skill_unit_move(bl, tick, 0);
			if (moveblock) map_delblock(bl);
			bl->x = src->bl.x;
			bl->y = src->bl.y;
			if (moveblock) map_addblock(bl);
			skill_unit_move(bl, tick, 1);
			if (bl->type == BL_MOB)
				clif_fixmobpos((struct mob_data *)bl);
			else if (bl->type == BL_PET)
				clif_fixpetpos((struct pet_data *)bl);
			else
				clif_fixpos(bl);
			sg->limit = DIFF_TICK(tick, sg->tick) + skill_get_time2(sg->skill_id, sg->skill_lv);
			sg->val2 = bl->id;
			sg->interval = -1;
			src->range = 0;
		}
		break;

	case 0xb8:
		if (skill_attack(BF_MAGIC, ss, &src->bl, bl, sg->skill_id, sg->skill_lv, tick, 0))
			skill_additional_effect(ss, bl, sg->skill_id, sg->skill_lv, BF_MAGIC, tick);
		break;
	}

	if(bl->type==BL_MOB && ss!=bl)
	{
		if(battle_config.mob_changetarget_byskill == 1)
		{
			int target=((struct mob_data *)bl)->target_id;
			if(ss->type == BL_PC)
				((struct mob_data *)bl)->target_id=ss->id;
			mobskill_use((struct mob_data *)bl,tick,MSC_SKILLUSED|(sg->skill_id<<16));
			((struct mob_data *)bl)->target_id=target;
		}
		else
			mobskill_use((struct mob_data *)bl,tick,MSC_SKILLUSED|(sg->skill_id<<16));
	}

	return 0;
}

/*==========================================
 * Skill Unit On Out
 *------------------------------------------
 */
int skill_unit_onout(struct skill_unit *src, struct block_list *bl, unsigned int tick) {
	struct skill_unit_group *sg;
	struct status_change *sc_data;
	int type;

	nullpo_retr(0, src);
	nullpo_retr(0, bl);
	nullpo_retr(0, sg = src->group);

	sc_data = status_get_sc_data(bl);
	type = SkillStatusChangeTable[sg->skill_id];

	if (bl->prev == NULL || !src->alive ||
	    (bl->type == BL_PC && pc_isdead((struct map_session_data *)bl)))
		return 0;

	switch(sg->unit_id)
	{
		case 0x7e:
		case 0x85:
		case 0x8e:
		case 0x9a:
		case 0x9b:
		case 0x9c:
			if (type == SC_QUAGMIRE && bl->type == BL_MOB)
				break;
			if (sc_data && sc_data[type].timer != -1 && sc_data[type].val2 == (intptr_t)src) {
				status_change_end(bl, type, -1);
			}
			break;
		case 0x91:	/* �A���N���X�l�A */
			{
				struct block_list *target = map_id2bl(sg->val2);
				if (target && target == bl) {
					status_change_end(bl, SC_ANKLE, -1);
					sg->limit = DIFF_TICK(tick, sg->tick) + 1000;
				}
			}
			break;
		case 0x9e:	/* �q��S */
		case 0x9f:	/* �j�����h�̉� */
		case 0xa0:	/* �i���̍��� */
		case 0xa1:	/* �푾�ۂ̋��� */
		case 0xa2:	/* �j�[�x�����O�̎w�� */
		case 0xa3:	/* ���L�̋��� */
		case 0xa4:	/* �[���̒��� */
		case 0xa5:	/* �s���g�̃W�[�N�t���[�h */
		case 0xad:	/* ����Y��Ȃ��Łc */
			if (sc_data[type].timer != -1 && sc_data[type].val4 == (intptr_t)src)
				status_change_end(bl, type, -1);
			break;
		case 0xa6:	/* �s���a�� */
		case 0xa7:	/* ���J */
		case 0xa8:	/* �[�z�̃A�T�V���N���X */
		case 0xa9:	/* �u���M�̎� */
		case 0xaa:	/* �C�h�D���̗ь� */
		case 0xab:	/* ��������ȃ_���X */
		case 0xac:	/* �n�~���O */
		case 0xae:	/* �K�^�̃L�X */
		case 0xaf:	/* �T�[�r�X�t�H�[���[ */
			status_change_start(bl, SkillStatusChangeTable[sg->skill_id], sg->skill_lv, 0, 0, 0, 20000, 0);
			break;
		case 0xb4:	// HP_BASILICA
		case 0xb9:	// CG_HERMODE
			sc_data = status_get_sc_data(bl);
			type = SkillStatusChangeTable[sg->skill_id];
			if(sc_data && sc_data[type].timer != -1)
				status_change_end(bl, type, -1);
			break;
		case 0xb6:
			{
				struct block_list *target = map_id2bl(sg->val2);
				if(target && target == bl) {
					status_change_end(bl, SC_FOGWALL, -1);
					if(sc_data && sc_data[SC_BLIND].timer != -1)
						sc_data[SC_BLIND].timer = add_timer(gettick_cache + 30000, status_change_timer, bl->id, 0);
				}
			}
			break;

		case 0xb7:
			{
				struct block_list *target = map_id2bl(sg->val2);
				if(target && target == bl)
					status_change_end(bl, SC_SPIDERWEB, -1);
				sg->limit = DIFF_TICK(tick, sg->tick) + 1000;
			}
			break;
	case 0xb8:
		sc_data = status_get_sc_data(bl);
		type = SkillStatusChangeTable[sg->skill_id];
		if(sc_data && sc_data[type].timer != -1)
			status_change_end(bl, type, -1);
		break;
	}
	return 0;
}

/*==========================================
 * Skill Unit Effect
 *------------------------------------------
 */
int skill_unit_effect(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit;
	struct skill_unit_group *group;
	int flag;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, unit = va_arg(ap, struct skill_unit*));
	tick = va_arg(ap, unsigned int);
	flag = va_arg(ap, unsigned int);

	if (bl->type != BL_PC && bl->type != BL_MOB)
		return 0;

	if (!unit->alive || bl->prev == NULL)
		return 0;

	nullpo_retr(0, group = unit->group);

	if (flag)
		skill_unit_onplace(unit, bl, tick);
	else {
		skill_unit_onout(unit, bl, tick);
		unit = map_find_skill_unit_oncell(bl, bl->x, bl->y, group->skill_id, unit);
		if (unit)
			skill_unit_onplace(unit, bl, tick);
	}

	return 0;
}

/*==========================================
 * Skill Unit On Limit
 *------------------------------------------
 */
int skill_unit_onlimit(struct skill_unit *src,unsigned int tick)
{
	struct skill_unit_group *sg;

	nullpo_retr(0, src);
	nullpo_retr(0, sg=src->group);

	switch(sg->unit_id){
	case 0x81:	/* ���[�v�|�[�^��(�����O) */
		{
			struct skill_unit_group *group=
				skill_unitsetting(map_id2bl(sg->src_id),sg->skill_id,sg->skill_lv,
					src->bl.x,src->bl.y,1);
			if (group == NULL)
				return 0;
			CALLOC(group->valstr, char, 25); // 24 + NULL
			strncpy(group->valstr, sg->valstr, 24);
			group->val2 = sg->val2;
		}
		break;

	case 0x8d:	/* �A�C�X�E�H�[�� */
		map_setcell(src->bl.m, src->bl.x, src->bl.y, src->val2);
		clif_changemapcell(src->bl.m, src->bl.x, src->bl.y, src->val2, 1);
		break;

	case 0xb2:	/* ���Ȃ��ɉ���� */
	  {
		struct map_session_data *sd = NULL;
		struct map_session_data *p_sd = NULL;
		if ((sd = map_id2sd(sg->src_id)) == NULL)
			return 0;
		if ((p_sd = pc_get_partner(sd)) == NULL)
			return 0;

		pc_setpos(p_sd, map[src->bl.m].name, src->bl.x, src->bl.y, 3);
	  }
		break;
	}

	return 0;
}

/*==========================================
 * Skill Unit On Damaged
 *------------------------------------------
 */
int skill_unit_ondamaged(struct skill_unit *src,struct block_list *bl,
	int damage,unsigned int tick)
{
	struct skill_unit_group *sg;

	nullpo_retr(0, src);
	nullpo_retr(0, sg=src->group);

	switch(sg->unit_id){
	case 0x8d:	/* �A�C�X�E�H�[�� */
		src->val1-=damage;
		break;
	case 0x8f:	/* �u���X�g�}�C�� */
	case 0x98:	/* �N���C���A�[�g���b�v */
		skill_blown(bl,&src->bl,2); //������΂��Ă݂�
		break;
	default:
		damage = 0;
		break;
	}

	return damage;
}

/*==========================================
 * Skill Cast End Pos
 *------------------------------------------
 */
TIMER_FUNC(skill_castend_pos) {
	struct map_session_data* sd = map_id2sd(id)/*,*target_sd=NULL*/;
	int range,maxcount;

	nullpo_retr(0, sd);

	if (sd->bl.prev == NULL)
		return 0;
	if (sd->skilltimer != tid) /* �^�C�}ID�̊m�F */
		return 0;
	if (sd->skilltimer != -1 && pc_checkskill(sd, SA_FREECAST) > 0) {
		sd->speed = sd->prev_speed;
		clif_updatestatus(sd, SP_SPEED);
	}

	sd->skilltimer = -1;

	if (pc_isdead(sd)) {
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}

	if ((!battle_config.pc_skill_reiteration &&
	     skill_get_unit_flag(sd->skillid) & UF_NOREITERATION &&
	     skill_check_unit_range(sd->bl.m, sd->skillx, sd->skilly, sd->skillid, sd->skilllv)) ||
	    (map_getcell(sd->bl.m, sd->skillx, sd->skilly, CELL_CHKNOPASS))) { // not "Wall trapping" fixed by [Mikey] from freya's bug report
		clif_skill_fail(sd, sd->skillid, 0, 0);
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}

	if (battle_config.pc_skill_nofootset &&
	    skill_get_unit_flag(sd->skillid) & UF_NOFOOTSET &&
	    skill_check_unit_range2(sd->bl.m, sd->skillx, sd->skilly, sd->skillid, sd->skilllv)) {
		clif_skill_fail(sd, sd->skillid, 0, 0);
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}

	if (battle_config.pc_land_skill_limit) {
		maxcount = skill_get_maxcount(sd->skillid);
		if(maxcount > 0) {
			int i,c;
			for(i=c=0;i<MAX_SKILLUNITGROUP;i++) {
				if(sd->skillunit[i].alive_count > 0 && sd->skillunit[i].skill_id == sd->skillid)
					c++;
			}
			if (c >= maxcount) {
				clif_skill_fail(sd, sd->skillid, 0, 0);
				sd->canact_tick = tick;
				sd->canmove_tick = tick;
				return 0;
			}
		}
	}

	if (sd->skilllv <= 0) return 0;
	range = skill_get_range(sd->skillid,sd->skilllv);
	if (range < 0)
		range = status_get_range(&sd->bl) - (range + 1);
	range += battle_config.pc_skill_add_range;
	if (battle_config.skill_out_range_consume) { // changed to allow casting when target walks out of range [Valaris]
		if (range < distance(sd->bl.x,sd->bl.y,sd->skillx,sd->skilly)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}
	if (!skill_check_condition(sd, 1)) { /* �g�p�����`�F�b�N */
		sd->canact_tick = tick;
		sd->canmove_tick = tick;
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}
	sd->skillitem = sd->skillitemlv = -1;
	if (battle_config.skill_out_range_consume) {
		if (range < distance(sd->bl.x, sd->bl.y, sd->skillx, sd->skilly)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			sd->canact_tick = tick;
			sd->canmove_tick = tick;
			return 0;
		}
	}

	if (battle_config.pc_skill_log)
		printf("PC %d skill castend skill=%d\n",sd->bl.id,sd->skillid);
	pc_stop_walking(sd,0);

	skill_castend_pos2(&sd->bl, sd->skillx, sd->skilly, sd->skillid, sd->skilllv, tick, 0);

	return 0;
}


static int skill_check_condition_char_sub(struct block_list *bl,va_list ap)
{
	int *c;
	struct block_list *src;
	struct map_session_data *sd;
	struct map_session_data *ssd;
	struct pc_base_job s_class;
	struct pc_base_job ss_class;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, sd = (struct map_session_data*)bl);
	nullpo_retr(0, src = va_arg(ap,struct block_list *));
	nullpo_retr(0, c = va_arg(ap,int *));
	nullpo_retr(0, ssd = (struct map_session_data*)src);

	s_class = pc_calc_base_job(sd->status.class);
	ss_class = pc_calc_base_job(ssd->status.class);

	if(!battle_config.player_skill_partner_check)
	{
		(*c) = 99;
		return 0;
	}

	switch(ssd->skillid)
	{
		case PR_BENEDICTIO:
			if(sd != ssd && (s_class.job == 4 || s_class.job == 8 || s_class.job == 15) && (sd->bl.x == ssd->bl.x - 1 || sd->bl.x == ssd->bl.x + 1) && sd->status.sp >= 10)
				(*c)++;
			break;
		case BD_LULLABY:
		case BD_RICHMANKIM:
		case BD_ETERNALCHAOS:
		case BD_DRUMBATTLEFIELD:
		case BD_RINGNIBELUNGEN:
		case BD_ROKISWEIL:
		case BD_INTOABYSS:
		case BD_SIEGFRIED:
		case BD_RAGNAROK:
		case CG_MOONLIT:
			if(sd != ssd && ((ss_class.job==19 && s_class.job==20) || (ss_class.job==20 && s_class.job==19)) && pc_checkskill(sd,ssd->skillid) > 0 && (*c)==0 && sd->status.party_id == ssd->status.party_id && !pc_issit(sd) && sd->sc_data[SC_DANCING].timer == -1)
				if((sd->weapontype1 == 13 || sd->weapontype1 == 14) && (ssd->weapontype1 == 13 || ssd->weapontype1 == 14))
					(*c) = pc_checkskill(sd,ssd->skillid);
		break;
	}

	return 0;
}

/*==========================================
 * Skill Check Condition Use (Sub)
 *------------------------------------------
 */

static int skill_check_condition_use_sub(struct block_list *bl,va_list ap)
{
	int *c;
	struct block_list *src;
	struct map_session_data *sd;
	struct map_session_data *ssd;
	struct pc_base_job s_class;
	struct pc_base_job ss_class;
	int skillid, skilllv;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, sd=(struct map_session_data*)bl);
	nullpo_retr(0, src=va_arg(ap,struct block_list *));
	nullpo_retr(0, c=va_arg(ap,int *));
	nullpo_retr(0, ssd=(struct map_session_data*)src);

	s_class = pc_calc_base_job(sd->status.class);

	if(!battle_config.player_skill_partner_check){
		(*c)=99;
		return 0;
	}

	ss_class = pc_calc_base_job(ssd->status.class);
	skillid=ssd->skillid;
	skilllv=ssd->skilllv;
	if (skillid > 0 && skilllv <= 0) return 0; // celest
	switch(skillid){
	case PR_BENEDICTIO:				/* ���̍~�� */
		if(sd != ssd && (s_class.job == 4 || s_class.job == 8 || s_class.job == 15) &&
			(sd->bl.x == ssd->bl.x - 1 || sd->bl.x == ssd->bl.x + 1) && sd->status.sp >= 10){
			sd->status.sp -= 10;
			status_calc_pc(sd,0);
			(*c)++;
		}
		break;
	case BD_LULLABY:				/* �q��� */
	case BD_RICHMANKIM:				/* �j�����h�̉� */
	case BD_ETERNALCHAOS:			/* �i���̍��� */
	case BD_DRUMBATTLEFIELD:		/* �푾�ۂ̋��� */
	case BD_RINGNIBELUNGEN:			/* �j�[�x�����O�̎w�� */
	case BD_ROKISWEIL:				/* ���L�̋��� */
	case BD_INTOABYSS:				/* �[���̒��� */
	case BD_SIEGFRIED:				/* �s���g�̃W�[�N�t���[�h */
	case BD_RAGNAROK:				/* �_�X�̉��� */
	case CG_MOONLIT:				/* ������̐�ɗ�����Ԃт� */
		if(sd != ssd && //�{�l�ȊO��
		  ((ss_class.job==19 && s_class.job==20) || //�������o�[�h�Ȃ�_���T�[��
		   (ss_class.job==20 && s_class.job==19)) && //�������_���T�[�Ȃ�o�[�h��
		   pc_checkskill(sd,skillid) > 0 && //�X�L���������Ă���
		   (*c)==0 && //�ŏ��̈�l��
		   sd->status.party_id == ssd->status.party_id && //�p�[�e�B�[��������
		   !pc_issit(sd) && //�����ĂȂ�
		   sd->sc_data[SC_DANCING].timer == -1 //�_���X������Ȃ�
		  ){
			ssd->sc_data[SC_DANCING].val4=bl->id;
			clif_skill_nodamage(bl,src,skillid,skilllv,1);
			status_change_start(bl,SC_DANCING,skillid,ssd->sc_data[SC_DANCING].val2,0,src->id,skill_get_time(skillid,skilllv)+1000,0);
			sd->skillid_dance = sd->skillid = skillid;
			sd->skilllv_dance = sd->skilllv = skilllv;
			(*c)++;
		}
		break;
	}

	return 0;
}

/*==========================================
 * Skill Check Condition Mob Master (Sub)
 *------------------------------------------
 */

static int skill_check_condition_mob_master_sub(struct block_list *bl,va_list ap)
{
	int *c,src_id=0,mob_class=0;
	struct mob_data *md;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=(struct mob_data*)bl);
	nullpo_retr(0, src_id=va_arg(ap,int));
	nullpo_retr(0, mob_class=va_arg(ap,int));
	nullpo_retr(0, c=va_arg(ap,int *));

	if(md->class==mob_class && md->master_id==src_id)
		(*c)++;

	return 0;
}

static int skill_check_condition_hermod_sub(struct block_list *bl,va_list ap)
{
	int *c;
	struct npc_data *nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd=(struct npc_data*)bl);
	nullpo_retr(0, c=va_arg(ap,int *));

	if (nd->bl.subtype == WARP)
		(*c)++;
	return 0;
}

/*==========================================
 * Skill Check Condition
 *------------------------------------------
 */
int skill_check_condition(struct map_session_data *sd, int type) {
	int i, hp, sp, hp_rate, sp_rate, zeny, weapon, state, spiritball, skill, lv, mhp;
	int idx[10], itemid[10], amount[10];
	int arrow_flag = 0;

	nullpo_retr(0, sd);

	if (sd->sc_data[SC_CHASEWALK].timer != -1 && sd->skillid != ST_CHASEWALK)
		return 0;

	if (battle_config.gm_skilluncond > 0 && sd->GM_level >= battle_config.gm_skilluncond) {
		sd->skillitem = sd->skillitemlv = -1;
		return 1;
	}

	if (sd->opt1 > 0) {
		clif_skill_fail(sd, sd->skillid, 0, 0);
		return 0;
	}
	if (pc_is90overweight(sd)) {
		clif_skill_fail(sd, sd->skillid, 9, 0);
		return 0;
	}

	if (sd->skillid == AC_MAKINGARROW && sd->state.make_arrow_flag == 1) {
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}
	if ((sd->skillid == AM_PHARMACY || sd->skillid == ASC_CDP || sd->skillid == CR_ALCHEMY) && sd->state.produce_flag == 1)
	{
		sd->skillitem = sd->skillitemlv = -1;
		return 0;
	}

	if (sd->skillitem == sd->skillid)
	{
		if (type & 1)
			sd->skillitem = sd->skillitemlv = -1;
		return 1;
	}
	if (sd->opt1 > 0) {
		clif_skill_fail(sd, sd->skillid, 0, 0);
		return 0;
	}
	if (sd->sc_count){
		if (sd->sc_data[SC_SILENCE].timer!=-1 ||
		    sd->sc_data[SC_ROKISWEIL].timer!=-1 ||
		    (sd->sc_data[SC_AUTOCOUNTER].timer != -1 && sd->skillid != KN_AUTOCOUNTER) ||
		     sd->sc_data[SC_STEELBODY].timer != -1 ||
		     sd->sc_data[SC_BERSERK].timer != -1 ||
		     (sd->sc_data[SC_MARIONETTE].timer != -1 && sd->skillid != CG_MARIONETTE)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			return 0; /* ��Ԉُ�Ⓘ�قȂ� */
		}
	}
	skill = sd->skillid;
	lv = sd->skilllv;
	if (lv <= 0)
		return 0;
	if (skill >= 10000 && skill < 10015)
		skill -= 9500;
	hp=skill_get_hp(skill, lv);	/* ����HP */
	sp=skill_get_sp(skill, lv);	/* ����SP */
	if ((sd->skillid_old == BD_ENCORE) && skill==sd->skillid_dance)
		sp=sp/2; //�A���R�[������SP�������
	hp_rate = (lv <= 0) ? 0:skill_db[skill].hp_rate[lv-1];
	sp_rate = (lv <= 0) ? 0:skill_db[skill].sp_rate[lv-1];
	zeny = skill_get_zeny(skill,lv);
	weapon = skill_db[skill].weapon;
	state = skill_db[skill].state;
	spiritball = (lv <= 0) ? 0 : skill_db[skill].spiritball[lv-1];
	mhp=skill_get_mhp(skill, lv);	/* ����HP */
	for(i=0;i<10;i++) {
		itemid[i] = skill_db[skill].itemid[i];
		amount[i] = skill_db[skill].amount[i];
	}
	if(mhp > 0)
		hp += (sd->status.max_hp * mhp)/100;
	if(hp_rate > 0)
		hp += (sd->status.hp * hp_rate)/100;
	else
		hp += (sd->status.max_hp * abs(hp_rate))/100;
	if(sp_rate > 0)
		sp += (sd->status.sp * sp_rate)/100;
	else
		sp += (sd->status.max_sp * abs(sp_rate))/100;
	if(sd->dsprate!=100)
		sp=sp*sd->dsprate/100;	/* ����SP�C�� */

	switch(skill) {
	case SA_CASTCANCEL:
		if (sd->skilltimer == -1) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case BS_MAXIMIZE:		/* �}�L�V�}�C�Y�p���[ */
	case NV_TRICKDEAD:		/* ���񂾂ӂ� */
	case TF_HIDING:			/* �n�C�f�B���O */
	case AS_CLOAKING:		/* �N���[�L���O */
	case CR_AUTOGUARD:				/* �I�[�g�K�[�h */
	case CR_DEFENDER:
	case ST_CHASEWALK:
		if(sd->sc_data[SkillStatusChangeTable[skill]].timer != -1)
			return 1;
		break;
	case HW_GANBANTEIN:
		break;

	case CG_HERMODE:
		{
			int c = 0;
			map_foreachinarea (skill_check_condition_hermod_sub, sd->bl.m,
				sd->bl.x-3, sd->bl.y-3, sd->bl.x+3, sd->bl.y+3, BL_NPC, &c);
			if (c < 1) {
				clif_skill_fail(sd,skill,0,0);
				return 0;
			}
		}
		break;

	case AL_TELEPORT:
	case AL_WARP:
		if(map[sd->bl.m].flag.noteleport) {
			clif_skill_teleportmessage(sd,0);
			return 0;
		}
		if(sd->d_status==1 || sd->d_status==2){
			clif_displaymessage(sd->fd, "It is not allowed to teleport on the duel!");
			return 0;
		}
		break;

	case MO_CALLSPIRITS:
		if (sd->spiritball >= lv) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;

	case CH_SOULCOLLECT:
		if (sd->spiritball >= 5) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case MO_FINGEROFFENSIVE:
		if (sd->spiritball > 0 && sd->spiritball < spiritball) {
			spiritball = sd->spiritball;
			sd->spiritball_old = sd->spiritball;
		} else
			sd->spiritball_old = lv;
		break;
	case MO_BODYRELOCATION:
		if (sd->sc_count && sd->sc_data[SC_EXPLOSIONSPIRITS].timer != -1)
			spiritball = 0;
		break;
	case MO_CHAINCOMBO: //�A�ŏ�
		if (sd->sc_data[SC_BLADESTOP].timer == -1) {
			if (sd->sc_data[SC_COMBO].timer == -1 || sd->sc_data[SC_COMBO].val1 != MO_TRIPLEATTACK)
				return 0;
		}
		break;
	case MO_COMBOFINISH: //�җ���
		if (sd->sc_data[SC_COMBO].timer == -1 || sd->sc_data[SC_COMBO].val1 != MO_CHAINCOMBO)
			return 0;
		break;
	case CH_TIGERFIST: //���Ռ�
		if ((sd->sc_data[SC_COMBO].timer == -1 || sd->sc_data[SC_COMBO].val1 != MO_COMBOFINISH) && !sd->state.skill_flag)
			return 0;
		break;
	case CH_CHAINCRUSH: //�A������
		if (sd->sc_data[SC_COMBO].timer == -1)
			return 0;
		if (sd->sc_data[SC_COMBO].val1 != MO_COMBOFINISH && sd->sc_data[SC_COMBO].val1 != CH_TIGERFIST)
			return 0;
		break;
	case MO_EXTREMITYFIST:
		if(sd->sc_data[SC_EXPLOSIONSPIRITS].timer == -1 || spiritball < 5 || (sd->sc_data[SC_COMBO].timer != -1 || sd->sc_data[SC_COMBO].val1 == MO_BODYRELOCATION))
			return 0;
		if((sd->sc_data[SC_COMBO].timer != -1 && (sd->sc_data[SC_COMBO].val1 == MO_COMBOFINISH || sd->sc_data[SC_COMBO].val1 == CH_CHAINCRUSH)) || sd->sc_data[SC_BLADESTOP].timer!=-1)
			spiritball--;
		break;
	case BD_ADAPTATION:
	  {
		struct skill_unit_group *group = NULL;
		if (sd->sc_data[SC_DANCING].timer == -1 || ((group = (struct skill_unit_group*)sd->sc_data[SC_DANCING].val2) && (skill_get_time(sd->sc_data[SC_DANCING].val1, group->skill_lv) - sd->sc_data[SC_DANCING].val3 * 1000) <= skill_get_time2(skill, lv))) { //�_���X���Ŏg�p��5�b�ȏ�̂݁H
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
	  }
		break;
	case PR_BENEDICTIO: /* ���̍~�� */
		{
			int range=1;
			int c=0;
			if(!(type&1)){
			map_foreachinarea(skill_check_condition_char_sub,sd->bl.m,
				sd->bl.x-range,sd->bl.y-range,
				sd->bl.x+range,sd->bl.y+range,BL_PC,&sd->bl,&c);
			if (c < 2) {
				clif_skill_fail(sd, skill, 0, 0);
				return 0;
			}
			}else{
				map_foreachinarea(skill_check_condition_use_sub,sd->bl.m,
					sd->bl.x-range,sd->bl.y-range,
					sd->bl.x+range,sd->bl.y+range,BL_PC,&sd->bl,&c);
			}
		}
		break;
	case WE_CALLPARTNER:		/* ���Ȃ��Ɉ������� */
		if (!sd->status.partner_id) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case AM_CANNIBALIZE: /* �o�C�I�v�����g */
	case AM_SPHEREMINE: /* �X�t�B�A�[�}�C�� */
		if (type & 1) {
			int c = 0;
			int summons[5] = { 1020, 1068, 1118, 1500, 1368 }; // kRO 14/12/04 Patch - Bio Cannibalize: Monsters that are spawned are different based on the skill level [Aalye] from freya' forum
			int maxcount = (skill == AM_CANNIBALIZE) ? ((lv < 6) ? 6 - lv : 1) : skill_get_maxcount(skill);
			int mob_class = (skill == AM_CANNIBALIZE) ? ((lv < 6) ? summons[lv-1] : 1368) : 1142;
			if (battle_config.pc_land_skill_limit && maxcount > 0) {
				map_foreachinarea(skill_check_condition_mob_master_sub, sd->bl.m, 0, 0, map[sd->bl.m].xs, map[sd->bl.m].ys, BL_MOB, sd->bl.id, mob_class, &c);
				if (c >= maxcount) {
					clif_skill_fail(sd, skill, 0, 0);
					return 0;
				}
			}
		}
		break;
	case MG_FIREWALL:		/* �t�@�C�A�[�E�H�[�� */
	case WZ_QUAGMIRE:
	case PF_FOGWALL:
		if (battle_config.pc_land_skill_limit) {
			int maxcount = skill_get_maxcount(skill);
			if (maxcount > 0) {
				int i, c;
				c = 0;
				for(i = 0; i < MAX_SKILLUNITGROUP; i++) {
					if (sd->skillunit[i].alive_count > 0 && sd->skillunit[i].skill_id == skill)
						c++;
				}
				if (c >= maxcount || c == MAX_SKILLUNITGROUP) {
					clif_skill_fail(sd, skill, 0, 0);
					return 0;
				}
			}
		}
		break;

	case WZ_ICEWALL:
		if (battle_config.pc_land_skill_limit) {
			if (battle_config.max_icewall > 0) {
				int i, c;
				c = 0;
				for(i = 0; i < MAX_SKILLUNITGROUP; i++) {
					if (sd->skillunit[i].alive_count > 0 && sd->skillunit[i].skill_id == skill)
						c++;
				}
				if (c >= battle_config.max_icewall || c == MAX_SKILLUNITGROUP) {
					clif_skill_fail(sd, skill, 0, 0);
					return 0;
				}
			}
		}
		break;

	case WZ_FIREPILLAR:
		if (lv <= 5) // no gems required at level 1-5
			itemid[0] = 0;
		if (battle_config.pc_land_skill_limit) {
			int maxcount = skill_get_maxcount(skill);
			if (maxcount > 0) {
				int i, c;
				c = 0;
				for(i= 0; i < MAX_SKILLUNITGROUP; i++) {
					if (sd->skillunit[i].alive_count > 0 && sd->skillunit[i].skill_id == skill)
						c++;
				}
				if (c >= maxcount || c == MAX_SKILLUNITGROUP) {
					clif_skill_fail(sd, skill, 0, 0);
					return 0;
				}
			}
		}
		break;


	case PR_REDEMPTIO:
		{
			int exp;
			if(((exp = pc_nextbaseexp(sd)) > 0 && sd->status.base_exp * 100 / exp < 1) ||
				((exp = pc_nextjobexp(sd)) > 0 && sd->status.job_exp*100/exp < 1)) {
				clif_skill_fail(sd, skill, 0, 0); //Not enough exp.
				return 0;
			}
			break;
		}

	case AC_DOUBLE:
	case AC_SHOWER:
	case AC_CHARGEARROW:
	case BA_MUSICALSTRIKE:
	case DC_THROWARROW:
	case SN_SHARPSHOOTING:
	case CG_ARROWVULCAN:
		if (sd->equip_index[10] < 0) {
			clif_arrow_fail(sd, 0);
			return 0;
		}
		arrow_flag = 1;
		break;

	case RG_BACKSTAP:
		if (sd->status.weapon == 11) {
			if (sd->equip_index[10] < 0) {
				clif_arrow_fail(sd, 0);
				return 0;
			}
			arrow_flag = 1;
		}
		break;
	}

	if (!(type & 2)) {
		if (hp > 0 && sd->status.hp < hp) {				/* HP�`�F�b�N */
			clif_skill_fail(sd, skill, 2, 0);		/* HP�s���F���s�ʒm */
			return 0;
		}
		if (sp > 0 && sd->status.sp < sp) {				/* SP�`�F�b�N */
			clif_skill_fail(sd, skill, 1, 0);		/* SP�s���F���s�ʒm */
			return 0;
		}
		if (zeny > 0 && sd->status.zeny < zeny) {
			clif_skill_fail(sd, skill, 5, 0);
			return 0;
		}
		if (!(weapon & (1 << sd->status.weapon))) {
			clif_skill_fail(sd, skill, 6, 0);
			return 0;
		}
		if (spiritball > 0 && sd->spiritball < spiritball) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
	}

	switch(state) {
	case ST_HIDING:
		if (!(sd->status.option & 2)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_CLOAKING:
		if (!pc_iscloaking(sd)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_HIDDEN:
		if (!pc_ishiding(sd)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_RIDING:
		if (!pc_isriding(sd)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_FALCON:
		if (!pc_isfalcon(sd)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_CART:
		if (!pc_iscarton(sd)) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_SHIELD:
		if (sd->status.shield <= 0) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_SIGHT:
		if (sd->sc_data[SC_SIGHT].timer == -1 && type & 1) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_EXPLOSIONSPIRITS:
		if (skill == MO_EXTREMITYFIST && ((sd->sc_data[SC_COMBO].timer != -1 && (sd->sc_data[SC_COMBO].val1 == MO_COMBOFINISH || sd->sc_data[SC_COMBO].val1 == CH_CHAINCRUSH)) || sd->sc_data[SC_BLADESTOP].timer != -1)) {
			break;
		}
		if (sd->sc_data[SC_EXPLOSIONSPIRITS].timer == -1) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_RECOV_WEIGHT_RATE:
		if (battle_config.natural_heal_weight_rate <= 100 && sd->weight * 100 / sd->max_weight >= battle_config.natural_heal_weight_rate) {
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	case ST_MOVE_ENABLE:
		{
			struct walkpath_data wpd;
			if (path_search(&wpd,sd->bl.m, sd->bl.x, sd->bl.y, sd->skillx, sd->skilly, 1) == -1) {
				clif_skill_fail(sd, skill, 0, 0);
				return 0;
			}
		}
		break;
	case ST_WATER:
		if((!map_getcell(sd->bl.m, sd->bl.x, sd->bl.y, CELL_CHKWATER)) && (sd->sc_data[SC_DELUGE].timer == -1)) { //���ꔻ��
			clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
		break;
	}

	for(i = 0; i < 10; i++) {
		int x = lv % 11 - 1;
		idx[i] = -1;
		if (itemid[i] <= 0)
			continue;
		if (itemid[i] >= 715 && itemid[i] <= 717 && sd->special_state.no_gemstone)
			continue;
		if (((itemid[i] >= 715 && itemid[i] <= 717) || itemid[i] == 1065) && sd->sc_data[SC_INTOABYSS].timer != -1)
			continue;
		if ((skill == AM_POTIONPITCHER ||
		     skill == CR_SLIMPITCHER) && i != x)
			continue;

		idx[i] = pc_search_inventory(sd,itemid[i]);
		if(idx[i] < 0 || sd->status.inventory[idx[i]].amount < amount[i]) {
			if (itemid[i] == 716 || itemid[i] == 717)
				clif_skill_fail(sd, skill, (7+(itemid[i]-716)), 0);
			else
				clif_skill_fail(sd, skill, 0, 0);
			return 0;
		}
	}

	if(!(type&1))
		return 1;

	if (skill != AM_POTIONPITCHER &&
	    skill != CR_SLIMPITCHER &&
	    skill != MG_STONECURSE) {
		if(skill == AL_WARP && !(type&2))
			return 1;
		for(i=0;i<10;i++) {
			if(idx[i] >= 0)
				pc_delitem(sd,idx[i],amount[i],0); // �A�C�e������
		}
		if (arrow_flag && battle_config.arrow_decrement)
			pc_delitem(sd, sd->equip_index[10], 1, 0);
	}

	if(type&2)
		return 1;

	if(sp > 0) {					// SP����
		sd->status.sp-=sp;
		clif_updatestatus(sd,SP_SP);
	}
	if(hp > 0) {					// HP����
		sd->status.hp-=hp;
		clif_updatestatus(sd,SP_HP);
	}
	if (zeny > 0)					// Zeny����
		pc_payzeny(sd,zeny);
	if (spiritball > 0)				// ��������
		pc_delspiritball(sd, spiritball, 0);


	return 1;
}

/*==========================================
 * Skill Cast Fix
 *------------------------------------------
 */
int skill_castfix(struct block_list *bl, int time_duration) {
	struct map_session_data *sd = NULL;
	struct mob_data *md;
	struct status_change *sc_data;
	int castrate;
	int skill = 0, lv = 0;

	nullpo_retr(0, bl);

	switch(bl->type) {
	case BL_MOB:
		md = (struct mob_data*)bl;
		skill = md->skillid;
		lv = md->skilllv;
		break;
	case BL_PC:
		sd = (struct map_session_data*)bl;
		skill = sd->skillid;
		lv = sd->skilllv;
		break;
	}

	if (lv <= 0) return 0;

	if (skill > MAX_SKILL_DB || skill < 0)
		return 0;

	if (time_duration == 0)
		return 0;

	if (sd) {
		if (skill_get_castnodex(sd->skillid, sd->skilllv) <= 0) {
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
			if (scale > 0) { // not instant cast
				castrate = ((struct map_session_data *)bl)->castrate;
				time_duration = time_duration * castrate * scale / (battle_config.castrate_dex_scale * 100);
			} else
				return 0; // instant cast
		}
		// config cast time multiplier
		if (battle_config.cast_rate != 100)
			time_duration = time_duration * battle_config.cast_rate / 100;
	}

	// calculate cast time reduced by skill bonuses
	sc_data = status_get_sc_data(bl);
	if (sc_data) {
		if (sc_data[SC_SUFFRAGIUM].timer != -1) {
			time_duration = time_duration * (100 - sc_data[SC_SUFFRAGIUM].val1 * 15) / 100;
			status_change_end(bl, SC_SUFFRAGIUM, -1);
		}
		if (sc_data[SC_POEMBRAGI].timer != -1)
			time_duration = time_duration * (100 - (sc_data[SC_POEMBRAGI].val1 * 3 + sc_data[SC_POEMBRAGI].val2 + (sc_data[SC_POEMBRAGI].val3 >> 16))) / 100;
	}

	return (time_duration > 0) ? time_duration : 0;
}

/*==========================================
 * Skill Delay Fix
 *------------------------------------------
 */
int skill_delayfix(struct block_list *bl, int time_duration) {
	struct status_change *sc_data;

	nullpo_retr(0, bl);

	if (bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data*)bl;
		nullpo_retr(0, sd);

		// instant cast attack skills depend on aspd as delay [celest]
		if (time_duration == 0) {
			if (skill_db[sd->skillid].skill_type == BF_WEAPON)
				time_duration = status_get_adelay(bl) / 2;
			else
				time_duration = 300; // default delay, according to official servers
		} else if (time_duration < 0)
			time_duration = abs(time_duration) + status_get_adelay(bl) / 2; // if set to <0, the aspd delay will be added

		if (battle_config.delay_dependon_dex && /* dex�̉e�����v�Z���� */
		    !skill_get_delaynodex(sd->skillid, sd->skilllv)) { // if skill casttime is allowed to be reduced by dex
			int scale = battle_config.castrate_dex_scale - status_get_dex(bl);
			if (scale < 0)
				scale = 0;
			time_duration = time_duration * scale / battle_config.castrate_dex_scale;
		}

		if (battle_config.delay_rate != 100)
			time_duration = time_duration * battle_config.delay_rate / 100;

		if (sd->delayrate != 100)
			time_duration = time_duration * sd->delayrate / 100;

		if (time_duration < battle_config.min_skill_delay_limit) // check minimum skill delay
			time_duration = battle_config.min_skill_delay_limit;
	}

	sc_data = status_get_sc_data(bl);

	if (sc_data && sc_data[SC_POEMBRAGI].timer != -1)
		time_duration = time_duration * (100 - (sc_data[SC_POEMBRAGI].val1 * 3 + sc_data[SC_POEMBRAGI].val2 + (sc_data[SC_POEMBRAGI].val3&0xffff))) / 100;

	return (time_duration > 0) ? time_duration : 0;
}

int skill_use_id(struct map_session_data *sd, int target_id, int skill_num, int skill_lv) {
	int casttime = 0, delay = 0, skill, range;
	struct map_session_data* target_sd = NULL;
	int forcecast = 0;
	struct block_list *bl;
	struct status_change *sc_data;

	nullpo_retr(0, sd);

	if ((bl = map_id2bl(target_id)) == NULL)
	{
		check_fake_id(sd, target_id);
		return 0;
	}
	if (sd->bl.m != bl->m || pc_isdead(sd))
		return 0;

	if (skillnotok(skill_num, sd))
		return 0;

	sc_data = sd->sc_data;

	if (sd->opt1 > 0)
		return 0;
	if (sc_data)
	{
		if(sc_data[SC_VOLCANO].timer != -1)
			if (skill_num == WZ_ICEWALL)
				return 0;
		if(sc_data[SC_ROKISWEIL].timer != -1)
			if(skill_num == BD_ADAPTATION)
				return 0;
		if(sc_data[SC_SILENCE].timer!=-1 ||
			sc_data[SC_ROKISWEIL].timer!=-1 ||
			(sc_data[SC_AUTOCOUNTER].timer != -1 && sd->skillid != KN_AUTOCOUNTER) ||
			sc_data[SC_STEELBODY].timer != -1 ||
			sc_data[SC_BERSERK].timer != -1 ||
			(sc_data[SC_GRAVITATION].timer != -1 && sc_data[SC_GRAVITATION].val3 == BCT_SELF) ||
			sc_data[SC_HERMODE].timer != -1 ||
			(sc_data[SC_MARIONETTE].timer != -1 && sd->skillid != CG_MARIONETTE)){
			return 0;
		}

		if(sc_data[SC_BLADESTOP].timer != -1)
		{
			if(sc_data[SC_BLADESTOP].val2 == 1)
				return 0;

			int lv = sc_data[SC_BLADESTOP].val1;
			if(lv == 1)
				return 0;
			if(lv == 2 && skill_num != MO_FINGEROFFENSIVE)
				return 0;
			if(lv == 3 && skill_num != MO_FINGEROFFENSIVE && skill_num!=MO_INVESTIGATE)
				return 0;
			if(lv == 4 && skill_num != MO_FINGEROFFENSIVE && skill_num!=MO_INVESTIGATE && skill_num!=MO_CHAINCOMBO)
				return 0;
			if(lv == 5 && skill_num != MO_FINGEROFFENSIVE && skill_num!=MO_INVESTIGATE && skill_num!=MO_CHAINCOMBO && skill_num!=MO_EXTREMITYFIST)
				return 0;
		}

// Moved code to another part of source because of a bug [akrus]
/*		if (sc_data[SC_BASILICA].timer != -1)
		{
			struct skill_unit_group *sg = (struct skill_unit_group *)sc_data[SC_BASILICA].val4;
			if (sg && sg->src_id == sd->bl.id && skill_num == HP_BASILICA)
				;
			else
				return 0;
		}
*/


		if(sc_data[SC_DANCING].timer != -1) {
			if (sc_data[SC_DANCING].val1 == CG_HERMODE && skill_num == BD_ADAPTATION)
				return 0;
			if (sc_data[SC_LONGING].timer != -1) {
				if (skill_get_unit_flag(skill_num) & UF_DANCE || skill_get_unit_flag(skill_num) & UF_ENSEMBLE) 
					return 0;
			}
			else {
				if (sc_data[SC_DANCING].val4 && skill_num != BD_ADAPTATION && skill_num != CG_LONGINGFREEDOM)
					return 0;
				if (skill_num != BD_ADAPTATION && skill_num != BA_MUSICALSTRIKE && skill_num != DC_THROWARROW && 
					skill_num != CG_LONGINGFREEDOM){
					return 0;
				}
			}
		}
	}

	if (pc_iscloaking(sd) && skill_num == TF_HIDING)
		return 0;
	if (sd->status.option & 2 && skill_num != TF_HIDING && skill_num != AS_GRIMTOOTH && skill_num != RG_BACKSTAP && skill_num != RG_RAID)
		return 0;
	if (pc_ischasewalk(sd) && skill_num != ST_CHASEWALK)
		return 0;

	if (skill_get_inf2(skill_num) & 0x200 && sd->bl.id == target_id)
		return 0;
	switch(skill_num) {
	case SA_CASTCANCEL:
		if (sd->skillid != skill_num) { //�L���X�g�L�����Z�����̂͊o���Ȃ�
			sd->skillid_old = sd->skillid;
			sd->skilllv_old = sd->skilllv;
			break;
		}
	case BD_ENCORE:
		if(!sd->skillid_dance)
		{
			clif_skill_fail(sd, skill_num, 0, 0);
			return 0;
		}
		sd->skillid_old = skill_num;
		break;
	case GD_BATTLEORDER:
	case GD_REGENERATION:
	case GD_RESTORE:
	case GD_EMERGENCYCALL:
	  {
		struct guild *g;
		if (!sd->status.guild_id)
			return 0;
		if ((g = guild_search(sd->status.guild_id)) == NULL)
			return 0;
		if (strcmp(sd->status.name, g->master))
			return 0;
		skill_lv = guild_checkskill(g, skill_num);
		if (skill_lv <= 0)
			return 0;
	  }
		break;
	}

	sd->skillid = skill_num;
	sd->skilllv = skill_lv;

	switch(skill_num){ //���O�Ƀ��x�����ς�����肷��X�L��
	case BD_LULLABY:				/* �q��� */
	case BD_RICHMANKIM:				/* �j�����h�̉� */
	case BD_ETERNALCHAOS:			/* �i���̍��� */
	case BD_DRUMBATTLEFIELD:		/* �푾�ۂ̋��� */
	case BD_RINGNIBELUNGEN:			/* �j�[�x�����O�̎w�� */
	case BD_ROKISWEIL:				/* ���L�̋��� */
	case BD_INTOABYSS:				/* �[���̒��� */
	case BD_SIEGFRIED:				/* �s���g�̃W�[�N�t���[�h */
	case BD_RAGNAROK:				/* �_�X�̉��� */
	case CG_MOONLIT:				/* ������̐�ɗ�����Ԃт� */
	  {
		if (battle_config.gm_skilluncond == 0 || sd->GM_level < battle_config.gm_skilluncond) {
			int c = 0;
			map_foreachinarea(skill_check_condition_char_sub, sd->bl.m,
			                  sd->bl.x - 1, sd->bl.y - 1,
			                  sd->bl.x + 1, sd->bl.y + 1, BL_PC, &sd->bl, &c);
			if (c < 1) {
				clif_skill_fail(sd, skill_num, 0, 0);
				return 0;
			} else if (c == 99) { //�����s�v�ݒ肾����
				;
			} else {
				sd->skilllv = (c + skill_lv) / 2;
			}
		}
		break;
	  }
	}

	if (!skill_check_condition(sd, 0)) return 0;

	{
		int check_range_flag = 0;

		range = skill_get_range(skill_num, skill_lv);
		if (range < 0)
			range = status_get_range(&sd->bl) - (range + 1);
		// be lenient if the skill was cast before we have moved to the correct position [Celest]
		if (sd->walktimer != -1)
			range += battle_config.skill_range_leniency;
		else check_range_flag = 1;
		if(!battle_check_range(&sd->bl,bl,range)) {
			if (check_range_flag && battle_check_range(&sd->bl,bl,range + battle_config.skill_range_leniency)) {
				int dir, mask[8][2] = {{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1}};
				dir = map_calc_dir(&sd->bl,bl->x,bl->y);
				pc_walktoxy (sd, sd->bl.x + mask[dir][0] * battle_config.skill_range_leniency,
					sd->bl.y + mask[dir][1] * battle_config.skill_range_leniency);
			} else
				return 0;
		}
	}

	if (bl->type == BL_PC) {
		target_sd = (struct map_session_data*)bl;
		if (target_sd && skill_num == ALL_RESURRECTION && !pc_isdead(target_sd))
			return 0;
	}
	if ((skill_num != MO_CHAINCOMBO && skill_num != MO_COMBOFINISH &&
		skill_num != MO_EXTREMITYFIST && skill_num != CH_TIGERFIST &&
		skill_num != CH_CHAINCRUSH) ||
		(skill_num == CH_CHAINCRUSH && sd->state.skill_flag) ||
		(skill_num == MO_EXTREMITYFIST && sd->state.skill_flag) ||
		(skill_num == KN_CHARGEATK && sd->state.skill_flag))
		pc_stopattack(sd);

	casttime=skill_castfix(&sd->bl, skill_get_cast( skill_num,skill_lv) );
	if(skill_num != SA_MAGICROD)
		delay=skill_delayfix(&sd->bl, skill_get_delay( skill_num,skill_lv) );
	//sd->state.skillcastcancel = skill_db[skill_num].castcancel;
	sd->state.skillcastcancel = skill_get_castcancel(skill_num);

	switch(skill_num){	/* ��������ȏ������K�v */
	case ALL_RESURRECTION:	/* ���U���N�V���� */
		if(bl->type != BL_PC && battle_check_undead(status_get_race(bl),status_get_elem_type(bl))){	/* �G���A���f�b�h�Ȃ� */
			forcecast=1;	/* �^�[���A���f�b�g�Ɠ����r������ */
			casttime=skill_castfix(&sd->bl, skill_get_cast(PR_TURNUNDEAD,skill_lv) );
		}
		break;
	case MO_FINGEROFFENSIVE:
		casttime += casttime * ((skill_lv > sd->spiritball)? sd->spiritball:skill_lv);
		break;
	case MO_CHAINCOMBO:
		target_id = sd->attacktarget;
		if( sc_data && sc_data[SC_BLADESTOP].timer!=-1 ){
			struct block_list *tbl;
			if((tbl=(struct block_list *)sc_data[SC_BLADESTOP].val4) == NULL)
				return 0;
			target_id = tbl->id;
		}
		break;
	case MO_COMBOFINISH:
	case CH_TIGERFIST:
	case CH_CHAINCRUSH:
		target_id = sd->attacktarget;
		break;

	case MO_EXTREMITYFIST:
		if (sc_data && sc_data[SC_COMBO].timer != -1 && (sc_data[SC_COMBO].val1 == MO_COMBOFINISH || sc_data[SC_COMBO].val1 == CH_CHAINCRUSH))
		{
			casttime = 0;
			target_id = sd->attacktarget;
		}
		forcecast=1;
		break;
	case SA_MAGICROD:
	case SA_SPELLBREAKER:
		forcecast=1;
		break;
	case KN_CHARGEATK:
		casttime *= distance(sd->bl.x, sd->bl.y, bl->x, bl->y);
		break;
	case WE_MALE:
	case WE_FEMALE:
		{
			struct map_session_data *p_sd;
			if ((p_sd = pc_get_partner(sd)) == NULL) // it's possible to get null if we're not married --> no use NULLPO
				return 0;
			if (skill_num == WE_MALE && sd->status.hp <= ((15 * sd->status.max_hp) / 100)) // Requires more than 15% of Max HP for WE_MALE
				return 0;
			else if (skill_num == WE_FEMALE && sd->status.sp <= ((15 * sd->status.max_sp) / 100)) // Requires more than 15% of Max SP for WE_FEMALE
				return 0;
			target_id = p_sd->bl.id;
			range = skill_get_range(skill_num,skill_lv);
			if (range < 0)
				range = status_get_range(&sd->bl) - (range + 1);
			if (!battle_check_range(&sd->bl, &p_sd->bl, range))
				return 0;
		}
		break;

	case PF_MEMORIZE: /* �������C�Y */
		break;
	case HW_MAGICPOWER:
		break;
	case HP_BASILICA:		/* �o�W���J */
		if (skill_check_unit_range(sd->bl.m, sd->bl.x, sd->bl.y, sd->skillid, sd->skilllv)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			return 0;
		}
		if (skill_check_unit_range2(sd->bl.m, sd->bl.x, sd->bl.y, sd->skillid, sd->skilllv)) {
			clif_skill_fail(sd, sd->skillid, 0, 0);
			return 0;
		}
		break;
	case GD_BATTLEORDER:
	case GD_REGENERATION:
	case GD_RESTORE:
	case GD_EMERGENCYCALL:
		casttime = 1000; // temporary [Celest]
		break;
	}

	if(sc_data && sc_data[SC_MEMORIZE].timer != -1 && casttime > 0){
		casttime = casttime/2;
		if((--sc_data[SC_MEMORIZE].val2)<=0)
			status_change_end(&sd->bl, SC_MEMORIZE, -1);
	}

	if(battle_config.pc_skill_log)
		printf("PC %d skill use target_id=%d skill=%d lv=%d cast=%d\n",sd->bl.id,target_id,skill_num,skill_lv,casttime);

	if( casttime>0 || forcecast ){ /* �r�����K�v */
		struct mob_data *md;
		clif_skillcasting( &sd->bl, sd->bl.id, target_id, 0,0, skill_num,casttime);

		if (bl->type==BL_MOB && (md=(struct mob_data *)bl) && mob_db[md->class].mode&0x10 &&
		    md->state.state != MS_ATTACK && sd->invincible_timer == -1){
				md->target_id=sd->bl.id;
				md->state.targettype = ATTACKABLE;
				md->min_chase=13;
		}
	}

	if(casttime <= 0)
		sd->state.skillcastcancel=0;

	sd->skilltarget	= target_id;
	sd->skillx       = 0;
	sd->skilly       = 0;
	sd->canact_tick  = gettick_cache + casttime + delay;
	sd->canmove_tick = gettick_cache;
	if (!(battle_config.pc_cloak_check_type & 2) && sc_data && sc_data[SC_CLOAKING].timer != -1 && sd->skillid != AS_CLOAKING)
		status_change_end(&sd->bl,SC_CLOAKING,-1);

	if (sd->skilltimer != -1 && skill_num != SA_CASTCANCEL) { // SA_CASTCANCEL is cast immediatly // normally, we never entering in this test
		int inf;
		if ((inf = skill_get_inf(sd->skillid)) == 2 || inf == 32)
			delete_timer(sd->skilltimer, skill_castend_pos);
		else
			delete_timer(sd->skilltimer, skill_castend_id);
		sd->skilltimer = -1;
	}
	if (casttime > 0) { // cast with castime
		sd->skilltimer = add_timer(gettick_cache + casttime, skill_castend_id, sd->bl.id, 0);
		if ((skill = pc_checkskill(sd, SA_FREECAST)) > 0) {
			sd->prev_speed = sd->speed;
			sd->speed = sd->speed * (175 - skill * 5) / 100;
			clif_updatestatus(sd, SP_SPEED);
		} else
			pc_stop_walking(sd, 0);
	} else // cast immediatly
		skill_castend_id(sd->skilltimer, gettick_cache, sd->bl.id, 0);

	return 0;
}

/*==========================================
 * Skill Use Pos
 *------------------------------------------
 */
int skill_use_pos(struct map_session_data *sd,
	int skill_x, int skill_y, int skill_num, int skill_lv)
{
	struct block_list bl;
	struct status_change *sc_data;
	int casttime = 0, delay = 0, skill, range;

	nullpo_retr(0, sd);

	if (pc_isdead(sd))
		return 0;

	if (skillnotok(skill_num, sd))
		return 0;

	if (skill_num == WZ_ICEWALL && map[sd->bl.m].flag.noicewall && !map[sd->bl.m].flag.pvp) { // noicewall flag [Valaris]
		clif_skill_fail(sd, sd->skillid, 0, 0);
		return 0;
	}

	sc_data = sd->sc_data;

	if (sd->opt1 > 0)
		return 0;
	if (sc_data) {
		if (sc_data[SC_SILENCE].timer!=-1 ||
		    sc_data[SC_ROKISWEIL].timer!=-1 ||
		    sc_data[SC_AUTOCOUNTER].timer != -1 ||
		    sc_data[SC_STEELBODY].timer != -1 ||
		    sc_data[SC_DANCING].timer!=-1 ||
		    sc_data[SC_BERSERK].timer != -1 ||
		    sc_data[SC_MARIONETTE].timer != -1 ||
		    (sc_data[SC_GRAVITATION].timer != -1 && sc_data[SC_GRAVITATION].val3 == BCT_SELF && skill_num != HW_GRAVITATION))
			return 0;
	}

	if (sd->status.option & 2)
		return 0;

	sd->skillid = skill_num;
	sd->skilllv = skill_lv;
	if (skill_lv <= 0) return 0;
	sd->skillx = skill_x;
	sd->skilly = skill_y;
	if (!skill_check_condition(sd,0)) return 0;

	bl.type = BL_NUL;
	bl.m = sd->bl.m;
	bl.x = skill_x;
	bl.y = skill_y;

  {
	int check_range_flag = 0;
	range = skill_get_range(skill_num, skill_lv);
	if (range < 0)
		range = status_get_range(&sd->bl) - (range + 1);
	if (sd->walktimer != -1)
		range++;
	else
		check_range_flag = 1;
	if (!battle_check_range(&sd->bl, &bl,range)) {
		if (check_range_flag && battle_check_range(&sd->bl, &bl, range + 1)) {
			int mask[8][2] = {{0,1}, {-1,1}, {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {1,0}, {1,1}};
			int dir = map_calc_dir(&sd->bl, bl.x, bl.y);
			pc_walktoxy(sd, sd->bl.x + mask[dir][0], sd->bl.y + mask[dir][1]);
		} else
			return 0;
	}
  }

	pc_stopattack(sd);

	casttime = skill_castfix(&sd->bl, skill_get_cast(skill_num,skill_lv));
	delay = skill_delayfix(&sd->bl, skill_get_delay(skill_num,skill_lv));
	sd->state.skillcastcancel = skill_db[skill_num].castcancel;

	if (battle_config.pc_skill_log)
		printf("PC %d skill use target_pos=(%d,%d) skill=%d lv=%d cast=%d\n", sd->bl.id, skill_x, skill_y, skill_num, skill_lv, casttime);

	if (sc_data && sc_data[SC_MEMORIZE].timer != -1 && casttime > 0){
		casttime = casttime / 2; // Memorize is supposed to reduce the cast time of the next 5 spells by half (thanks to [Mikey] from freya's bug report)
		if ((--sc_data[SC_MEMORIZE].val2) <= 0)
			status_change_end(&sd->bl, SC_MEMORIZE, -1);
	}

	if (casttime > 0)
		clif_skillcasting(&sd->bl, sd->bl.id, 0, skill_x, skill_y, skill_num, casttime);

	if (casttime <= 0)
		sd->state.skillcastcancel = 0;

	sd->skilltarget = 0;
	sd->canact_tick = gettick_cache + casttime + delay;
	sd->canmove_tick = gettick_cache;
	if (!(battle_config.pc_cloak_check_type&2) && sc_data && sc_data[SC_CLOAKING].timer != -1)
		status_change_end(&sd->bl, SC_CLOAKING, -1);

	if (sd->skilltimer != -1) { // normally, we never entering in this test
		int inf;
		if ((inf = skill_get_inf(sd->skillid)) == 2 || inf == 32)
			delete_timer(sd->skilltimer, skill_castend_pos);
		else
			delete_timer(sd->skilltimer, skill_castend_id);
		sd->skilltimer = -1;
	}
	if (casttime > 0) { // cast with castime
		sd->skilltimer = add_timer(gettick_cache + casttime, skill_castend_pos, sd->bl.id, 0);
		if ((skill = pc_checkskill(sd, SA_FREECAST)) > 0) {
			sd->prev_speed = sd->speed;
			sd->speed = sd->speed*(175 - skill * 5) / 100;
			clif_updatestatus(sd, SP_SPEED);
		} else
			pc_stop_walking(sd, 0);
	} else // cast immediatly
		skill_castend_pos(sd->skilltimer, gettick_cache, sd->bl.id, 0);

	if (sc_data && sc_data[SC_MAGICPOWER].timer != -1 && skill_num != HW_MAGICPOWER)
		status_change_end(&sd->bl,SC_MAGICPOWER,-1);

	return 0;
}

/*==========================================
 * Skill Cast Cancel
 *------------------------------------------
 */
int skill_castcancel(struct block_list *bl, int type)
{
	int inf;
	int ret = 0;

	nullpo_retr(0, bl);

	if (bl->type == BL_PC) {
		struct map_session_data *sd = (struct map_session_data *)bl;
		nullpo_retr(0, sd);
		sd->canact_tick = gettick_cache;
		sd->canmove_tick = gettick_cache;
		if (sd->skilltimer != -1) {
			if (pc_checkskill(sd, SA_FREECAST) > 0) {
				sd->speed = sd->prev_speed;
				clif_updatestatus(sd, SP_SPEED);
			}
			if (!type) {
				if ((inf = skill_get_inf(sd->skillid)) == 2 || inf == 32) {
					ret = delete_timer(sd->skilltimer, skill_castend_pos);
					if (ret < 0)
						printf("delete timer error (skill_castend_pos): skillid : %d\n", sd->skillid);
				} else {
					ret = delete_timer(sd->skilltimer, skill_castend_id);
					if (ret < 0)
						printf("delete timer error (skill_castend_id): skillid : %d\n", sd->skillid);
				}
			} else {
				if ((inf = skill_get_inf(sd->skillid_old)) == 2 || inf == 32) {
					ret = delete_timer(sd->skilltimer, skill_castend_pos);
					if (ret < 0)
						printf("delete timer error (skill_castend_pos): skillid : %d\n", sd->skillid_old);
				} else {
					ret = delete_timer(sd->skilltimer, skill_castend_id);
					if (ret < 0)
						printf("delete timer error (skill_castend_id): skillid : %d\n", sd->skillid_old);
				}
			}
			sd->skilltimer = -1;
			clif_skillcastcancel(bl);
		}
		return 0;
	} else if (bl->type == BL_MOB) {
		struct mob_data *md = (struct mob_data *)bl;
		nullpo_retr(0, md);
		if (md->skilltimer != -1) {
			if ((inf = skill_get_inf(md->skillid)) == 2 || inf == 32) {
				ret = delete_timer(md->skilltimer, mobskill_castend_pos);
				if (ret < 0)
					printf("delete timer error (mobskill_castend_pos): skillid : %d\n", md->skillid);
			} else {
				ret = delete_timer(md->skilltimer, mobskill_castend_id);
				if (ret < 0)
					printf("delete timer error (mobskill_castend_id): skillid : %d\n", md->skillid);
			}
			md->skilltimer = -1;
			clif_skillcastcancel(bl);
		}
		return 0;
	}

	return 1;
}

/*=========================================
 * Skill Brandish Spear First
 *----------------------------------------
 */
void skill_brandishspear_first(struct square *tc,int dir,int x,int y){

	nullpo_retv(tc);

	if(dir == 0){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y-1;
	}
	else if(dir==2){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x+1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	}
	else if(dir==4){
		tc->val1[0]=x-2;
		tc->val1[1]=x-1;
		tc->val1[2]=x;
		tc->val1[3]=x+1;
		tc->val1[4]=x+2;
		tc->val2[0]=
		tc->val2[1]=
		tc->val2[2]=
		tc->val2[3]=
		tc->val2[4]=y+1;
	}
	else if(dir==6){
		tc->val1[0]=
		tc->val1[1]=
		tc->val1[2]=
		tc->val1[3]=
		tc->val1[4]=x-1;
		tc->val2[0]=y+2;
		tc->val2[1]=y+1;
		tc->val2[2]=y;
		tc->val2[3]=y-1;
		tc->val2[4]=y-2;
	}
	else if(dir==1){
		tc->val1[0]=x-1;
		tc->val1[1]=x;
		tc->val1[2]=x+1;
		tc->val1[3]=x+2;
		tc->val1[4]=x+3;
		tc->val2[0]=y-4;
		tc->val2[1]=y-3;
		tc->val2[2]=y-1;
		tc->val2[3]=y;
		tc->val2[4]=y+1;
	}
	else if(dir==3){
		tc->val1[0]=x+3;
		tc->val1[1]=x+2;
		tc->val1[2]=x+1;
		tc->val1[3]=x;
		tc->val1[4]=x-1;
		tc->val2[0]=y-1;
		tc->val2[1]=y;
		tc->val2[2]=y+1;
		tc->val2[3]=y+2;
		tc->val2[4]=y+3;
	}
	else if(dir==5){
		tc->val1[0]=x+1;
		tc->val1[1]=x;
		tc->val1[2]=x-1;
		tc->val1[3]=x-2;
		tc->val1[4]=x-3;
		tc->val2[0]=y+3;
		tc->val2[1]=y+2;
		tc->val2[2]=y+1;
		tc->val2[3]=y;
		tc->val2[4]=y-1;
	}
	else if(dir==7){
		tc->val1[0]=x-3;
		tc->val1[1]=x-2;
		tc->val1[2]=x-1;
		tc->val1[3]=x;
		tc->val1[4]=x+1;
		tc->val2[1]=y;
		tc->val2[0]=y+1;
		tc->val2[2]=y-1;
		tc->val2[3]=y-2;
		tc->val2[4]=y-3;
	}

	return;
}

/*=========================================
 * Skill Brandish Spear Dir
 *-----------------------------------------
 */
void skill_brandishspear_dir(struct square *tc,int dir,int are){

	int c;

	nullpo_retv(tc);

	for(c=0;c<5;c++){
		if(dir==0){
			tc->val2[c]+=are;
		}else if(dir==1){
			tc->val1[c]-=are; tc->val2[c]+=are;
		}else if(dir==2){
			tc->val1[c]-=are;
		}else if(dir==3){
			tc->val1[c]-=are; tc->val2[c]-=are;
		}else if(dir==4){
			tc->val2[c]-=are;
		}else if(dir==5){
			tc->val1[c]+=are; tc->val2[c]-=are;
		}else if(dir==6){
			tc->val1[c]+=are;
		}else if(dir==7){
			tc->val1[c]+=are; tc->val2[c]+=are;
		}
	}
}

/*==========================================
 * Skill Devotion
 *------------------------------------------
 */
void skill_devotion(struct map_session_data *md,int target)
{
	int n;

	nullpo_retv(md);

	for(n=0;n<5;n++){
		if(md->dev.val1[n]){
			struct map_session_data *sd = map_id2sd(md->dev.val1[n]);
			if( sd == NULL || (sd->sc_data && (md->bl.id != sd->sc_data[SC_DEVOTION].val1)) || skill_devotion3(&md->bl,md->dev.val1[n])){
				skill_devotion_end(md,sd,n);
			}
		}
	}
}

void skill_devotion2(struct block_list *bl, int crusader)
{
	struct map_session_data *sd;

	nullpo_retv(bl);

	sd = map_id2sd(crusader);
	if (sd) skill_devotion3(&sd->bl, bl->id);
}

int skill_devotion3(struct block_list *bl, int target)
{
	struct map_session_data *md;
	struct map_session_data *sd;
	int n,r=0;

	nullpo_retr(1, bl);

	md = (struct map_session_data *)bl;

	if ((sd = map_id2sd(target)) == NULL)
		return 1;
	else
		r = distance(bl->x, bl->y, sd->bl.x, sd->bl.y);

	if (pc_checkskill(md, CR_DEVOTION) + 6 < r) { // ���e�͈͂𒴂��Ă�
		for(n = 0; n < 5; n++)
			if (md->dev.val1[n] == target)
				md->dev.val2[n] = 0; // ���ꂽ���́A����؂邾��
		clif_devotion(md, sd->bl.id);
		return 1;
	}

	return 0;
}

void skill_devotion_end(struct map_session_data *md,struct map_session_data *sd,int target)
{
	nullpo_retv(md);
	nullpo_retv(sd);

	md->dev.val1[target]=md->dev.val2[target]=0;
	if(sd && sd->sc_data){
		sd->sc_data[SC_DEVOTION].val1=0;
		sd->sc_data[SC_DEVOTION].val2=0;
		clif_status_change(&sd->bl,SC_DEVOTION,0);
		clif_devotion(md,sd->bl.id);
	}
}

/*==========================================
 * Skill Autospell
 *------------------------------------------
 */
void skill_autospell(struct map_session_data *sd, int skillid) {
	int skilllv;
	int maxlv = 1, lv;

//	nullpo_retv(sd); // checked before to call function

	skilllv = pc_checkskill(sd, SA_AUTOSPELL);
	if (skilllv <= 0) return;

	if (skillid == MG_NAPALMBEAT)
		maxlv = 3;
	else if (skillid == MG_COLDBOLT || skillid == MG_FIREBOLT || skillid == MG_LIGHTNINGBOLT) {
		if (skilllv == 2) maxlv = 1;
		else if (skilllv == 3) maxlv = 2;
		else if (skilllv >= 4) maxlv = 3;
	} else if (skillid == MG_SOULSTRIKE){
		if (skilllv == 5) maxlv = 1;
		else if (skilllv == 6) maxlv = 2;
		else if (skilllv >= 7) maxlv = 3;
	} else if (skillid == MG_FIREBALL) {
		if (skilllv == 8) maxlv = 1;
		else if (skilllv >= 9) maxlv = 2;
	} else if (skillid == MG_FROSTDIVER)
		maxlv = 1;
	else
		return;

	if (maxlv > (lv = pc_checkskill(sd, skillid)))
		maxlv = lv;

	status_change_start(&sd->bl, SC_AUTOSPELL, skilllv, skillid, maxlv, 0,
		skill_get_time(SA_AUTOSPELL, skilllv), 0);

	return;
}

/*==========================================
 * Skill Gangster Count
 *------------------------------------------
 */

static int skill_gangster_count(struct block_list *bl, va_list ap)
{
	int *c;
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	sd = (struct map_session_data*)bl;
	c = va_arg(ap, int *);

	if (sd && c && pc_issit(sd) && pc_checkskill(sd, RG_GANGSTER) > 0)
		(*c)++;

	return 0;
}

static int skill_gangster_in(struct block_list *bl, va_list ap)
{
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	sd = (struct map_session_data*)bl;
	if (sd && pc_issit(sd) && pc_checkskill(sd, RG_GANGSTER) > 0)
		sd->state.gangsterparadise = 1;

	return 0;
}

static int skill_gangster_out(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	sd=(struct map_session_data*)bl;
	if(sd && sd->state.gangsterparadise)
		sd->state.gangsterparadise=0;

	return 0;
}

int skill_gangsterparadise(struct map_session_data *sd, int type)
{
	int range = 1;
	int c = 0;

	nullpo_retr(0, sd);

	if (pc_checkskill(sd, RG_GANGSTER) <= 0)
		return 0;

	if (type == 1) {
		map_foreachinarea(skill_gangster_count, sd->bl.m,
		                  sd->bl.x - range, sd->bl.y - range,
		                  sd->bl.x + range, sd->bl.y + range, BL_PC, &c);
		if (c > 1) {
			map_foreachinarea(skill_gangster_in, sd->bl.m,
			                  sd->bl.x - range, sd->bl.y - range,
			                  sd->bl.x + range, sd->bl.y + range, BL_PC);
			sd->state.gangsterparadise = 1;
		}
		return 0;
	}
	else if (type == 0) {
		map_foreachinarea(skill_gangster_count, sd->bl.m,
		                  sd->bl.x - range, sd->bl.y - range,
		                  sd->bl.x + range, sd->bl.y + range, BL_PC, &c);
		if (c < 2)
			map_foreachinarea(skill_gangster_out, sd->bl.m,
			                  sd->bl.x - range, sd->bl.y - range,
			                  sd->bl.x + range, sd->bl.y + range, BL_PC);
		sd->state.gangsterparadise = 0;
		return 0;
	}

	return 0;
}

/*==========================================
 * Skill Frost Joke Scream
 *------------------------------------------
 */
int skill_frostjoke_scream(struct block_list *bl, va_list ap)
{
	struct block_list *src;
	int skillnum, skilllv;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, src = va_arg(ap,struct block_list*));

	skillnum = va_arg(ap,int);
	skilllv = va_arg(ap,int);
	if (skilllv <= 0) return 0;
	tick = va_arg(ap, unsigned int);

	if (src == bl || bl->prev == NULL || status_isdead(bl))
		return 0;

	if (battle_check_target(src, bl, BCT_ENEMY) > 0)
		skill_additional_effect(src, bl, skillnum, skilllv, BF_MISC, tick);
	else if (battle_check_target(src, bl, BCT_PARTY) > 0) {
		if (rand() % 100 < 10)
			skill_additional_effect(src, bl, skillnum, skilllv, BF_MISC, tick);
	}

	return 0;
}

/*==========================================
 * Moonlit creates a 'safe zone' [celest]
 *------------------------------------------
 */
static int skill_moonlit_count(struct block_list *bl, va_list ap)
{
	int *c, id;
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, (sd = (struct map_session_data *)bl));

	id=va_arg(ap, int);
	c=va_arg(ap, int *);

	if (sd->bl.id != id && sd->sc_count && sd->sc_data[SC_MOONLIT].timer != -1 && c)
		(*c)++;

	return 0;
}

int skill_check_moonlit (struct block_list *bl, int dx, int dy)
{
	int c = 0;

	nullpo_retr(0, bl);

	map_foreachinarea(skill_moonlit_count, bl->m, dx-1, dy-1, dx+1, dy+1, BL_PC, bl->id, &c);

	return (c > 0);
}

/*==========================================
 * Skill Abra Data Set
 *------------------------------------------
 */
int skill_abra_dataset(int skilllv)
{
	int skill = rand() % 331;
	if (skilllv <= 0) return 0;
	// db�Ɋ�Â����x���E�m������
	if (skill_abra_db[skill].req_lv > skilllv || rand() % 10000 >= skill_abra_db[skill].per) return 0;
	// NPC�X�L���̓_��
	if (skill >= NPC_PIERCINGATT && skill <= NPC_SUMMONMONSTER) return 0;
	// ���t�X�L���̓_��
	if (skill_get_unit_flag(skill) & UF_DANCE) return 0;

	return skill;
}

/*==========================================
 * Skill Basilica Cell
 *------------------------------------------
 */
void skill_basilica_cell(struct skill_unit *unit, int flag) {
	int i, x, y, range = skill_get_unit_range(HP_BASILICA);
	int size = range * 2 + 1;

	for (i = 0; i < size * size; i++) {
		x = unit->bl.x + (i % size - range);
		y = unit->bl.y + (i / size - range);
		map_setcell(unit->bl.m, x, y, flag);
	}
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_attack_area(struct block_list *bl,va_list ap)
{
	struct block_list *src,*dsrc;
	int atk_type,skillid,skilllv,flag,type;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	atk_type = va_arg(ap,int);
	if((src=va_arg(ap,struct block_list*)) == NULL)
		return 0;
	if((dsrc=va_arg(ap,struct block_list*)) == NULL)
		return 0;
	skillid=va_arg(ap,int);
	skilllv=va_arg(ap,int);
	if (skillid > 0 && skilllv <= 0) return 0; // celest
	tick=va_arg(ap,unsigned int);
	flag=va_arg(ap,int);
	type=va_arg(ap,int);

	if(battle_check_target(dsrc,bl,type) > 0)
		skill_attack(atk_type,src,dsrc,bl,skillid,skilllv,tick,flag);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_clear_element_field(struct block_list *bl)
{
	struct mob_data *md = NULL;
	struct map_session_data *sd = NULL;
	int i, max, skillid;

	nullpo_retr(0, bl);

	if (bl->type == BL_MOB) {
		max = MAX_MOBSKILLUNITGROUP;
		md = (struct mob_data *)bl;
	} else if (bl->type == BL_PC) {
		max = MAX_SKILLUNITGROUP;
		sd = (struct map_session_data *)bl;
	} else
		return 0;

	for (i = 0; i < max; i++) {
		if(sd){
			skillid=sd->skillunit[i].skill_id;
			if(skillid==SA_DELUGE||skillid==SA_VOLCANO||skillid==SA_VIOLENTGALE||skillid==SA_LANDPROTECTOR)
				skill_delunitgroup(&sd->skillunit[i]);
		}else if(md){
			skillid=md->skillunit[i].skill_id;
			if(skillid==SA_DELUGE||skillid==SA_VOLCANO||skillid==SA_VIOLENTGALE||skillid==SA_LANDPROTECTOR)
				skill_delunitgroup(&md->skillunit[i]);
		}
	}

	return 0;
}

// === LAND PROTECTOR SKILL ===
// ============================
int skill_landprotector(struct block_list *bl, va_list ap )
{
	int skillid;
	int *alive;
	struct skill_unit *unit;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	skillid = va_arg(ap,int);
	alive = va_arg(ap,int *);
	if((unit = (struct skill_unit *)bl) == NULL)
		return 0;

	if(skillid == SA_LANDPROTECTOR || skillid == HW_GANBANTEIN)
		skill_delunit(unit);
	else {
		if(alive && unit->group->skill_id == SA_LANDPROTECTOR)
			(*alive) = 0;
	}
	return 0;
}

/*==========================================
 * Ganbantein Skill
 *------------------------------------------
 */
int skill_ganbatein(struct block_list *bl, va_list ap )
{
	struct skill_unit *unit;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	if ((unit = (struct skill_unit *)bl) == NULL || unit->group == NULL)
		return 0;

	if (unit->group->skill_id == SA_LANDPROTECTOR)
		skill_delunit(unit);
	else skill_delunitgroup(unit->group);

	return 0;
}

/*==========================================
 * Apple of Idun Skill (Healing Part)
 *------------------------------------------
 */
int skill_idun_heal(struct block_list *bl, va_list ap )
{
	struct skill_unit *unit;
	struct skill_unit_group *sg;
	int heal;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, unit = va_arg(ap,struct skill_unit *));
	nullpo_retr(0, sg = unit->group);

	heal = 30 + sg->skill_lv * 5 + ((sg->val1) >> 16) * 5 + ((sg->val1) & 0xfff) / 2;

	if(bl->type == BL_SKILL || bl->id == sg->src_id)
		return 0;

	if(bl->type == BL_PC || bl->type == BL_MOB){
	clif_skill_nodamage(&unit->bl,bl,AL_HEAL,heal,1);
	battle_heal(NULL,bl,heal,0,0);
	}

	return 0;
}

/*==========================================
 * Skill Count Target
 *------------------------------------------
 */
int skill_count_target(struct block_list *bl, va_list ap) {
	struct block_list *src;
	int *c;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	if((src = va_arg(ap,struct block_list *)) == NULL)
		return 0;
	if((c = va_arg(ap,int *)) == NULL)
		return 0;
	if(battle_check_target(src,bl,BCT_ENEMY) > 0)
		(*c)++;

	return 0;
}

/*==========================================
 * Skill Trap Splash
 *------------------------------------------
 */
int skill_trap_splash(struct block_list *bl, va_list ap )
{
	struct block_list *src;
	int tick;
	int splash_count;
	struct skill_unit *unit;
	struct skill_unit_group *sg;
	struct block_list *ss;
	int i;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, src = va_arg(ap,struct block_list *));
	nullpo_retr(0, unit = (struct skill_unit *)src);
	nullpo_retr(0, sg = unit->group);
	nullpo_retr(0, ss = map_id2bl(sg->src_id));

	tick = va_arg(ap,int);
	splash_count = va_arg(ap,int);

	if(battle_check_target(src,bl,BCT_ENEMY) > 0){
		switch(sg->unit_id){
			case 0x95:	/* �T���h�}�� */
			case 0x96:	/* �t���b�V���[ */
			case 0x94:	/* �V���b�N�E�F�[�u�g���b�v */
				skill_additional_effect(ss,bl,sg->skill_id,sg->skill_lv,BF_MISC,tick);
				break;
			case 0x8f:	/* �u���X�g�}�C�� */
			case 0x98:	/* �N���C���A�[�g���b�v */
				for(i=0;i<splash_count;i++){
					skill_attack(BF_MISC,ss,src,bl,sg->skill_id,sg->skill_lv,tick,(sg->val2)?0x0500:0);
				}
			case 0x97:	/* �t���[�W���O�g���b�v */
					skill_attack(BF_WEAPON,	ss,src,bl,sg->skill_id,sg->skill_lv,tick,(sg->val2)?0x0500:0);
				break;
			default:
				break;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int skill_encchant_eremental_end(struct block_list *bl,int type)
{
	struct status_change *sc_data;

	nullpo_retr(0, bl);
	nullpo_retr(0, sc_data=status_get_sc_data(bl));

	if(type != SC_ENCPOISON && sc_data[SC_ENCPOISON].timer != -1)
		status_change_end(bl, SC_ENCPOISON, -1);
	if(type != SC_ASPERSIO && sc_data[SC_ASPERSIO].timer != -1)
		status_change_end(bl, SC_ASPERSIO, -1);
	if(type != SC_FLAMELAUNCHER && sc_data[SC_FLAMELAUNCHER].timer != -1)
		status_change_end(bl, SC_FLAMELAUNCHER, -1);
	if(type != SC_FROSTWEAPON && sc_data[SC_FROSTWEAPON].timer != -1)
		status_change_end(bl, SC_FROSTWEAPON, -1);
	if(type != SC_LIGHTNINGLOADER && sc_data[SC_LIGHTNINGLOADER].timer != -1)
		status_change_end(bl, SC_LIGHTNINGLOADER, -1);
	if(type != SC_SEISMICWEAPON && sc_data[SC_SEISMICWEAPON].timer != -1)
		status_change_end(bl, SC_SEISMICWEAPON, -1);

	return 0;
}

int skill_check_cloaking(struct block_list *bl)
{
	struct map_session_data *sd = NULL;
	static int dx[] = {-1, 0, 1,-1, 1,-1, 0, 1};
	static int dy[] = {-1,-1,-1, 0, 0, 1, 1, 1};
	int end = 1, i;

	nullpo_retr(1, bl);

	if (bl->type == BL_PC) {
		nullpo_retr(1, sd = (struct map_session_data *)bl);
		//if (!(battle_config.pc_cloak_check_type & 1)) // If it's No it shouldn't be checked // if we doesn't check walls.
		if (!(battle_config.pc_cloak_check_type & 1) || *status_get_option(bl) == 16388) // If it's No it shouldn't be checked; added stalker check [BeoWulf]
			return 0;
	} else if (bl->type == BL_MOB && !battle_config.monster_cloak_check_type)
		return 0;

	for(i = 0;i < sizeof(dx) / sizeof(dx[0]); i++) {
		if (map_getcell(bl->m, bl->x + dx[i], bl->y + dy[i], CELL_CHKNOPASS)) { // must not be CELL_CHKWALL ?
			end = 0;
			break;
		}
	}

	if (end) {
		if ((sd && pc_checkskill(sd, AS_CLOAKING) < 3) || bl->type == BL_MOB) {
			status_change_end(bl, SC_CLOAKING, -1);
			*status_get_option(bl) &= ~4; /* �O�̂��߂̏��� */
		}
		else if (sd && sd->sc_data[SC_CLOAKING].val3 != 130) {
			sd->sc_data[SC_CLOAKING].val3 = 130;
			status_calc_speed(sd);
		}
	}
	else {
		if (sd && sd->sc_data[SC_CLOAKING].val3 != 103) {
			sd->sc_data[SC_CLOAKING].val3 = 103;
			status_calc_speed(sd);
		}
	}

	return end;
}

/*==========================================
 * Stop Dancing
 *------------------------------------------
 */
void skill_stop_dancing(struct block_list *src, int flag)
{
	struct status_change* sc_data;
	struct skill_unit_group* group;
	short* sc_count;

	nullpo_retv(src);
	nullpo_retv(sc_data = status_get_sc_data(src));
	nullpo_retv(sc_count = status_get_sc_count(src));

	if ((*sc_count) > 0 && sc_data[SC_DANCING].timer != -1) {
		group=(struct skill_unit_group *)sc_data[SC_DANCING].val2; //�_���X�̃X�L�����j�b�gID��val2�ɓ����Ă�
		if(group && src->type==BL_PC && sc_data && sc_data[SC_DANCING].val4){ //���t���f
			struct map_session_data* dsd=map_id2sd(sc_data[SC_DANCING].val4); //������sd�擾
			if(flag){ //���O�A�E�g�ȂǕЕ��������Ă����t���p�������
				if(dsd && src->id == group->src_id){ //�O���[�v�������Ă�PC��������
					group->src_id=sc_data[SC_DANCING].val4; //�����ɃO���[�v��C����
					if(flag&1) //���O�A�E�g
					dsd->sc_data[SC_DANCING].val4=0; //�����̑�����0�ɂ��č��t�I�����ʏ�̃_���X���
					if(flag&2) //�n�G��тȂ�
						return; //���t���_���X��Ԃ��I�������Ȃ����X�L�����j�b�g�͒u���Ă��ڂ�
				}else if(dsd && dsd->bl.id == group->src_id){ //�������O���[�v�������Ă���PC��������(�����̓O���[�v�������Ă��Ȃ�)
					if(flag&1) //���O�A�E�g
					dsd->sc_data[SC_DANCING].val4=0; //�����̑�����0�ɂ��č��t�I�����ʏ�̃_���X���
					if(flag&2) //�n�G��тȂ�
						return; //���t���_���X��Ԃ��I�������Ȃ����X�L�����j�b�g�͒u���Ă��ڂ�
				}
				status_change_end(src,SC_DANCING,-1);//�����̃X�e�[�^�X���I��������
				return;
			}else{
				if(dsd && src->id == group->src_id){ //�O���[�v�������Ă�PC���~�߂�
					status_change_end((struct block_list *)dsd,SC_DANCING,-1);//����̃X�e�[�^�X���I��������
				}
				if(dsd && dsd->bl.id == group->src_id){ //�������O���[�v�������Ă���PC���~�߂�(�����̓O���[�v�������Ă��Ȃ�)
					status_change_end(src,SC_DANCING,-1);//�����̃X�e�[�^�X���I��������
				}
			}
		}
		if(flag&2 && group && src->type==BL_PC){ //�n�G�Ŕ�񂾂Ƃ��Ƃ��̓��j�b�g�����
			struct map_session_data *sd = (struct map_session_data *)src;
			skill_unit_move_unit_group(group, sd->bl.m,(sd->to_x - sd->bl.x),(sd->to_y - sd->bl.y));
			return;
		}
		skill_delunitgroup(group);
		if(src->type==BL_PC)
			status_calc_pc((struct map_session_data *)src,0);
	}
}

/*==========================================
 * Skill Unit
 *------------------------------------------
 */
struct skill_unit *skill_initunit(struct skill_unit_group *group,int idx,int x,int y)
{
	struct skill_unit *unit;

	nullpo_retr(NULL, group);
	nullpo_retr(NULL, unit=&group->unit[idx]);

	if(!unit->alive)
		group->alive_count++;

	unit->bl.id=map_addobject(&unit->bl);
	unit->bl.type=BL_SKILL;
	unit->bl.m=group->map;
	unit->bl.x=x;
	unit->bl.y=y;
	unit->group=group;
	unit->val1=unit->val2=0;
	unit->alive=1;

	map_addblock(&unit->bl);
	clif_skill_setunit(unit);

	if (group->skill_id == HP_BASILICA)
		skill_basilica_cell(unit, CELL_SETBASILICA);

	return unit;
}

/*==========================================
 * Skill Delete Unit
 *------------------------------------------
 */
int skill_delunit(struct skill_unit *unit)
{
	struct skill_unit_group *group;

	nullpo_retr(0, unit);
	if (!unit->alive)
		return 0;
	nullpo_retr(0, group = unit->group);

	skill_unit_onlimit(unit, gettick_cache);

	if (!unit->range) {
		map_foreachinarea(skill_unit_effect, unit->bl.m,
			unit->bl.x, unit->bl.y, unit->bl.x, unit->bl.y, 0,
			&unit->bl, gettick_cache, 0);
	}

	if (group->skill_id == HP_BASILICA)
		skill_basilica_cell(unit, CELL_CLRBASILICA);

	clif_skill_delunit(unit);

	unit->group = NULL;
	unit->alive = 0;
	map_delobjectnofree(unit->bl.id);
	if (group->alive_count > 0 && (--group->alive_count) <= 0)
		skill_delunitgroup(group);

	return 0;
}

/*==========================================
 * Skill Unit
 *------------------------------------------
 */
static int skill_unit_group_newid = MAX_SKILL_DB;
struct skill_unit_group *skill_initunitgroup(struct block_list *src,
    int count, int skillid, int skilllv, int unit_id)
{
	int i;
	struct skill_unit_group *group = NULL, *list = NULL;
	int maxsug = 0;

	nullpo_retr(NULL, src);

	if(skilllv <= 0) return 0;

	if(src->type==BL_PC){
		list=((struct map_session_data *)src)->skillunit;
		maxsug=MAX_SKILLUNITGROUP;
	}else if(src->type==BL_MOB){
		list=((struct mob_data *)src)->skillunit;
		maxsug=MAX_MOBSKILLUNITGROUP;
	}else if(src->type==BL_PET){
		list=((struct pet_data *)src)->skillunit;
		maxsug=MAX_MOBSKILLUNITGROUP;
	}
	if(list){
		for(i=0;i<maxsug;i++)	/* �󂢂Ă�����̌��� */
			if(list[i].group_id==0){
				group=&list[i];
				break;
			}

		if(group==NULL){	/* �󂢂ĂȂ��̂ŌÂ����̌��� */
			int j = 0;
			unsigned maxdiff = 0, x;
			for(i = 0; i < maxsug; i++)
				if((x = DIFF_TICK(gettick_cache, list[i].tick)) > maxdiff) {
					maxdiff = x;
					j = i;
				}
			skill_delunitgroup(&list[j]);
			group=&list[j];
		}
	}

	if(group==NULL){
		printf("skill_initunitgroup: error unit group !\n");
		exit(1);
	}

	group->src_id = src->id;
	group->party_id = status_get_party_id(src);
	group->guild_id = status_get_guild_id(src);
	group->group_id = skill_unit_group_newid++;
	if (skill_unit_group_newid <= 0)
		skill_unit_group_newid = MAX_SKILL_DB;
	CALLOC(group->unit, struct skill_unit, count);
	group->unit_count=count;
	group->val1=group->val2=0;
	group->skill_id=skillid;
	group->skill_lv=skilllv;
	group->unit_id=unit_id;
	group->map=src->m;
	group->limit=10000;
	group->interval=1000;
	group->tick = gettick_cache;
	group->valstr = NULL;

	if (skill_get_unit_flag(skillid) & UF_DANCE) {
		struct map_session_data *sd = NULL;
		if(src->type==BL_PC && (sd=(struct map_session_data *)src) ){
			sd->skillid_dance=skillid;
			sd->skilllv_dance=skilllv;
		}
		status_change_start(src,SC_DANCING,skillid,(intptr_t)group,0,0,skill_get_time(skillid,skilllv)+1000,0);
		if (battle_config.gm_skilluncond == 0 || sd->GM_level < battle_config.gm_skilluncond) {
			if (sd && skill_get_unit_flag(skillid) & UF_ENSEMBLE) {
				int c=0;
				map_foreachinarea(skill_check_condition_use_sub,sd->bl.m,
				                  sd->bl.x - 1, sd->bl.y - 1, sd->bl.x + 1, sd->bl.y + 1, BL_PC, &sd->bl, &c);
			}
		}
	}

	return group;
}

/*==========================================
 * Skill Delete Unit Group
 *------------------------------------------
 */
int skill_delunitgroup(struct skill_unit_group *group)
{
	struct block_list *src;
	int i;

	nullpo_retr(0, group);
	if(group->unit_count<=0)
		return 0;

	src=map_id2bl(group->src_id);
	if (skill_get_unit_flag(group->skill_id) & UF_DANCE) {
		if(src)
			status_change_end(src,SC_DANCING,-1);
		}

	group->alive_count=0;
	if(group->unit!=NULL){
		for(i=0;i<group->unit_count;i++)
			if(group->unit[i].alive)
				skill_delunit(&group->unit[i]);
	}
	if(group->valstr!=NULL){
		map_freeblock(group->valstr);
		group->valstr=NULL;
	}

	map_freeblock(group->unit);	/* free()�̑ւ�� */
	group->unit=NULL;
	group->src_id=0;
	group->group_id=0;
	group->unit_count=0;

	return 0;
}

/*==========================================
 * Skill Clear Unit Group
 *------------------------------------------
 */
int skill_clear_unitgroup(struct block_list *src)
{
	struct skill_unit_group *group=NULL;
	int maxsug=0;

	nullpo_retr(0, src);

	switch(src->type) {
	case BL_PC:
		group = ((struct map_session_data *)src)->skillunit;
		maxsug = MAX_SKILLUNITGROUP;
		break;
	case BL_MOB:
		group = ((struct mob_data *)src)->skillunit;
		maxsug = MAX_MOBSKILLUNITGROUP;
		break;
	case BL_PET:
		group = ((struct pet_data *)src)->skillunit;
		maxsug = MAX_MOBSKILLUNITGROUP;
		break;
	default:
		return 0;
		break;
	}

	if(group){
		int i;
		for(i=0;i<maxsug;i++)
			if(group[i].group_id>0 && group[i].src_id == src->id)
				skill_delunitgroup(&group[i]);
	}

	return 0;
}

/*==========================================
 * Skill Unit Group Tick Set
 *------------------------------------------
 */
struct skill_unit_group_tickset *skill_unitgrouptickset_search(struct block_list *bl, struct skill_unit_group *group, int tick)
{
	int i, j = -1, k, s, id;
	struct skill_unit_group_tickset *set;

	nullpo_retr(0, bl);

	if (group->interval == -1)
		return NULL;

	switch(bl->type){
	case BL_PC:
		set = ((struct map_session_data *)bl)->skillunittick;
		break;
	case BL_MOB:
		set = ((struct mob_data *)bl)->skillunittick;
		break;
	default:
		return 0;
		break;
	}

	if (skill_get_unit_flag(group->skill_id) & UF_NOOVERLAP)
		id = s = group->skill_id;
	else
		id = s = group->group_id;

	for (i = 0; i < MAX_SKILLUNITGROUPTICKSET; i++) {
		k = (i + s) % MAX_SKILLUNITGROUPTICKSET;
		if (set[k].id == id)
			return &set[k];
		else if (j == -1 && (DIFF_TICK(tick, set[k].tick) > 0 || set[k].id == 0))
			j = k;
	}

	if (j == -1) {
		if (battle_config.error_log) {
			printf("skill_unitgrouptickset_search: tickset is full.\n");
		}
		j = id % MAX_SKILLUNITGROUPTICKSET;
	}

	set[j].id = id;
	set[j].tick = tick;

	return &set[j];
}

/*==========================================
 * Skill Unit Timer (Sub On Place)
 *------------------------------------------
 */
int skill_unit_timer_sub_onplace(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit;
	struct skill_unit_group *group;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	unit = va_arg(ap, struct skill_unit *);
	tick = va_arg(ap, unsigned int);

	if (bl->type != BL_PC && bl->type != BL_MOB)
		return 0;
	if (!unit->alive || bl->prev == NULL)
		return 0;

	nullpo_retr(0, group = unit->group);

	if (battle_check_target(&unit->bl, bl, group->target_flag) <= 0)
		return 0;

	skill_unit_onplace_timer(unit, bl, tick);

	return 0;
}

/*==========================================
 * Skill Unit Timer (Sub)
 *------------------------------------------
 */
int skill_unit_timer_sub(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit;
	struct skill_unit_group *group;
	int range;
	unsigned int tick;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, unit = (struct skill_unit *)bl);
	tick = va_arg(ap, unsigned int);

	if (!unit->alive)
		return 0;

	nullpo_retr(0, group = unit->group);
	range = unit->range;

	if (range >= 0 && group->interval != -1) {
		map_foreachinarea(skill_unit_timer_sub_onplace, bl->m,
		                  bl->x - range, bl->y - range, bl->x + range, bl->y + range, 0, bl, tick);
		if (!unit->alive)
			return 0;
		// �}�O�k�X�͔����������j�b�g�͍폜����
		if (group->skill_id == PR_MAGNUS && unit->val2) {
			skill_delunit(unit);
			return 0;
		}
	}
	if (group->unit_id == 0xaa && DIFF_TICK(tick, group->tick) >= 6000 * group->val3) {
		struct block_list *src = map_id2bl(group->src_id);
		int range2;
		nullpo_retr(0, src);
		range2 = skill_get_unit_layout_type(group->skill_id, group->skill_lv);
		map_foreachinarea(skill_idun_heal, src->m,
		                  src->x - range2, src->y - range2, src->x + range2, src->y + range2, 0, unit);
		group->val3++;
	}
	if ((DIFF_TICK(tick,group->tick) >= group->limit || DIFF_TICK(tick, group->tick) >= unit->limit)) {
		switch(group->unit_id) {
			case 0x8f:	/* �u���X�g�}�C�� */
				group->unit_id = 0x8c;
				clif_changelook(bl,LOOK_BASE,group->unit_id);
				group->limit=DIFF_TICK(tick+1500,group->tick);
				unit->limit=DIFF_TICK(tick+1500,group->tick);
				break;
			case 0x90:	/* �X�L�b�h�g���b�v */
			case 0x91:	/* �A���N���X�l�A */
			case 0x93:	/* �����h�}�C�� */
			case 0x94:	/* �V���b�N�E�F�[�u�g���b�v */
			case 0x95:	/* �T���h�}�� */
			case 0x96:	/* �t���b�V���[ */
			case 0x97:	/* �t���[�W���O�g���b�v */
			case 0x98:	/* �N���C���A�[�g���b�v */
			case 0x99:	/* �g�[�L�[�{�b�N�X */
				{
					struct block_list *src = map_id2bl(group->src_id);
					if (group->unit_id == 0x91 && group->val2)
						;
					else {
						if (src && src->type == BL_PC){
							struct item item_tmp;
							memset(&item_tmp, 0, sizeof(item_tmp));
							item_tmp.nameid = 1065;
							item_tmp.identify = 1;
							map_addflooritem(&item_tmp, 1, bl->m, bl->x, bl->y, NULL, NULL, NULL, bl->id, 0);	// ?�Ԋ�
						}
					}
					skill_delunit(unit);
				}
				break;

			case 0xc1:
			case 0xc2:
			case 0xc3:
			case 0xc4:
				{
					struct block_list *src=map_id2bl(group->src_id);
					if (src)
						group->tick = tick;
				}
				break;

			default:
				skill_delunit(unit);
		}
	}

	if(group->unit_id == 0x8d) {
		unit->val1 -= 5;
		if(unit->val1 <= 0 && unit->limit + group->tick > tick + 700)
			unit->limit = DIFF_TICK(tick+700,group->tick);
	}

	return 0;
}

/*==========================================
 * Skill Unit Timer
 *------------------------------------------
 */
TIMER_FUNC(skill_unit_timer) {
	map_foreachobject(skill_unit_timer_sub, BL_SKILL, tick);

	return 0;
}

/*==========================================
 * Skill Unit Move (Sub)
 *------------------------------------------
 */
int skill_unit_move_sub(struct block_list *bl, va_list ap)
{
	struct skill_unit *unit = (struct skill_unit *)bl;
	struct skill_unit_group *group;
	struct block_list *target;
	unsigned int tick, flag;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, target = va_arg(ap, struct block_list*));
	tick = va_arg(ap, unsigned int);
	flag = va_arg(ap, int);

	if (target->type != BL_PC && target->type != BL_MOB)
		return 0;

	nullpo_retr(0, group = unit->group);
	if (group->interval != -1)
		return 0;

	if (!unit->alive || target->prev == NULL)
		return 0;

	if (flag)
		skill_unit_onplace(unit, target, tick);
	else
		skill_unit_onout(unit, target, tick);

	return 0;
}

/*==========================================
 * Skill Unit Move
 *------------------------------------------
 */
int skill_unit_move(struct block_list *bl, unsigned int tick, int flag)
{
	nullpo_retr(0, bl);

	if (bl->prev == NULL)
		return 0;

	map_foreachinarea(skill_unit_move_sub, bl->m, bl->x, bl->y, bl->x, bl->y, BL_SKILL, bl, tick, flag);

	return 0;
}

/*==========================================
 * Skill Unit Move Unit Group
 *------------------------------------------
 */
int skill_unit_move_unit_group( struct skill_unit_group *group, int m,int dx,int dy)
{
	int i, j;
	int *m_flag;
	struct skill_unit *unit1;
	struct skill_unit *unit2;

	nullpo_retr(0, group);
	if (group->unit_count <= 0)
		return 0;
	if (group->unit == NULL)
		return 0;

	if (!(skill_get_unit_flag(group->skill_id) & UF_DANCE) &&
	    group->skill_id != HT_CLAYMORETRAP && group->skill_id != HT_BLASTMINE)
		return 0;

	CALLOC(m_flag, int, group->unit_count);
	for(i = 0; i < group->unit_count; i++) {
		unit1 = &group->unit[i];
		if (!unit1->alive || unit1->bl.m != m)
			continue;
		for(j = 0; j < group->unit_count; j++) {
			unit2 = &group->unit[j];
			if (!unit2->alive)
				continue;
			if (unit1->bl.x + dx == unit2->bl.x && unit1->bl.y + dy == unit2->bl.y) {
				m_flag[i] |= 0x1;
			}
			if (unit1->bl.x - dx == unit2->bl.x && unit1->bl.y - dy == unit2->bl.y) {
				m_flag[i] |= 0x2;
			}
		}
	}
	j = 0;
	for (i = 0; i < group->unit_count; i++) {
		unit1 = &group->unit[i];
		if (!unit1->alive)
			continue;
		if (!(m_flag[i] & 0x2)) {
			map_foreachinarea(skill_unit_effect, unit1->bl.m,
			                  unit1->bl.x, unit1->bl.y, unit1->bl.x, unit1->bl.y, 0,
			                  &unit1->bl, gettick_cache, 0);
		}
		if (m_flag[i] == 0) {
			map_delblock(&unit1->bl);
			unit1->bl.m = m;
			unit1->bl.x += dx;
			unit1->bl.y += dy;
			map_addblock(&unit1->bl);
			clif_skill_setunit(unit1);
		} else if (m_flag[i] == 1) {
			for( ; j < group->unit_count; j++) {
				if (m_flag[j] == 2) {
					unit2 = &group->unit[j];
					if (!unit2->alive)
						continue;
					map_delblock(&unit1->bl);
					unit1->bl.m = m;
					unit1->bl.x = unit2->bl.x + dx;
					unit1->bl.y = unit2->bl.y + dy;
					map_addblock(&unit1->bl);
					clif_skill_setunit(unit1);
					j++;
					break;
				}
			}
		}
		if (!(m_flag[i] & 0x2)) {
			map_foreachinarea(skill_unit_effect, unit1->bl.m,
			                  unit1->bl.x, unit1->bl.y, unit1->bl.x, unit1->bl.y, 0,
			                  &unit1->bl, gettick_cache, 1);
		}
	}
	FREE(m_flag);

	return 0;
}

/*==========================================
 * Skill Can Produce Mix
 *------------------------------------------
 */
int skill_can_produce_mix( struct map_session_data *sd, int nameid, int trigger, unsigned int flag) {
	int i, j;

	nullpo_retr(0, sd);

	if (nameid <= 0)
		return 0;

	for(i = 0; i < MAX_SKILL_PRODUCE_DB; i++) {
		if (skill_produce_db[i].nameid == nameid)
			break;
	}
	if (i >= MAX_SKILL_PRODUCE_DB) /* �f�[�^�x�[�X�ɂȂ� */
		return 0;

	if (trigger >= 0) {
		if(trigger > 20) { // Non-weapon, non-food item (itemlv must match)
			if(skill_produce_db[i].itemlv != trigger)
				return 0;
		} else {
			if (skill_produce_db[i].itemlv >= 16)
				return 0;
			if (itemdb_wlv(nameid) > trigger)
				return 0;
		}
	}
	if ((j = skill_produce_db[i].req_skill) > 0 && pc_checkskill(sd, j) <= 0)
		return 0; /* �X�L��������Ȃ� */



	for(j = 0; j < MAX_PRODUCE_RESOURCE; j++) {
		int id, x, y;
		if((id = skill_produce_db[i].mat_id[j]) <= 0)
			continue;
		if(skill_produce_db[i].mat_amount[j] <= 0) {
			if(pc_search_inventory(sd, id) < 0)
				return 0;
			else if(flag&1)
				return i + 1;
		}
		else {
			for(y = 0, x = 0; y < MAX_INVENTORY; y++) {
				if(sd->status.inventory[y].nameid == id)
					x += sd->status.inventory[y].amount;
			}
			if(x < skill_produce_db[i].mat_amount[j]) {
				switch(skill_produce_db[i].req_skill) {
					case AM_PHARMACY:
						clif_produceeffect(sd, 3, nameid);
						clif_misceffect(&sd->bl, 6);
						return 0;
					case ASC_EDP:
						clif_skill_fail(sd, skill_produce_db[i].req_skill, 3, 0); //Insufficient Materials
					default:
						return 0;
				}
			}
		}
	}

	return i+1;
}

/*==========================================
 * Skill Produce Mix
 *------------------------------------------
 */
int skill_produce_mix( struct map_session_data *sd, int nameid, int slot1, int slot2, int slot3) {
	int slot[3];
	int i, sc, ele, idx, equip, wlv, make_per, flag;

//	nullpo_retv(sd); // checked before to call function

	if (!(idx = skill_can_produce_mix(sd, nameid, -1, 2)))
		return 0;

	idx--;
	slot[0] = slot1;
	slot[1] = slot2;
	slot[2] = slot3;

	for(i = 0, sc = 0, ele = 0; i < 3; i++) {
		int j;
		if (slot[i] <= 0)
			continue;
		j = pc_search_inventory(sd, slot[i]);
		if (j < 0)
			continue;
		if (slot[i] == 1000) {
			pc_delitem(sd, j, 1, 1);
			sc++;
		}
		if (slot[i] >= 994 && slot[i] <= 997 && ele == 0) {
			static const int ele_table[4] = {3, 1, 4, 2};
			pc_delitem(sd, j, 1, 1);
			ele = ele_table[slot[i] - 994];
		}
	}


	for(i = 0; i < MAX_PRODUCE_RESOURCE; i++) {
		int j, id, x;
		id = skill_produce_db[idx].mat_id[i];
		if (id <= 0)
			continue;
		x = skill_produce_db[idx].mat_amount[i];
		do{
			int y = 0;
			j = pc_search_inventory(sd,id);

			if (j >= 0) {
				y = sd->status.inventory[j].amount;
				if (y > x) y = x;
				pc_delitem(sd, j, y, 0);
			}else {
				if (battle_config.error_log)
					printf("skill_produce_mix: material item error\n");
			}

			x -= y;
		}while(j >= 0 && x > 0);
	}


	if((equip = itemdb_isequip(nameid)))
		wlv = itemdb_wlv(nameid);
	if (!equip) {
		switch(skill_produce_db[idx].req_skill) {
			case BS_IRON:
			case BS_STEEL:
			case BS_ENCHANTEDSTONE:
				{ // Ores & Metals Refining - skill bonuses are straight from kRO website [DracoRPG]
				int skill = pc_checkskill(sd, skill_produce_db[idx].req_skill);
				make_per = sd->status.job_level * 20 + sd->paramc[4] * 10 + sd->paramc[5] * 10; //Base chance
				switch(nameid) {
					case 998: // Iron
						make_per += 4000 + skill * 500; // Temper Iron bonus: +26/+32/+38/+44/+50
						break;
					case 999: // Steel
						make_per += 3000 + skill * 500; // Temper Steel bonus: +35/+40/+45/+50/+55
						break;
					case 1000: //Star Crumb
						make_per = 100000; // Star Crumbs are 100% success crafting rate? (made 1000% so it succeeds even after penalties) [Skotlex]
						break;
					default: // Enchanted Stones
						make_per += 1000 + skill * 500; // Enchantedstone Craft bonus: +15/+20/+25/+30/+35
					break;
				}
				}
				break;
			case ASC_CDP:
				make_per = (2000 + 40 * sd->paramc[4] + 20 * sd->paramc[5]);
				break;	
			case AL_HOLYWATER:
				make_per = 100000; //100% success
				break;
			case AM_PHARMACY: // Potion Preparation - reviewed with the help of various Ragnainfo sources [DracoRPG]
				make_per = pc_checkskill(sd, AM_LEARNINGPOTION) * 100
					+ pc_checkskill(sd, AM_PHARMACY) * 300 + sd->status.job_level * 20
					+ sd->paramc[3] * 5 + sd->paramc[4] * 10 + sd->paramc[5] * 10;
				switch(nameid){
					case 501: // Red Potion
					case 503: // Yellow Potion
					case 504: // White Potion
					case 605: // Anodyne
					case 606: // Aloevera
						make_per += 2000;
						break;
					case 505: // Blue Potion
						make_per -= 500;
						break;
					case 545: // Condensed Red Potion
					case 546: // Condensed Yellow Potion
					case 547: // Condensed White Potion
						flag = 1; //Condensed potions
						make_per -= 1000;
					  break;
				 	case 970: // Alcohol
						make_per += 1000;
						break;
					case 7139: // Glistening Coat
						make_per -= 1000;
						break;
					case 7135: // Bottle Grenade
					case 7136: // Acid Bottle
					case 7137: // Plant Bottle
					case 7138: // Marine Sphere Bottle
					default:
						break;
				}
				if(battle_config.pp_rate != 100)
					make_per = make_per * battle_config.pp_rate / 100;
				break;
			case SA_CREATECON: // Elemental Converter Creation - skill bonuses are from kRO [DracoRPG]
				make_per = pc_checkskill(sd, SA_ADVANCEDBOOK) * 100 + //TODO: Advanced Book bonus is custom! [Skotlex]
					sd->status.job_level * 20 + sd->paramc[3] * 10 + sd->paramc[4] * 10;
				switch(nameid) {
					case 12114:
						flag = pc_checkskill(sd, SA_FLAMELAUNCHER);
						if (flag > 0)
							make_per += 1000 * flag - 500;
						break;
					case 12115:
						flag = pc_checkskill(sd, SA_FROSTWEAPON);
						if (flag > 0)
							make_per += 1000 * flag - 500;
						break;
					case 12116:
						flag = pc_checkskill(sd, SA_SEISMICWEAPON);
						if (flag > 0)
							make_per += 1000 * flag - 500;
						break;
					case 12117:
						flag = pc_checkskill(sd, SA_LIGHTNINGLOADER);
						if (flag > 0)
							make_per += 1000 * flag - 500;
						break;
				}
				break;
			default:
				make_per = 5000;
				break;
		} //end switch skill_produce_db[idx].req_skill
	} else { // Weapon Forging - skill bonuses are straight from kRO website, other things from a jRO calculator [DracoRPG]
		make_per = 5000 + sd->status.job_level * 20 + sd->paramc[4] * 10 + sd->paramc[5] * 10; // Base chance
		make_per += pc_checkskill(sd, skill_produce_db[idx].req_skill) * 500; // Smithing skills bonus: +5/+10/+15
		make_per += pc_checkskill(sd, BS_WEAPONRESEARCH) * 100 +((wlv >= 3)? pc_checkskill(sd,BS_ORIDEOCON) * 100 : 0); // Weaponry Research bonus: +1/+2/+3/+4/+5/+6/+7/+8/+9/+10, Oridecon Research bonus (custom): +1/+2/+3/+4/+5
		make_per -= (ele?2000:0) + sc * 1500 + (wlv > 1 ? wlv * 1000 : 0); // Element Stone: -20%, Star Crumb: -15% each, Weapon level malus: -0/-20/-30
		if(pc_search_inventory(sd, 989) > 0) make_per+= 1000; // Emperium Anvil: +10
		else if(pc_search_inventory(sd, 988) > 0) make_per+= 500; // Golden Anvil: +5
		else if(pc_search_inventory(sd, 987) > 0) make_per+= 300; // Oridecon Anvil: +3
		else if(pc_search_inventory(sd, 986) > 0) make_per+= 0; // Anvil: +0?
		if(battle_config.wp_rate != 100)
			make_per = make_per * battle_config.wp_rate / 100;
	}

	if(make_per < 1)
		make_per = 1;

	if(skill_produce_db[idx].req_skill == AM_PHARMACY || skill_produce_db[idx].req_skill == ASC_CDP || skill_produce_db[idx].req_skill == CR_ALCHEMY) {
		if(battle_config.pp_rate != 100)
			make_per = make_per * battle_config.pp_rate / 100;
	} else {
		if (battle_config.wp_rate != 100)
			make_per = make_per * battle_config.wp_rate / 100;
	}



	if (rand() % 10000 < make_per) {
		struct item tmp_item;
		memset(&tmp_item, 0, sizeof(tmp_item));
		tmp_item.nameid = nameid;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		if (equip) {
			tmp_item.card[0] = 0x00ff;
			tmp_item.card[1] = ((sc*5)<<8) + ele;
			*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
			clif_produceeffect(sd,0,nameid);
			clif_misceffect(&sd->bl,3);
		} else {
			switch(skill_produce_db[idx].req_skill) {
				case AM_PHARMACY:
					flag = battle_config.produce_potion_name_input;
					break;
				case AL_HOLYWATER:
					flag = battle_config.holywater_name_input;
					break;
				default:
					flag = battle_config.produce_item_name_input;
					break;
			}
			if (flag) {
				tmp_item.card[0] = 0x00fe;
				tmp_item.card[1] = 0;
				*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
			}
		}

		switch (skill_produce_db[idx].req_skill) {
			case AM_PHARMACY:
			case ASC_CDP:
				clif_produceeffect(sd, 2, nameid);
				clif_misceffect(&sd->bl, 5);
				break;
			case AL_HOLYWATER:
				clif_skill_nodamage(&sd->bl, &sd->bl, AL_HOLYWATER, 1, 1);
				break;
			default:
				clif_produceeffect(sd, 0, nameid);
				clif_misceffect(&sd->bl, 3);
			break;
		}

		if ((flag = pc_additem(sd, &tmp_item, 1))) {
			clif_additem(sd, 0, 0, flag);
			map_addflooritem(&tmp_item, 1, sd->bl.m, sd->bl.x, sd->bl.y, NULL, NULL, NULL, sd->bl.id, 0);
		}
		return 1;
	} else {
		switch (skill_produce_db[idx].req_skill) {
			case AM_PHARMACY:
				clif_produceeffect(sd, 3, nameid);
				clif_misceffect(&sd->bl, 6);
				break;
			case ASC_CDP:
				clif_produceeffect(sd, 3, nameid);
				clif_misceffect(&sd->bl, 6);
				pc_heal(sd, -(sd->status.max_hp>>2), 0);
				break;
			default:
				clif_produceeffect(sd, 1, nameid);
				clif_misceffect(&sd->bl, 2);
				break;
		}
	}

	return 0;
}

void skill_arrow_create(struct map_session_data *sd, unsigned short nameid) {
	int i, j, flag, idx;
	struct item tmp_item;

//	nullpo_retv(sd); // checked before to call function

	for(idx = 0; idx < num_skill_arrow_db; idx++) // MAX_SKILL_ARROW_DB -> dynamic now
		if (nameid == skill_arrow_db[idx].nameid)
			break;

	if (idx == num_skill_arrow_db || (j = pc_search_inventory(sd, nameid)) < 0)
		return;

	pc_delitem(sd, j, 1, 0);
	for(i = 0; i < 5; i++) {
		memset(&tmp_item, 0, sizeof(tmp_item));
		tmp_item.nameid = skill_arrow_db[idx].cre_id[i];
		if (tmp_item.nameid <= 0)
			continue;
		tmp_item.amount = skill_arrow_db[idx].cre_amount[i];
		if (tmp_item.amount <= 0)
			continue;
		tmp_item.identify = 1;
		if (battle_config.making_arrow_name_input) {
			tmp_item.card[0] = 0x00fe;
			tmp_item.card[1] = 0;
			*((unsigned long *)(&tmp_item.card[2])) = sd->char_id; /* �L����ID */
		}
		if ((flag = pc_additem(sd, &tmp_item, tmp_item.amount))) {
			clif_additem(sd, 0, 0, flag);
			map_addflooritem(&tmp_item, tmp_item.amount, sd->bl.m, sd->bl.x, sd->bl.y, NULL, NULL, NULL, sd->bl.id, 0);
		}
	}

	return;
}

/*--------------------------------------------------------------------------------------
 Plagiarism to work on passive damaging skill, and implement Preserve later - [Aalye]
 ------------------------------------------------------------------------------------ */
void skill_copy_skill(struct map_session_data *tsd, short skillid, short skilllv) {

	// don't plagia a known skill
	if (tsd->status.skill[skillid].lv > 0)
		return;

	if (tsd->cloneskill_id && tsd->status.skill[tsd->cloneskill_id].flag == 13) {
		tsd->status.skill[tsd->cloneskill_id].id = 0;
		tsd->status.skill[tsd->cloneskill_id].lv = 0;
		tsd->status.skill[tsd->cloneskill_id].flag = 0; // flag: 0 (normal), 1 (only card), 2-12 (card and skill (skill level +2)), 13 (cloneskill)
	}
	tsd->cloneskill_id = skillid;
	tsd->status.skill[skillid].id = skillid;
	tsd->status.skill[skillid].lv = (pc_checkskill(tsd, RG_PLAGIARISM) > skill_get_max(skillid)) ?
	                                skill_get_max(skillid) : pc_checkskill(tsd, RG_PLAGIARISM);
	tsd->status.skill[skillid].flag = 13; // flag: 0 (normal), 1 (only card), 2-12 (card and skill (skill level +2)), 13 (cloneskill)
	clif_skillinfoblock(tsd);

	return;
}

int skill_split_str(char *str, char **val, int num)
{
	int i;

	for (i = 0; i < num && str; i++){
		val[i] = str;
		str = strchr(str, ',');
		if (str)
			*str++ = 0;
	}

	return i;
}

int skill_split_atoi(char *str, int *val)
{
	int i, max = 0;

	for (i = 0; i < MAX_SKILL_LEVEL; i++) {
		if (str) {
			val[i] = max = atoi(str);
			str = strchr(str, ':');
			if (str)
				*str++ = 0;
		} else {
			val[i] = max;
		}
	}

	return i;
}

void skill_init_unit_layout()
{
	int i, j, size, pos = 0;

	memset(skill_unit_layout, 0, sizeof(skill_unit_layout));
	for (i = 0; i <= MAX_SQUARE_LAYOUT; i++) {
		size = i * 2 + 1;
		skill_unit_layout[i].count = size * size;
		for (j = 0; j < size * size; j++) {
			skill_unit_layout[i].dx[j] = (j % size - i);
			skill_unit_layout[i].dy[j] = (j / size - i);
		}
	}
	pos = i;
	for (i = 0; i < MAX_SKILL_DB; i++) {
		if (!skill_db[i].unit_id[0] || skill_db[i].unit_layout_type[0] != -1)
			continue;
		switch (i) {
			case MG_FIREWALL:
			case WZ_ICEWALL:
				break;
			case PR_SANCTUARY:
			{
				static const int dx[] = {
					-1, 0, 1,-2,-1, 0, 1, 2,-2,-1,
					 0, 1, 2,-2,-1, 0, 1, 2,-1, 0, 1};
				static const int dy[]={
					-2,-2,-2,-1,-1,-1,-1,-1, 0, 0,
					 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};
				skill_unit_layout[pos].count = 21;
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
				break;
			}
			case PR_MAGNUS:
			{
				static const int dx[] = {
					-1, 0, 1,-1, 0, 1,-3,-2,-1, 0,
					 1, 2, 3,-3,-2,-1, 0, 1, 2, 3,
					-3,-2,-1, 0, 1, 2, 3,-1, 0, 1,-1, 0, 1};
				static const int dy[] = {
					-3,-3,-3,-2,-2,-2,-1,-1,-1,-1,
					-1,-1,-1, 0, 0, 0, 0, 0, 0, 0,
					 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};
				skill_unit_layout[pos].count = 33;
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
				break;
			}
			case AS_VENOMDUST:
			{
				static const int dx[] = {-1, 0, 0, 0, 1};
				static const int dy[] = { 0,-1, 0, 1, 0};
				skill_unit_layout[pos].count = 5;
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
				break;
			}
			case CR_GRANDCROSS:
			case NPC_DARKGRANDCROSS:
			{
				static const int dx[] = {
					 0, 0,-1, 0, 1,-2,-1, 0, 1, 2,
					-4,-3,-2,-1, 0, 1, 2, 3, 4,-2,
					-1, 0, 1, 2,-1, 0, 1, 0, 0};
				static const int dy[] = {
					-4,-3,-2,-2,-2,-1,-1,-1,-1,-1,
					 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
					 1, 1, 1, 1, 2, 2, 2, 3, 4};
				skill_unit_layout[pos].count = 29;
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
				break;
			}
			case PF_FOGWALL:
			{
				static const int dx[] = {
					-2,-1, 0, 1, 2,-2,-1, 0, 1, 2,-2,-1, 0, 1, 2};
				static const int dy[] = {
					-1,-1,-1,-1,-1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1};
				skill_unit_layout[pos].count = 15;
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
				break;
			}
			default:
				printf("unknown unit layout at skill %d\n", i);
				break;
		}
		if (!skill_unit_layout[pos].count)
			continue;
		for (j = 0; j < MAX_SKILL_LEVEL; j++)
			skill_db[i].unit_layout_type[j] = pos;
		pos++;
	}
	firewall_unit_pos = pos;
	for (i = 0; i < 8; i++) {
		if (i & 1) { /* �΂ߔz�u */
			skill_unit_layout[pos].count = 5;
			if (i & 0x2) {
				int dx[] = {-1,-1, 0, 0, 1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			} else {
				int dx[] = { 1, 1 ,0, 0,-1};
				int dy[] = { 1, 0, 0,-1,-1};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			}
		} else {	/* �c���z�u */
			skill_unit_layout[pos].count = 3;
			if (i % 4 == 0) { /* �㉺ */
				int dx[] = {-1, 0, 1};
				int dy[] = { 0, 0, 0};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			} else { /* ���E */
				int dx[] = { 0, 0, 0};
				int dy[] = {-1, 0, 1};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			}
		}
		pos++;
	}
	icewall_unit_pos = pos;
	for (i = 0; i < 8; i++) {
		skill_unit_layout[pos].count = 5;
		if (i & 1) { /* �΂ߔz�u */
			if (i & 0x2) {
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 2,-1, 0,-1,-2};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			} else {
				int dx[] = { 2, 1 ,0,-1,-2};
				int dy[] = { 2, 1, 0,-1,-2};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			}
		} else { /* �c���z�u */
			if (i % 4 == 0) { /* �㉺ */
				int dx[] = {-2,-1, 0, 1, 2};
				int dy[] = { 0, 0, 0, 0, 0};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			} else {			/* ���E */
				int dx[] = { 0, 0, 0, 0, 0};
				int dy[] = {-2,-1, 0, 1, 2};
				memcpy(skill_unit_layout[pos].dx, dx, sizeof(dx));
				memcpy(skill_unit_layout[pos].dy, dy, sizeof(dy));
			}
		}
		pos++;
	}
}

/*==========================================
 * Skills Database Reading
 *------------------------------------------
 */
int skill_readdb(void) {
	int i,j,k,l,m;
	FILE *fp;
	char line[1024],*p;
	char *filename[] = {"db/produce_db.txt","db/produce_db2.txt"};

	memset(skill_db, 0, sizeof(skill_db));
	fp=fopen("db/skill_db.txt","r");
	if(fp==NULL){
		printf("can't read db/skill_db.txt\n");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[50];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split,0,sizeof(split));
		j = skill_split_str(line, split, 14);
		if (split[13] == NULL || j < 14)
			continue;

		i = atoi(split[0]);
		if (i >= 10000 && i < 10015) // for guild skills [Celest]
			i -= 9500;
		else if (i <= 0 || i > MAX_SKILL_DB)
			continue;

		skill_split_atoi(split[1], skill_db[i].range);
		skill_db[i].hit=atoi(split[2]);
		skill_db[i].inf=atoi(split[3]);
		skill_db[i].pl=atoi(split[4]);
		skill_db[i].nk=atoi(split[5]);
		skill_db[i].max=atoi(split[6]);
		skill_split_atoi(split[7], skill_db[i].num);

		if (strcasecmp(split[8],"yes") == 0)
			skill_db[i].castcancel=1;
		else
			skill_db[i].castcancel=0;
		skill_db[i].cast_def_rate=atoi(split[9]);
		skill_db[i].inf2=atoi(split[10]);
		skill_db[i].maxcount=atoi(split[11]);
		if (strcasecmp(split[12],"weapon") == 0)
			skill_db[i].skill_type=BF_WEAPON;
		else if (strcasecmp(split[12],"magic") == 0)
			skill_db[i].skill_type=BF_MAGIC;
		else if (strcasecmp(split[12],"misc") == 0)
			skill_db[i].skill_type=BF_MISC;
		else
			skill_db[i].skill_type=0;
		skill_split_atoi(split[13], skill_db[i].blewcount);
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_db.txt" CL_RESET "' readed.\n");

	fp=fopen("db/skill_require_db.txt","r");
	if(fp==NULL){
		printf("can't read db/skill_require_db.txt\n");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[50];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 30);
		if(split[29]==NULL || j<30)
			continue;

		i=atoi(split[0]);
		if (i>=10000 && i<10015) // for guild skills [Celest]
			i -= 9500;
		else if(i<=0 || i>MAX_SKILL_DB)
			continue;

		skill_split_atoi(split[1], skill_db[i].hp);
		skill_split_atoi(split[2], skill_db[i].mhp);
		skill_split_atoi(split[3], skill_db[i].sp);
		skill_split_atoi(split[4], skill_db[i].hp_rate);
		skill_split_atoi(split[5], skill_db[i].sp_rate);
		skill_split_atoi(split[6], skill_db[i].zeny);

		p = split[7];
		for(j = 0; j < 32; j++) {
			l = atoi(p);
			if (l == 99) {
				skill_db[i].weapon = 0xffffffff;
				break;
			}
			else
				skill_db[i].weapon |= 1 << l;
			p = strchr(p, ':');
			if (!p)
				break;
			p++;
		}

		if ( strcasecmp(split[8], "hiding")==0 ) skill_db[i].state=ST_HIDING;
		else if ( strcasecmp(split[8], "cloaking")==0 ) skill_db[i].state=ST_CLOAKING;
		else if ( strcasecmp(split[8], "hidden")==0 ) skill_db[i].state=ST_HIDDEN;
		else if ( strcasecmp(split[8], "riding")==0 ) skill_db[i].state=ST_RIDING;
		else if ( strcasecmp(split[8], "falcon")==0 ) skill_db[i].state=ST_FALCON;
		else if ( strcasecmp(split[8], "cart")==0 ) skill_db[i].state=ST_CART;
		else if ( strcasecmp(split[8], "shield")==0 ) skill_db[i].state=ST_SHIELD;
		else if ( strcasecmp(split[8], "sight")==0 ) skill_db[i].state=ST_SIGHT;
		else if ( strcasecmp(split[8], "explosionspirits")==0 ) skill_db[i].state=ST_EXPLOSIONSPIRITS;
		else if ( strcasecmp(split[8], "recover_weight_rate")==0 ) skill_db[i].state=ST_RECOV_WEIGHT_RATE;
		else if ( strcasecmp(split[8], "move_enable")==0 ) skill_db[i].state=ST_MOVE_ENABLE;
		else if ( strcasecmp(split[8], "water")==0 ) skill_db[i].state=ST_WATER;
		else skill_db[i].state = ST_NONE;

		skill_split_atoi(split[9], skill_db[i].spiritball);
		skill_db[i].itemid[0]=atoi(split[10]);
		skill_db[i].amount[0]=atoi(split[11]);
		skill_db[i].itemid[1]=atoi(split[12]);
		skill_db[i].amount[1]=atoi(split[13]);
		skill_db[i].itemid[2]=atoi(split[14]);
		skill_db[i].amount[2]=atoi(split[15]);
		skill_db[i].itemid[3]=atoi(split[16]);
		skill_db[i].amount[3]=atoi(split[17]);
		skill_db[i].itemid[4]=atoi(split[18]);
		skill_db[i].amount[4]=atoi(split[19]);
		skill_db[i].itemid[5]=atoi(split[20]);
		skill_db[i].amount[5]=atoi(split[21]);
		skill_db[i].itemid[6]=atoi(split[22]);
		skill_db[i].amount[6]=atoi(split[23]);
		skill_db[i].itemid[7]=atoi(split[24]);
		skill_db[i].amount[7]=atoi(split[25]);
		skill_db[i].itemid[8]=atoi(split[26]);
		skill_db[i].amount[8]=atoi(split[27]);
		skill_db[i].itemid[9]=atoi(split[28]);
		skill_db[i].amount[9]=atoi(split[29]);
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_require_db.txt" CL_RESET "' readed.\n");

	fp=fopen("db/skill_cast_db.txt","r");
	if(fp==NULL){
		printf("can't read db/skill_cast_db.txt\n");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[50];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 5);
		if(split[4]==NULL || j<5)
			continue;

		i=atoi(split[0]);
		if (i>=10000 && i<10015) // for guild skills [Celest]
			i -= 9500;
		else if(i<=0 || i>MAX_SKILL_DB)
			continue;

		skill_split_atoi(split[1], skill_db[i].cast);
		skill_split_atoi(split[2], skill_db[i].delay);
		skill_split_atoi(split[3], skill_db[i].upkeep_time);
		skill_split_atoi(split[4], skill_db[i].upkeep_time2);
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_cast_db.txt" CL_RESET "' readed.\n");

	fp = fopen("db/skill_unit_db.txt", "r");
	if (fp == NULL) {
		printf("can't read db/skill_unit_db.txt\n");
		return 1;
	}
	while (fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[50];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 8);
		if (split[7] == NULL || j < 8)
			continue;

		i = atoi(split[0]);
		if (i >= 10000 && i < 10015) // for guild skills [Celest]
			i -= 9500;
		else if (i <= 0 || i > MAX_SKILL_DB)
			continue;
		skill_db[i].unit_id[0] = strtol(split[1], NULL, 16);
		skill_db[i].unit_id[1] = strtol(split[2], NULL, 16);
		skill_split_atoi(split[3], skill_db[i].unit_layout_type);
		skill_db[i].unit_range = atoi(split[4]);
		skill_db[i].unit_interval = atoi(split[5]);

		if (strcasecmp(split[6], "noenemy") == 0) skill_db[i].unit_target = BCT_NOENEMY;
		else if (strcasecmp(split[6], "friend") == 0) skill_db[i].unit_target = BCT_NOENEMY;
		else if (strcasecmp(split[6], "party") == 0) skill_db[i].unit_target = BCT_PARTY;
		else if (strcasecmp(split[6], "all") == 0) skill_db[i].unit_target = BCT_ALL;
		else if (strcasecmp(split[6], "enemy") == 0) skill_db[i].unit_target = BCT_ENEMY;
		else if (strcasecmp(split[6], "self") == 0) skill_db[i].unit_target = BCT_SELF;
		else skill_db[i].unit_target = strtol(split[6], NULL, 16);

		skill_db[i].unit_flag = strtol(split[7], NULL, 16);
		k++;
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_unit_db.txt" CL_RESET "' readed.\n");

	skill_init_unit_layout();

	memset(skill_produce_db,0,sizeof(skill_produce_db));
	for(m = 0; m < 2; m++) {
		fp=fopen(filename[m],"r");
		if(fp==NULL){
			if(m>0)
				continue;
			printf("can't read %s\n",filename[m]);
			return 1;
		}
		k=0;
		while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
			char *split[6 + MAX_PRODUCE_RESOURCE * 2];
			int x, y;
			if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
				continue;
			// it's not necessary to remove 'carriage return ('\n' or '\r')
			memset(split,0,sizeof(split));
			j = skill_split_str(line, split, (3 + MAX_PRODUCE_RESOURCE * 2));
			if (split[0] == 0)
				continue;
			i=atoi(split[0]);
			if(i<=0)
				continue;

			skill_produce_db[k].nameid=i;
			skill_produce_db[k].itemlv=atoi(split[1]);
			skill_produce_db[k].req_skill=atoi(split[2]);

			for(x=3,y=0; split[x] && split[x+1] && y<MAX_PRODUCE_RESOURCE; x+=2,y++){
				skill_produce_db[k].mat_id[y]=atoi(split[x]);
				skill_produce_db[k].mat_amount[y]=atoi(split[x+1]);
			}
			k++;
			if(k >= MAX_SKILL_PRODUCE_DB)
				break;
		}
		fclose(fp);
		printf("DB '" CL_WHITE "%s" CL_RESET "' readed ('" CL_WHITE "%d" CL_RESET "' entrie%s).\n", filename[m], k, (k > 1) ? "s" : "");
	}

	num_skill_arrow_db = 0;
	FREE(skill_arrow_db);
	fp = fopen("db/create_arrow_db.txt", "r");
	if (fp == NULL) {
		printf("can't read db/create_arrow_db.txt\n");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[16];
		int x, y;
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 13);
		if (split[0] == 0 || j < 3) // at least 'SourceID,MakeID1,MakeNum1'
			continue;
		i = atoi(split[0]);
		if (i <= 0)
			continue;

		if (num_skill_arrow_db == 0) {
			CALLOC(skill_arrow_db, struct skill_arrow_db, 1);
		} else {
			REALLOC(skill_arrow_db, struct skill_arrow_db, num_skill_arrow_db + 1);
			memset(skill_arrow_db + num_skill_arrow_db, 0, sizeof(struct skill_arrow_db)); // need for the list of created items
		}

		skill_arrow_db[num_skill_arrow_db].nameid = i;

		y = 0;
		for(x = 1; split[x] && split[x + 1] && y < 5; x += 2) {
			if (atoi(split[x]) > 0 && atoi(split[x + 1]) > 0) {
				skill_arrow_db[num_skill_arrow_db].cre_id[y] = atoi(split[x]);
				skill_arrow_db[num_skill_arrow_db].cre_amount[y] = atoi(split[x + 1]);
				y++;
			}
		}
		num_skill_arrow_db++;
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/create_arrow_db.txt" CL_RESET "' readed ('" CL_WHITE "%d" CL_RESET "' entrie%s).\n", num_skill_arrow_db, (num_skill_arrow_db > 1) ? "s" : "");

	memset(skill_abra_db, 0, sizeof(skill_abra_db));
	fp=fopen("db/abra_db.txt","r");
	if(fp==NULL){
		printf("can't read db/abra_db.txt\n");
		return 1;
	}
	k = 0;
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[16];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split,0,sizeof(split));
		j = skill_split_str(line, split, 13);
		if (split[0] == 0)
			continue;
		i=atoi(split[0]);
		if(i<=0)
			continue;

		skill_abra_db[i].req_lv=atoi(split[2]);
		skill_abra_db[i].per=atoi(split[3]);

		k++;
		if(k >= MAX_SKILL_ABRA_DB)
			break;
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/abra_db.txt" CL_RESET "' readed ('" CL_WHITE "%d" CL_RESET "' entrie%s).\n", k, (k > 1) ? "s" : "");

	fp=fopen("db/skill_castnodex_db.txt","r");
	if(fp==NULL){
		printf("can't read db/skill_castnodex_db.txt\n");
		return 1;
	}
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[50];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 3);
		if (split[0] == 0)
			continue;

		i = atoi(split[0]);
		if (i >= 10000 && i < 10015) // for guild skills [Celest]
			i -= 9500;
		else if (i <= 0 || i > MAX_SKILL_DB)
			continue;

		skill_split_atoi(split[1], skill_db[i].castnodex);
		if (split[2] == 0)
			continue;
		skill_split_atoi(split[2], skill_db[i].delaynodex);
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_castnodex_db.txt" CL_RESET "' readed.\n");

	fp = fopen("db/skill_nocast_db.txt","r");
	if (fp == NULL) {
		printf("can't read db/skill_nocast_db.txt\n");
		return 1;
	}
	k = 0;
	while(fgets(line, sizeof(line), fp)) { // fgets reads until maximum one less than size and add '\0' -> so, it's not necessary to add -1
		char *split[16];
		if ((line[0] == '/' && line[1] == '/') || line[0] == '\0' || line[0] == '\n' || line[0] == '\r')
			continue;
		// it's not necessary to remove 'carriage return ('\n' or '\r')
		memset(split, 0, sizeof(split));
		j = skill_split_str(line, split, 2);
		if (split[0] == 0)
			continue;
		i=atoi(split[0]);
		if (i>=10000 && i<10015) // for guild skills [Celest]
			i -= 9500;
		else if(i<=0 || i>MAX_SKILL_DB)
			continue;
		skill_db[i].nocast=atoi(split[1]);
		k++;
	}
	fclose(fp);
	printf("DB '" CL_WHITE "db/skill_nocast_db.txt" CL_RESET "' readed ('" CL_WHITE "%d" CL_RESET "' entrie%s).\n", k, (k > 1) ? "s" : "");

	return 0;
}

/*===============================================
 * For reading leveluseskillspamount.txt [Celest]
 *-----------------------------------------------
 */
static int skill_read_skillspamount(void) {
	char *buf, *p;
	struct skill_db *skill = NULL;
	int s, idx, new_flag = 1, level = 1, sp = 0;

	buf = grfio_reads("data\\leveluseskillspamount.txt", &s);

	if (buf == NULL)
		return -1;

	buf[s] = 0;
	p = buf;
	while(p - buf < s) {
		char buf2[64];

		if (sscanf(p, "%[@]", buf2) == 1) {
			level = 1;
			new_flag = 1;
		} else if (new_flag && sscanf(p, "%[^#]#", buf2) == 1) {
			for (idx = 0; skill_names[idx].id != 0; idx++) {
				if (strstr(buf2, skill_names[idx].name) != NULL) {
					skill = &skill_db[skill_names[idx].id];
					new_flag = 0;
					break;
				}
			}
		} else if (!new_flag && sscanf(p, "%d#", &sp) == 1) {
			skill->sp[level-1] = sp;
			level++;
		}

		p = strchr(p, 10);
		if (!p)
			break;
		p++;
	}
	FREE(buf);

	printf("File '" CL_WHITE "data\\leveluseskillspamount.txt" CL_RESET "' readed.\n");

	return 0;
}

void skill_reload(void) {
	do_init_skill();
}

/*==========================================
 * Init Skill
 *------------------------------------------
 */
int do_init_skill(void) {
	skill_readdb();
	if (battle_config.skill_sp_override_grffile)
		skill_read_skillspamount();

	add_timer_func_list(skill_unit_timer, "skill_unit_timer");
	add_timer_func_list(skill_castend_id, "skill_castend_id");
	add_timer_func_list(skill_castend_pos, "skill_castend_pos");
	add_timer_func_list(skill_timerskill, "skill_timerskill");

	add_timer_interval(gettick_cache + SKILLUNITTIMER_INVERVAL, skill_unit_timer, 0, 0, SKILLUNITTIMER_INVERVAL);

	return 0;
}

