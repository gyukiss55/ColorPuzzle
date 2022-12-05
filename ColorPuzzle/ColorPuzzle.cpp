// ColorPuzzle.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <commdlg.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include <string>

#include "ColorPuzzle.h"
#include "SolvePuzzle.h"
#include "Recognize.h"

#define MAX_LOADSTRING 100
#define IDT_TIMER1		10000

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

std::string strLocalTime;
std::string strDCCFeedback;
std::wstring jpgFileName;
std::wstring pzlFileName;
COLORREF colorPixel = 0;
uint32_t xPixel = 0;
uint32_t yPixel = 0;

bool stepSolve = false;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void OnPaintDraw(HDC hDC);
void OnMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnKeyPress(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void ShowStatus(HWND hWnd, HDC hDC);
void RefreshStatus(HWND hWnd);
bool LoadPuzzle(HWND hWnd, std::wstring& filename);
bool SavePuzzle(HWND hWnd, std::wstring& filename);
bool LoadPicture(HWND hWnd, std::wstring& filename);
bool StaticLoopPuzzle(HWND hWnd, HDC hDC);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize GDI+.
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);


    // Initialize global strings

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COLORPUZZLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COLORPUZZLE));

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

    GdiplusShutdown(gdiplusToken);
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COLORPUZZLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_COLORPUZZLE);
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
   SetTimer(hWnd,             // handle to main window 
       IDT_TIMER1,            // timer identifier 
       100,                   // 10 - milli second interval 
       (TIMERPROC)NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
            case ID_FILE_LOADPUZZLE:
                LoadPuzzle(hWnd, pzlFileName);
                StaticLoadPuzzle(pzlFileName);
                {
                    HDC hDC = GetDC(hWnd);
                    RECT rect;
                    GetClientRect(hWnd, &rect);
                    FillRect(hDC, &rect, (HBRUSH)(COLOR_WINDOW + 1));
                    StaticResetPuzzle(hWnd, hDC);
                    ReleaseDC(hWnd, hDC);
                }
                break;
            case ID_FILE_SAVEPUZZLE:
                SavePuzzle(hWnd, pzlFileName);
                StaticSavePuzzle(pzlFileName);
                break;
            case ID_FILE_LOADPICTURE:
                LoadPicture(hWnd, jpgFileName);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        {
            switch (wParam)
            {
            case IDT_TIMER1:
                RefreshStatus(hWnd);
                if (stepSolve) {
                    HDC hDC = GetDC(hWnd);
                    StaticLoopPuzzle(hWnd, hDC);
                    ReleaseDC(hWnd, hDC);
                }
                return 0;
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hDC = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            ShowStatus(hWnd, hDC);

            OnPaintDraw(hDC);

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_CHAR:
        OnKeyPress(hWnd, message, wParam, lParam);
        break;

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
        OnMouseMsg(hWnd, message, wParam, lParam);
        break;

    case WM_DESTROY:
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

void OnMouseMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEMOVE:
        {
            WPARAM xPos = LOWORD(lParam);
            WPARAM yPos = HIWORD(lParam);
            HDC hDC = GetDC(hWnd);
            COLORREF color = GetPixel(hDC, xPos, yPos);
            ReleaseDC(hWnd, hDC);
            xPixel = xPos;
            yPixel = yPos;
            colorPixel = color;

            InputReferencePointsMouse(hWnd, message, wParam, lParam);
            InputReferencePointsMouse(hWnd, message, wParam, lParam);
            ShowRawStatusMouse(hWnd, message, wParam, lParam);

            PuzzleTask::ManualSolvePuzzle(hWnd, message, wParam, lParam);
        }
        break;
        case WM_MOUSEHWHEEL:
        {
            WPARAM zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        }
        break;

    }

}

void OnKeyPress(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CHAR && (wParam == 's' || wParam == 'S')) {
        HDC hDC = GetDC(hWnd);
        StaticLoopPuzzle(hWnd, hDC);
        ReleaseDC(hWnd, hDC);
    }
    if (message == WM_CHAR && (wParam == 'd' || wParam == 'D')) {
        HDC hDC = GetDC(hWnd);
        StaticShowPuzzle(hWnd, hDC);
        ReleaseDC(hWnd, hDC);
    }
    if (message == WM_CHAR && (wParam == 'r' || wParam == 'R')) {
        HDC hDC = GetDC(hWnd);
        StaticResetPuzzle(hWnd, hDC);
        ReleaseDC(hWnd, hDC);
    }
    if (message == WM_CHAR && (wParam == 'a' || wParam == 'A')) {
       stepSolve = !stepSolve;
    }
    if (message == WM_CHAR && (wParam == '1')) {
        InputReferencePointsKeyboard(hWnd, message, wParam, lParam);
    }
    if (message == WM_CHAR && (wParam == '2')) {
        ShowRowStatusKeyboard();
    }
    if (message == WM_CHAR && (wParam == '3')) {
        RecognizePicturePuzzle(hWnd);
    }
}


void OnPaintDraw(HDC hDC)
{
    Graphics graphics(hDC);
    Pen      pen(Color(255, 0, 0, 255));
    //graphics.DrawLine(&pen, 0, 0, 200, 100);

    SolidBrush  brush(Color(255, 0, 0, 255));
    FontFamily  fontFamily(L"Times New Roman");
    Font        font(&fontFamily, 24, FontStyleRegular, UnitPixel);
    PointF      pointF(10.0f, 20.0f);
    //graphics.DrawString(L"DCC Controller!", -1, &font, pointF, &brush);

    if (jpgFileName.size() > 0) {
        Image image(jpgFileName.c_str());
        uint32_t w = image.GetWidth();
        uint32_t h = image.GetHeight();
        Rect imageRect(10, 10, w, h);
        graphics.DrawImage(&image, imageRect.GetLeft (), imageRect.GetTop ());
        SetImageRect(imageRect);
    }
}

void ShowStatus(HWND hWnd, HDC hDC)
{
    RECT rect;
    GetClientRect(hWnd, &rect);

    TextOutA(hDC, 10, rect.bottom - 30, strLocalTime.c_str(), (int)strLocalTime.size());
    TextOutA(hDC, 10, rect.bottom - 60, strDCCFeedback.c_str(), (int)strDCCFeedback.size());
}


void RefreshStatus(HWND hWnd)
{
    SYSTEMTIME lt;
    GetLocalTime(&lt);
    char buffer[128];
    snprintf(buffer, sizeof(buffer), " The local time is: %02d:%02d:%02d pixel x:%d, y:%d, R:%d, G:%d, B:%d  ",
        lt.wHour,
        lt.wMinute,
        lt.wSecond,
        xPixel,
        yPixel,
        colorPixel & 0xff,
        (colorPixel >> 8) & 0xff, 
        (colorPixel >> 16) & 0xff);
    strLocalTime = buffer;

    RECT rect;
    GetClientRect(hWnd, &rect);
    rect.top = rect.bottom - 80;
    InvalidateRect(hWnd, &rect, TRUE);
}


bool LoadPicture(HWND hWnd, std::wstring& filename)
{
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("All\0*.*\0JPG\0*.jpg\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL; // L"Read Picture";
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        // use ofn.lpstrFile
        filename = szFile;
        return true;
    }
    return false;
}

bool LoadPuzzle(HWND hWnd, std::wstring& filename)
{
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("All\0*.*\0PZL\0*.pzl\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL; // L"Read Puzzle";
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE)
    {
        // use ofn.lpstrFile
        filename = szFile;
        return true;
    }
    return false;
}

bool SavePuzzle(HWND hWnd, std::wstring& filename)
{
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("All\0*.*\0PZL\0*.pzl\0\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL; // L"Save Puzzle";
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_CREATEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE)
    {
        // use ofn.lpstrFile
        filename = szFile;
        return true;
    }
    return false;
}
