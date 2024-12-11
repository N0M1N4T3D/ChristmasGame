#include <windows.h>
#include <gl/gl.h>
#include <gl/GLU.h>
#include <math.h>
#include "main.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#pragma comment(lib, "opengl32.lib")
#define M_PI 3.1415926535 
#define abs(a) ((a)>0 ?(a) : -(a))
#define sqr(a) (a)*(a)
#define zakat 40


void CalcNormals(TCell a, TCell b, TCell c, TCell* n) {
    float wrki;
    TCell v1, v2;

    v1.x = a.x - b.x;
    v1.y = a.y - b.y;
    v1.z = a.z - b.z;
    v2.x = b.x - c.x;
    v2.y = b.y - c.y;
    v2.y = b.y - c.y;
    v2.z = b.z - c.z;
    
    n->x = (v1.y * v2.z - v1.z * v2.y);
    n->y = (v1.z * v2.x - v1.x * v2.z);
    n->z = (v1.x * v2.y - v1.y * v2.x);
    wrki = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));
    n->x /= wrki;
    n->y /= wrki;
    n->z /= wrki;
}

void LoadTexture(const char* file_name, GLuint* target) {
    int width, height, cnt;
    unsigned char* data = stbi_load(file_name, &width, &height, &cnt, 0);
    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

BOOL IsCoordInMap(float x, float y) {
    return (x >= 0) && (x < mapW) && (y >= 0) && (y < mapH);
}

void Map_CreateHill(int posX, int posY, int rad, int height) {
    for (int i = posX - rad; i <= posX + rad; i++) {
        for (int j = posY - rad; j <= posY + rad; j++) {
            if (IsCoordInMap(i, j)) {
                float len = sqrt(pow(posX - i, 2) + pow(posY - j, 2));

                if (len < rad) {
                    len = len / rad * M_PI / 2;
                    map[i][j].z += cos(len) * height;
                }
            }
        }
    }
}

float Map_GetHeight(float x, float y) {
    if (!IsCoordInMap(x, y)) return 0;
    int cX = (int)x;
    int cY = (int)y;
    float h1 = ((1 - (x - cX)) * map[cX][cY].z + (x - cX) * map[cX + 1][cY].z);
    float h2 = ((1 - (x - cX)) * map[cX][cY + 1].z + (x - cX) * map[cX + 1][cY+1].z);
    return (1 - (y - cY)) * h1 + (y - cY) * h2;
}

void generateSun() {
    int idx = 0;
    sun[idx++] = 0.0f; sun[idx++] = 0.0f; sun[idx++] = 0.0f;

    for (int i = 0; i <= NUM_CIRCLE_POINTS; i++) {
        float angle = 2.0f * M_PI * i / NUM_CIRCLE_POINTS;
        sun[idx++] = RADIUS * cos(angle); 
        sun[idx++] = RADIUS * sin(angle); 
        sun[idx++] = 0.0f;                
    }

    for (int i = 0; i < NUM_RAYS; i++) {
        float angle = 2.0f * M_PI * i / NUM_RAYS;

        sun[idx++] = RADIUS * cos(angle); 
        sun[idx++] = RADIUS * sin(angle);
        sun[idx++] = 0.0f;               

        sun[idx++] = RAY_LENGTH * cos(angle);
        sun[idx++] = RAY_LENGTH * sin(angle); 
        sun[idx++] = 0.0f;                   
    }
}

void Map_Init() {
    LoadTexture("textures/christmas_tree.png", &tex_tree);
    LoadTexture("textures/tree.png", &tex_tree2);
    LoadTexture("textures/tree2.png", &tex_tree3);
    LoadTexture("textures/tree3.png", &tex_tree4);
    LoadTexture("textures/snow.png", &tex_floor);
    LoadTexture("textures/snowman.png", &tex_snowman);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.99);

    for (int i = 0; i < mapW; i++) {
        for (int j = 0; j < mapH; j++) {
            map[i][j].x = i;
            map[i][j].y = j;
            map[i][j].z = (rand() % 10) * 0.1;

            mapUV[i][j].u = i;
            mapUV[i][j].v = j;
        }
    }

    for (int i = 0; i < mapW - 1; i++) {
        int pos = i * mapH;
        for (int j = 0; j < mapH - 1; j++) {
            mapInd[i][j][0] = pos;
            mapInd[i][j][1] = pos + 1;
            mapInd[i][j][2] = pos + 1 + mapH;

            mapInd[i][j][3] = pos + 1 + mapH;
            mapInd[i][j][4] = pos + mapH;
            mapInd[i][j][5] = pos;
            pos++;
        }
    }

    for (int i = 0; i < 10; i++) {
        Map_CreateHill(rand() % mapW, rand() % mapH, rand() % 50, rand() % 10);
    }

    for (int i = 0; i < mapW - 1; i++) {
        for (int j = 0;j < mapH - 1; j++) {
            CalcNormals(map[i][j], map[i+1][j], map[i][j+1], &mapNormal[i][j]);
        }
    }

    int treeN = 100, tree2N = 100, tree3N = 100, tree4N = 100, snowmanN = 150;
    plantCnt = treeN + tree2N +  tree3N + tree4N + snowmanN;
    plantMas = static_cast<TObject*>(realloc(plantMas, sizeof(*plantMas) * plantCnt));

    for (int i = 0; i < plantCnt; i++) {
        if (i < treeN) {
            plantMas[i].type = tex_tree;
            plantMas[i].scale = 4 + (rand() % 14);
        }
        else if (i < tree2N + treeN) {
            plantMas[i].type = tex_tree2;
            plantMas[i].scale = 4 + (rand() % 14);
        }
        else if (i < tree3N + treeN + tree2N) {
            plantMas[i].type = tex_tree3;
            plantMas[i].scale = 4 + (rand() % 14);
        }
        else if (i < treeN + tree2N + tree3N + tree4N) {
            plantMas[i].type = tex_tree4;
            plantMas[i].scale = 4 + (rand() % 14);
        }
        else {
            plantMas[i].type = tex_snowman;
            plantMas[i].scale = 0.5 + (rand() % 5);
            plantMas[i].x = rand() % mapW;
            plantMas[i].y = rand() % mapH;
            plantMas[i].z = Map_GetHeight(plantMas[i].x, plantMas[i].y) - 0.5;
            continue;
        }
        plantMas[i].x = rand() % mapW;
        plantMas[i].y = rand() % mapH;
        plantMas[i].z = Map_GetHeight(plantMas[i].x, plantMas[i].y) - 1.5;
    }
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
#define M_PI 3.1415926535 

void WndResize(int x, int y) {
    glViewport(0, 0, x, y);
    float k = x / (float)y;
    float sz = 0.1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-k * sz, k * sz, -sz, sz, sz * 2, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Player_Move() {
    Camera_MoveDirection(GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0),
        GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0), 0.1);
    Camera_AutoMoveByMouse(400, 400, 0.2);
    camera.z = Map_GetHeight(camera.x, camera.y) + 1.7;
}

void Map_Show() {
    static float alpha = 0;
    alpha += 0.2;
    if (alpha > 180) alpha -= 360;
    float kcc = 1 - (abs(alpha) / 180);
    
    float k = 90 - abs(alpha);
    k = (zakat - abs(k));
    k = k < 0 ? 0 : k / zakat;
    glClearColor(0.6f * kcc, 0.8f * kcc, 1.0f * kcc, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glPushMatrix();
    glRotatef(-camera.Xrot, 1, 0, 0);
    glRotatef(-camera.Zrot, 0, 0, 1);
    glRotatef(alpha, 0, 1, 0);
    glTranslatef(0, 0, 20);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glColor3f(1, 1 - k * 0.8, 1 - k);
    glEnableClientState(GL_VERTEX_ARRAY);
    generateSun();
    glVertexPointer(3, GL_FLOAT, 0, sun);
    glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_CIRCLE_POINTS + 2);              
    glDrawArrays(GL_LINES, NUM_CIRCLE_POINTS + 2, NUM_RAYS * 2);           
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    Camera_Apply();
    
    glPushMatrix();
    glRotatef(alpha, 0, 1, 0);
    GLfloat position[] = { 0,0,1,0 };
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    float mas[] = { 1 + k * 2, 1, 1, 0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mas);

    float clr = kcc * 0.15 + 0.05;
    float mas0[] = { clr, clr, clr, 0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, mas0);
    glPopMatrix();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, map);
    glTexCoordPointer(2, GL_FLOAT, 0, mapUV);
    glColor3f(0.7, 0.7, 0.7);
    glNormalPointer(GL_FLOAT, 0, mapNormal);
    glBindTexture(GL_TEXTURE_2D, tex_floor);
    glDrawElements(GL_TRIANGLES, mapIndCnt, GL_UNSIGNED_INT, mapInd);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, plant);
    glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
    glColor3f(0.7, 0.7, 0.7);
    glNormal3f(0, 0, 1);
    for (int i = 0; i < plantCnt; i++) {
        glBindTexture(GL_TEXTURE_2D, plantMas[i].type);
        glPushMatrix();
        glTranslatef(plantMas[i].x, plantMas[i].y, plantMas[i].z);
        glScalef(plantMas[i].scale, plantMas[i].scale, plantMas[i].scale);
        glDrawElements(GL_TRIANGLES, plantIndCnt, GL_UNSIGNED_INT, plantInd);
        glPopMatrix();
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPopMatrix();
}

int WINAPI WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
        L"GLSample",
        L"ChristmasGame",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        1080,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    RECT rct;
    GetClientRect(hwnd, &rct);
    WndResize(rct.right, rct.bottom);
    Map_Init();
    glEnable(GL_DEPTH_TEST);
  
    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */
            if (GetForegroundWindow() == hwnd)
                Player_Move();
            Map_Show();
            
            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep(1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
        WndResize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_SETCURSOR:
        ShowCursor(FALSE);
        break;
    case WM_DESTROY:
        return 0;

    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));
    
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL(HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

