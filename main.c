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

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("SnowRunner_time_control");

HWND TextField;
HWND DonateButton;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    //INIT
    DWORD PID;
    HANDLE hProcess;
    DWORD_PTR BaseAddress;
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    float time = 12.0f;
    char new_mem_buf[50];
    char inj_pnt_buf[9];

    const char *PrName = "SnowRunner.exe";
    /*if(get_PID("SnowRunner_time_control.exe")>0) {
        printf("Only one instance of this app is allowed!\n\n");
        message_box("Only one instance of this app is allowed!", MB_ICONERROR);
        return -1;
    }*/
    if(!(PID = get_PID(PrName))) {
        printf("Process %s not found!\n\n", PrName);
        message_box("SnowRunner.exe not found in memory!\n\nPlease launch SnowRunner before this app.", MB_ICONERROR);
        return -1;
    }
    printf("Process %s found!\n", PrName);
    printf("PID = %d\n\n", PID);
    if(!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID))) {
        printf("OpenProcess error!\n\n");
        message_box("OpenProcess error!", MB_ICONERROR);
        return -1;
    }
    printf("OpenProcess is OK\n");
    printf("Handle of process = %d\n\n", hProcess);
    if(!(BaseAddress = GetModuleBase(PrName, PID))) {
        printf("GetModuleBase error!\n\n");
        message_box("GetModuleBase error!", MB_ICONERROR);
        return -1;
    }
    printf("GetModuleBase is OK\n");
    printf("BaseAddress = %llx\n\n", BaseAddress);

    //SEARCH SIGNATURE OFFSET
    DWORD_PTR pBuffer = search_process_memory(hProcess, BaseAddress, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00\xF3", 10);
    if(pBuffer != 0)
        printf("Found pattern address = %llx\n", pBuffer);
    else {
        printf("Pattern not found in memory!\n\n");
        message_box("Pattern not found in memory!\n\nProcess SnowRunner.exe already patched or this app may not be compatible with your version of the game.", MB_ICONERROR);
        return -1;
    }

    //ALLOCATE NEW MEMORY REGION
    DWORD_PTR pNewMemoryRegion = (DWORD_PTR)VirtualAllocEx(hProcess, BaseAddress-0x1000, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    printf("New memory region address = %llx\n", pNewMemoryRegion);
    DWORD jmp_offset = pNewMemoryRegion - (pBuffer + 5); // 5 - length of JMP operation for return to injection point
    DWORD jmp_return_offset = (pBuffer + 5 + 4) - (pNewMemoryRegion + 50);
    printf("jmp_offset = ");
    print_hex(&jmp_offset, 4);
    printf("\n");
    printf("jmp_return_offset = ");
    print_hex(&jmp_return_offset, 4);
    printf("\n");

    //WRITE TO NEW MEMORY REGION
    memset(new_mem_buf, '\x90', 50); //50 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    memcpy(new_mem_buf + 8, "\x41\xC7\x85\x38\x01\x00\x00\x00\x00\x00\x00", 11); //MOV dword ptr [r13+0x138],(float)time
    memcpy(new_mem_buf + 15, &time, 4); //new time = 12:00
    //memcpy(new_mem_buf + 8, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9); //MOVSS dword ptr [r13+0x138],xmm2
    memcpy(new_mem_buf + 45, "\xE9", 1); //JMP opcode
    memcpy(new_mem_buf + 46, &jmp_return_offset, 4); //JMP offset
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 50, &bytes_written);

    //INJECT JMP
    memcpy(inj_pnt_buf, "\xE9", 1); //JMP opcode
    memcpy(inj_pnt_buf + 1, &jmp_offset, 4); //JMP offset
    memcpy(inj_pnt_buf + 5, "\x90\x90\x90\x90", 4); //4 x NOP
    result = patch_process_memory(hProcess, pBuffer, inj_pnt_buf, 9);

    //GUI section
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */
    char WindowTitle[50] = "SnowRunner time control v";
    strcat(WindowTitle, VERSION);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    wincl.hIconSm = LoadIcon (hThisInstance, MAKEINTRESOURCE(IDI_CLOCK_ICON));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T(WindowTitle),       /* Title Text */
           WS_MINIMIZEBOX | WS_SYSMENU, /* default window */
           (GetSystemMetrics (SM_CXSCREEN) >> 1) - (WINDOW_WIDTH >> 1),       /* Windows decides the position */
           (GetSystemMetrics (SM_CYSCREEN) >> 1) - (WINDOW_HEIGHT >> 1),       /* where the window ends up on the screen */
           WINDOW_WIDTH,                 /* The programs width */
           WINDOW_HEIGHT,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Register HotKeys */
    RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_MULTIPLY);
    RegisterHotKey(NULL, 2, MOD_NOREPEAT, VK_DIVIDE);
    RegisterHotKey(NULL, 3, MOD_NOREPEAT, VK_SUBTRACT);
    RegisterHotKey(NULL, 4, MOD_NOREPEAT, VK_ADD);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0)) {
        if(messages.message == WM_HOTKEY)
            switch(messages.wParam) {
                case 1:
                    printf("\nNumPad * hotkey press has been detected!\n");
                    result = stop_time(hProcess, pNewMemoryRegion);
                    if(result)
                        printf("SnowRunner timer has been stopped!\n");
                    break;
                case 2:
                    printf("\nNumPad / hotkey press has been detected!\n");
                    result = start_time(hProcess, pNewMemoryRegion);
                    if(result)
                        printf("SnowRunner timer has been started!\n");
                    break;
                case 3:
                    printf("\nNumPad - hotkey press has been detected!\n");
                    get_time(hProcess, pNewMemoryRegion, &time);
                    inc_time(&time, -2.0f);
                    result = set_time(hProcess, pNewMemoryRegion, &time);
                    if(result)
                        printf("SnowRunner timer has been reduced by 2 hours!\n");
                    break;
                case 4:
                    printf("\nNumPad + hotkey press has been detected!\n");
                    get_time(hProcess, pNewMemoryRegion, &time);
                    inc_time(&time, 2.0f);
                    result = set_time(hProcess, pNewMemoryRegion, &time);
                    //sleep(1);
                    //result = start_time(hProcess, pNewMemoryRegion);
                    if(result)
                        printf("SnowRunner timer has been increased by 2 hours!\n");
                    break;
            }
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    //RESTORE ALL
    patch_process_memory(hProcess, pBuffer, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9); //MOVSS dword ptr [r13+0x138],xmm2
    VirtualFreeEx(hProcess, pNewMemoryRegion, 0, MEM_RELEASE);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
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
        case WM_DESTROY:
            PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
