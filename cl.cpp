#include <windows.h>
#include <gdiplus.h>
#include <math.h>

using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

#define cx GetSystemMetrics(SM_CXSCREEN)
#define cy GetSystemMetrics(SM_CYSCREEN)
#define class "wclass"

static HHOOK mHook;
static HHOOK kHook;
static HWND window;
static u_long token;

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    SendMessage(window, WM_DESTROY, wParam, lParam);
    return CallNextHookEx(mHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    SendMessage(window, WM_DESTROY, wParam, lParam);
    return CallNextHookEx(kHook, nCode, wParam, lParam);
}

bool range(unsigned l, unsigned h, unsigned x)
{
    return ((x - l) <= (h - l));
}

DWORD WINAPI clock(LPVOID lpParam) 
{
    HDC hdc = GetDC(window);
    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    Pen pen(Color(200, 64, 64, 64), 5);
    Pen spen(Color(200, 100, 100, 100), 4);
    Pen cpen(Color(180, 25, 25, 30), 8);
    SolidBrush brush(Color(30, 0, 0, 0));
    SolidBrush cbrush(Color(200, 80, 80, 80));
    SolidBrush bbrush(Color(0, 0, 0));

    ShowWindow(window, SW_SHOW);

    int r = cy / 2 - 150;
    graphics.DrawEllipse(&cpen, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r);
    for (int i = 0; i < 360; i += 30) 
    {
        float a = i * M_PI / 180;
        int x1 = cx / 2;
        int y1 = cy / 2;
        int y2 = cy / 2 + r + 4;
        int x3 = -sinf(a) * (y2 - y1) + x1;
        int y3 = cosf(a) * (y2 - y1) + y1;
        graphics.DrawLine(&pen, x1, y1, x3, y3);
    }
    r = r - 4;
    graphics.FillEllipse(&bbrush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r);
    int angl = M_PI / 9;
    float tpi = M_PI * 2;
    float todeg = 180 / M_PI;

    while (true) 
    {
        r = cy / 2 - 180;
        SYSTEMTIME time = {0};
        GetLocalTime(&time);

        float sa = (float)(time.wSecond * 1000 + time.wMilliseconds) / 60000 * tpi;
        int x1 = cx / 2;
        int y1 = cy / 2;
        int sy2 = cy / 2 - r + 10;
        int sx3 = -sinf(sa) * (sy2 - y1) + x1;
        int sy3 = cosf(sa) * (sy2 - y1) + y1;

        int ssy2 = cy / 2 + r - 280;
        int ssx3 = -sinf(sa) * (ssy2 - y1) + x1;
        int ssy3 = cosf(sa) * (ssy2 - y1) + y1;

        float ma = (float)(time.wMinute * 60 * 100 + time.wSecond * 100 + time.wMilliseconds / 10) / 360000 * tpi;
        int my2 = cy / 2 - r + 85;
        int mx3 = -sinf(ma) * (my2 - y1) + x1;
        int my3 = cosf(ma) * (my2 - y1) + y1;

        float ha = (float)(time.wMinute * 60 + time.wHour * 60 * 60 + time.wSecond) / 43200 * tpi;
        int hy2 = cy / 2 - r + 170;
        int hx3 = -sinf(ha) * (hy2 - y1) + x1;
        int hy3 = cosf(ha) * (hy2 - y1) + y1;

        if (range(sa - angl, sa + angl, ha))
            graphics.FillPie(&brush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r, ha * todeg - 90.5, -10);
        if (!range(sa - angl, sa + angl, ma))
            graphics.FillPie(&brush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r, ma * todeg - 90.5, -10);
        graphics.FillPie(&brush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r, sa * todeg - 90.5, -20);
        graphics.FillPie(&brush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r, sa * todeg + 90.5, -20);

        graphics.DrawLine(&pen, x1, y1, hx3, hy3);
        graphics.DrawLine(&pen, x1, y1, mx3, my3);
        graphics.DrawLine(&spen, x1, y1, sx3, sy3);
        graphics.DrawLine(&spen, x1, y1, ssx3, ssy3);

        r = 6;
        graphics.FillEllipse(&cbrush, cx / 2 - r, cy / 2 - r, 2 * r, 2 * r);
        Sleep(15);
    }
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            GdiplusShutdown(token);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    mHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
    kHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

    GdiplusStartupInput input = {0};
    input.GdiplusVersion = 1;
    GdiplusStartup(&token, &input, NULL);

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = class;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    RegisterClassEx(&wc);

    window = CreateWindowEx(WS_EX_TOOLWINDOW, class, 0, WS_BORDER, 0, 0, cx, cy, NULL, NULL, hInstance, NULL); 

    if (window != NULL)
    {
        SetWindowLongPtr(window, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
        SetWindowLongPtr(window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(window, HWND_TOPMOST, 0, 0, cx, cy, SWP_HIDEWINDOW);
        SetCursor(NULL);
        CreateThread(NULL, 0, clock, NULL, 0, NULL); 
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return 0;
}