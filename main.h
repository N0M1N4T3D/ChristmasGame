#pragma once
#define NUM_CIRCLE_POINTS 36 
#define NUM_RAYS 12          
#define RADIUS 0.5f         
#define RAY_LENGTH 1.5f      

typedef struct {
    float x, y, z;
} TCell;

typedef struct {
    float r, g, b;
} TColor;

typedef struct {
    float u, v;
} TUV;

typedef struct {
    float x, y, z;
    int type;
    float scale;
} TObject;


#define mapW 1000
#define mapH 1000
TCell map[mapW][mapH];
TCell mapNormal[mapW][mapH];
TUV mapUV[mapW][mapH];

GLuint mapInd[mapW - 1][mapH - 1][6];
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);
float plant[] = { -0.5,0,0,0.5,0,0,0.5,0,1,-0.5,0,1,
                   0,-0.5,0,0,0.5,0,0,0.5,1,0,-0.5,1 };
float plantUV[] = { 0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0 };
GLuint plantInd[] = { 0,1,2,2,3,0,4,5,6,6,7,4 };
int plantIndCnt = sizeof(plantInd) / sizeof(GLuint);

TObject* plantMas = nullptr;
int plantCnt = 0;
GLuint tex_tree, tex_floor, tex_tree2, tex_tree3, tex_tree4, tex_snowman;


float sun[3 * (NUM_CIRCLE_POINTS + NUM_RAYS * 2 + 1)]; // Массив вершин