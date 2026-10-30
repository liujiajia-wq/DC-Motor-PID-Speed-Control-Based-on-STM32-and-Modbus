#ifndef __OLED_H__
#define __OLED_H__



extern void oled_clear(unsigned char col);
extern void oled_show_string(int x, int y, unsigned char *str);
extern void oled_show_char(int x, int y, unsigned char chr);
extern int oled_init();
#endif