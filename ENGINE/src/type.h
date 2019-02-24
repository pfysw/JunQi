#ifndef JUNQITYPE_H_
#define JUNQITYPE_H_

typedef unsigned char  u8;
typedef unsigned int   u32;
typedef unsigned short u16;

# define ALWAYS(X)      ((X)?1:(assert(0),0))
# define NEVER(X)       ((X)?(assert(0),1):0)

typedef enum SearchType{
    SEARCH_DEFAULT,
    SEARCH_RIGHT,
    SEARCH_LEFT,
    SEARCH_SINGLE,
    SEARCH_PATH,
    SEARCH_CONNECT,
    SEARCH_DEEP,
    SEARCH_SUM
}SearchType;


typedef struct Junqi Junqi;
typedef struct BoardChess BoardChess;
typedef struct GraphPath GraphPath;
typedef struct PositionList PositionList;
typedef struct MoveHash MoveHash;
typedef struct BestMove BestMove;
typedef struct BoardGraph BoardGraph;
typedef struct PositionData PositionData;
typedef struct ChessLineup ChessLineup;
typedef struct JunqiPath JunqiPath;
typedef struct MoveSort MoveSort;
typedef struct AlphaBetaData AlphaBetaData;

#endif
