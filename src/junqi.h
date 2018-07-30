#ifndef JUNQI_H
#define JUNQI_H
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

enum ChessColor {ORANGE,GREEN,BLUE,PURPLE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
u8 aMagic[4]={0x57,0x04,0,0};

typedef struct ChessLineup
{
	enum ChessType type;
	GtkWidget *pImage;
}ChessLineup;
typedef struct Junqi Junqi;
struct Junqi
{
	GdkPixbuf *ChessImage[4];
	GdkPixbuf *Chess[4][14];
	ChessLineup ChessPos[4][30];
	GtkWidget *fixed;
};

typedef struct Jql
{
	u8 not_use[16];
	u8 aMagic[4];
	u8 chess[30];
}Jql;


#endif
