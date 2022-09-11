// kgsws' ACE Engine
////
// This file contains required engine types and function prototypes.

//
// basic

// extra include to manage amount of text
#include "e_mobj.h"

#define MAXWADFILES	20

#define SCREENWIDTH	320
#define SCREENHEIGHT	200
#define SCREENSIZE	(SCREENWIDTH*SCREENHEIGHT)

#define FRACBITS	16
#define FRACUNIT	(1 << 16)

typedef int32_t fixed_t;
typedef uint32_t angle_t;

//
// ACE Engine stuff

// do not change
#define NUM_WPN_SLOTS	10
#define NUMMOBJTYPES	137
#define NUMSTATES	967
#define NUMSPRITES	138

//
// tables

#define ANGLETOFINESHIFT	19
#define FINEANGLES	8192
#define FINEMASK	(FINEANGLES - 1)

#define ANG45	0x20000000
#define ANG90	0x40000000
#define ANG180	0x80000000
#define ANG270	0xC0000000

#define SLOPERANGE	2048
#define SLOPEBITS	11
#define DBITS	(FRACBITS - SLOPEBITS)

//
// skill

#define sk_baby	0
#define sk_easy	1
#define sk_medium	2
#define sk_hard	3
#define sk_nightmare	4

//
// cheats

#define CF_NOCLIP	1
#define CF_GODMODE	2
#define CF_NOMOMENTUM	4
#define CF_BUDDHA	8
#define CF_REVENGE	16
#define CF_IS_CHEATER	0x80000000

//
// zone

#define PU_STATIC	1
#define PU_LEVEL	50
#define PU_LEVELSPEC	51
#define PU_CACHE	101

//
// menu

typedef struct menuitem_s
{
	int16_t status;
	uint8_t name[10];
	void (*func)(uint32_t) __attribute((regparm(2),no_caller_saved_registers));
	uint8_t key;
} __attribute__((packed)) menuitem_t;

typedef struct menu_s
{
	uint16_t count;
	struct menuitem_s *prev;
	menuitem_t *items;
	void (*draw)();
	int16_t x, y;
	uint16_t last;
} __attribute__((packed)) menu_t;

//
// tics

#define BT_ATTACK	1
#define BT_USE	2
#define BT_ALTACK	64
#define BT_SPECIAL	128

typedef struct
{
	int8_t forwardmove;
	int8_t sidemove;
	int16_t angleturn;
	int16_t consistancy;
	uint8_t chatchar;
	uint8_t buttons;
} ticcmd_t;

typedef struct thinker_s
{
	struct thinker_s *prev;
	struct thinker_s *next;
	void *function;
} thinker_t;

//
// weapons

typedef struct weaponinfo_s
{
	uint32_t ammo;
	uint32_t upstate;
	uint32_t downstate;
	uint32_t readystate;
	uint32_t atkstate;
	uint32_t flashstate;
} weaponinfo_t;

typedef struct
{
	struct state_s *state;
	int32_t tics;
	fixed_t	sx;
	fixed_t	sy;
} pspdef_t;

//
// player

#define	NUMPSPRITES	2
#define MAXPLAYERS	4

#define NUMWEAPONS	9
#define NUMAMMO		4
#define NUMCARDS		6

#define WEAPONBOTTOM	(128 << FRACBITS)
#define WEAPONTOP	(32 << FRACBITS)
#define WEAPONRAISE	(6 << FRACBITS)

#define USERANGE	(64 << FRACBITS)
#define MELEERANGE	(64 << FRACBITS)
#define MISSILERANGE	(2048 << FRACBITS)
#define AIMRANGE	(1024 << FRACBITS)

enum
{
	PST_LIVE,
	PST_DEAD,
	PST_REBORN
};

enum
{
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	NUMPOWERS
};

typedef struct player_s
{ // this structure has been changed
	struct mobj_s *mo;
	uint32_t playerstate;
	ticcmd_t cmd;
	fixed_t viewz;
	fixed_t viewheight;
	fixed_t deltaviewheight;
	fixed_t bob;
	int health;
	int armorpoints;
	int armortype;
	int powers[NUMPOWERS];
	uint32_t cards[NUMCARDS];
	struct inventory_s *inventory; // for level transition
	int frags[MAXPLAYERS];
	struct mobjinfo_s *readyweapon;
	struct mobjinfo_s *pendingweapon;
	uint32_t stbar_update;
	uint32_t __unused[8+4+4];
	uint16_t attackdown;
	uint8_t weapon_ready;
	uint8_t backpack;
	int usedown;
	int cheats;
	int refire;
	int killcount;
	int itemcount;
	int secretcount;
	char *message;
	int damagecount;
	int bonuscount;
	struct mobj_s *attacker;
	int extralight;
	int fixedcolormap;
	int colormap;
	pspdef_t psprites[NUMPSPRITES];
	uint32_t didsecret;
} player_t;

//
// video

typedef struct patch_s
{
	uint16_t width, height;
	int16_t x, y;
	uint32_t offs[];
} patch_t;

struct column_s;

//
// WAD

typedef struct
{
	union
	{
		uint8_t name[8];
		uint32_t same[2];
		uint64_t wame;
	};
	int32_t fd;
	uint32_t offset;
	uint32_t size;
} lumpinfo_t;

typedef struct
{
	uint32_t id;
	uint32_t numlumps;
	uint32_t diroffs;
} wadhead_t;

typedef struct
{
	uint32_t offset;
	uint32_t size;
	union
	{
		uint8_t name[8];
		uint32_t same[2];
		uint64_t wame;
	};
} wadlump_t;

//
// info

enum
{
	ETYPE_NONE, // must be first
	ETYPE_PLAYERPAWN,
	ETYPE_HEALTH,
	ETYPE_INV_SPECIAL,
	ETYPE_INVENTORY,
	ETYPE_INVENTORY_CUSTOM,
	ETYPE_WEAPON,
	ETYPE_AMMO,
	ETYPE_AMMO_LINK,
	ETYPE_KEY,
	ETYPE_ARMOR,
	ETYPE_ARMOR_BONUS,
	ETYPE_POWERUP,
	//
	NUM_EXTRA_TYPES,
};

typedef struct
{
	uint16_t type;
	int16_t chance;
	uint16_t amount;
	uint16_t __padding;
} mobj_dropitem_t;

typedef struct
{
	uint16_t type;
	uint16_t count;
} plrp_start_item_t;

typedef struct
{
	uint16_t *wpn_slot[NUM_WPN_SLOTS];
	fixed_t view_height;
	fixed_t attack_offs;
} ei_player_t;

typedef struct
{
	uint16_t count;
	uint16_t max_count;
	uint16_t hub_count;
	uint16_t flash_type;
	uint16_t respawn_tics;
	uint16_t sound_use;
	uint16_t sound_pickup;
	uint16_t special; // backpack, ammo parent ...
	patch_t *icon;
	uint8_t *message;
} ei_inventory_t;

typedef struct
{
	ei_inventory_t inventory;
	uint16_t selection_order;
	uint16_t kickback;
	uint16_t ammo_use[2];
	uint16_t ammo_give[2];
	uint16_t ammo_type[2];
	uint16_t sound_up;
	uint16_t sound_ready;
} ei_weapon_t;

typedef struct
{
	ei_inventory_t inventory;
	uint16_t count;
	uint16_t max_count;
} ei_ammo_t;

typedef struct
{
	ei_inventory_t inventory;
	uint16_t count;
	uint16_t max_count;
	uint16_t percent;
} ei_armor_t;

typedef struct
{
	ei_inventory_t inventory;
	int32_t duration;
	uint8_t type;
	uint8_t mode;
	uint16_t strength;
} ei_powerup_t;

typedef struct mobjinfo_s
{ // this structure has been changed
	uint16_t doomednum;
	uint16_t spawnid;
	uint32_t state_spawn;
	int32_t spawnhealth;
	uint32_t state_see;
	uint16_t seesound;
	uint16_t __free_18;
	int32_t reactiontime;
	uint16_t attacksound;
	uint16_t __free_24;
	uint32_t state_pain;
	int32_t painchance;
	uint16_t painsound;
	uint16_t __free_34;
	uint32_t state_melee;
	uint32_t state_missile;
	uint32_t state_death;
	uint32_t state_xdeath;
	uint16_t deathsound;
	uint16_t __free_4C;
	int32_t speed;
	int32_t radius;
	int32_t height;
	int32_t mass;
	int32_t damage;
	uint16_t activesound;
	uint16_t __free_68;
	uint32_t flags;
	uint32_t state_raise;
	// new stuff
	uint64_t actor_name;
	uint32_t state_idx_first;
	uint32_t state_idx_limit;
	uint32_t replacement;
	uint32_t flags1;
	uint32_t eflags;
	// extra stuff list
	union
	{
		void *extra_stuff[2];
		struct
		{
			mobj_dropitem_t *start;
			mobj_dropitem_t *end;
		} dropitem;
		struct
		{
			plrp_start_item_t *start;
			plrp_start_item_t *end;
		} start_item;
	};
	// new states
	uint16_t state_heal;
	uint16_t state_crush;
	// shared states
	union
	{
		uint16_t extra_states[4];
		struct
		{
			uint16_t ready;
			uint16_t lower;
			uint16_t raise;
			uint16_t deadlow;
			uint16_t fire;
			uint16_t fire_alt;
			uint16_t hold;
			uint16_t hold_alt;
			uint16_t flash;
			uint16_t flash_alt;
		} st_weapon;
		struct
		{
			uint16_t pickup;
			uint16_t use;
		} st_custinv;
	};
	// type based stuff
	uint32_t extra_type;
	union
	{
		ei_player_t player;
		ei_inventory_t inventory;
		ei_weapon_t weapon;
		ei_ammo_t ammo;
		ei_armor_t armor;
		ei_powerup_t powerup;
	};
} mobjinfo_t;

typedef uint32_t(*stfunc_t)(struct mobj_s*,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

typedef struct state_s
{
	uint16_t sprite;
	uint16_t frame;
	const void *arg;
	int32_t tics;
	union
	{
		void *action;
		void (*acp)(struct mobj_s*,struct state_s*,stfunc_t) __attribute((regparm(2),no_caller_saved_registers));
	};
	uint32_t nextstate;
	int32_t misc1;
	int32_t misc2;
} state_t;

//
// MAP

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t angle;
	int16_t type;
	int16_t options;
} mapthing_t;

typedef struct
{
	fixed_t x, y;
} vertex_t;

typedef struct
{
	thinker_t thinker;
	fixed_t x;
	fixed_t y;
	fixed_t z;
} degenmobj_t;

typedef struct
{
	fixed_t floorheight;
	fixed_t ceilingheight;
	uint16_t floorpic;
	uint16_t ceilingpic;
	uint16_t lightlevel;
	uint16_t special;
	uint16_t tag;
	int32_t sndtraversed;
	struct mobj_s *soundtarget;
	int32_t blockbox[4];
	degenmobj_t soundorg;
	uint32_t validcount;
	struct mobj_s *thinglist;
	void *specialdata;
	uint32_t linecount;
	struct line_s **lines;
} __attribute__((packed)) sector_t;

typedef struct line_s
{
	vertex_t *v1, *v2;
	fixed_t dx, dy;
	uint16_t flags;
	uint16_t special;
	uint16_t tag;
	uint16_t sidenum[2];
	fixed_t bbox[4];
	uint32_t slopetype;
	sector_t *frontsector;
	sector_t *backsector;
	uint32_t validcount;
	void *specialdata;
} __attribute__((packed)) line_t;

typedef struct
{
	fixed_t textureoffset;
	fixed_t rowoffset;
	uint16_t toptexture;
	uint16_t bottomtexture;
	uint16_t midtexture;
	sector_t *sector;
} __attribute__((packed)) side_t;

typedef struct subsector_s
{
	sector_t *sector;
	uint16_t numlines;
	uint16_t firstline;
} subsector_t;

//
// sound

typedef struct sfxinfo_s
{ // this structure has been changed
	uint64_t alias;
	uint32_t priority;
	uint8_t reserved; // can be used for $limit
	uint8_t rng_count;
	uint16_t rng_id[5];
	void *data;
	int32_t usefulness;
	int32_t lumpnum;
} sfxinfo_t;

//
// data

typedef struct texpatch_s
{
	int32_t originx;
	int32_t originy;
	uint32_t patch;
} __attribute__((packed)) texpatch_t;

typedef struct texture_s
{
	union
	{
		uint8_t name[8];
		uint32_t same[2];
		uint64_t wame;
	};
	uint16_t width;
	uint16_t height;
	uint16_t patchcount;
	texpatch_t patch[];
} __attribute__((packed)) texture_t;

typedef struct vissprite_s
{
	struct vissprite_s *next;
	struct vissprite_s *prev;
	int32_t x, y;
	fixed_t gx, gy;
	fixed_t gz, gzt;
	fixed_t startfrac;
	fixed_t scale;
	fixed_t xiscale;
	fixed_t texturemid;
	int32_t patch;
	void *colormap;
	uint32_t mobjflags;
} vissprite_t;

typedef struct spriteframe_s
{
	uint32_t rotate;
	uint16_t lump[8];
	uint8_t flip[8];
} spriteframe_t;

typedef struct spritedef_s
{
	uint32_t numframes;
	spriteframe_t *spriteframes;
} spritedef_t;

//
// mobj

typedef struct mobj_s
{ // this structure has been changed
	thinker_t thinker;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	struct mobj_s *snext;
	struct mobj_s *sprev;
	angle_t angle;
	uint16_t sprite;
	uint16_t frame;
	uint32_t flags1;	// new flags
	struct mobj_s *bnext;
	struct mobj_s *bprev;
	struct subsector_s *subsector;
	fixed_t floorz;
	fixed_t ceilingz;
	fixed_t radius;
	fixed_t height;
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	uint32_t validcount;
	uint32_t type;
	mobjinfo_t *info;
	int tics;
	state_t *state;
	uint32_t flags;
	int health;
	int movedir;
	int movecount;
	struct mobj_s *target;
	int reactiontime;
	int threshold;
	struct player_s *player;
	int lastlook;
	mapthing_t spawnpoint;
	struct mobj_s *tracer;
	uint8_t animation;	// animation system
	uint8_t __unused;
	uint32_t netid;	// unique identification
	struct inventory_s *inventory;
	mobjinfo_t *custom_inventory; // activating item, nesting is unsupported
	uint32_t custom_state; // custom inventory state jumps
} __attribute__((packed)) mobj_t;

// ASM hooks
void hook_mobj_damage();
void hook_obj_key();

// some variables
extern uint32_t *gamemode;
extern fixed_t *finesine;
extern fixed_t *finecosine;
extern uint32_t *viewheight;
extern uint32_t *viewwidth;

// math
fixed_t FixedDiv(fixed_t, fixed_t) __attribute((regparm(2),no_caller_saved_registers));
#define FixedMul(a,b)	(((int64_t)(a) * (int64_t)(b)) >> FRACBITS)

// main.c
extern uint8_t *ldr_alloc_message;
void gfx_progress(int32_t step);
void *ldr_malloc(uint32_t size);
void *ldr_realloc(void *ptr, uint32_t size);

// i_video
void I_InitGraphics() __attribute((regparm(2),no_caller_saved_registers));
void I_FinishUpdate() __attribute((regparm(2),no_caller_saved_registers));

// m_argv
uint32_t M_CheckParm(uint8_t*) __attribute((regparm(2),no_caller_saved_registers));

// m_random
int32_t P_Random() __attribute((regparm(2),no_caller_saved_registers));

// st_stuff
void ST_Init() __attribute((regparm(2),no_caller_saved_registers));
void ST_Start() __attribute((regparm(2),no_caller_saved_registers));

// hu_stuff
void HU_Start() __attribute((regparm(2),no_caller_saved_registers));

// m_menu
void M_WriteText(uint32_t,uint32_t,uint8_t*) __attribute((regparm(2),no_caller_saved_registers));
void M_ClearMenus() __attribute((regparm(2),no_caller_saved_registers));

// p_door
uint32_t EV_DoDoor(line_t*,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// p_enemy
void doom_A_Look(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));
void doom_A_Chase(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_NoiseAlert(mobj_t*,mobj_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_map
uint32_t P_TryMove(mobj_t*,fixed_t,fixed_t) __attribute((regparm(2),no_caller_saved_registers));
void P_UseLines(player_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_LineAttack(mobj_t*,angle_t,fixed_t,fixed_t,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// p_maputl
void P_SetThingPosition(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_UnsetThingPosition(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_mobj
void P_RemoveMobj(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));
mobj_t *P_SpawnMobj(fixed_t,fixed_t,fixed_t,uint32_t) __attribute((regparm(2),no_caller_saved_registers));
void P_SpawnPlayerMissile(mobj_t*,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// p_pspr
void P_BulletSlope(mobj_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_inter
void P_DamageMobj(mobj_t*,mobj_t*,mobj_t*,int32_t) __attribute((regparm(2),no_caller_saved_registers));
void P_TouchSpecialThing(mobj_t*,mobj_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_KillMobj(mobj_t*,mobj_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_setup
void P_SetupLevel() __attribute((regparm(2),no_caller_saved_registers));

// p_spec
void P_PlayerInSpecialSector(player_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_tick
void P_AddThinker(thinker_t*) __attribute((regparm(2),no_caller_saved_registers));

// p_user
void P_CalcHeight(player_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_MovePlayer(player_t*) __attribute((regparm(2),no_caller_saved_registers));
void P_DeathThink(player_t*) __attribute((regparm(2),no_caller_saved_registers));

// r_data
void R_GenerateLookup(uint32_t) __attribute((regparm(2),no_caller_saved_registers));
void *R_GetColumn(uint32_t,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// r_main
void R_RenderPlayerView(player_t*) __attribute((regparm(2),no_caller_saved_registers));
angle_t R_PointToAngle2(fixed_t,fixed_t,fixed_t,fixed_t) __attribute((regparm(2),no_caller_saved_registers));

// r_things
void R_DrawMaskedColumn(struct column_s*) __attribute((regparm(2),no_caller_saved_registers));
void R_InstallSpriteLump(uint32_t,uint32_t,uint32_t,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// s_sound.c
void S_StartSound(mobj_t*,uint32_t) __attribute((regparm(2),no_caller_saved_registers));

// v_video
void V_DrawPatchDirect(int32_t, int32_t, uint32_t, patch_t*) __attribute((regparm(2),no_caller_saved_registers));

// w_wad
int32_t W_CheckNumForName(uint8_t *name) __attribute((regparm(2),no_caller_saved_registers));
void *W_CacheLumpName(uint8_t *name, uint32_t tag) __attribute((regparm(2),no_caller_saved_registers));
void *W_CacheLumpNum(int32_t lump, uint32_t tag) __attribute((regparm(2),no_caller_saved_registers));
uint32_t W_LumpLength(int32_t lump) __attribute((regparm(2),no_caller_saved_registers));
void W_ReadLump(int32_t lump, void *dst) __attribute((regparm(2),no_caller_saved_registers));

// z_zone
void *Z_Malloc(uint32_t size, uint32_t tag, void *owner) __attribute((regparm(2),no_caller_saved_registers));
void Z_Free(void *ptr) __attribute((regparm(2),no_caller_saved_registers));

// extra inline

static inline fixed_t P_AproxDistance(fixed_t dx, fixed_t dy)
{
	dx = abs(dx);
	dy = abs(dy);
	if(dx < dy)
		return dx + dy - (dx >> 1);
	return dx + dy - (dy >> 1);
}

