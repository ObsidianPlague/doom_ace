// kgsws' ACE Engine
////
#include "sdk.h"
#include "engine.h"
#include "utils.h"
#include "render.h"
#include "wadfile.h"
#include "draw.h"
#include "font.h"

typedef struct
{
	uint32_t magic;
	uint8_t version;
	uint8_t line_height;
	int8_t size_over;
	int8_t size_under;
	int8_t space;
	int8_t size_inner;
	union
	{
		struct
		{
			uint8_t colors_used;
			uint8_t color_max;
		};
		uint16_t num_chars; // overwritten
	};
	uint32_t reserved; // overwritten with data offset
	uint8_t pal_count;
	//
	uint8_t pal_src[];
} __attribute__((packed)) bmf_head_t;

typedef struct
{
	uint8_t code;
	uint8_t width, height;
	int8_t ox, oy;
	int8_t space;
} __attribute__((packed)) bmf_char_t;

// original ZDoom color ranges
// values are sourced from https://c.eev.ee/doom-text-generator
static const uint8_t colortable[] =
{
	//// brick
	255,
	0x47, 0x00, 0x00, 0xFF, 0xB8, 0xB8,
	//// tan
	255,
	0x33, 0x2B, 0x13, 0xFF, 0xEB, 0xDF,
	//// gray
	255,
	0x27, 0x27, 0x27, 0xEF, 0xEF, 0xEF,
	//// green
	255,
	0x0B, 0x17, 0x07, 0x77, 0xFF, 0x6F,
	//// brown
	255,
	0x53, 0x3F, 0x2F, 0xBF, 0xA7, 0x8F,
	//// gold
	255,
	0x73, 0x2B, 0x00, 0xFF, 0xFF, 0x73,
	//// red
	255,
	0x3F, 0x00, 0x00, 0xFF, 0x00, 0x00,
	//// blue
	255,
	0x00, 0x00, 0x27, 0x00, 0x00, 0xFF,
	//// orange
	255,
	0x20, 0x00, 0x00, 0xFF, 0x80, 0x00,
	//// white
	255,
	0x24, 0x24, 0x24, 0xFF, 0xFF, 0xFF,
	//// yellow
	64,
	0x27, 0x27, 0x27, 0x51, 0x51, 0x51,
	207,
	0x78, 0x49, 0x18, 0xF3, 0xA7, 0x18,
	255,
	0xF3, 0xA8, 0x2A, 0xFC, 0xD0, 0x43,
	//// untranslated
	// this one is not generated
	//// black
	255,
	0x13, 0x13, 0x13, 0x50, 0x50, 0x50,
	//// lightblue
	255,
	0x00, 0x00, 0x73, 0xB4, 0xB4, 0xFF,
	//// cream
	255,
	0xCF, 0x83, 0x53, 0xFF, 0xD7, 0xBB,
	//// olive
	255,
	0x2F, 0x37, 0x1F, 0x7B, 0x7F, 0x50,
	//// darkgreen
	255,
	0x0B, 0x17, 0x07, 0x43, 0x93, 037,
	//// darkred
	255,
	0x2B, 0x00, 0x00, 0xAF, 0x2B, 0x2B,
	//// darkbrown
	255,
	0x1F, 0x17, 0x0B, 0xA3, 0x6B, 0x3F,
	//// purple
	255,
	0x23, 0x00, 0x23, 0xCF, 0x00, 0xCF,
	//// darkgray
	255,
	0x23, 0x23, 0x23, 0x8B, 0x8B, 0x8B,
	//// cyan
	255,
	0x00, 0x1F, 0x1F, 0x00, 0xF0, 0xF0,
	//// ice
	94,
	0x34, 0x34, 0x50, 0x7C, 0x7C, 0x98,
	255,
	0x7C, 0x7C, 0x98, 0xE0, 0xE0, 0xE0,
	//// fire
	104,
	0x66, 0x00, 0x00, 0xD5, 0x76, 0x04,
	255,
	0xD5, 0x76, 0x04, 0xFF, 0xFF, 0x00,
	//// sapphire
	94,
	0x00, 0x04, 0x68, 0x50, 0x6C, 0xFC,
	255,
	0x50, 0x6C, 0xFC, 0x50, 0xEC, 0xFC,
	//// teal
	90,
	0x00, 0x1F, 0x1F, 0x23, 0x67, 0x73,
	255,
	0x23, 0x67, 0x73, 0x7B, 0xB3, 0xC3,
};

static uint8_t *font_palidx;
static uint8_t *font_color;

static void *smallfont;

static patch_t fake_stcfn;

static bmf_char_t *finale_bc;

static const hook_t patch_smallfont[];

//
// range generator

uint32_t scale_range(uint32_t value)
{
	// first color is transparent
	// other colors are range from darkest to brightest
	value *= (FONT_COLOR_COUNT-2);
	value /= 255;
	return 1 + value;
}

void generate_range(uint32_t first, uint32_t last, const uint8_t *src, uint8_t *dst)
{
	uint32_t count;
	fixed_t vr, vg, vb;
	fixed_t sr, sg, sb;

	first = scale_range(first);
	last = scale_range(last);

	count = last - first + 1;

	vr = (fixed_t)*src++ << 22;
	vg = (fixed_t)*src++ << 22;
	vb = (fixed_t)*src++ << 22;

	sr = (fixed_t)*src++ << 22;
	sg = (fixed_t)*src++ << 22;
	sb = (fixed_t)*src++ << 22;

	sr -= vr;
	sr /= count;

	sg -= vg;
	sg /= count;

	sb -= vb;
	sb /= count;

	for(uint32_t i = 0; i < count; i++)
	{
		dst[first + i] = r_find_color(vr >> 22, vg >> 22, vb >> 22);
		vr += sr;
		vg += sg;
		vb += sb;
	}
}

//
// stuff

static bmf_char_t *find_char(void *ptr, uint32_t count, uint8_t code)
{
	bmf_char_t *backup = NULL;
	uint8_t bkup = code;

	if(code >= 'A' && code <= 'Z')
		bkup |= 0x20;
	else
	if(code >= 'a' && code <= 'z')
		bkup &= ~0x20;

	for(uint32_t i = 0; i < count; i++)
	{
		bmf_char_t *bc = (void*)ptr;
		if(bc->code == code)
			return bc;
		if(bc->code == bkup)
			backup = bc;
		ptr += sizeof(bmf_char_t) + bc->width * bc->height;
	}

	return backup;
}

void change_color(uint8_t color)
{
	if(color >= 'a' && color < 'l')
		font_color = &render_tables->fmap[FONT_COLOR_COUNT * (color - 'a')];
	else
	if(color > 'l' && color <= 'z')
		font_color = &render_tables->fmap[FONT_COLOR_COUNT * (color - 'b')];
	else
		font_color = NULL;
}

//
// drawing

static void font_draw_char(int32_t x, int32_t y, bmf_char_t *bc)
{
	uint8_t *src = (void*)(bc + 1);
	uint8_t *dst = screen_buffer;
	int32_t yy, xx, ss;

	x += bc->ox;
	y += bc->oy;

	if(x >= SCREENWIDTH)
		return;

	if(y >= SCREENHEIGHT)
		return;

	xx = x + bc->width;

	if(xx < 0)
		return;

	yy = y + bc->height;

	if(yy < 0)
		return;

	if(x < 0)
	{
		src -= x;
		x = 0;
	}
	if(xx > SCREENWIDTH)
		xx = SCREENWIDTH;

	if(y < 0)
	{
		src -= y * (int32_t)bc->width;
		y = 0;
	}
	if(yy > SCREENHEIGHT)
		yy = SCREENHEIGHT;

	dst += x;
	dst += y * SCREENWIDTH;

	xx -= x;
	ss = SCREENWIDTH - xx;

	if(font_color)
	{
		for( ; y < yy; y++)
		{
			for(int32_t i = 0; i < xx; i++)
			{
				if(src[i])
					*dst = font_color[font_palidx[(uint32_t)src[i] * 2 + 1]];
				dst++;
			}
			src += bc->width;
			dst += ss;
		}
	} else
	{
		for( ; y < yy; y++)
		{
			for(int32_t i = 0; i < xx; i++)
			{
				if(src[i])
					*dst = font_palidx[(uint32_t)src[i] * 2];
				dst++;
			}
			src += bc->width;
			dst += ss;
		}
	}
}

void font_center_text(int32_t y, const uint8_t *text, void *font, uint32_t linecount)
{
	int32_t x = SCREENWIDTH / 2;
	bmf_head_t *head = font;
	bmf_char_t *bc;

	linecount *= head->line_height;
	y -= linecount / 2;

	font_palidx = head->pal_src - 2; // compensate missing transparent color

	while(1)
	{
		const uint8_t *ptr;
		int32_t xx;

		// get width
		xx = 0;
		ptr = text;
		while(*ptr && *ptr != '\n')
		{
			if(*ptr == 0x1C)
			{
				ptr++;
				if(!*ptr || *ptr == '\n')
					break;
			} else
			{
				bc = find_char(font + head->reserved, head->num_chars, *ptr);
				if(bc)
					xx += head->space + bc->space;
			}

			ptr++;
		}

		// draw centered
		xx = x - xx / 2;
		ptr = text;
		while(*ptr && *ptr != '\n')
		{
			if(*ptr == 0x1C)
			{
				uint8_t color = *++ptr;
				if(!color || color == '\n')
					break;
				change_color(color);
			} else
			{
				bc = find_char(font + head->reserved, head->num_chars, *ptr);
				if(bc)
				{
					font_draw_char(xx, y, bc);
					xx += head->space + bc->space;
				}
			}

			ptr++;
		}

		// end?
		if(!*ptr)
			break;

		// next line
		font_color = NULL;
		text = ptr + 1;
		y += head->line_height;
	}
}

uint32_t font_draw_text(int32_t x, int32_t y, const uint8_t *text, void *font)
{
	int32_t xx = x;
	bmf_head_t *head = font;
	bmf_char_t *bc;

	font_palidx = head->pal_src - 2; // compensate missing transparent color

	while(*text)
	{
		if(*text == '\n')
		{
			x = xx;
			y += head->line_height;
			text++;
			continue;
		}

		if(*text == 0x1C)
		{
			uint8_t color = *++text;
			if(!color)
				break;
			change_color(color);
			continue;
		}

		bc = find_char(font + head->reserved, head->num_chars, *text);
		if(bc)
		{
			font_draw_char(x, y, bc);
			x += head->space + bc->space;
		}

		text++;
	}

	return x;
}

//
// API

void font_generate()
{
	const uint8_t *ptr = colortable;
	uint8_t *dst = render_tables->fmap;

	for(uint32_t i = 0; i < FONT_TRANSLATION_COUNT; i++)
	{
		uint8_t first = 0;

		while(1)
		{
			uint8_t last = (uint32_t)*ptr++;

			generate_range(first, last, ptr, dst);
			ptr += 6;

			if(last >= 255)
				break;

			first = last + 1;
		}

		dst += FONT_COLOR_COUNT;
	}
}

void *font_load(int32_t lump)
{
	// WARNING: There are no bound checks!
	void *data;
	uint8_t *ptr, *dst;
	bmf_head_t *head;

	if(lumpcache[lump])
		// font is already processed
		return lumpcache[lump];

	data = W_CacheLumpNum(lump, PU_CACHE);
	ptr = data;
	head = data;

	if(head->magic != FONT_MAGIC_ID || head->version != FONT_VERSION)
		engine_error("FONT", "Invalid font %.8s.", lumpinfo[lump].name);

	ptr += sizeof(bmf_head_t);
	dst = ptr;

	for(uint32_t i = 0; i < head->pal_count; i++)
	{
		uint32_t top;

		top = ptr[0];
		if(top < ptr[1])
			top = ptr[1];
		if(top < ptr[2])
			top = ptr[2];

		dst[0] = r_find_color(ptr[0] << 2, ptr[1] << 2, ptr[2] << 2);
		dst[1] = 1 + (top * 30) / 63;

		dst += 2;
		ptr += 3;
	}

	ptr += (uint32_t)*ptr + 1; // font description

	head->num_chars = *((uint16_t*)ptr);
	ptr += sizeof(uint16_t);

	head->reserved = (void*)ptr - data;

	return data;
}

//
// replacement

uint32_t font_message_to_print()
{
	uint32_t linecount;
	uint8_t *text;

	if(!messageToPrint)
		return 0;

	if(!smallfont)
		return 1;

	text = messageString;
	linecount = 1;
	while(*text)
	{
		if(*text == '\n')
			linecount++;
		text++;
	}

	font_center_text(SCREENHEIGHT / 2, messageString, smallfont, linecount);
	skip_menu_draw();
}

static __attribute((regparm(2),no_caller_saved_registers))
void hu_draw_text_line(hu_textline_t *l, uint32_t cursor)
{
	int32_t x;
	bmf_char_t *bc;
	bmf_head_t *head;

	x = font_draw_text(l->x, l->y, l->l, smallfont);

	if(!cursor)
		return;

	head = smallfont;
	bc = find_char(smallfont + head->reserved, head->num_chars, '_');
	if(bc)
		font_draw_char(x, l->y, bc);
}

static __attribute((regparm(3),no_caller_saved_registers)) // three!
void font_menu_text(int32_t x, int32_t y, const uint8_t *text)
{
	if(draw_patch_color)
		font_color = &render_tables->fmap[FONT_COLOR_COUNT * 9];
	font_draw_text(x, y, text, smallfont);
	font_color = NULL;
	return;
}

static __attribute((regparm(2),no_caller_saved_registers))
void hud_font_init()
{
	uint8_t buffer[32];
	int32_t lump;
	bmf_head_t *head;

	lump = wad_check_lump("SMALLFNT");
	if(lump < 0)
	{
		// load the original font
		for(uint32_t i = 0; i < HU_FONTSIZE; i++)
		{
			doom_sprintf(buffer, dtxt_stcfn, i + HU_FONTSTART);
			hu_font[i] = (patch_t*)W_CacheLumpName(buffer, PU_STATIC);
		}
		return;
	}

	// load font as STATIC
	smallfont = font_load(lump);
	Z_ChangeTag2(smallfont, PU_STATIC);

	// fake hu_font[0] as it is used for size calulations
	head = smallfont;
	fake_stcfn.width = head->space;
	fake_stcfn.height = head->line_height;
	hu_font[0] = &fake_stcfn;

	// patch some extra stuff
	utils_install_hooks(patch_smallfont, 0);
}

static __attribute((regparm(2),no_caller_saved_registers))
void finale_text()
{
	uint8_t *ptr;
	uint8_t bkup;
	int32_t count;
	int32_t idx = 0;

	count = (finalecount - 10) / 3;
	if(count <= 0)
		return;

	ptr = finaletext;
	while(*ptr)
	{
		if(*ptr == 0x1C)
		{
			ptr++;
			if(!*ptr)
				break;
			ptr++;
		} else
		if(*ptr != '\n')
		{
			idx++;
			if(idx >= count)
				break;
		}

		ptr++;
	}

	bkup = *ptr;
	*ptr = 0;
	font_draw_text(10, 10, finaletext, smallfont);
	*ptr = bkup;
}

static __attribute((regparm(2),no_caller_saved_registers))
void cast_text(uint8_t *text)
{
	font_center_text(180, text, smallfont, 0);
}

//
// hooks

static const hook_t patch_smallfont[] =
{
	// replace 'HUlib_drawTextLine'
	{0x0003BDA0, CODE_HOOK | HOOK_JMP_ACE, (uint32_t)hu_draw_text_line},
	// replace 'M_WriteText'
	{0x00022FA1, CODE_HOOK | HOOK_UINT16, 0xD989},
	{0x00022FA3, CODE_HOOK | HOOK_CALL_ACE, (uint32_t)font_menu_text},
	{0x00022FA8, CODE_HOOK | HOOK_UINT16, 0xC359},
	// fix 'F_TextWrite'
	{0x0001C645, CODE_HOOK | HOOK_CALL_ACE, (uint32_t)finale_text},
	{0x0001C64A, CODE_HOOK | HOOK_JMP_DOOM, 0x0001C6DA},
	// replace call to 'F_CastPrint'
	{0x0001CD3B, CODE_HOOK | HOOK_CALL_ACE, (uint32_t)cast_text},
	// terminator
	{0}
};

static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	// replace font loading in 'HU_Init'
	{0x0003B4D5, CODE_HOOK | HOOK_CALL_ACE, (uint32_t)hud_font_init},
	{0x0003B4DA, CODE_HOOK | HOOK_JMP_DOOM, 0x0003B50D},
};
