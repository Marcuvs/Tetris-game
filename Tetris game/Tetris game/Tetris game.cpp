// Tetris game.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Tetris game.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <windows.h>
#include <iostream>
#include <mmsystem.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TETRISGAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TETRISGAME));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETRISGAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TETRISGAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

const int numBlocks = 7;
/**
List of numbers corresponding to different block types. 
0: Straight 1*4 block. 2 states: ****, *. Cyan.
                                       *
                                       * 
                                       * 

1: Block 2*2 block. 1 state: **. Yellow.
                             **

2: T-shaped block. 4 states: ***  *    *    *. Pink.
                              * , **, ***, **
                                  *         *

3: L-shaped block. 4 states:   *  *   ***  **. Orange.
                             ***, * , *  ,  *
                                  **        *

4: J-shaped block. 4 states: *    **  ***   *. Purple.
                             ***, * ,   *,  *, 
                                  *        **  

5: Left Zig-Zag block. 2 states: **    *. Red.
                                  **, **
                                      * 

6: Right Zig-Zag block. 2 states:  **  * . Green.
                                  ** , **
                                        * 
*/

const int gameWidth = 10;
const int gameHeight = 20;
static int field[gameHeight][gameWidth];

bool started = false;
bool gameOver = false;
bool recentlystored = false;
bool usermovedblock = false;
    
// starting values outside of possible ranges, effectively null.
static int nextblock = 100;
static int nextblockstate = 100;
static int currentblock = 100;
static int currentblockstate = 100;
static int currentblockX = 0;
static int currentblockY = 0;
static int storedblock = 100;
static int storedblockstate = 100;

static int score = 0;
static int highscore = 0;
static int combo = 0;
static int linesForLevelIncrement = 0;

static int minutes = 0;
static int seconds = 0;

static int blockWidth;
static int blockHeight;

static HBITMAP hBitmap = NULL;
static HDC hdcMem = NULL;

POINT topleft;
POINT bottomright;

HBRUSH black;
HBRUSH cyan;
HBRUSH yellow;
HBRUSH pink;
HBRUSH orange;
HBRUSH purple;
HBRUSH red;
HBRUSH green;

// method meant to only be used in paint.
void placeBlockHere(POINT point, int blocknum, int statenum, HDC hdc) {
    HBRUSH black = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    switch (blocknum) {
    case (0): // Cyan line
    {   
        if (statenum > 1) statenum = statenum % 2;

        if (statenum == 0) {
            for (int i = 0; i < 4; i++) {
                RECT block = { point.x + i * blockWidth,point.y, point.x + (i + 1) * blockWidth,point.y + blockHeight };
                FillRect(hdc, &block, cyan);
                FrameRect(hdc, &block, black);
            }
        }
        if (statenum == 1) {
            for (int i = 0; i < 4; i++) {
                RECT block = { point.x ,point.y + i * blockHeight,point.x + blockWidth,point.y + (i+1) * blockHeight };
                FillRect(hdc, &block, cyan);
                FrameRect(hdc, &block, black);
            }
        }
    }
    break;
    
    case (1): // Yellow block
    {
        for (int i = 0; i < 2; i++) {
            RECT blockleft = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (1 + i) * blockHeight };
            FillRect(hdc, &blockleft, yellow);
            FrameRect(hdc, &blockleft, black);
            RECT blockright = { point.x + blockWidth,point.y + i * blockHeight,point.x + 2 * blockWidth,point.y + (1 + i) * blockHeight };
            FillRect(hdc, &blockright, yellow);
            FrameRect(hdc, &blockright, black);
        }
    }
    break;
    case (2): // Pink T
    {
        switch (statenum)
        {
            case 0:
            {
                RECT block = { point.x + blockWidth,point.y + blockHeight,point.x + 2 * blockWidth,point.y + 2 * blockHeight };
                FillRect(hdc, &block, pink);
                FrameRect(hdc, &block, black);
                for (int i = 0; i < 3; i++) {
                    RECT block = { point.x + i * blockWidth,point.y,point.x + (i+1) * blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, pink);
                    FrameRect(hdc, &block, black);
                }
            }
                break;
            case 1:
            {
                RECT block = { point.x + blockWidth,point.y + blockHeight,point.x + 2 * blockWidth,point.y + 2 * blockHeight };
                FillRect(hdc, &block, pink);
                FrameRect(hdc, &block, black);
                for (int i = 0; i < 3; i++) {
                    RECT block = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (i+1) * blockHeight };
                    FillRect(hdc, &block, pink);
                    FrameRect(hdc, &block, black);
                }
            }
                break;
            case 2:
            {
                RECT block = { point.x + blockWidth,point.y - blockHeight,point.x + 2 * blockWidth,point.y };
                FillRect(hdc, &block, pink);
                FrameRect(hdc, &block, black);
                for (int i = 0; i < 3; i++) {
                    RECT block = { point.x + i * blockWidth,point.y,point.x + (i+1) * blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, pink);
                    FrameRect(hdc, &block, black);
                }
            }
                break;
            case 3:
            {
                RECT block = { point.x,point.y,point.x + blockWidth,point.y + blockHeight };
                FillRect(hdc, &block, pink);
                FrameRect(hdc, &block, black);
                for (int i = 0; i < 3; i++) {
                    RECT block = { point.x + blockWidth,point.y + (i - 1) * blockHeight,point.x + 2 * blockWidth,point.y + i * blockHeight };
                    FillRect(hdc, &block, pink);
                    FrameRect(hdc, &block, black);
                }
            }
                break;
        }
    }
    break;
    case (3): // L-shaped block. Orange.
    {
        switch (statenum)
        {
        case 0:
        {
            RECT block = { point.x + 2 * blockWidth,point.y - blockHeight,point.x + 3 * blockWidth,point.y };
            FillRect(hdc, &block, orange);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + i * blockWidth,point.y,point.x + (i + 1) * blockWidth,point.y + blockHeight };
                FillRect(hdc, &block, orange);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 1:
        {
            RECT block = { point.x + blockWidth,point.y + 2 * blockHeight,point.x + 2 * blockWidth,point.y + 3 * blockHeight };
            FillRect(hdc, &block, orange);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (i + 1) * blockHeight };
                FillRect(hdc, &block, orange);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 2:
        {
            RECT block = { point.x,point.y + blockHeight,point.x + blockWidth,point.y + 2 * blockHeight };
            FillRect(hdc, &block, orange);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + i * blockWidth,point.y,point.x + (i + 1) * blockWidth,point.y + blockHeight };
                FillRect(hdc, &block, orange);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 3:
        {
            RECT block = { point.x,point.y,point.x + blockWidth,point.y + blockHeight };
            FillRect(hdc, &block, orange);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + blockWidth,point.y + i * blockHeight,point.x + 2 * blockWidth,point.y + (i + 1) * blockHeight };
                FillRect(hdc, &block, orange);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        }
    }
            break;
    case (4): // J-shaped block. Purple.
    {
        switch (statenum)
        {
        case 0:
        {
            RECT block = { point.x,point.y,point.x + blockWidth,point.y + blockHeight };
            FillRect(hdc, &block, purple);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + i * blockWidth,point.y + blockHeight,point.x + (i + 1) * blockWidth,point.y + 2 * blockHeight };
                FillRect(hdc, &block, purple);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 1:
        {
            RECT block = { point.x + blockWidth,point.y,point.x + 2 * blockWidth,point.y + blockHeight };
            FillRect(hdc, &block, purple);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (i + 1) * blockHeight };
                FillRect(hdc, &block, purple);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 2:
        {
            RECT block = { point.x + 2 * blockWidth,point.y + blockHeight,point.x + 3 * blockWidth,point.y + 2 * blockHeight };
            FillRect(hdc, &block, purple);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + i * blockWidth,point.y,point.x + (i + 1) * blockWidth,point.y + blockHeight };
                FillRect(hdc, &block, purple);
                FrameRect(hdc, &block, black);
            }
        }
        break;
        case 3:
        {
            RECT block = { point.x,point.y,point.x + blockWidth,point.y + blockHeight };
            FillRect(hdc, &block, purple);
            FrameRect(hdc, &block, black);
            for (int i = 0; i < 3; i++) {
                RECT block = { point.x + blockWidth,point.y + (i - 2) * blockHeight,point.x + 2 * blockWidth,point.y + (i - 1) * blockHeight };
                FillRect(hdc, &block, purple);
                FrameRect(hdc, &block, black);
            }
        }
        break;

        }
    }
            break;
    case (5): // Left zig-zag. Red
    {
        if (statenum > 1) statenum = statenum % 2;

        if (statenum == 0) {
            for (int i = 0; i < 2; i++) {
                RECT blocktop = { point.x + i * blockWidth,point.y,point.x + (i + 1) * blockWidth,point.y + blockHeight };
                FillRect(hdc, &blocktop, red);
                FrameRect(hdc, &blocktop, black);
                RECT blockbot = { point.x + (i + 1) * blockWidth,point.y + blockHeight,point.x + (i + 2) * blockWidth,point.y + 2 * blockHeight };
                FillRect(hdc, &blockbot, red);
                FrameRect(hdc, &blockbot, black);
            }
        }
        if (statenum == 1) {
            for (int i = 0; i < 2; i++) {
                RECT blockleft = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (1 + i) * blockHeight };
                FillRect(hdc, &blockleft, red);
                FrameRect(hdc, &blockleft, black);
                RECT blockright = { point.x + blockWidth,point.y + (i - 1) * blockHeight,point.x + 2 * blockWidth,point.y + i * blockHeight };
                FillRect(hdc, &blockright, red);
                FrameRect(hdc, &blockright, black);
            }
        }
    }
            break;
    case (6): // Right zig-zag. Green.
    {
        if (statenum > 1) statenum = statenum % 2;

        if (statenum == 0) {
            for (int i = 0; i < 2; i++) {
                RECT blockbot = { point.x + i * blockWidth,point.y,point.x + (i + 1) * blockWidth,point.y + 1 * blockHeight };
                FillRect(hdc, &blockbot, green);
                FrameRect(hdc, &blockbot, black);
                RECT blocktop = { point.x + (i + 1) * blockWidth,point.y - blockHeight,point.x + (i + 2) * blockWidth,point.y };
                FillRect(hdc, &blocktop, green);
                FrameRect(hdc, &blocktop, black);
            }
        }
        if (statenum == 1) {
            for (int i = 0; i < 2; i++) {
                RECT blockleft = { point.x,point.y + i * blockHeight,point.x + blockWidth,point.y + (1 + i) * blockHeight };
                FillRect(hdc, &blockleft, green);
                FrameRect(hdc, &blockleft, black);

                RECT blockright = { point.x + blockWidth,point.y + (i + 1) * blockHeight,point.x + 2 * blockWidth,point.y + (i + 2) * blockHeight };
                FillRect(hdc, &blockright, green);
                FrameRect(hdc, &blockright, black);
            }
        }
    }
            break;
        default:
            break;
    }
}

// Method for drawing the blocks that have been placed on screen.
void drawPlacedBlocks(HDC hdc) {
    for (int i = 0; i < gameHeight; i++) {
        for (int j = 0; j < gameWidth; j++) {
            if (field[i][j] <= 6) {
                POINT point = { topleft.x + j * blockWidth, topleft.y + i * blockHeight };
                if (field[i][j] == 0) 
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, cyan);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 1)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, yellow);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 2)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, pink);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 3)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, orange);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 4)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, purple);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 5)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, red);
                    FrameRect(hdc, &block, black);
                } else if (field[i][j] == 6)
                {
                    RECT block = { point.x,point.y, point.x + blockWidth,point.y + blockHeight };
                    FillRect(hdc, &block, green);
                    FrameRect(hdc, &block, black);
                }
            }
        }
    }
}

// method for checking if the current block is in contact with any existing placed blocks.
bool currentblockHitSomething()
{
    bool blockhitsomething = false;
    
    int x = (currentblockX - topleft.x) / blockWidth;
    int y = (currentblockY - topleft.y) / blockHeight;

    switch (currentblock) {
    case 0:
        currentblockstate = currentblockstate % 2;

        switch (currentblockstate) {
        case 0:
            {
                for (int i = 0; i < 4; i++) {
                    int num = field[y + 1][x + i];
                    if (y == gameHeight-1 || num <= 6) {
                        blockhitsomething = true;
                        break;
                    }
                }
                if (blockhitsomething) {
                    for (int i = 0; i < 4; i++) {
                        field[y][x + i] = currentblock;
                    }
                }
            }
            break;

        case 1:
            {
                // In this state only the bottom block can hit something.
                if ((y + 3) == gameHeight-1 || field[y + 4][x] <= 6) {
                    blockhitsomething = true;
                    for (int i = 0; i < 4; i++) {
                        field[y + i][x] = currentblock;
                    }
                }
            }
            break;
        }
        break;

    case 1:
        {
            // 1 case, so no switch.
            for (int i = 0; i < 2; i++) {
                if ((y + 1) == gameHeight - 1 || field[y + 2][x + i] <= 6) {
                    blockhitsomething = true;
                }
            }
            if (blockhitsomething) {
                for (int i = 0; i < 2; i++) {
                    field[y][x + i] = currentblock;
                    field[y + 1][x + i] = currentblock;
                }
            }
        }
        break;

    case 2:
    {
        switch (currentblockstate) {
        case 0:
        {
            if (y + 1 == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 2][x + 1] <= 6 || field[y + 1][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y][x + i] = currentblock;
                }
                field[y + 1][x + 1] = currentblock;
            }
        }
        break;

        case 1:
        {
            if (y + 2 == gameHeight - 1 || field[y + 3][x] <= 6 || field[y + 2][x + 1] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y + i][x] = currentblock;
                }
                field[y + 1][x + 1] = currentblock;
            }
        }
        break;

        case 2:
        {
            if (y == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 1][x + 1] <= 6 || field[y + 1][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y][x + i] = currentblock;
                }
                field[y - 1][x + 1] = currentblock;
            }
        }
        break;

        case 3:
        {
            if (y + 1 == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 2][x + 1] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y + (i - 1)][x + 1] = currentblock;
                }
                field[y][x] = currentblock;
            }
        }
        break;
        }
        break;
    }
    break;

    case 3:
    {
        switch (currentblockstate) {
        case 0:
        {
            if (y == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 1][x + 1] <= 6 || field[y + 1][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y][x + i] = currentblock;
                }
                field[y - 1][x + 2] = currentblock;
            }
        }
        break;

        case 1:
        {
            if (y + 2 == gameHeight - 1 || field[y + 3][x] <= 6 || field[y + 3][x + 1] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y + i][x] = currentblock;
                }
                field[y + 2][x + 1] = currentblock;
            }
        }
        break;

        case 2:
        {
            if (y+1 == gameHeight - 1 || field[y + 2][x] <= 6 || field[y + 1][x + 1] <= 6 || field[y + 1][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y][x + i] = currentblock;
                }
                field[y + 1][x] = currentblock;
            }
        }
        break;

        case 3:
        {
            if (y + 2 == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 3][x + 1] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y + i][x + 1] = currentblock;
                }
                field[y][x] = currentblock;
            }
        }
        break;
        }
        break;
    }
    break;

    case 4:
    {
        switch (currentblockstate) {
        case 0:
        {
            if (y+1 == gameHeight - 1 || field[y + 2][x] <= 6 || field[y + 2][x + 1] <= 6 || field[y + 2][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y+1][x + i] = currentblock;
                }
                field[y][x] = currentblock;
            }
        }
        break;

        case 1:
        {
            if (y + 2 == gameHeight - 1 || field[y + 3][x] <= 6 || field[y + 1][x + 1] <= 6) {
                blockhitsomething = true;
                
                for (int i = 0; i < 3; i++) {
                    field[y + i][x] = currentblock;
                }
                field[y][x + 1] = currentblock;
            }
        }
        break;

        case 2:
        {
            if (y + 1 == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 1][x + 1] <= 6 || field[y + 2][x + 2] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y][x + i] = currentblock;
                }
                field[y + 1][x+2] = currentblock;
            }
        }
        break;

        case 3:
        {
            if (y == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 1][x + 1] <= 6) {
                blockhitsomething = true;

                for (int i = 0; i < 3; i++) {
                    field[y + (i-2)][x + 1] = currentblock;
                }
                field[y][x] = currentblock;
            }
        }
        break;
        }
        break;
    }
    break;

    case 5:
        currentblockstate = currentblockstate % 2;

        switch (currentblockstate) {
        case 0:
        {
            if (y+1 == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 2][x + 1] <= 6 || field[y+2][x + 2] <= 6) {
                blockhitsomething = true;
                for (int i = 0; i < 2; i++) {
                    field[y][x + i] = currentblock;
                    field[y + 1][(x + 1) + i] = currentblock;
                }
            }
        }
        break;

        case 1:
        {
            if (y + 1 == gameHeight - 1 || field[y + 2][x] <= 6 || field[y+1][x+1] <= 6) {
                blockhitsomething = true;
                for (int i = 0; i < 2; i++) {
                    field[y + i][x] = currentblock;
                    field[(y - 1) + i][x + 1] = currentblock;
                }
            }
        }
        break;
        }
        break;

    case 6:
        currentblockstate = currentblockstate % 2;

        switch (currentblockstate) {
        case 0:
        {
            if (y == gameHeight - 1 || field[y + 1][x] <= 6 || field[y + 1][x + 1] <= 6 || field[y][x + 2] <= 6) {
                blockhitsomething = true;
                for (int i = 0; i < 2; i++) {
                    field[y-1][(x+1) + i] = currentblock;
                    field[y][x + i] = currentblock;
                }
            }
        }
        break;

        case 1:
        {
            if (y + 2 == gameHeight - 1 || field[y + 2][x] <= 6 || field[y + 3][x + 1] <= 6) {
                blockhitsomething = true;
                for (int i = 0; i < 2; i++) {
                    field[y + i][x] = currentblock;
                    field[(y+1) + i][x + 1] = currentblock;
                }
            }
        }
        break;
        }
        break;
    }
    return blockhitsomething;
}

// method for removing a given line and moving down the lines above it.
void clearLineAndMove(int line, HWND hWnd) {
    for (int i = line; i > 0; i--) {
        for (int j = 0; j < gameWidth; j++) {
            field[i][j] = field[i - 1][j];
        }
    }

    for (int j = 0; j < gameWidth; j++) {
        field[0][j] = 100;
    }
    InvalidateRect(hWnd, 0, 1);
}

void checkIfBlockIsBlocked(HWND hWnd) {
    int x = (currentblockX - topleft.x) / blockWidth;
    int y = (currentblockY - topleft.y) / blockHeight;
    bool tempGameOver = true;
    switch (currentblock) {
    case 0:
        currentblockstate = currentblockstate % 2;
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y][x + 3] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 3][x] > 6) tempGameOver = false;
            break;
        }
        break;
    case 1:
        if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y + 1][x] > 6 && field[y + 1][x + 1] > 6) tempGameOver = false;
        break;
    case 2:
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y+1][x + 1] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 1][x+1] > 6) tempGameOver = false;
            break;
        case 2:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y-1][x + 1] > 6) tempGameOver = false;
            break;

        case 3:
            if (field[y][x] > 6 && field[y + 1][x+1] > 6 && field[y][x+1] > 6 && field[y -1][x+1] > 6) tempGameOver = false;
            break;
        }
        break;
    case 3:
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y - 1][x + 2] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 2][x + 1] > 6) tempGameOver = false;
            break;
        case 2:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y + 1][x] > 6) tempGameOver = false;
            break;

        case 3:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y+1][x + 1] > 6 && field[y + 2][x + 1] > 6) tempGameOver = false;
            break;
        }
        break;
    case 4:
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y+1][x + 1] > 6 && field[y+1][x + 2] > 6 && field[y + 1][x] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y][x + 1] > 6) tempGameOver = false;
            break;
        case 2:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y + 1][x + 2] > 6) tempGameOver = false;
            break;

        case 3:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x - 1] > 6 && field[y][x - 2] > 6) tempGameOver = false;
            break;
        }
        break;
    case 5:
        currentblockstate = currentblockstate % 2;
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y+1][x + 1] > 6 && field[y+1][x + 2] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y][x+1] > 6 && field[y - 1][x+1] > 6) tempGameOver = false;
            break;
        }
        break;
    case 6:
        currentblockstate = currentblockstate % 2;
        switch (currentblockstate) {
        case 0:
            if (field[y][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6 && field[y - 1][x + 2] > 6) tempGameOver = false;
            break;

        case 1:
            if (field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 1][x + 1] > 6 && field[y + 2][x + 1] > 6) tempGameOver = false;
            break;
        }
        break;
    }
    if (tempGameOver) {
        gameOver = true;
        KillTimer(hWnd, 1);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    
    static int width = 0;
    static int height = 0;

    RECT window;
    GetClientRect(hWnd, &window);

    std::srand(std::time(nullptr));

    const int timerId = 1;
    const int speedtimerId = 2;
    static int speedtimerSpeed = 1000;

    static int gameTime = 0;
    static int level = 1;
    static int linesDone = 0;
    
    topleft = { (window.right / 2) - blockWidth * gameWidth / 2, (window.bottom / 2) - blockHeight * gameHeight / 2 };
    bottomright = { (window.right / 2) + blockWidth * gameWidth / 2, (window.bottom / 2) + blockHeight * gameHeight / 2 };

    if (window.right > 0) {
        blockWidth = ((window.right / 3) / (gameWidth*2.5))*2;
    }
    if (window.bottom > 0) {
        blockHeight = ((window.bottom / 2) / (gameHeight + 4))*2;
    }

    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_CREATE:
        {
            cyan = CreateSolidBrush(RGB(0, 255, 255));
            yellow = CreateSolidBrush(RGB(255, 255, 0));
            pink = CreateSolidBrush(RGB(207, 50, 255));
            orange = CreateSolidBrush(RGB(255, 120, 13));
            purple = CreateSolidBrush(RGB(93, 63, 211));
            red = CreateSolidBrush(RGB(255, 0, 0));
            green = CreateSolidBrush(RGB(0, 255, 0));
            black = (HBRUSH)GetStockObject(BLACK_BRUSH);

            for (int i = 0; i < gameHeight; i++) {
                for (int j = 0; j < gameWidth; j++) {
                    field[i][j] = 100;
                }
            }
        }
        break;

    case WM_TIMER:
        {
        switch (wParam) 
            {
        case timerId:
            seconds++;
            if (seconds >= 60) {
                seconds = seconds % 60;
                minutes++;
            }
            break;

        case speedtimerId:
            if (gameOver) {
                started = false;
            }

            if (started) {
                if (!usermovedblock && !currentblockHitSomething())
                {
                    currentblockY += blockHeight;
                } else {
                    if (!usermovedblock) {
                        int count = 0;
                        for (int i = 0; i < gameHeight; i++) {
                            bool lineFull = true;
                            for (int j = 0; j < gameWidth; j++) {
                                if (field[i][j] > 6) {
                                    lineFull = false;
                                    break;
                                }
                            }
                            if (lineFull) {
                                clearLineAndMove(i, hWnd);
                                linesDone++;
                                linesForLevelIncrement++;
                                count++;
                            }
                        }

                        if (count > 0) {
                            // 100 single, 300 double, 500 tripple, 800 tetris. Mutiplied by level. Combo = +50 per. 
                            switch (count) {
                            case 1:
                                score += 100 * level + 50 * combo;
                                break;
                            case 2:
                                score += 300 * level + 50 * combo;
                                break;
                            case 3:
                                score += 500 * level + 50 * combo;
                                break;
                            case 4:
                                score += 800 * level + 50 * combo;
                                break;
                            }
                            combo++;
                        }
                        else {
                            combo = 0;
                        }
                        count = 0;

                        currentblock = nextblock;
                        currentblockstate = nextblockstate;
                        nextblock = std::rand() % 7;
                        nextblockstate = std::rand() % 4;
                        recentlystored = false;

                        currentblockY = topleft.y;
                        currentblockX = topleft.x + 4 * blockWidth;
                        // Checks height of the blocks before placing them.
                        if (currentblockY == topleft.y) {
                            switch (currentblock) {
                            case 2:
                                if (currentblockstate > 1) {
                                    currentblockY += blockHeight;
                                }
                                break;
                            case 3:
                                if (currentblockstate == 0) {
                                    currentblockY += blockHeight;
                                }
                                break;
                            case 4:
                                if (currentblockstate == 3) {
                                    currentblockY += 2 * blockHeight;
                                }
                                break;
                            case 5:
                                if (currentblockstate == 1) {
                                    currentblockY += blockHeight;
                                }
                                break;
                            case 6:
                                if (currentblockstate == 0) {
                                    currentblockY += blockHeight;
                                }
                                break;
                            default:
                                break;
                            }
                        }
                        checkIfBlockIsBlocked(hWnd);
                    }
                    else {
                        usermovedblock = false;
                    }
                }
                if ((linesForLevelIncrement / level) >= 10 && speedtimerSpeed > 200) {
                    linesForLevelIncrement -= level * 10;
                    level++;
                    speedtimerSpeed -= 80;
                    KillTimer(hWnd, speedtimerId); // Stop the current timer
                    SetTimer(hWnd, speedtimerId, speedtimerSpeed, NULL);
                }
            }
            
            break;
            }
        InvalidateRect(hWnd, 0, 1);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Adding a buffer to prevent flickering.
            if (!hdcMem) {
                HDC hdc = GetDC(hWnd);
                hdcMem = CreateCompatibleDC(hdc);
                hBitmap = CreateCompatibleBitmap(hdc, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top);
                SelectObject(hdcMem, hBitmap);
                ReleaseDC(hWnd, hdc);
            }

            // No need to use DeleteObject on brushes obtained from GetStockObject as they are fetched, not created.
            HBRUSH gray = (HBRUSH)GetStockObject(GRAY_BRUSH);
            HBRUSH hOrg = (HBRUSH)SelectObject(hdcMem, black);

            RECT game = { topleft.x, 
                          topleft.y,
                          bottomright.x, 
                          bottomright.y};
            FillRect(hdcMem, &game, black);

            // Making blocks around the game area to make it easier to see/decoration.
            for (int i = 0; i < gameHeight; i++) { 

                RECT blockleft = { topleft.x - blockWidth,topleft.y + i*blockHeight,topleft.x,topleft.y + i*blockHeight + blockHeight };
                FillRect(hdcMem, &blockleft, gray);
                FrameRect(hdcMem, &blockleft, black);

                RECT blockright = { bottomright.x,topleft.y + i * blockHeight,bottomright.x + blockWidth,topleft.y + i * blockHeight + blockHeight };
                FillRect(hdcMem, &blockright, gray);
                FrameRect(hdcMem, &blockright, black);
            }
            for (int i = 0; i < (gameWidth+2); i++) { // + 2 to fill in the corners
                RECT blocktop = { topleft.x + i*blockWidth - blockWidth,topleft.y - blockHeight,topleft.x + i * blockWidth,topleft.y};
                FillRect(hdcMem, &blocktop, gray);
                FrameRect(hdcMem, &blocktop, black);

                RECT blockbot = { topleft.x + i * blockWidth - blockWidth,bottomright.y,topleft.x + i * blockWidth, bottomright.y + blockHeight};
                FillRect(hdcMem, &blockbot, gray);
                FrameRect(hdcMem, &blockbot, black);
            }

            // Making the next, lines done and timer block windows.

            RECT next = { topleft.x - 5*blockWidth,
                          topleft.y,
                          topleft.x - blockWidth,
                          topleft.y + 6*blockHeight };
            FillRect(hdcMem, &next, black);

            for (int i = 5; i > 0; i--) {
                RECT blocktop = { topleft.x - i * blockWidth - blockWidth,topleft.y - blockHeight,topleft.x - i * blockWidth,topleft.y };
                FillRect(hdcMem, &blocktop, gray);
                FrameRect(hdcMem, &blocktop, black);

                RECT blockbot = { topleft.x - i * blockWidth - blockWidth,topleft.y + 5*blockHeight,topleft.x - i * blockWidth,topleft.y + 6*blockHeight };
                FillRect(hdcMem, &blockbot, gray);
                FrameRect(hdcMem, &blockbot, black);

                RECT blockbottimer = { topleft.x - i * blockWidth - blockWidth,topleft.y + 7 * blockHeight,topleft.x - i * blockWidth,topleft.y + 8 * blockHeight };
                FillRect(hdcMem, &blockbottimer, gray);
                FrameRect(hdcMem, &blockbottimer, black);

                RECT botlinesdone = { topleft.x - i * blockWidth - blockWidth,topleft.y + 9 * blockHeight,topleft.x - i * blockWidth,topleft.y + 10 * blockHeight };
                FillRect(hdcMem, &botlinesdone, gray);
                FrameRect(hdcMem, &botlinesdone, black);

                RECT botlevel = { topleft.x - i * blockWidth - blockWidth,topleft.y + 11 * blockHeight,topleft.x - i * blockWidth,topleft.y + 12 * blockHeight };
                FillRect(hdcMem, &botlevel, gray);
                FrameRect(hdcMem, &botlevel, black);
            }

            for (int i = 11; i > 0; i--) {
                RECT blockleft = { topleft.x - 6 * blockWidth,topleft.y + (i - 1) * blockHeight,topleft.x - 5 * blockWidth,topleft.y + (i - 1) * blockHeight + blockHeight };
                FillRect(hdcMem, &blockleft, gray);
                FrameRect(hdcMem, &blockleft, black);
            }

            RECT textbox = { topleft.x - 5 * blockWidth, topleft.y, topleft.x - blockWidth, topleft.y + blockHeight };
            FillRect(hdcMem, &textbox, black);
            SetBkMode(hdcMem, TRANSPARENT);
            SetTextColor(hdcMem, RGB(255,255,255));
            HFONT hFont = CreateFont(
                blockHeight,    // Height of the font
                blockWidth/1.8, // Average character width
                0,              // Angle of escapement
                0,              // Base-line orientation angle
                FW_BOLD,        // Font weight
                FALSE,          // Italic
                FALSE,          // Underline
                FALSE,          // Strikeout
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                0,
                L"Futura Black"        // Font name
            );
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);
            DrawText(hdcMem, L"NEXT", -1, &textbox, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);

            switch (nextblock) {
                case (0):
                {
                    nextblockstate = nextblockstate % 2;

                    if (nextblockstate == 0) {
                        POINT point = { topleft.x - 5 * blockWidth,topleft.y + 2.5 * blockHeight};
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                    if (nextblockstate == 1) {
                        POINT point = { topleft.x - 3.5 * blockWidth,topleft.y + blockHeight };
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                }
                    break;
                case (1):
                {
                    // since 2*2 block only has one case, no need for more than one.
                    POINT point = { topleft.x - 4 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                }
                    break;
                case (2):
                {
                    switch (nextblockstate)
                    {
                        case 0:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 2 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 1:
                        {
                            POINT point = { topleft.x - 4 * blockWidth,topleft.y + 1.5 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 2:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 3 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 3:
                            {
                                POINT point = { topleft.x - 4 * blockWidth,topleft.y + 2.5 * blockHeight };
                                placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                            }
                            break;
                    }
                }
                    break;
                case (3):
                {
                    switch (nextblockstate)
                    {
                        case 0:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 3 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 1:
                        {
                            POINT point = { topleft.x - 4 * blockWidth,topleft.y + 1.5 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 2:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 2 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 3:
                        {
                            POINT point = { topleft.x - 4 * blockWidth,topleft.y + 1.5 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                    }
                }
                    break;
                case (4):
                    switch (nextblockstate)
                    {
                        case 0:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 2 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 1:
                        {
                            POINT point = { topleft.x - 4 * blockWidth,topleft.y + 1.5 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 2:
                        {
                            POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 2 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                        case 3:
                        {
                            POINT point = { topleft.x - 4 * blockWidth,topleft.y + 3.5 * blockHeight };
                            placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                        }
                            break;
                    }
                        break;
                case (5):
                    nextblockstate = nextblockstate % 2;

                    if (nextblockstate == 0) {
                        POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 2 * blockHeight };
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                    if (nextblockstate == 1) {
                        POINT point = { topleft.x - 4 * blockWidth,topleft.y + 2.5 * blockHeight };
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                    break;
                case (6):
                    nextblockstate = nextblockstate % 2;

                    if (nextblockstate == 0) {
                        POINT point = { topleft.x - 4.5 * blockWidth,topleft.y + 3 * blockHeight };
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                    if (nextblockstate == 1) {
                        POINT point = { topleft.x - 4 * blockWidth,topleft.y + 1.5 * blockHeight };
                        placeBlockHere(point, nextblock, nextblockstate, hdcMem);
                    }
                    break;
                default:
                    break;
            }

            // Making the stored block and score windows.

            RECT stored = { bottomright.x + blockWidth,
                          topleft.y,
                          bottomright.x + 5 * blockWidth,
                          topleft.y + 5 * blockHeight };
            FillRect(hdcMem, &stored, black);

            for (int i = 5; i > 0; i--) {
                RECT blocktop = { bottomright.x + i * blockWidth,topleft.y - blockHeight,bottomright.x + (i+1) * blockWidth,topleft.y };
                FillRect(hdcMem, &blocktop, gray);
                FrameRect(hdcMem, &blocktop, black);

                RECT blockright = { bottomright.x + 5 * blockWidth,topleft.y + (i - 1) * blockHeight,bottomright.x + 6 * blockWidth,topleft.y + (i - 1) * blockHeight + blockHeight };
                FillRect(hdcMem, &blockright, gray);
                FrameRect(hdcMem, &blockright, black);
            }

            for (int i = 9; i > 0; i--) {
                RECT blockbot = { bottomright.x + i * blockWidth - blockWidth,topleft.y + 5 * blockHeight,bottomright.x + i * blockWidth,topleft.y + 6 * blockHeight };
                FillRect(hdcMem, &blockbot, gray);
                FrameRect(hdcMem, &blockbot, black);

                RECT blockscorebot = { bottomright.x + i * blockWidth - blockWidth,topleft.y + 7 * blockHeight,bottomright.x + i * blockWidth,topleft.y + 8 * blockHeight };
                FillRect(hdcMem, &blockscorebot, gray);
                FrameRect(hdcMem, &blockscorebot, black);

                RECT blockhighscorebot = { bottomright.x + i * blockWidth - blockWidth,topleft.y + 9 * blockHeight,bottomright.x + i * blockWidth,topleft.y + 10 * blockHeight };
                FillRect(hdcMem, &blockhighscorebot, gray);
                FrameRect(hdcMem, &blockhighscorebot, black);

                RECT blockcontrolsbot = { bottomright.x + i * blockWidth - blockWidth,topleft.y + 20 * blockHeight,bottomright.x + i * blockWidth,topleft.y + 21 * blockHeight };
                FillRect(hdcMem, &blockcontrolsbot, gray);
                FrameRect(hdcMem, &blockcontrolsbot, black);
            }

            for (int i = 15; i > 0; i--) {
                RECT blockrightscore = { bottomright.x + 8 * blockWidth,topleft.y + (i + 4) * blockHeight,bottomright.x + 9 * blockWidth,topleft.y + (i + 4) * blockHeight + blockHeight };
                FillRect(hdcMem, &blockrightscore, gray);
                FrameRect(hdcMem, &blockrightscore, black);
            }
                

            RECT textboxstored = { bottomright.x + 2 * blockWidth, topleft.y, bottomright.x + 4 * blockWidth, topleft.y + blockHeight };
            FillRect(hdcMem, &textboxstored, black);
            DrawText(hdcMem, L"HOLD", -1, &textboxstored, DT_CENTER | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP);

            // Making the timer and lines done text
            RECT timer = { topleft.x - 5 * blockWidth, topleft.y + 6 * blockHeight, topleft.x - blockWidth, topleft.y + 7 * blockHeight };
            FillRect(hdcMem, &timer, black);
            std::wstring timerText = std::to_wstring(minutes) + L" : " + std::to_wstring(seconds);
            DrawText(hdcMem, timerText.c_str(), -1, &timer, DT_CENTER);

            RECT linesdonebox = { topleft.x - 5 * blockWidth, topleft.y + 8 * blockHeight, topleft.x - blockWidth, topleft.y + 9 * blockHeight };
            FillRect(hdcMem, &linesdonebox, black);
            std::wstring lines = std::to_wstring(linesDone);
            DrawText(hdcMem, lines.c_str(), -1, &linesdonebox, DT_CENTER);

            RECT levelbox = { topleft.x - 5 * blockWidth, topleft.y + 10 * blockHeight, topleft.x - blockWidth, topleft.y + 11 * blockHeight };
            FillRect(hdcMem, &levelbox, black);
            std::wstring levelstring = L"Lvl: " + std::to_wstring(level);
            DrawText(hdcMem, levelstring.c_str(), -1, &levelbox, DT_CENTER);
            

            // Making the scores show up on screen.

            HFONT scoreFont = CreateFont(
                blockHeight,    // Height of the font
                blockWidth / 3, // Average character width
                0,              // Angle of escapement
                0,              // Base-line orientation angle
                FW_BOLD,        // Font weight
                FALSE,          // Italic
                FALSE,          // Underline
                FALSE,          // Strikeout
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                0,
                L"Futura Black" // Font name
            );
            SelectObject(hdcMem, scoreFont);

            RECT scoreTextRect = { bottomright.x + blockWidth, topleft.y + 6 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 7 * blockHeight };
            FillRect(hdcMem, &scoreTextRect, black);
            std::wstring scoreText = L"Score: " + std::to_wstring(score);
            DrawText(hdcMem, scoreText.c_str(), -1, &scoreTextRect, DT_LEFT);

            RECT highscoreTextRect = { bottomright.x + blockWidth, topleft.y + 8 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 9 * blockHeight };
            FillRect(hdcMem, &highscoreTextRect, black);
            std::wstring highscoreText = L"High Score: " + std::to_wstring(highscore);
            DrawText(hdcMem, highscoreText.c_str(), -1, &highscoreTextRect, DT_LEFT);

            // Text for the control panel:

            RECT controlsTitleRect = { bottomright.x + blockWidth, topleft.y + 10 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 11 * blockHeight };
            FillRect(hdcMem, &controlsTitleRect, black);
            DrawText(hdcMem, L"Controls:", -1, &controlsTitleRect, DT_CENTER);

            SelectObject(hdcMem, hOldFont);

            RECT controlsStoreRect = { bottomright.x + blockWidth, topleft.y + 11.5 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 12.5 * blockHeight };
            FillRect(hdcMem, &controlsStoreRect, black);
            DrawText(hdcMem, L"Hit TAB to store.", -1, &controlsStoreRect, DT_LEFT);

            RECT controlsMoveRect1 = { bottomright.x + blockWidth, topleft.y + 13.5 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 14.5 * blockHeight };
            FillRect(hdcMem, &controlsMoveRect1, black);
            DrawText(hdcMem, L"ASD or Left, Down and Right", -1, &controlsMoveRect1, DT_LEFT);

            RECT controlsMoveRect2 = { bottomright.x + blockWidth, topleft.y + 14.3 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 15.3 * blockHeight };
            FillRect(hdcMem, &controlsMoveRect2, black);
            DrawText(hdcMem, L"arrows to move.", -1, &controlsMoveRect2, DT_LEFT);

            RECT controlsFlipRect = { bottomright.x + blockWidth, topleft.y + 16.3 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 17.3 * blockHeight };
            FillRect(hdcMem, &controlsFlipRect, black);
            DrawText(hdcMem, L"W or Up arrow to flip.", -1, &controlsFlipRect, DT_LEFT);

            RECT controlsSpaceRect1 = { bottomright.x + blockWidth, topleft.y + 18.3 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 19.3 * blockHeight };
            FillRect(hdcMem, &controlsSpaceRect1, black);
            DrawText(hdcMem, L"Hit Space to go all the", -1, &controlsSpaceRect1, DT_LEFT);

            RECT controlsSpaceRect2 = { bottomright.x + blockWidth, topleft.y + 19.1 * blockHeight, bottomright.x + 8 * blockWidth, topleft.y + 19.9 * blockHeight };
            FillRect(hdcMem, &controlsSpaceRect2, black);
            DrawText(hdcMem, L"way down instantly", -1, &controlsSpaceRect2, DT_LEFT);

            SetTextColor(hdcMem, BLACK_BRUSH);

            // Should really make it a method instead of copy-pasting, but would take more time than its worth as it is only ever used twice.

            switch (storedblock) {
            case (0):
            {
                storedblockstate = storedblockstate % 2;

                if (storedblockstate == 0) {
                    POINT point = { topleft.x + 11 * blockWidth,topleft.y + 2.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                if (storedblockstate == 1) {
                    POINT point = { topleft.x + 12.5 * blockWidth,topleft.y + blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
            }
            break;
            case (1):
            {
                // since 2*2 block only has one case, no need for more than one.
                POINT point = { topleft.x + 12 * blockWidth,topleft.y + 2 * blockHeight };
                placeBlockHere(point, storedblock, storedblockstate, hdcMem);
            }
            break;
            case (2):
            {
                switch (storedblockstate)
                {
                case 0:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 1:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 1.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 2:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 3 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 3:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 2.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                }
            }
            break;
            case (3):
            {
                switch (storedblockstate)
                {
                case 0:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 3 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 1:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 1.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 2:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 3:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 1.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                }
            }
            break;
            case (4):
                switch (storedblockstate)
                {
                case 0:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 1:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 1.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 2:
                {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                case 3:
                {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 3.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
                }
                break;
            case (5):
                storedblockstate = storedblockstate % 2;

                if (storedblockstate == 0) {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 2 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                if (storedblockstate == 1) {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 2.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
            case (6):
                storedblockstate = storedblockstate % 2;

                if (storedblockstate == 0) {
                    POINT point = { topleft.x + 11.5 * blockWidth,topleft.y + 3 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                if (storedblockstate == 1) {
                    POINT point = { topleft.x + 12 * blockWidth,topleft.y + 1.5 * blockHeight };
                    placeBlockHere(point, storedblock, storedblockstate, hdcMem);
                }
                break;
            default:
                break;
            }

            //Text for when game is yet to start.

            if (!started) {
                RECT textbox = {topleft.x, window.bottom/2 - blockHeight/2, bottomright.x, window.bottom/2+blockHeight/2};
                FillRect(hdcMem, &textbox, hOrg);
                DrawText(hdcMem, L"Press enter to start.", -1, &textbox, DT_CENTER);
                if (gameOver) {
                    RECT result = { topleft.x, window.bottom / 2 + blockHeight, bottomright.x, window.bottom / 2 + 2 * blockHeight };
                    FillRect(hdcMem, &result, hOrg);
                    std::wstring resultText;
                    if (score < highscore) {
                        resultText = L"Game Over. Score: " + std::to_wstring(score);
                        
                    }
                    else {
                        resultText = L"Game Over. Score: " + std::to_wstring(score) + L", New Highscore!";
                        highscore = score;
                    }
                    DrawText(hdcMem, resultText.c_str(), -1, &result, DT_CENTER);
                }
            }
            else {
                POINT currentpos = { currentblockX, currentblockY };

               if(!gameOver) {
                placeBlockHere(currentpos, currentblock, currentblockstate, hdcMem);
                }
                drawPlacedBlocks(hdcMem);
            }
            
            BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOrg);
            DeleteObject(hFont);
            DeleteObject(scoreFont);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_SIZE:
    {
        blockWidth = (window.right / 3) / (gameWidth * 2.5);
        blockHeight = (window.bottom / 2) / (gameHeight + 4);
        currentblockX = topleft.x + 4 * blockWidth;
        currentblockY = topleft.y;

        width = LOWORD(lParam);
        height = HIWORD(lParam);

        if (hdcMem) {
            DeleteDC(hdcMem);
            hdcMem = NULL;
        }

        if (hBitmap) {
            DeleteObject(hBitmap);
            hBitmap = NULL;
        }

        // Create a new memory device context and bitmap
        HDC hdc = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hdcMem, hBitmap);
        ReleaseDC(hWnd, hdc);
        break;
    }

    case WM_KEYDOWN:
        {
        switch (wParam)
        {
        case VK_RETURN:
            if (!started) {
                KillTimer(hWnd, timerId);
                KillTimer(hWnd, speedtimerId);
                speedtimerSpeed = 1000;
                seconds = 0;
                minutes = 0;
                SetTimer(hWnd, timerId, 1000, 0);
                SetTimer(hWnd, speedtimerId, speedtimerSpeed, 0);

                //PlaySound(L"music.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);


                nextblock = std::rand() % 7;
                currentblock = std::rand() % 7;
                storedblock = 9999; // number so high it can't correspond to a block type, hence empty.

                nextblockstate = std::rand() % 4;
                currentblockstate = std::rand() % 4;

                currentblockX = topleft.x + 4 * blockWidth;
                currentblockY = topleft.y;
                // Checks height of the blocks before placing them.
                if (currentblockY == topleft.y) {
                    switch (currentblock) {
                    case 2:
                        if (currentblockstate > 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 3:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 4:
                        if (currentblockstate == 3) {
                            currentblockY += 2 * blockHeight;
                        }
                        break;
                    case 5:
                        if (currentblockstate == 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 6:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    default:
                        break;
                    }
                }
                started = true;
                gameOver = false;
                recentlystored = false;
                usermovedblock = false;
                score = 0;
                linesDone = 0;
                linesForLevelIncrement = 0;
                level = 1;
                combo = 0;
                for (int i = 0; i < gameHeight; i++) {
                    for (int j = 0; j < gameWidth; j++) {
                        field[i][j] = 100;
                    }
                }
            }
            break;
        case VK_TAB:
            if (!recentlystored && started) {
                int temp = currentblock;
                int tempstate = currentblockstate;
                if (storedblock <= 6) {
                    currentblock = storedblock;
                    currentblockstate = storedblockstate;
                }
                else {
                    currentblock = nextblock;
                    currentblockstate = nextblockstate;
                    nextblock = std::rand() % 7;
                    nextblockstate = std::rand() % 4;
                }
                storedblock = temp;
                storedblockstate = tempstate;
                currentblockY = topleft.y;
                currentblockX = topleft.x + 4 * blockWidth;
                // Checks height of the blocks before placing them.
                if (currentblockY == topleft.y) {
                    switch (currentblock) {
                    case 2:
                        if (currentblockstate > 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 3:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 4:
                        if (currentblockstate == 3) {
                            currentblockY += 2 * blockHeight;
                        }
                        break;
                    case 5:
                        if (currentblockstate == 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 6:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    default:
                        break;
                    }
                }
                checkIfBlockIsBlocked(hWnd);
                recentlystored = true;
            }
            break;
        case VK_LEFT:
        case 'A':
        case 'a':
            if (started) {
                if (currentblockX > topleft.x) {
                    int x = (currentblockX - topleft.x) / blockWidth;
                    int y = (currentblockY - topleft.y) / blockHeight;

                    switch (currentblock) {
                    case 0:
                    {
                        currentblockstate = currentblockstate % 2;
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                            bool clear = true;
                            for (int i = 0; i < 4; i++) {
                                if (field[y + i][x - 1] <= 6) {
                                    clear = false;
                                }
                            }
                            if (clear) {
                                currentblockX -= blockWidth;
                            }
                            break;
                        }
                    }
                        break;

                    case 1:
                    {
                        bool clear = true;
                        for (int i = 0; i < 2; i++) {
                            if (field[y + i][x - 1] <= 6) {
                                clear = false;
                            }
                        }
                        if (clear) {
                            currentblockX -= blockWidth;
                        }
                    }
                        break;

                    case 2:
                    {
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6 && field[y + 1][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                        {
                            bool clear = true;
                            for (int i = 0; i < 3; i++) {
                                if (field[y + i][x - 1] <= 6) {
                                    clear = false;
                                }
                            }
                            if (clear) {
                                currentblockX -= blockWidth;
                            }
                        }
                            break;

                        case 2:
                            if (field[y][x - 1] > 6 && field[y - 1][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 3:
                            if (field[y - 1][x] > 6 && field[y][x - 1] > 6 && field[y + 1][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;
                        }
                    }
                        break;

                    case 3:
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6 && field[y - 1][x + 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                            if (field[y][x - 1] > 6 && field[y + 1][x - 1] > 6 && field[y + 2][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 2:
                            if (field[y][x - 1] > 6 && field[y + 1][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 3:
                            if (field[y][x - 1] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;
                        }
                        break;

                    case 4:
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6 && field[y + 1][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                            if (field[y][x - 1] > 6 && field[y + 1][x - 1] > 6 && field[y + 2][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 2:
                            if (field[y][x - 1] > 6 && field[y + 1][x + 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 3:
                            if (field[y][x - 1] > 6 && field[y - 1][x] > 6 && field[y - 2][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;
                        }
                        break;

                    case 5:
                        currentblockstate = currentblockstate % 2;
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6 && field[y + 1][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                            if (field[y - 1][x] > 6 && field[y][x - 1] > 6 && field[y + 1][x - 1] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;
                        }
                        break;

                    case 6:
                        currentblockstate = currentblockstate % 2;
                        switch (currentblockstate) {
                        case 0:
                            if (field[y][x - 1] > 6 && field[y - 1][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                            break;

                        case 1:
                            if (field[y][x - 1] > 6 && field[y + 1][x - 1] > 6 && field[y + 2][x] > 6) {
                                currentblockX -= blockWidth;
                            }
                        }
                        break;
                    }
                }
                else {
                    currentblockX = topleft.x;
                }
                break;
            }

        case VK_RIGHT:
        case 'D':
        case 'd':
            if (started) {
                int x = (currentblockX - topleft.x) / blockWidth;
                int y = (currentblockY - topleft.y) / blockHeight;
                switch (currentblock)
                {
                case 0:

                    if (currentblockstate > 1) currentblockstate = currentblockstate % 2;

                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 5 * blockWidth <= bottomright.x && field[y][x+4] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 2 * blockWidth <= bottomright.x && field[y][x+1] > 6 && field[y+1][x+1] > 6 && field[y+2][x+1] > 6 && field[y+3][x+1] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;

                case 1:
                    if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+2] > 6 && field[y+1][x+2] > 6) {
                        currentblockX += blockWidth;
                    }
                    break;

                case 2:
                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+3] > 6 && field[y+1][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+1] > 6 && field[y+1][x+2] > 6 && field[y+2][x+1] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 2:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+3] > 6 && field[y-1][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 3:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y-1][x+2] > 6 && field[y][x+2] > 6 && field[y+1][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;

                case 3:
                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+3] > 6 && field[y-1][x+3] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+1] > 6 && field[y+1][x+1] > 6 && field[y+2][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 2:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+3] > 6 && field[y+1][x+1] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 3:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+2] > 6 && field[y+1][x+2] > 6 && field[y+2][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;

                case 4:
                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+1] > 6 && field[y+1][x+3] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+2] > 6 && field[y+1][x+1] > 6 && field[y+2][x+1] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 2:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+3] > 6 && field[y+1][x+3] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 3:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y-2][x+2] > 6 && field[y-1][x+2] > 6 && field[y][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;

                case 5:

                    if (currentblockstate > 1) currentblockstate = currentblockstate % 2;

                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y][x+2] > 6 && field[y+1][x+3] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y-1][x+2] > 6 && field[y][x+2] > 6 && field[y+1][x+1] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;

                case 6:

                    if (currentblockstate > 1) currentblockstate = currentblockstate % 2;

                    switch (currentblockstate) {
                    case 0:
                        if (currentblockX + 4 * blockWidth <= bottomright.x && field[y-1][x+3] > 6 && field[y][x+2] > 6) {
                            currentblockX += blockWidth;
                        }
                        break;

                    case 1:
                        if (currentblockX + 3 * blockWidth <= bottomright.x && field[y][x+1] > 6 && field[y+1][x+2] > 6 && field[y+2][x+2] > 6 ) {
                            currentblockX += blockWidth;
                        }
                        break;
                    }
                    break;
                }
                break;
            }
            break;

        case VK_DOWN:
        case 'S':
        case 's':
            if (started && !gameOver) {
                if (!currentblockHitSomething())
                {
                    currentblockY += blockHeight;
                    score++;
                }
                else {
                    int count = 0;
                    for (int i = 0; i < gameHeight; i++) {
                        bool lineFull = true;
                        for (int j = 0; j < gameWidth; j++) {
                            if (field[i][j] > 6) {
                                lineFull = false;
                                break;
                            }
                        }
                        if (lineFull) {
                            clearLineAndMove(i, hWnd);
                            count++;
                            linesDone++;
                            linesForLevelIncrement++;
                        }
                    }

                    if (count > 0) {
                        // 100 single, 300 double, 500 tripple, 800 tetris. Mutiplied by level. Combo = +50 per.
                        
                        switch (count) {
                        case 1:
                            score += 100 * level + 50 * combo;
                            break;
                        case 2:
                            score += 300 * level + 50 * combo;
                            break;
                        case 3:
                            score += 500 * level + 50 * combo;
                            break;
                        case 4:
                            score += 800 * level + 50 * combo;
                            break;
                        }
                        combo++;
                    }
                    else {
                        combo = 0;
                    }
                    count = 0;

                    currentblock = nextblock;
                    currentblockstate = nextblockstate;
                    nextblock = std::rand() % 7;
                    nextblockstate = std::rand() % 4;
                    recentlystored = false;

                    currentblockY = topleft.y;
                    currentblockX = topleft.x + 4 * blockWidth;
                    // Checks height of the blocks before placing them.
                    if (currentblockY == topleft.y) {
                        switch (currentblock) {
                        case 2:
                            if (currentblockstate > 1) {
                                currentblockY += blockHeight;
                            }
                            break;
                        case 3:
                            if (currentblockstate == 0) {
                                currentblockY += blockHeight;
                            }
                            break;
                        case 4:
                            if (currentblockstate == 3) {
                                currentblockY += 2 * blockHeight;
                            }
                            break;
                        case 5:
                            if (currentblockstate == 1) {
                                currentblockY += blockHeight;
                            }
                            break;
                        case 6:
                            if (currentblockstate == 0) {
                                currentblockY += blockHeight;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    checkIfBlockIsBlocked(hWnd);
                }
            usermovedblock = true;
            }
            break;

        case VK_SPACE:
        {
            if (started) {
                while (!currentblockHitSomething()) {
                    currentblockY += blockHeight;
                    score++;
                }
                int count = 0;
                for (int i = 0; i < gameHeight; i++) {
                    bool lineFull = true;
                    for (int j = 0; j < gameWidth; j++) {
                        if (field[i][j] > 6) {
                            lineFull = false;
                            break;
                        }
                    }
                    if (lineFull) {
                        clearLineAndMove(i, hWnd);
                        count++;
                        linesDone++;
                        linesForLevelIncrement++;
                    }
                }

                if (count > 0) {
                    // 100 single, 300 double, 500 tripple, 800 tetris. Mutiplied by level. Combo = +50 per.

                    switch (count) {
                    case 1:
                        score += 100 * level + 50 * combo;
                        break;
                    case 2:
                        score += 300 * level + 50 * combo;
                        break;
                    case 3:
                        score += 500 * level + 50 * combo;
                        break;
                    case 4:
                        score += 800 * level + 50 * combo;
                        break;
                    }
                    combo++;
                }
                else {
                    combo = 0;
                }
                count = 0;

                currentblock = nextblock;
                currentblockstate = nextblockstate;
                nextblock = std::rand() % 7;
                nextblockstate = std::rand() % 4;
                recentlystored = false;

                currentblockY = topleft.y;
                currentblockX = topleft.x + 4 * blockWidth;
                // Checks height of the blocks before placing them.
                if (currentblockY == topleft.y) {
                    switch (currentblock) {
                    case 2:
                        if (currentblockstate > 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 3:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 4:
                        if (currentblockstate == 3) {
                            currentblockY += 2 * blockHeight;
                        }
                        break;
                    case 5:
                        if (currentblockstate == 1) {
                            currentblockY += blockHeight;
                        }
                        break;
                    case 6:
                        if (currentblockstate == 0) {
                            currentblockY += blockHeight;
                        }
                        break;
                    default:
                        break;
                    }
                }
                checkIfBlockIsBlocked(hWnd);
                usermovedblock = true;
            }
        }
            break;

        case VK_UP:
        case 'W':
        case 'w':
        {
            if (started) {
                int x = (currentblockX - topleft.x) / blockWidth;
                int y = (currentblockY - topleft.y) / blockHeight;

                switch (currentblock)
                {
                case 0:
                {
                    currentblockstate = currentblockstate % 2;
                    switch (currentblockstate) {
                    case 0:
                    {
                        if (y >= 0 && y + 3 <= gameHeight - 1 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 3][x] > 6) {
                            currentblockstate++;
                        }
                        else {
                            bool clear = false;
                            int orgY = currentblockY;
                            do {
                                if (y >= 0 && y + 3 <= gameHeight - 1 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 3][x] > 6) {
                                    clear = true;
                                }
                                else {
                                    currentblockY -= blockHeight;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                            } while (!clear && currentblockY >= topleft.y);
                            if (clear) {
                                currentblockstate++;
                            }
                            else {
                                currentblockY = orgY;
                            }
                        }
                    }
                        break;

                    case 1:
                    {
                        if (x + 3 <= gameWidth - 1 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y][x + 3] > 6) {
                            currentblockstate++;
                        }
                        else {
                            bool done = false;
                            int orgX = currentblockX;
                            int orgY = currentblockY;
                            if (x >= 0 && field[y][x-1] > 6) {
                                bool clear = false;
                                bool possible = true;
                                do {
                                    if (x + 3 <= gameWidth - 1 && field[y][x]  > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y][x + 3] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        if (x - 1 <= 0) {
                                            possible = false;
                                        }
                                        else {
                                            if (field[y][x - 1] > 6) {
                                                currentblockX -= blockWidth;
                                                x = (currentblockX - topleft.x) / blockWidth;
                                            }
                                            else {
                                                possible = false;
                                            }
                                        }
                                    }
                                } while (!clear && possible && currentblockX >= topleft.x);
                                if (clear) {
                                    currentblockstate++;
                                    done = true;
                                }
                            } 
                            if (!done) {
                                currentblockX = orgX;
                                x = (currentblockX - topleft.x) / blockWidth;

                                bool clear = false;
                                do {
                                    if (x + 3 <= gameWidth - 1 && field[y][x] && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y][x + 3] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                        if (x + 3 >= 10 && field[y][x] > 6 && field[y][x - 1] > 6) {
                                            currentblockX -= blockWidth;
                                            x = (currentblockX - topleft.x) / blockWidth;
                                        }
                                    }
                                } while (!clear && currentblockY >= topleft.y && currentblockX >= topleft.x);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        }
                    }
                        break;
                    }
                }
                break;

                case 1: // Yellow block. Square that caan't turn, so nothing.
                    break;

                case 2: // Pink block
                switch (currentblockstate) 
                {
                case 0:
                if (y + 2 <= gameHeight-1 && x+1 <= gameWidth-1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 1][x + 1] > 6) {
                    currentblockstate++;
                }
                else {
                    int orgY = currentblockY;
                    bool clear = false;
                    do {
                        if (y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 1][x + 1] > 6) {
                            clear = true;
                        }
                        else {
                            currentblockY -= blockHeight;
                            y = (currentblockY - topleft.y) / blockHeight;
                        }
                    } while (!clear && currentblockY >= topleft.y);
                    if (clear) {
                        currentblockstate++;
                    }
                    else {
                        currentblockY = orgY;
                    }
                }
                break;

                case 1:
                if (x + 2 <= gameWidth-1 && field[y][x] > 6 && field[y - 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                    currentblockstate++;
                }
                else if (x - 1 > 0 && field[y][x-1] > 6 && field[y - 1][x] > 6 && field[y][x] > 6 && field[y][x + 1] > 6) {
                    currentblockX -= blockWidth;
                    currentblockstate++;
                }
                else {
                    int orgY = currentblockY;
                    bool clear = false;
                    if (x + 2 > gameWidth - 1) {
                            currentblockX -= blockWidth;
                            x = (currentblockX - topleft.x) / blockWidth;
                        }
                    do {
                        if (x + 2 <= gameWidth-1 && field[y][x] > 6 && field[y - 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                            clear = true;
                        }
                        else {
                            currentblockY -= blockHeight;
                            y = (currentblockY - topleft.y) / blockHeight;
                        }
                    } while (!clear && currentblockY >= topleft.y);
                    if (clear) {
                        currentblockstate++;
                    }
                    else {
                        currentblockY = orgY;
                    }
                }
                break;

                case 2:
                    if (y + 1 <= gameHeight - 1 && field[y][x] > 6 && field[y + 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6) {
                        currentblockstate++;
                    }
                    else {
                        int orgY = currentblockY;
                        bool clear = false;
                        do {
                            if (y + 1 <= gameHeight - 1 && field[y][x] > 6 && field[y + 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6) {
                                clear = true;
                            }
                            else {
                                currentblockY -= blockHeight;
                                y = (currentblockY - topleft.y) / blockHeight;
                            }
                        } while (!clear && currentblockY >= topleft.y);
                        if (clear) {
                            currentblockstate++;
                        }
                        else {
                            currentblockY = orgY;
                        }
                    }
                break;

                case 3:
                    if (x + 2 <= gameWidth - 1 && y + 1 <= gameHeight -1 && field[y][x] > 6 && field[y + 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                        currentblockstate++;
                    }
                    else if (x - 1 > 0 && y + 1 <= gameHeight -1 && field[y][x-1] > 6 && field[y + 1][x] > 6 && field[y][x] > 6 && field[y][x + 1] > 6) {
                        currentblockX -= blockWidth;
                        currentblockstate++;
                    }
                    else {
                        int orgY = currentblockY;
                        bool clear = false;

                        if (x + 2 > gameWidth - 1) {
                            currentblockX -= blockWidth;
                            x = (currentblockX - topleft.x) / blockWidth;
                        }
                        do {
                            
                            if (x + 2 <= gameWidth - 1 && y + 1 <= gameHeight - 1&& field[y][x] > 6 && field[y + 1][x + 1] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                                clear = true;
                            }
                            else {
                                currentblockY -= blockHeight;
                                y = (currentblockY - topleft.y) / blockHeight;
                            }
                        } while (!clear && currentblockY >= topleft.y);
                        if (clear) {
                            currentblockstate++;
                        }
                        else {
                            currentblockY = orgY;
                        }
                    }
                break;
                }
                break;

                case 3: // L-Shaped orange block.
                switch (currentblockstate) {
                    case 0:
                        if (y + 2 <= gameHeight-1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y+1][x] > 6 && field[y+2][x] > 6 && field[y + 2][x + 1] > 6) {
                            currentblockstate++;
                        }
                        else {
                            int orgY = currentblockY;
                            bool clear = false;
                            do {
                                if (y-1 > 0 && y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y + 2][x + 1] > 6) {
                                    clear = true;
                                }
                                else {
                                    currentblockY -= blockHeight;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                            } while (!clear && currentblockY >= topleft.y);
                            if (clear) {
                                currentblockstate++;
                            }
                            else {
                                currentblockY = orgY;
                            }
                        }
                    break;

                    case 1:
                        if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y][x+1] > 6 && field[y][x + 2] > 6) {
                            currentblockstate++;
                        }
                        else {
                            int orgY = currentblockY;
                            bool clear = false;
                            do {
                                if (x + 2 > gameWidth - 1 && field[y][x-1] > 6 && field[y+1][x-1] > 6) {
                                    currentblockX -= blockWidth;
                                    x = (currentblockX - topleft.x) / blockWidth;
                                    currentblockY = orgY;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                                if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y][x+1] > 6 && field[y][x + 2] > 6) {
                                    clear = true;
                                }
                                else {
                                    currentblockY -= blockHeight;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                            } while (!clear && currentblockY >= topleft.y);
                            if (clear) {
                                currentblockstate++;
                            }
                            else {
                                currentblockY = orgY;
                            }
                        }
                    break;

                    case 2:
                        if (y + 2 <= gameHeight - 1 && field[y][x] > 6 && field[y][x+1] > 6 && field[y + 1][x+1] > 6 && field[y + 2][x + 1] > 6) {
                            currentblockstate++;
                        }
                        else {
                            int orgY = currentblockY;
                            bool clear = false;
                            do {
                                if (y + 2 <= gameHeight - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y + 1][x + 1] > 6 && field[y + 2][x + 1] > 6) {
                                    clear = true;
                                }
                                else {
                                    currentblockY -= blockHeight;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                            } while (!clear && currentblockY >= topleft.y);
                            if (clear) {
                                currentblockstate++;
                            }
                            else {
                                currentblockY = orgY;
                            }
                        }
                    break;

                    case 3:
                        if (y - 1 >= 0 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x+1] > 6 && field[y][x + 2] > 6 && field[y-1][x + 2] > 6) {
                            currentblockstate++;
                        }
                        else {
                            int orgY = currentblockY;
                            bool clear = false;
                            do {
                                if (x + 2 >= gameWidth - 1 && field[y][x - 1] > 6 && field[y + 1][x + 1] > 6) {
                                    currentblockX -= blockWidth;
                                    x = (currentblockX - topleft.x) / blockWidth;
                                    currentblockY = orgY;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                                if (y - 1 >= 0 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6 && field[y - 1][x + 2] > 6) {
                                    clear = true;
                                }
                                else {
                                    currentblockY -= blockHeight;
                                    y = (currentblockY - topleft.y) / blockHeight;
                                }
                            } while (!clear && currentblockY >= topleft.y);
                            if (clear) {
                                currentblockstate++;
                            }
                            else {
                                currentblockY = orgY;
                            }
                        }
                    break;
                }
                break;

                    case 4: //J-Shaped purple block.
                    switch (currentblockstate) {
                        case 0:
                            if (y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y][x + 1] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 2][x] > 6 && field[y][x + 1] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;

                        case 1:
                            if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x+2] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (x + 2 > gameWidth - 1 && field[y][x - 1] > 6 && field[y + 1][x + 1] > 6) {
                                        currentblockX -= blockWidth;
                                        x = (currentblockX - topleft.x) / blockWidth;
                                        currentblockY = orgY;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                    if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x + 2] > 6 && field[y][x + 1] > 6 && field[y][x + 2] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;

                        case 2:
                            if (y - 2 >= 0 && y + 2 <= gameHeight - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6 && field[y - 2][x + 1] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (y - 2 >= 0 && y + 2 <= gameHeight - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6 && field[y - 2][x + 1] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;

                        case 3:
                            if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 1][x + 1] > 6 && field[y + 1][x + 2] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (x + 2 >= gameWidth - 1 && field[y][x - 1] > 6 && field[y + 1][x - 1] > 6 && field[y+1][x] > 6 && field[y+1][x+1] > 6) {
                                        currentblockX -= blockWidth;
                                        x = (currentblockX - topleft.x) / blockWidth;
                                    }
                                    if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 1][x + 1] > 6 && field[y + 1][x + 2] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;
                        }
                        break;

                    case 5: // Left-Pointing zig-zag. Red.
                    if (currentblockstate > 1) currentblockstate = currentblockstate % 2;
                    
                    switch (currentblockstate) {
                        case 0:
                            if (y + 1 <= gameHeight - 1 && y - 1 >= 0 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (y + 1 <= gameHeight - 1 && y - 1 >= 0 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;

                        case 1:
                            if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x+1] > 6 && field[y+1][x + 1] > 6 && field[y + 1][x + 2] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (x + 2 >= gameWidth - 1 && field[y][x - 1] > 6 && field[y + 1][x] > 6 && field[y + 1][x+1] > 6 && field[y][x] > 6) {
                                        currentblockX -= blockWidth;
                                        x = (currentblockX - topleft.x) / blockWidth;
                                    }
                                    if (y + 1 <= gameHeight - 1 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y + 1][x + 1] > 6 && field[y + 1][x + 2] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;
                        }
                        break;

                    case 6: // Right-Pointing zig-zag. Green.
                    if (currentblockstate > 1) currentblockstate = currentblockstate % 2;

                    switch (currentblockstate) {
                        case 0:
                            if (y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y+1][x + 1] > 6 && field[y + 2][x + 1] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (y + 2 <= gameHeight - 1 && x + 1 <= gameWidth - 1 && field[y][x] > 6 && field[y + 1][x] > 6 && field[y + 1][x + 1] > 6 && field[y + 2][x + 1] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;

                        case 1:
                            if (y <= gameHeight - 1 && y - 1 >= 0 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6 && field[y - 1][x + 2] > 6) {
                                currentblockstate++;
                            }
                            else {
                                int orgY = currentblockY;
                                bool clear = false;
                                do {
                                    if (x + 2 >= gameWidth - 1 && field[y][x - 1] > 6 && field[y][x] > 6 && field[y - 1][x] > 6 && field[y-1][x+1] > 6) {
                                        currentblockX -= blockWidth;
                                        x = (currentblockX - topleft.x) / blockWidth;
                                    }
                                    if (y <= gameHeight - 1 && y - 1 >= 0 && x + 2 <= gameWidth - 1 && field[y][x] > 6 && field[y][x + 1] > 6 && field[y - 1][x + 1] > 6 && field[y - 1][x + 2] > 6) {
                                        clear = true;
                                    }
                                    else {
                                        currentblockY -= blockHeight;
                                        y = (currentblockY - topleft.y) / blockHeight;
                                    }
                                } while (!clear && currentblockY >= topleft.y);
                                if (clear) {
                                    currentblockstate++;
                                }
                                else {
                                    currentblockY = orgY;
                                }
                            }
                        break;
                        }
                        break;
                        }
                        currentblockstate = currentblockstate % 4;
                    } 
                }
            }
            InvalidateRect(hWnd, 0, 1);
        }
        break;

    case WM_ERASEBKGND:
        return 1;


    case WM_DESTROY:
        if (hdcMem) {
            DeleteDC(hdcMem);
        }
        if (hBitmap) {
            DeleteObject(hBitmap);
        }
        DeleteObject(cyan);
        DeleteObject(yellow);
        DeleteObject(purple);
        DeleteObject(pink);
        DeleteObject(orange);
        DeleteObject(red);
        DeleteObject(green);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;

}
