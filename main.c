#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include "time_control.h"
#include "resource.h"

//GLOBAL VARS
extern float time;
extern BOOL time_stopped;
extern BOOL custom_time_rate;
TCHAR szClassName[] = _T("SnowRunner_time_control"); /*  Make the class name into a global variable  */
HWND TextField;
HWND DonateButton;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
    //INIT
    BOOL result = FALSE;
    result = init_memory();
    if(result == -1) return 0;

    //GUI section
    HWND hwnd;               /* This is the handle for our window */
    MSG msg;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    char WindowTitle[50] = "SnowRunner time control v";
    strcat(WindowTitle, VERSION);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    wincl.hIconSm = LoadIcon(hThisInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if(!RegisterClassEx(&wincl)) return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx(
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T(WindowTitle),       /* Title Text */
           WS_MINIMIZEBOX | WS_SYSMENU, /* default window */
           (GetSystemMetrics(SM_CXSCREEN) >> 1) - (WINDOW_WIDTH >> 1),       /* Window will appear in the center of the screen */
           (GetSystemMetrics(SM_CYSCREEN) >> 1) - (WINDOW_HEIGHT >> 1),
           WINDOW_WIDTH,                 /* The programs width */
           WINDOW_HEIGHT,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);

    /* Register HotKeys */
    RegisterHotKey(NULL, MUL, MOD_NOREPEAT, VK_MULTIPLY);
    RegisterHotKey(NULL, DIV, MOD_NOREPEAT, VK_DIVIDE);
    RegisterHotKey(NULL, SUB, MOD_NOREPEAT, VK_SUBTRACT);
    RegisterHotKey(NULL, ADD, MOD_NOREPEAT, VK_ADD);
    RegisterHotKey(NULL, CTRL_SUB, MOD_NOREPEAT | MOD_CONTROL, VK_SUBTRACT);
    RegisterHotKey(NULL, SHIFT_SUB, MOD_NOREPEAT | MOD_SHIFT, VK_SUBTRACT);
    RegisterHotKey(NULL, ALT_SUB, MOD_NOREPEAT | MOD_ALT, VK_SUBTRACT);
    RegisterHotKey(NULL, CTRL_ADD, MOD_NOREPEAT | MOD_CONTROL, VK_ADD);
    RegisterHotKey(NULL, SHIFT_ADD, MOD_NOREPEAT | MOD_SHIFT, VK_ADD);
    RegisterHotKey(NULL, ALT_ADD, MOD_NOREPEAT | MOD_ALT, VK_ADD);
    RegisterHotKey(NULL, ALT_DIV, MOD_NOREPEAT | MOD_ALT, VK_DIVIDE);
    RegisterHotKey(NULL, ALT_0, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD0);
    RegisterHotKey(NULL, ALT_1, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD1);
    RegisterHotKey(NULL, ALT_2, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD2);
    RegisterHotKey(NULL, ALT_3, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD3);
    RegisterHotKey(NULL, ALT_4, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD4);
    RegisterHotKey(NULL, ALT_5, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD5);
    RegisterHotKey(NULL, ALT_6, MOD_NOREPEAT | MOD_ALT, VK_NUMPAD6);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while(GetMessage(&msg, NULL, 0, 0)) {
        if(msg.message == WM_HOTKEY)
            switch(msg.wParam) {
                case MUL:
                    printf("\nNumPad * hotkey press has been detected!\n");
                    KillTimer(hwnd, IDT_TIMER);
                    result = stop_time();
                    if(result) {
                        time_stopped = TRUE;
                        printf("SnowRunner timer has been stopped!\n");
                    }
                    break;
                case DIV:
                    printf("\nNumPad / hotkey press has been detected!\n");
                    KillTimer(hwnd, IDT_TIMER);
                    result = start_time();
                    if(result) {
                        time_stopped = FALSE;
                        custom_time_rate = FALSE;
                        printf("SnowRunner timer has been started!\n");
                    }
                    break;
                case SUB:
                    printf("\nNumPad - hotkey press has been detected!\n");
                    result = shift_time(&time, -2.0f);
                    if(result)
                        printf("SnowRunner timer has been reduced by 2 hours!\n");
                    break;
                case ADD:
                    printf("\nNumPad + hotkey press has been detected!\n");
                    result = shift_time(&time, 2.0f);
                    if(result)
                        printf("SnowRunner timer has been increased by 2 hours!\n");
                    break;
                case CTRL_SUB:
                    result = shift_time(&time, -1.0f);
                    break;
                case SHIFT_SUB:
                    result = shift_time(&time, -3.0f);
                    break;
                case ALT_SUB:
                    result = shift_time(&time, -4.0f);
                    break;
                case CTRL_ADD:
                    result = shift_time(&time, 1.0f);
                    break;
                case SHIFT_ADD:
                    result = shift_time(&time, 3.0f);
                    break;
                case ALT_ADD:
                    result = shift_time(&time, 4.0f);
                    break;
                case ALT_DIV:
                    //KillTimer(hwnd, IDT_TIMER);
                    result = set_time_rate(hwnd, &time, 1, TRUE);
                    break;
                case ALT_0:
                    result = set_time_rate(hwnd, &time, 10, FALSE);
                    break;
                case ALT_1:
                    result = set_time_rate(hwnd, &time, 1, FALSE);
                    break;
                case ALT_2:
                    result = set_time_rate(hwnd, &time, 2, FALSE);
                    break;
                case ALT_3:
                    result = set_time_rate(hwnd, &time, 3, FALSE);
                    break;
                case ALT_4:
                    result = set_time_rate(hwnd, &time, 4, FALSE);
                    break;
                case ALT_5:
                    result = set_time_rate(hwnd, &time, 5, FALSE);
                    break;
                case ALT_6:
                    result = set_time_rate(hwnd, &time, 6, FALSE);
                    break;
            }
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&msg);
        /* Send message to WindowProcedure */
        DispatchMessage(&msg);
    }

    //RESTORE ALL
    KillTimer(hwnd, IDT_TIMER);
    restore_memory();

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return msg.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    BOOL result = FALSE;
    /* handle the messages */
    switch(message) {
        case WM_CREATE:
            TextField = CreateWindow("STATIC", INFO_MESSAGE, WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 10, 474, 252, hwnd, NULL, NULL, NULL);
            DonateButton = CreateWindow(
                "BUTTON",    // Predefined class
                "DONATE",    // Button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
                200,         // x position
                272,         // y position
                100,         // Button width
                40,          // Button height
                hwnd,        // Parent window
                (HMENU)BTN_DONATE,  // menu id
                NULL,
                NULL);
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == BTN_DONATE)
                ShellExecute(NULL, "open", "https://www.donationalerts.com/r/equdevel", NULL, NULL, SW_SHOWNORMAL);
            break;
        case WM_TIMER:
            if(wParam == IDT_TIMER) {
                inc_time(&time, (float)(1.0/60), FALSE);
                result = set_time(&time);
                //printf("\nTICK!\n");
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
