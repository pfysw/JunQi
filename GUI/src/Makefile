# 请自行新建一个bld文件夹
OBJ_DIR = ../bld
# 获取所以源文件     
SRC = board.c comm.c junqi.c main.c rule.c

# 把.c后缀替换为.o，并加上目录        
OBJ = ${patsubst %.c, ${OBJ_DIR}/%.o, ${SRC}}

LIBS = -L/d/msys64/mingw64/bin -L/d/msys64/mingw64/lib \
-lgtk-3-0 -lgdk-3-0 -lgdi32 -limm32 -lshell32 -lole32 \
-Wl,-luuid -lwinmm -ldwmapi -lsetupapi -lcfgmgr32 -lz \
-lpangowin32-1.0-0 -lpangocairo-1.0-0 -lpango-1.0-0 \
-lfribidi-0 -latk-1.0-0 -lcairo-gobject-2 -lcairo-2 \
-lgdk_pixbuf-2.0-0 -lgio-2.0-0 -lgobject-2.0-0 -lglib-2.0-0 -lintl-8


GTK_INC = -pthread -mms-bitfields -I/mingw64/include/gtk-3.0 \
-I/mingw64/include/cairo -I/mingw64/include -I/mingw64/include/pango-1.0 \
-I/mingw64/include/fribidi -I/mingw64/include -I/mingw64/include/atk-1.0 \
-I/mingw64/include/cairo -I/mingw64/include/pixman-1 -I/mingw64/include \
-I/mingw64/include/freetype2 -I/mingw64/include -I/mingw64/include/harfbuzz \
-I/mingw64/include -I/mingw64/include/libpng16 -I/mingw64/include/gdk-pixbuf-2.0 \
-I/mingw64/include -I/mingw64/include/glib-2.0 -I/mingw64/lib/glib-2.0/include -I/mingw64/include

TARGET=../junqi
CC=cc
CCFLAGS=-g -Wall ${LIBS} ${GTK_INC}

all:${TARGET}

${TARGET}: ${OBJ} 
	${CC} $^ -o $@ ${LIBS} -lwinmm 
	@echo "Compile done."
    

${OBJ_DIR}/%.o:%.c
	@echo "Compiling $< ==> $@"
	${CC} ${CCFLAGS} -c $< -o $@

    
clean:
# -r删除目录，-f删除文件
	rm -f ${OBJ_DIR}/*.o
	rm -f ${TARGET}