/*
 * board.h
 *
 *  Created on: Jul 31, 2018
 *      Author: Administrator
 */

#ifndef BOARD_H_
#define BOARD_H_

//棋子放在一个正方形的格子里，这里定义边长
//自家与对家格子的长要稍微宽一点，是40
#define LENGTH1   39
#define LENGTH2   40

//分别为上家、自家、下家左上角的横坐标
#define LEFTX     38
#define MIDX      265
#define RIGHTX    467

//分别为对家、上家、自家左上角的纵坐标
#define TOPY      13
#define MIDY      242
#define BOTTOMY   442

//九宫格左上角的横纵坐标
#define NINE_GRID_X   MIDX
#define NINE_GRID_Y   (TOPY+6*LENGTH1)

//每家方阵的长和宽，自己和对家宽度要稍微长一点
#define HORIZONTAL_LONG1  198 //5*40-2
#define HORIZONTAL_LONG2  191 //5*39-2
#define VERTICAL_LONG     232 //6*39-2

#define RECTANGLE_WHITE 0
#define RECTANGLE_RED 1

GtkWidget *GetSelectImage(int isVertical, int color);
GdkPixbuf* get_from_pixbuf_position(GdkPixbuf* pixbuf,
								gint src_x,
								gint src_y,
								gint width,
								gint height );
void SendSoundEvent(Junqi *pJunqi, enum CompareType type);
#endif /* BOARD_H_ */
