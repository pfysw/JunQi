#ifndef JUNQI_H
#define JUNQI_H
#include <stdio.h>
#include <gtk/gtk.h>
#include <assert.h>

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

enum ChessColor {ORANGE,PURPLE,GREEN,BLUE};
enum ChessType {NONE,DARK,JUNQI,DILEI,ZHADAN,SILING,JUNZH,SHIZH,
	LVZH,TUANZH,YINGZH,LIANZH,PAIZH,GONGB};
enum ChessDir {HOME,RIGHT,OPPS,LEFT};

#define GAME_OPEN             0
#define GAME_BEAGIN           1
#define GAME_NO_SELECT        2
#define GAME_SELECT           3


const static u8 aMagic[4]={0x57,0x04,0,0};

typedef struct ChessLineup
{
	enum ChessType type;
	GtkWidget *pImage[4];
}ChessLineup;

typedef struct BoardChess BoardChess;
struct BoardChess
{
	enum ChessType type;
	enum ChessDir iDir;
	ChessLineup *pLineup;
	int xPos;
	int yPos;
};

typedef struct Junqi Junqi;
struct Junqi
{
	u8 eState;
	u8 bSelect;
	BoardChess *pSelect;
	GdkPixbuf *ChessImage[4];
	GdkPixbuf *Chess[4][14];
	ChessLineup Lineup[4][30];
	BoardChess ChessPos[4][30];
	GtkWidget *fixed;
	//BoardChess ChessHome[4][30];
	BoardChess NineGrid[9];

	GtkWidget *whileRectangle[2];
	GtkWidget *redRectangle[2];

};

typedef struct Jql
{
	u8 not_use[16];
	u8 aMagic[4];
	u8 chess[30];
}Jql;

GdkPixbuf* get_from_pixbuf_position(GdkPixbuf* pixbuf,
								gint src_x,
								gint src_y,
								gint width,
								gint height );


#endif
