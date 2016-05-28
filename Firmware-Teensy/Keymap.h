/*
            IRON-HID Firmware for At90USB1286 in Teensy2.0++       
                    Copyright (C) 2016 Seunghun Han 
         at National Security Research Institute of South Korea
*/

/*
Copyright (c) 2016 Seunghun Han at NSR of South Kora

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#ifndef __KEYMAP_H__
#define __KEYMAP_H__

////////////////////////////////////////////////////////////////////
// Key Value Define
////////////////////////////////////////////////////////////////////
#define MAX_ROW           8
#define MAX_COL           18

#define KEY_NONE          0xFF
#define	KEY_A             4
#define	KEY_B             5
#define	KEY_C             6
#define	KEY_D             7
#define	KEY_E             8
#define	KEY_F             9
#define	KEY_G             10
#define	KEY_H             11
#define	KEY_I             12
#define	KEY_J             13
#define	KEY_K             14
#define	KEY_L             15
#define	KEY_M             16
#define	KEY_N             17
#define	KEY_O             18
#define	KEY_P             19
#define	KEY_Q             20
#define	KEY_R             21
#define	KEY_S             22
#define	KEY_T             23
#define	KEY_U             24
#define	KEY_V             25
#define	KEY_W             26
#define	KEY_X             27
#define	KEY_Y             28
#define	KEY_Z             29
#define	KEY_1             30
#define	KEY_2             31
#define	KEY_3             32
#define	KEY_4             33
#define	KEY_5             34
#define	KEY_6             35
#define	KEY_7             36
#define	KEY_8             37
#define	KEY_9             38
#define	KEY_0             39
#define	KEY_ENTER         40
#define	KEY_ESC           41
#define	KEY_BACKSPACE     42
#define	KEY_TAB           43
#define	KEY_SPACE         44
#define	KEY_MINUS         45
#define	KEY_EQUAL         46
#define	KEY_OPENBRAKET    47
#define	KEY_CLOSEBRAKET   48
#define	KEY_INVERSESLASH  49
#define	KEY_TILDE         50
#define	KEY_SEMICOLON     51
#define	KEY_QUOTATION     52
#define KEY_TILDE         53
#define	KEY_COMMA         54
#define	KEY_PERIOD        55
#define	KEY_SLASH         56
#define	KEY_CAPSLOCK      57
#define	KEY_F1            58
#define	KEY_F2            59
#define	KEY_F3            60
#define	KEY_F4            61
#define	KEY_F5            62
#define	KEY_F6            63
#define	KEY_F7            64
#define	KEY_F8            65
#define	KEY_F9            66
#define	KEY_F10           67
#define	KEY_F11           68
#define	KEY_F12           69
#define	KEY_PRINTSCREEN   70
#define	KEY_SCROLLLOCK    71
#define	KEY_PAUSE         72
#define	KEY_INSERT        73
#define	KEY_HOME          74
#define	KEY_PAGEUP        75
#define	KEY_DELETE        76
#define	KEY_END           77
#define	KEY_PAGEDOWN      78
#define	KEY_RIGHT         79
#define	KEY_LEFT          80
#define	KEY_DOWN          81
#define	KEY_UP            82
#define	KEY_NUMLOCK       83
#define	KEY_PAD_SLASH     84
#define	KEY_PAD_ASTERISK  85
#define	KEY_PAD_MINUS     86
#define	KEY_PAD_PLUS      87
#define	KEY_PAD_ENTER     88
#define	KEY_PAD_1         89
#define	KEY_PAD_2         90
#define	KEY_PAD_3         91
#define	KEY_PAD_4         92
#define	KEY_PAD_5         93
#define	KEY_PAD_6         94
#define	KEY_PAD_7         95
#define	KEY_PAD_8         96
#define	KEY_PAD_9         97
#define	KEY_PAD_0         98
#define	KEY_PAD_PERIOD    99
#define	KEY_APPLICATION   101
#define	KEY_POWER         102
#define	KEY_PAD_EQUAL     103
#define	KEY_F13           104
#define	KEY_F14           105
#define	KEY_F15           106
#define	KEY_F16           107
#define	KEY_F17           108
#define	KEY_F18           109
#define	KEY_F19           110
#define	KEY_F20           111
#define	KEY_F21           112
#define	KEY_F22           113
#define	KEY_F23           114
#define	KEY_F24           115
#define	KEY_EXECUTE       116
#define	KEY_HELP          117
#define	KEY_MENU          118
#define	KEY_SELECT        119
#define	KEY_STOP          120
#define	KEY_AGAIN         121
#define	KEY_UNDO          122
#define	KEY_CUT           123
#define	KEY_COPY          124
#define	KEY_PASTE         125
#define	KEY_FIND          126
#define	KEY_MUTE          127
#define	KEY_VOLUMEUP      128
#define	KEY_VOLUMEDOWN    129
#define KEY_EJECT         184
#define	KEY_LCONTROL      224
#define	KEY_LSHIFT        225
#define	KEY_LALT          226
#define	KEY_LGUI          227
#define	KEY_RCONTROL      228
#define	KEY_RSHIFT        229
#define	KEY_RALT          230
#define	KEY_RGUI          231
#define KEY_FN            254

/**
 *  Consumer page key code
 */
#ifdef MATIAS_LAPTOP_PRO
#define KEY_VOLUMEUP      0x10
#define KEY_VOLUMEDOWN    0x20
#define KEY_MUTE          0x40
#define KEY_PLAYPAUSE     0x80
#define KEY_SCANNEXTTRACK 0x100
#define KEY_SCANPREVTRACK 0x200
#define KEY_STOP          0x400
#define KEY_EJECT         0x800
#define KEY_FASTFORWARD   0x1000
#define KEY_REWIND        0x2000
#define KEY_STOPEJECT     0x4000
#define KEY_ALBROWSER     0x8000
#else
#define KEY_PLAY          0xB0
#define KEY_PLAYPAUSE     0xB1
#define KEY_RECORD        0xB2
#define KEY_FASTFORWARD   0xB3
#define KEY_REWIND        0xB4
#endif

// Order is as same as Key Code
#define MODIFIER_LCONTROL  (1 << 0)
#define MODIFIER_LSHIFT    (1 << 1)
#define MODIFIER_LALT      (1 << 2)
#define MODIFIER_LGUI      (1 << 3)
#define MODIFIER_RCONTROL  (1 << 4)
#define MODIFIER_RSHIFT    (1 << 5)
#define MODIFIER_RALT      (1 << 6)
#define MODIFIER_RGUI      (1 << 7)

// LED Flag
#define LED_COUNT          3
#define LED_NUMLOCK        (1 << 0)
#define LED_CAPSLOCK       (1 << 1)
#define LED_SCROLLLOCK     (1 << 2)


#if defined CHERRY_COMPACT
// Key Matrix for Cherry Compact
unsigned char g_vcKeyMaxtrix[MAX_ROW][MAX_COL] = { 
  { KEY_NONE, KEY_NONE, KEY_LGUI, KEY_NONE, KEY_ESC, KEY_NONE, KEY_F4, KEY_G, KEY_H, KEY_F6, KEY_F5, KEY_QUOTATION, KEY_F5, KEY_NONE, KEY_PAD_0, KEY_PAD_PERIOD, KEY_UP, KEY_LALT },
  { KEY_NONE, KEY_RCONTROL, KEY_NONE, KEY_RSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_NONE, KEY_ENTER, KEY_NUMLOCK, KEY_PAD_SLASH, KEY_PAD_ASTERISK, KEY_PAUSE, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_7, KEY_8, KEY_9, KEY_0, KEY_F10, KEY_F11, KEY_F12, KEY_PAGEDOWN, KEY_END, KEY_PRINTSCREEN },
  { KEY_NONE, KEY_LCONTROL, KEY_NONE, KEY_LSHIFT, KEY_TILDE, KEY_F1, KEY_F2, KEY_5, KEY_6, KEY_EQUAL, KEY_F8, KEY_MINUS, KEY_F9, KEY_DELETE, KEY_INSERT, KEY_PAGEUP, KEY_HOME, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_U, KEY_I, KEY_O, KEY_P, KEY_NONE, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_PLUS, KEY_SCROLLLOCK },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_LSHIFT, KEY_TAB, KEY_CAPSLOCK, KEY_F3, KEY_T, KEY_Y, KEY_CLOSEBRAKET, KEY_F7, KEY_OPENBRAKET, KEY_BACKSPACE, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6, KEY_APPLICATION, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_A, KEY_S, KEY_D, KEY_F, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_INVERSESLASH, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3, KEY_PAD_ENTER, KEY_NONE },
  { KEY_RGUI, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_B, KEY_N, KEY_NONE, KEY_NONE, KEY_SLASH, KEY_SPACE, KEY_DOWN, KEY_RIGHT, KEY_PAD_MINUS, KEY_LEFT, KEY_RALT }
};

/*
// Key Matrix for Cherry Compact with built-in hub
unsigned char g_vcKeyMaxtrix[MAX_ROW][MAX_COL] = { 
  { KEY_NONE, KEY_LGUI, KEY_NONE, KEY_NONE, KEY_ESC, KEY_NONE, KEY_F4, KEY_G, KEY_H, KEY_NONE, KEY_F6, KEY_QUOTATION, KEY_F5, KEY_NONE, KEY_PAD_0, KEY_PAD_PERIOD, KEY_UP, KEY_LALT },
  { KEY_NONE, KEY_NONE, KEY_RCONTROL, KEY_RSHIFT, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_M, KEY_PERIOD, KEY_COMMA, KEY_NONE, KEY_ENTER, KEY_NUMLOCK, KEY_PAD_SLASH, KEY_PAD_ASTERISK, KEY_PAUSE, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_7, KEY_9, KEY_8, KEY_0, KEY_F10, KEY_F11, KEY_F12, KEY_PAGEDOWN, KEY_END, KEY_PRINTSCREEN },
  { KEY_NONE, KEY_NONE, KEY_LCONTROL, KEY_NONE, KEY_TILDE, KEY_F1, KEY_F2, KEY_5, KEY_6, KEY_F8, KEY_EQUAL, KEY_MINUS, KEY_F9, KEY_DELETE, KEY_INSERT, KEY_PAGEUP, KEY_HOME, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_U, KEY_O, KEY_I, KEY_P, KEY_NONE, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9, KEY_PAD_PLUS, KEY_SCROLLLOCK },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_LSHIFT, KEY_TAB, KEY_CAPSLOCK, KEY_F3, KEY_T, KEY_Y, KEY_F7, KEY_CLOSEBRAKET, KEY_OPENBRAKET, KEY_BACKSPACE, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6, KEY_APPLICATION, KEY_NONE },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_A, KEY_S, KEY_D, KEY_F, KEY_J, KEY_L, KEY_K, KEY_SEMICOLON, KEY_INVERSESLASH, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3, KEY_PAD_ENTER, KEY_NONE },
  { KEY_RGUI, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_B, KEY_N, KEY_NONE, KEY_NONE, KEY_SLASH, KEY_SPACE, KEY_DOWN, KEY_RIGHT, KEY_PAD_MINUS, KEY_LEFT, KEY_RALT }
};
*/

#elif defined MATIAS_TACTILE_PRO
///*
// Key Matrix for Mac Version
unsigned char g_vcKeyMaxtrix[MAX_ROW][MAX_COL] = { 
  { KEY_Z, KEY_X, KEY_C, KEY_V, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_PAD_EQUAL, KEY_NONE, KEY_NONE, KEY_RCONTROL, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NUMLOCK, KEY_PAD_SLASH, KEY_PAD_ASTERISK },
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_B, KEY_N, KEY_NONE, KEY_NONE, KEY_LEFT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_RALT, KEY_SLASH, KEY_F12, KEY_NONE, KEY_DOWN, KEY_RIGHT, KEY_PAD_MINUS },
  { KEY_TILDE, KEY_F1, KEY_F2, KEY_5, KEY_6, KEY_EQUAL, KEY_F8, KEY_HOME, KEY_MUTE, KEY_VOLUMEDOWN, KEY_LCONTROL, KEY_NONE, KEY_MINUS, KEY_F9, KEY_NONE, KEY_DELETE, KEY_HELP, KEY_PAGEUP },
  { KEY_1, KEY_2, KEY_3, KEY_4, KEY_7, KEY_8, KEY_9, KEY_END, KEY_NONE, KEY_VOLUMEUP, KEY_F5, KEY_F13, KEY_0, KEY_F10, KEY_NONE, KEY_NONE, KEY_NONE, KEY_PAGEDOWN },
  { KEY_ESC, KEY_NONE, KEY_F4, KEY_G, KEY_H, KEY_F6, KEY_NONE, KEY_UP, KEY_NONE, KEY_EJECT, KEY_NONE, KEY_LALT, KEY_QUOTATION, KEY_F11, KEY_NONE, KEY_SPACE, KEY_PAD_0, KEY_PAD_PERIOD },
  { KEY_A, KEY_S, KEY_D, KEY_F, KEY_J, KEY_K, KEY_L, KEY_PAD_ENTER, KEY_RGUI, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SEMICOLON, KEY_INVERSESLASH, KEY_RSHIFT, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3 },
  { KEY_TAB, KEY_CAPSLOCK, KEY_F3, KEY_T, KEY_Y, KEY_CLOSEBRAKET, KEY_F7, KEY_NONE, KEY_NONE, KEY_LGUI, KEY_NONE, KEY_NONE, KEY_OPENBRAKET, KEY_BACKSPACE, KEY_LSHIFT, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6 },
  { KEY_Q, KEY_W, KEY_E, KEY_R, KEY_U, KEY_I, KEY_O, KEY_PAD_PLUS, KEY_NONE, KEY_NONE, KEY_F15, KEY_F14, KEY_P, KEY_NONE, KEY_NONE, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9 }
};
//*/
/*
// Key Matrix for PC Version
unsigned char g_vcKeyMaxtrix[MAX_ROW][MAX_COL] = { 
   { KEY_Z, KEY_X, KEY_C, KEY_V, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_PAD_EQUAL, KEY_NONE, KEY_NONE, KEY_RCONTROL, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NUMLOCK, KEY_PAD_SLASH, KEY_PAD_ASTERISK },
   { KEY_NONE, KEY_NONE, KEY_NONE, KEY_B, KEY_N, KEY_NONE, KEY_NONE, KEY_LEFT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_MENU, KEY_SLASH, KEY_F12, KEY_NONE, KEY_DOWN, KEY_RIGHT, KEY_PAD_MINUS },
   { KEY_TILDE, KEY_F1, KEY_F2, KEY_5, KEY_6, KEY_EQUAL, KEY_F8, KEY_HOME, KEY_MUTE, KEY_VOLUMEDOWN, KEY_LCONTROL, KEY_NONE, KEY_MINUS, KEY_F9, KEY_NONE, KEY_DELETE, KEY_INSERT, KEY_PAGEUP },
   { KEY_1, KEY_2, KEY_3, KEY_4, KEY_7, KEY_8, KEY_9, KEY_END, KEY_NONE, KEY_VOLUMEUP, KEY_F5, KEY_PRINTSCREEN, KEY_0, KEY_F10, KEY_NONE, KEY_NONE, KEY_NONE, KEY_PAGEDOWN },
   { KEY_ESC, KEY_NONE, KEY_F4, KEY_G, KEY_H, KEY_F6, KEY_NONE, KEY_UP, KEY_NONE, KEY_EJECT, KEY_NONE, KEY_LGUI, KEY_QUOTATION, KEY_F11, KEY_NONE, KEY_SPACE, KEY_PAD_0, KEY_PAD_PERIOD },
   { KEY_A, KEY_S, KEY_D, KEY_F, KEY_J, KEY_K, KEY_L, KEY_PAD_ENTER, KEY_RALT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SEMICOLON, KEY_INVERSESLASH, KEY_RSHIFT, KEY_PAD_1, KEY_PAD_2, KEY_PAD_3 },
   { KEY_TAB, KEY_CAPSLOCK, KEY_F3, KEY_T, KEY_Y, KEY_CLOSEBRAKET, KEY_F7, KEY_NONE, KEY_NONE, KEY_LALT, KEY_NONE, KEY_NONE, KEY_OPENBRAKET, KEY_BACKSPACE, KEY_LSHIFT, KEY_PAD_4, KEY_PAD_5, KEY_PAD_6 },
   { KEY_Q, KEY_W, KEY_E, KEY_R, KEY_U, KEY_I, KEY_O, KEY_PAD_PLUS, KEY_NONE, KEY_NONE, KEY_PAUSE, KEY_SCROLLLOCK, KEY_P, KEY_NONE, KEY_NONE, KEY_PAD_7, KEY_PAD_8, KEY_PAD_9 }
 };
*/

#elif defined MATIAS_LAPTOP_PRO
// Key Matrix for PC Version
unsigned char g_vcKeyMaxtrix[MAX_ROW][MAX_COL] = { 
  { KEY_RCONTROL, KEY_Z, KEY_X, KEY_C, KEY_M, KEY_COMMA, KEY_PERIOD, KEY_NONE, KEY_NONE, KEY_NONE, KEY_V, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE }, 
  { KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_N, KEY_NONE, KEY_NONE, KEY_LEFT, KEY_NONE, KEY_NONE, KEY_B, KEY_RALT, KEY_SLASH, KEY_F12, KEY_NONE, KEY_DOWN, KEY_RIGHT, KEY_NONE },
  { KEY_LCONTROL, KEY_TILDE, KEY_F1, KEY_F2, KEY_6, KEY_EQUAL, KEY_F8, KEY_NONE, KEY_NONE, KEY_NONE, KEY_5, KEY_NONE, KEY_MINUS, KEY_F9, KEY_NONE, KEY_DELETE, KEY_NONE, KEY_PAGEUP },
  { KEY_F5, KEY_1, KEY_2, KEY_3, KEY_7, KEY_8, KEY_9, KEY_NONE, KEY_NONE, KEY_NONE, KEY_4, KEY_NONE, KEY_0, KEY_F10, KEY_NONE, KEY_NONE, KEY_NONE, KEY_PAGEDOWN },
  { KEY_NONE, KEY_ESC, KEY_NONE, KEY_F4, KEY_H, KEY_F6, KEY_NONE, KEY_UP, KEY_NONE, KEY_NONE, KEY_G, KEY_LALT, KEY_QUOTATION, KEY_F11, KEY_NONE, KEY_SPACE, KEY_FN, KEY_NONE },
  { KEY_NONE, KEY_A, KEY_S, KEY_D, KEY_J, KEY_K, KEY_L, KEY_NONE, KEY_RGUI, KEY_NONE, KEY_F, KEY_NONE, KEY_SEMICOLON, KEY_INVERSESLASH, KEY_LSHIFT, KEY_NONE, KEY_NONE, KEY_NONE },
  { KEY_NONE, KEY_TAB, KEY_CAPSLOCK, KEY_F3, KEY_Y, KEY_CLOSEBRAKET, KEY_F7, KEY_NONE, KEY_NONE, KEY_LGUI, KEY_T, KEY_NONE, KEY_OPENBRAKET, KEY_BACKSPACE, KEY_RSHIFT, KEY_NONE, KEY_NONE, KEY_NONE },
  { KEY_NONE, KEY_Q, KEY_W, KEY_E, KEY_U, KEY_I, KEY_O, KEY_NONE, KEY_NONE, KEY_NONE, KEY_R, KEY_NONE, KEY_P, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE }
};

// Multimedia key need to process different way
#define MULTIMEDIA_KEY_INDEX      5

unsigned char g_vcLayer1Key[] = { KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F9, KEY_F10, KEY_F11, KEY_DELETE, KEY_PAGEUP, KEY_PAGEDOWN };
unsigned short g_vcLayer2Key[] = { KEY_MUTE, KEY_VOLUMEDOWN, KEY_VOLUMEUP, KEY_PLAYPAUSE, KEY_REWIND, KEY_FASTFORWARD, KEY_PRINTSCREEN, KEY_SCROLLLOCK, KEY_PAUSE, KEY_INSERT, KEY_HOME, KEY_END };
#endif

#endif
