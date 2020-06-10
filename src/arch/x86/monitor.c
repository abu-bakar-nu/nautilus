/* 
 * This file is part of the Nautilus AeroKernel developed
 * by the Hobbes and V3VEE Projects with funding from the 
 * United States National  Science Foundation and the Department of Energy.  
 *
 * The V3VEE Project is a joint project between Northwestern University
 * and the University of New Mexico.  The Hobbes Project is a collaboration
 * led by Sandia National Laboratories that includes several national 
 * laboratories and universities. You can find out more at:
 * http://www.v3vee.org  and
 * http://xstack.sandia.gov/hobbes
 *
 * Copyright (c) 2020, Drew Kersnar <drewkersnar2021@u.northwestern.edu>
 * Copyright (c) 2020, The Interweaving Project <http://interweaving.org>
 *                     The V3VEE Project  <http://www.v3vee.org> 
 *                     The Hobbes Project <http://xstack.sandia.gov/hobbes>
 * All rights reserved.
 *
 * Authors: Drew Kersnar <drewkersnar2021@u.northwestern.edu>
 *          
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "LICENSE.txt".
 */


#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int 
#define uint64_t unsigned long long

#define NULL ((void*)0)

#define TARGET_PORT 0xe9

#define ITERATIONS 100
#define NUM_READINGS 100.0

static inline uint64_t __attribute__((always_inline))
rdtsc (void)
{
    uint32_t lo, hi;
    asm volatile("CPUID"::: "eax","ebx","ecx","edx", "memory");
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return lo | ((uint64_t)(hi) << 32);
}


#define DB(x) vga_putchar(x); outb(x, TARGET_PORT)
#define DHN(x) vga_putchar(((x & 0xF) >= 10) ? (((x & 0xF) - 10) + 'a') : ((x & 0xF) + '0')); outb(((x & 0xF) >= 10) ? (((x & 0xF) - 10) + 'a') : ((x & 0xF) + '0'), TARGET_PORT)
#define DHB(x) DHN(x >> 4) ; DHN(x);
#define DHW(x) DHB(x >> 8) ; DHB(x);
#define DHL(x) DHW(x >> 16) ; DHW(x);
#define DHQ(x) DHL(x >> 32) ; DHL(x);

static char * long_to_string(long x)
{
  static char buf[20];
  for(int i=0; i<19; i++){
    buf[i] = '0';
  }
  for(int i=18; x>0; i--)
  {
    buf[i] = (char) ((x%10) + 48);
    x = x/10;
  }
  buf[19] = 0;
  return buf;
}

static inline void 
tlb_flush(void)
{
	__asm__ __volatile__( "movl %%cr3,%%eax;\
	movl  %%eax,%%cr3;" ::: "%eax");
}

static void outb (unsigned char val, unsigned short port)
{
    asm volatile ("outb %0, %1"::"a" (val), "dN" (port));
}

static uint8_t inb (uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0":"=a" (ret):"dN" (port));
    return ret;
}

static void print(char *b)
{
    while (b && *b) {
        outb(*b,TARGET_PORT);
        b++;
    }
}

static void my_memset(void *b,uint8_t val, uint32_t count){
	uint32_t i;
	for(i=0;i<count;i++){
	  ((uint8_t*)b)[i]=val;	
	}
}

static void my_memcpy(void *b,void *s, uint32_t count){
        uint32_t i;
        for(i=0;i<count;i++){
	  ((uint8_t*)b)[i]=((uint8_t*)s)[i];
        }
}

static int my_strlen(char *b)
{
  int count=0;

  while (*b++) { count++; }
  return count;
}

int my_strcmp (const char * s1, const char * s2) 
{
    while (1) {
    int cmp = (*s1 - *s2);

    if ((cmp != 0) || (*s1 == '\0') || (*s2 == '\0')) {
        return cmp;
    }

    ++s1;
    ++s2;
    }
}

//#include <nautilus/nautilus.h>
//#include <nautilus/shell.h>
//#include <nautilus/monitor.h>
//#include <nautilus/dr.h>
//#include <nautilus/smp.h>
//#include <dev/apic.h>

#define COLOR_FOREGROUND        COLOR_LIGHT_CYAN
#define COLOR_BACKGROUND        COLOR_BLACK
#define COLOR_PROMPT_FOREGROUND COLOR_LIGHT_BROWN
#define COLOR_PROMPT_BACKGROUND COLOR_BLACK


// VGA code repeats here to be self-contained
// future version will also support serial port

#define VGA_BASE_ADDR 0xb8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define CRTC_ADDR 0x3d4
#define CRTC_DATA 0x3d5
#define CURSOR_LOW 0xf
#define CURSOR_HIGH 0xe

static uint8_t vga_x, vga_y;
static uint8_t vga_attr;

enum vga_color
{
  COLOR_BLACK = 0,
  COLOR_BLUE = 1,
  COLOR_GREEN = 2,
  COLOR_CYAN = 3,
  COLOR_RED = 4,
  COLOR_MAGENTA = 5,
  COLOR_BROWN = 6,
  COLOR_LIGHT_GREY = 7,
  COLOR_DARK_GREY = 8,
  COLOR_LIGHT_BLUE = 9,
  COLOR_LIGHT_GREEN = 10,
  COLOR_LIGHT_CYAN = 11,
  COLOR_LIGHT_RED = 12,
  COLOR_LIGHT_MAGENTA = 13,
  COLOR_LIGHT_BROWN = 14,
  COLOR_WHITE = 15,
};

// Encodes the text and color into a short
static uint16_t
vga_make_entry(char c, uint8_t color)
{
  uint16_t c16 = c;
  uint16_t color16 = color;
  return c16 | color16 << 8;
}

// Encodes the color in a char
static uint8_t
vga_make_color(enum vga_color fg, enum vga_color bg)
{
  static int i = 0;
  return fg | bg << 4;
}

// Writes the encoded text and color to position x, y on the screen
static inline void vga_write_screen(uint8_t x, uint8_t y, uint16_t val)
{
  uint16_t *addr = ((uint16_t *)VGA_BASE_ADDR) + y * VGA_WIDTH + x;
  __asm__ __volatile__(" movw %0, (%1) "
                       :
                       : "r"(val), "r"(addr)
                       :);
}

// clears the screen and fills it with val
static inline void vga_clear_screen(uint16_t val)
{
  my_memset((void *)VGA_BASE_ADDR, val, 2*VGA_WIDTH * VGA_HEIGHT);
}

static inline void vga_set_cursor(uint8_t x, uint8_t y)
{
  uint16_t pos = y * VGA_WIDTH + x;

  vga_x = x;
  vga_y = y;

  outb(CURSOR_HIGH, CRTC_ADDR);
  outb(pos >> 8, CRTC_DATA);
  outb(CURSOR_LOW, CRTC_ADDR);
  outb(pos & 0xff, CRTC_DATA);
}

static inline void vga_get_cursor(uint8_t *x, uint8_t *y)
{
  *x = vga_x;
  *y = vga_y;
}

static void vga_scrollup(void)
{
  uint16_t *buf = (uint16_t *)VGA_BASE_ADDR;

  my_memcpy(buf, buf + VGA_WIDTH, 2 * VGA_WIDTH * (VGA_HEIGHT - 1));

  my_memset(buf + VGA_WIDTH * (VGA_HEIGHT - 1), vga_make_entry(' ', vga_attr), 2 * VGA_WIDTH);
}

static void vga_putchar(char c)
{
  if (c == '\n')
  {
    vga_x = 0;

    if (++vga_y == VGA_HEIGHT)
    {
      vga_scrollup();
      vga_y--;
    }
  }
  else
  {

    vga_write_screen(vga_x, vga_y, vga_make_entry(c, vga_attr));

    if (++vga_x == VGA_WIDTH)
    {
      vga_x = 0;
      if (++vga_y == VGA_HEIGHT)
      {
        vga_scrollup();
        vga_y--;
      }
    }
  }
  vga_set_cursor(vga_x, vga_y);
}

static void vga_print(char *buf)
{
  while (*buf)
  {
    vga_putchar(*buf);
    buf++;
  }
}

static void vga_puts(char *buf)
{
  vga_print(buf);
  vga_putchar('\n');
}

static inline void vga_copy_out(void *dest, uint32_t n)
{
  my_memcpy((void *)dest,(void*)VGA_BASE_ADDR,n);
}

static inline void vga_copy_in(void *src, uint32_t n)
{
  my_memcpy((void*)VGA_BASE_ADDR, src, n);
}


// Private output formatting routines since we
// do not want to reply on printf being functional
#define DS(x) { char *__curr = x; while(*__curr) { DB(*__curr); __curr++; } }

static char screen_saved[VGA_HEIGHT*VGA_WIDTH*2];
static uint8_t cursor_saved_x;
static uint8_t cursor_saved_y;

// Keyboard stuff repeats here to be self-contained

// Keys and scancodes are represented internally as 16 bits
// so we can indicate nonexistence (via -1)
//
// A "keycode" consists of the translated key (lower 8 bits)
// combined with an upper byte that reflects status of the 
// different modifier keys
typedef uint16_t nk_keycode_t;
typedef uint16_t nk_scancode_t;

#define KBD_DATA_REG 0x60
#define KBD_ACK_REG  0x61
#define KBD_CMD_REG  0x64
#define KBD_STATUS_REG  0x64

// Special tags to indicate unavailabilty
#define NO_KEY      ((nk_keycode_t)(0xffff))
#define NO_SCANCODE ((nk_scancode_t)(0xffff))

// Special flags for a keycode that reflect status of 
// modifiers
#define KEY_SPECIAL_FLAG 0x0100
#define KEY_KEYPAD_FLAG  0x0200
#define KEY_SHIFT_FLAG   0x1000
#define KEY_ALT_FLAG     0x2000
#define KEY_CTRL_FLAG    0x4000
#define KEY_CAPS_FLAG    0x8000

// Special ascii characters
#define ASCII_ESC 0x1B
#define ASCII_BS  0x08

// PC-specific keys
#define _SPECIAL(num) (KEY_SPECIAL_FLAG | (num))
#define KEY_UNKNOWN _SPECIAL(0)
#define KEY_F1      _SPECIAL(1)
#define KEY_F2      _SPECIAL(2)
#define KEY_F3      _SPECIAL(3)
#define KEY_F4      _SPECIAL(4)
#define KEY_F5      _SPECIAL(5)
#define KEY_F6      _SPECIAL(6)
#define KEY_F7      _SPECIAL(7)
#define KEY_F8      _SPECIAL(8)
#define KEY_F9      _SPECIAL(9)
#define KEY_F10     _SPECIAL(10)
#define KEY_F11     _SPECIAL(11)
#define KEY_F12     _SPECIAL(12)
#define KEY_LCTRL   _SPECIAL(13)
#define KEY_RCTRL   _SPECIAL(14)
#define KEY_LSHIFT  _SPECIAL(15)
#define KEY_RSHIFT  _SPECIAL(16)
#define KEY_LALT    _SPECIAL(17)
#define KEY_RALT    _SPECIAL(18)
#define KEY_PRINTSCRN _SPECIAL(19)
#define KEY_CAPSLOCK _SPECIAL(20)
#define KEY_NUMLOCK _SPECIAL(21)
#define KEY_SCRLOCK _SPECIAL(22)
#define KEY_SYSREQ  _SPECIAL(23)

// more pc-specific keys
#define KEYPAD_START 128
#define _KEYPAD(num) (KEY_KEYPAD_FLAG | KEY_SPECIAL_FLAG | (num+KEYPAD_START))
#define KEY_KPHOME  _KEYPAD(0)
#define KEY_KPUP    _KEYPAD(1)
#define KEY_KPPGUP  _KEYPAD(2)
#define KEY_KPMINUS _KEYPAD(3)
#define KEY_KPLEFT  _KEYPAD(4)
#define KEY_KPCENTER _KEYPAD(5)
#define KEY_KPRIGHT _KEYPAD(6)
#define KEY_KPPLUS  _KEYPAD(7)
#define KEY_KPEND   _KEYPAD(8)
#define KEY_KPDOWN  _KEYPAD(9)
#define KEY_KPPGDN  _KEYPAD(10)
#define KEY_KPINSERT _KEYPAD(11)
#define KEY_KPDEL   _KEYPAD(12)


static nk_keycode_t kbd_flags = 0;

// we pull much of the PS2 setup from ps2.h indirectly
// We need to replicate the translation tables

static const nk_keycode_t NoShiftNoCaps[] = {
    KEY_UNKNOWN, ASCII_ESC, '1', '2',                  /* 0x00 - 0x03 */
    '3', '4', '5', '6',                                /* 0x04 - 0x07 */
    '7', '8', '9', '0',                                /* 0x08 - 0x0B */
    '-', '=', ASCII_BS, '\t',                          /* 0x0C - 0x0F */
    'q', 'w', 'e', 'r',                                /* 0x10 - 0x13 */
    't', 'y', 'u', 'i',                                /* 0x14 - 0x17 */
    'o', 'p', '[', ']',                                /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'a', 's',                         /* 0x1C - 0x1F */
    'd', 'f', 'g', 'h',                                /* 0x20 - 0x23 */
    'j', 'k', 'l', ';',                                /* 0x24 - 0x27 */
    '\'', '`', KEY_LSHIFT, '\\',                       /* 0x28 - 0x2B */
    'z', 'x', 'c', 'v',                                /* 0x2C - 0x2F */
    'b', 'n', 'm', ',',                                /* 0x30 - 0x33 */
    '.', '/', KEY_RSHIFT, KEY_PRINTSCRN,               /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1,               /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,                    /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,                    /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,     /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,     /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,   /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, /* 0x54 - 0x57 */
};

static const nk_keycode_t ShiftNoCaps[] = {
    KEY_UNKNOWN, ASCII_ESC, '!', '@',                  /* 0x00 - 0x03 */
    '#', '$', '%', '^',                                /* 0x04 - 0x07 */
    '&', '*', '(', ')',                                /* 0x08 - 0x0B */
    '_', '+', ASCII_BS, '\t',                          /* 0x0C - 0x0F */
    'Q', 'W', 'E', 'R',                                /* 0x10 - 0x13 */
    'T', 'Y', 'U', 'I',                                /* 0x14 - 0x17 */
    'O', 'P', '{', '}',                                /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'A', 'S',                         /* 0x1C - 0x1F */
    'D', 'F', 'G', 'H',                                /* 0x20 - 0x23 */
    'J', 'K', 'L', ':',                                /* 0x24 - 0x27 */
    '"', '~', KEY_LSHIFT, '|',                         /* 0x28 - 0x2B */
    'Z', 'X', 'C', 'V',                                /* 0x2C - 0x2F */
    'B', 'N', 'M', '<',                                /* 0x30 - 0x33 */
    '>', '?', KEY_RSHIFT, KEY_PRINTSCRN,               /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1,               /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,                    /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,                    /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,     /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,     /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,   /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, /* 0x54 - 0x57 */
};

static const nk_keycode_t NoShiftCaps[] = {
    KEY_UNKNOWN, ASCII_ESC, '1', '2',                  /* 0x00 - 0x03 */
    '3', '4', '5', '6',                                /* 0x04 - 0x07 */
    '7', '8', '9', '0',                                /* 0x08 - 0x0B */
    '-', '=', ASCII_BS, '\t',                          /* 0x0C - 0x0F */
    'Q', 'W', 'E', 'R',                                /* 0x10 - 0x13 */
    'T', 'Y', 'U', 'I',                                /* 0x14 - 0x17 */
    'O', 'P', '[', ']',                                /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'A', 'S',                         /* 0x1C - 0x1F */
    'D', 'F', 'G', 'H',                                /* 0x20 - 0x23 */
    'J', 'K', 'L', ';',                                /* 0x24 - 0x27 */
    '\'', '`', KEY_LSHIFT, '\\',                       /* 0x28 - 0x2B */
    'Z', 'X', 'C', 'V',                                /* 0x2C - 0x2F */
    'B', 'N', 'M', ',',                                /* 0x30 - 0x33 */
    '.', '/', KEY_RSHIFT, KEY_PRINTSCRN,               /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1,               /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,                    /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,                    /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,     /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,     /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,   /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, /* 0x54 - 0x57 */
};

static const nk_keycode_t ShiftCaps[] = {
    KEY_UNKNOWN, ASCII_ESC, '!', '@',                  /* 0x00 - 0x03 */
    '#', '$', '%', '^',                                /* 0x04 - 0x07 */
    '&', '*', '(', ')',                                /* 0x08 - 0x0B */
    '_', '+', ASCII_BS, '\t',                          /* 0x0C - 0x0F */
    'q', 'w', 'e', 'r',                                /* 0x10 - 0x13 */
    't', 'y', 'u', 'i',                                /* 0x14 - 0x17 */
    'o', 'p', '{', '}',                                /* 0x18 - 0x1B */
    '\r', KEY_LCTRL, 'a', 's',                         /* 0x1C - 0x1F */
    'd', 'f', 'g', 'h',                                /* 0x20 - 0x23 */
    'j', 'k', 'l', ':',                                /* 0x24 - 0x27 */
    '"', '~', KEY_LSHIFT, '|',                         /* 0x28 - 0x2B */
    'z', 'x', 'c', 'v',                                /* 0x2C - 0x2F */
    'b', 'n', 'm', '<',                                /* 0x30 - 0x33 */
    '>', '?', KEY_RSHIFT, KEY_PRINTSCRN,               /* 0x34 - 0x37 */
    KEY_LALT, ' ', KEY_CAPSLOCK, KEY_F1,               /* 0x38 - 0x3B */
    KEY_F2, KEY_F3, KEY_F4, KEY_F5,                    /* 0x3C - 0x3F */
    KEY_F6, KEY_F7, KEY_F8, KEY_F9,                    /* 0x40 - 0x43 */
    KEY_F10, KEY_NUMLOCK, KEY_SCRLOCK, KEY_KPHOME,     /* 0x44 - 0x47 */
    KEY_KPUP, KEY_KPPGUP, KEY_KPMINUS, KEY_KPLEFT,     /* 0x48 - 0x4B */
    KEY_KPCENTER, KEY_KPRIGHT, KEY_KPPLUS, KEY_KPEND,  /* 0x4C - 0x4F */
    KEY_KPDOWN, KEY_KPPGDN, KEY_KPINSERT, KEY_KPDEL,   /* 0x50 - 0x53 */
    KEY_SYSREQ, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, /* 0x54 - 0x57 */
};

#define KB_KEY_RELEASE 0x80

#define KBD_DATA_REG 0x60
#define KBD_CMD_REG 0x64
#define KBD_STATUS_REG 0x64

static nk_keycode_t kbd_translate_monitor(nk_scancode_t scan)
{
  int release;
  const nk_keycode_t *table = 0;
  nk_keycode_t cur;
  nk_keycode_t flag;

  // update the flags

  release = scan & KB_KEY_RELEASE;
  scan &= ~KB_KEY_RELEASE;

  if (kbd_flags & KEY_CAPS_FLAG)
  {
    if (kbd_flags & KEY_SHIFT_FLAG)
    {
      table = ShiftCaps;
    }
    else
    {
      table = NoShiftCaps;
    }
  }
  else
  {
    if (kbd_flags & KEY_SHIFT_FLAG)
    {
      table = ShiftNoCaps;
    }
    else
    {
      table = NoShiftNoCaps;
    }
  }

  cur = table[scan];

  flag = 0;
  switch (cur)
  {
  case KEY_LSHIFT:
  case KEY_RSHIFT:
    flag = KEY_SHIFT_FLAG;
    break;
  case KEY_CAPSLOCK:
    flag = KEY_CAPS_FLAG;
    break;
  default:
    goto do_noflags;
    break;
  }

  // do_flags:
  if (flag == KEY_CAPS_FLAG)
  {
    if (!release)
    {
      if ((kbd_flags & KEY_CAPS_FLAG))
      {
        // turn off caps lock on second press
        kbd_flags &= ~KEY_CAPS_FLAG;
        flag = 0;
      }
      else
      {
        kbd_flags |= flag;
      }
    }
    return NO_KEY;
  }

  if (release)
  {
    kbd_flags &= ~(flag);
  }
  else
  {
    kbd_flags |= flag;
  }

  return NO_KEY;

do_noflags:
  if (release)
  { // Chose to display on press, if that is wrong it's an easy change
    return kbd_flags | cur;
  }
  else
  {
    return NO_KEY;
  }
}

typedef union ps2_status {
  uint8_t val;
  struct
  {
    uint8_t obf : 1;  // output buffer full (1=> can read from 0x60)
    uint8_t ibf : 1;  // input buffer full (0=> can write to either port)
    uint8_t sys : 1;  // set on successful reset
    uint8_t a2 : 1;   // last address (0=>0x60, 1=>0x64)
    uint8_t inh : 1;  // keyboard inhibit, active low
    uint8_t mobf : 1; // mouse buffer full
    uint8_t to : 1;   // timeout
    uint8_t perr : 1; // parity error
  };
} __attribute__((packed)) ps2_status_t;

typedef union ps2_cmd {
  uint8_t val;
  struct
  {
    uint8_t kint : 1; // enable keyboard interrupts
    uint8_t mint : 1; // enable mouse interrupts
    uint8_t sys : 1;  // set/clear sysflag / do BAT
    uint8_t rsv1 : 1;
    uint8_t ken : 1;  // keyboard enable, active low
    uint8_t men : 1;  // mouse enable, active low
    uint8_t xlat : 1; // do scancode translation
    uint8_t rsv2 : 1;
  };
} __attribute__((packed)) ps2_cmd_t;

static int ps2_wait_for_key()
{
  volatile ps2_status_t status;
  nk_scancode_t scan;

  do
  {
    status.val = inb(KBD_STATUS_REG);
    // INFO("obf=%d ibf=%d req=%s\n",status.obf, status.ibf, w==INPUT ? "input" : "output");
  } while (!status.obf); // wait for something to be in the output buffer for us to read

  //io_delay();

  scan = inb(KBD_DATA_REG);
  //io_delay();

  return scan;
}


// Private input routines since we cannot rely on
// vc or lower level functionality in the monitor

// writes a line of text into a buffer
static void wait_for_command(char *buf, int buffer_size)
{

  int key;
  int curr = 0;
  while (1)
  {
    key = ps2_wait_for_key();
    uint16_t key_encoded = kbd_translate_monitor(key);
    //vga_clear_screen(vga_make_entry(key, vga_make_color(COLOR_FOREGROUND, COLOR_BACKGROUND)));

    if (key_encoded == '\r')
    {
      // return completed command
      buf[curr] = 0;
      return;
    }

    if (key_encoded != NO_KEY)
    {
      char key_char = (char)key_encoded;

      if (curr < buffer_size - 1)
      {
        vga_putchar(key_char);
        buf[curr] = key_char;
        curr++;
      }
      else
      {
        // they can't type any more
        buf[curr] = 0;
        return;
      }

      //strncat(*buf, &key_char, 1);
      //printk(buf);
    }
  };
}

static int is_hex_addr(char *addr_as_str)
{
  int str_len = my_strlen(addr_as_str);

  // 64 bit addresses can't be larger than 16 hex digits, 18 including 0x
  if(str_len > 18) {
    return 0;
  }
  
  if((addr_as_str[0] != '0') || (addr_as_str[1] != 'x')) {
    return 0;
  }

  for(int i = 2; i < str_len; i++) {
    char curr = addr_as_str[i];
    if((curr >= '0' && curr <= '9')
      || (curr >= 'A' && curr <= 'F')
      || (curr >= 'a' && curr <= 'f'))  {
      continue;
    } else {
      return 0;
    }
  }
  return 1;
}

static uint64_t get_hex_addr(char *addr_as_str)
{
  
  uint64_t addr = 0;
  int power_of_16 = 1;
  // iterate backwards from the end of the address to the beginning, stopping before 0x
  for(int i = (my_strlen(addr_as_str) - 1); i > 1; i--) {
    char curr = addr_as_str[i];
    if(curr >= '0' && curr <= '9')  {
      addr += (curr - '0')*power_of_16;
    } 
    else if(curr >= 'A' && curr <= 'F') {
      addr += (curr - 'A' + 10)*power_of_16;
    }
    else if(curr >= 'a' && curr <= 'f') {
      addr += (curr - 'a' + 10)*power_of_16;
    } else {
      // something broken
      //ASSERT(false);
    }
    
    power_of_16*=16;
  }
  return addr;

}


static int is_dec_addr(char *addr_as_str)
{
  int str_len = my_strlen(addr_as_str);

  // // 64 bit addresses can't be larger than 16 hex digits, 18 including 0x
  // if(str_len > 18) {
  //   return 0;
  // } 
  // TODO: check if dec_addr too big

  for(int i = 0; i < str_len; i++) {
    char curr = addr_as_str[i];
    if((curr >= '0' && curr <= '9'))  {
      continue;
    } else {
      return 0;
    }
  }
  return 1;
}

static uint64_t get_dec_addr(char *addr_as_str)
{
  uint64_t addr = 0;
  int power_of_10 = 1;
  // iterate backwards from the end of the address to the beginning, stopping before 0x
  for(int i = (my_strlen(addr_as_str) - 1); i >= 0; i--) {
    char curr = addr_as_str[i];
    if(curr >= '0' && curr <= '9')  {
      addr += (curr - '0')*power_of_10;
    } else {
      // something broken
      //ASSERT(false);
    }
    
    power_of_10*=10;
  }
  return addr;

}

static int is_dr_num(char *addr_as_str)
{
  int str_len = my_strlen(addr_as_str);

  // TODO: check if dec_addr too big

  for(int i = 0; i < str_len; i++) {
    char curr = addr_as_str[i];
    if((curr >= '0' && curr <= '3'))  {
      continue;
    } else {
      return 0;
    }
  }
  return 1;
}

static uint64_t get_dr_num(char *num_as_str)
{
  
  uint64_t num = 0;
  int power_of_10 = 1;
  // iterate backwards from the end of the address to the beginning, stopping before 0x
  for(int i = (my_strlen(num_as_str) - 1); i >= 0; i--) {
    char curr = num_as_str[i];
    if(curr >= '0' && curr <= '9')  {
      num += (curr - '0')*power_of_10;
    } else {
      // something broken
      //ASSERT(false);
    }
    
    power_of_10*=10;
  }
  return num;

}

// Private tokenizer since we may want to use a non-reentrant version at
// some point.., and also because we don't want to rely on the other
// copy in naut_string, which looks lua-specific
//

static char *
__strtok_r_monitor(char *s, const char *delim, char **last)
{
	char *spanp, *tok;
	int c, sc;

	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * 	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 * 	 	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * 	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * 	 	 * Note that delim must have one NUL; we stop if we see that, too.
	 * 	 	 	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = '\0';
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

static char *
strtok_monitor(char *s, const char *delim)
{
	static char *last;

	return (__strtok_r_monitor(s, delim, &last));
}

static char *
get_next_word(char *s)
{
  const char delim[3] = " \t";
  return strtok_monitor(s, delim);
}

// Executing user commands

static int execute_quit(char command[])
{
  vga_puts("quit executed");
  return 1;
}

static int execute_help(char command[])
{
  vga_puts("commands:");
  vga_puts("  quit");
  vga_puts("  help");
  vga_puts("  paging_on");
  vga_puts("  pf");
  vga_puts("  test");
  vga_puts("  pwrstats");
  return 0;
}

void paging_on();
static int execute_paging(char command[])
{
  paging_on();
  print("paging is on now\n\r");
  return 0;
}

static int execute_pf(char command[])
{
  print("executing test\n\r");
  // __asm__ __volatile__("movl %cr3,%eax;\n"
  // 		       "andl $0xfffffffffffffffe, 0(%eax);\n"
  //  		       "mov (0x3fffff), %eax;");

  __asm__ __volatile__("movl %%cr3, %%eax;\
                       andl $0xfffffffffffffffe, 0(%%eax);\
                       invlpg 0(%%eax);\
                       mov (0x3fffff), %%eax;" ::: "%eax", "memory");

  print("test executed successfully\n\r");
  return 0;
}

inline void 
msr_write (uint32_t msr, uint64_t data)
{
    uint32_t lo = data;
    uint32_t hi = data >> 32;
    asm volatile("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}


inline uint64_t 
msr_read (uint32_t msr) 
{
    uint32_t lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}


static int execute_rapl(char command[])
{
  print("Testing msr_read:");
  uint64_t v = (msr_read(0x611));
	DHQ(v);
	print("\n");
  return 0;
}

static unsigned long low_locality()   // 512 accesses, all from different pages
{
  // flush TLB
  tlb_flush();
  unsigned long *x = (unsigned long *)0x40000000U;
  unsigned long sum = 0;
  for (int i=0; i<ITERATIONS; i++)
  {
    for (unsigned long j=0; j<0x10000000U; j+=0x80000)
    {
      sum += x[j];
    }
  }
  return sum;
}
static unsigned long medium_locality()   // 512 accesses, 8 from same page
{
  // flush TLB
  tlb_flush();
  unsigned long *x = (unsigned long *)0x40000000U;
  unsigned long sum = 0;
  for (int i=0; i<ITERATIONS; i++)
  {
    for (unsigned long j=0; j<0x2000000U; j+=0x10000)
    {
      sum += x[j];
    }
  }
  return sum;
}
static unsigned long high_locality()   // 512 accesses, all from same page
{
  // flush TLB
  tlb_flush();
  unsigned long *x = (unsigned long *)0x40000000U;
  unsigned long sum = 0;
  for (int i=0; i<ITERATIONS; i++)
  {
    for (unsigned long j=0; j<512; j+=1)
    {
      sum += x[j];
    }
  }
  return sum;
}

static int execute_test(char command[])
{
  double avg_cycles = 0;
  unsigned long avg_sum = 0;
  unsigned long init_pwr = 0;
  unsigned long latest_pwr = 0;

  print("========================== PAGING OFF ==========================\n\r");
  vga_print("========================== PAGING OFF ==========================\n");
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = high_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with high locality: ");
  vga_print("avg cycles with high locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = medium_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with medium locality: ");
  vga_print("avg cycles with medium locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = low_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with low locality: ");
  vga_print("avg cycles with low locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\n\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = high_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with high locality: ");
  vga_print("avg power consumption with high locality:: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long offPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = medium_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with medium locality: ");
  vga_print("avg power consumption with medium locality:: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long offPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = low_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with low locality: ");
  vga_print("avg power consumption with low locality:: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long offPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");


  

  

  print("========================== PAGING ON ==========================\n\r");
  vga_print("========================== PAGING ON ==========================\n");
  paging_on();
  
  avg_cycles = 0;
  avg_sum = 0;
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = high_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with high locality: ");
  vga_print("avg cycles with high locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = medium_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with medium locality: ");
  vga_print("avg cycles with medium locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i=0; i<NUM_READINGS; i++)
  {
    unsigned long t1 = rdtsc();
    unsigned long temp = low_locality();
    unsigned long t2 = rdtsc();
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg cycles with low locality: ");
  vga_print("avg cycles with low locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  print("\n\r");
  vga_print("\n");

  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = high_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with high locality: ");
  vga_print("avg power consumption with high locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long onPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");


  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = medium_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with medium locality: ");
  vga_print("avg power consumption with medium locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long onPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");



  avg_cycles = 0;
  avg_sum = 0;
  for (int i = 0; i < NUM_READINGS; i++)
  {
    unsigned long t1 = msr_read(0x611);
    unsigned long temp = low_locality();
    unsigned long t2 = msr_read(0x611);
    avg_cycles += (t2-t1);
    avg_sum += temp;
  }
  if(avg_sum){
    print(long_to_string(avg_sum));
    print("\n\r");
  }
  print("avg power consumption with low locality: ");
  vga_print("avg power consumption with low locality: ");
  print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  vga_print(long_to_string((unsigned long)(avg_cycles/NUM_READINGS)));
  // unsigned long onPwr = (unsigned long)(avg_cycles/NUM_READINGS);
  print("\n\r");
  vga_print("\n");
  return 0;
}

static int execute_potential_command(char command[])
{
  vga_attr = vga_make_color(COLOR_FOREGROUND, COLOR_BACKGROUND);

  int quit = 0;
  char* print_string = "";

  char* word = get_next_word(command);

  if (my_strcmp(word, "quit") == 0)
  {
    quit = execute_quit(command);
  }
  if (my_strcmp(word, "help") == 0)
  {
    quit = execute_help(command);
  }
  else if (my_strcmp(word, "paging_on") == 0)
  {
    quit = execute_paging(command);
  }
  else if (my_strcmp(word, "pf") == 0)
  {
    quit = execute_pf(command);
  }
  else if (my_strcmp(word, "test") == 0)
  {
    quit = execute_test(command);
  }
  else if (my_strcmp(word, "pwrstats") == 0)
  {
    quit = execute_rapl(command);
  }
  else /* default: */
  {
    vga_puts("command not recognized");
  }
  vga_attr = vga_make_color(COLOR_PROMPT_FOREGROUND, COLOR_PROMPT_BACKGROUND);
  return quit;

}

static int nk_monitor_loop()
{
  int buffer_size = VGA_WIDTH * 2; 
  char buffer[buffer_size];

  // Inner loop is indvidual keystrokes, outer loop handles commands
  int done = 0;
  while (!done) {
    DS("monitor> ");
    wait_for_command(buffer, buffer_size);
    vga_putchar('\n');
    done = execute_potential_command(buffer);
  };
  return 0;
}



// ordinary reques for entry into the monitor
// we just wait our turn and then alert everyone else
static int monitor_init_lock()
{
    return 0; // handle processing...
}


// Main CPU wraps up its use of the monitor and updates everyone
static int monitor_deinit_lock(void)
{
    return 0;
}

// every entry to the monitor
// returns int flag for later restore
static uint8_t monitor_init(void)
{

    monitor_init_lock();
    
    vga_copy_out(screen_saved, VGA_WIDTH*VGA_HEIGHT*2);
    vga_get_cursor(&cursor_saved_x, &cursor_saved_y);
    
    vga_x = vga_y = 0;
    vga_attr = vga_make_color(COLOR_PROMPT_FOREGROUND, COLOR_PROMPT_BACKGROUND);
    vga_clear_screen(vga_make_entry(' ', vga_attr));
    vga_set_cursor(vga_x, vga_y);

    kbd_flags = 0;

    // probably should reset ps2 here...

    return 0;

}

// called right before every exit from the monitor
// takes int flag to restore
static void monitor_deinit(uint8_t intr_flags)
{
    vga_copy_in(screen_saved, VGA_WIDTH*VGA_HEIGHT*2);
    vga_set_cursor(cursor_saved_x, cursor_saved_y);

    monitor_deinit_lock();

}

// Entrypoints to the monitor

// Entering through the shell command or f9
int my_monitor_entry()
{
    uint8_t intr_flags = monitor_init();
    
    vga_attr = vga_make_color(COLOR_FOREGROUND, COLOR_BACKGROUND);
    vga_puts("My monitor Entered");
    vga_attr = vga_make_color(COLOR_PROMPT_FOREGROUND, COLOR_PROMPT_BACKGROUND);
    
    nk_monitor_loop();
    
    monitor_deinit(intr_flags);
    
    return 0;
}
