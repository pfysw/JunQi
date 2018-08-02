/*
 * board.h
 *
 *  Created on: Jul 31, 2018
 *      Author: Administrator
 */

#ifndef BOARD_H_
#define BOARD_H_

#define LENGTH1   39
#define LENGTH2   40

#define LEFTX     38
#define MIDX      265
#define RIGHTX    467

#define TOPY      13
#define MIDY      242
#define BOTTOMY   442

#define NINE_GRID_X   MIDX
#define NINE_GRID_Y   (TOPY+6*LENGTH1)

#define HORIZONTAL_LONG1  198 //5*40-2
#define HORIZONTAL_LONG2  191 //5*39-2
#define VERTICAL_LONG     232 //6*39-2

#define RECTANGLE_WHITE 0
#define RECTANGLE_RED 1

GtkWidget *GetSelectImage(int isVertical, int color);

#endif /* BOARD_H_ */
