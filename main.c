#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "time_control.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("SnowRunner_time_control");

HWND TextField;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
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
           _T("SnowRunner time control"),       /* Title Text */
           WS_MINIMIZEBOX | WS_SYSMENU, /* default window */
           (GetSystemMetrics (SM_CXSCREEN) >> 1) - (500  >> 1),       /* Windows decides the position */
           (GetSystemMetrics (SM_CYSCREEN) >> 1) - (300 >> 1),       /* where the window ends up on the screen */
           500,                 /* The programs width */
           300,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           NULL,                /* No menu */
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    //INIT
    DWORD PID;
    HANDLE hProcess;
    DWORD_PTR BaseAddress;
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

    //DWORD_PTR pBuffer = BaseAddress + 0xA8C306;
    DWORD_PTR pBuffer = search_process_memory(hProcess, BaseAddress, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00\xF3", 10);
    if(pBuffer != 0)
        printf("Found pattern address = %llx\n", pBuffer);
    else {
        printf("Pattern not found in memory!\n\n");
        message_box("Pattern not found in memory!\n\nProcess SnowRunner.exe already patched or this app may not be compatible with your version of the game.", MB_ICONERROR);
        return -1;
    }

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

    //HANDLE hProcThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)pInjectedBuffer, NULL, NULL, NULL);

    /* Make the window visible on the screen */
    ShowWindow (hwnd, nCmdShow);

    /* Register HotKeys */
    RegisterHotKey(NULL, 1, MOD_CONTROL | MOD_NOREPEAT, VK_MULTIPLY);
    RegisterHotKey(NULL, 2, MOD_CONTROL | MOD_NOREPEAT, VK_DIVIDE);
    RegisterHotKey(NULL, 3, MOD_CONTROL | MOD_NOREPEAT, VK_SUBTRACT);
    RegisterHotKey(NULL, 4, MOD_CONTROL | MOD_NOREPEAT, VK_ADD);
    RegisterHotKey(NULL, 5, MOD_CONTROL | MOD_NOREPEAT, VK_NUMPAD0);

    BOOL result = FALSE;
    //BOOL patched = FALSE;

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0)) {
        if(messages.message == WM_HOTKEY)
            switch(messages.wParam) {
                case 1:
                    printf("\nCtrl + NumPad* hotkey press has been detected!\n");
                    //INJECTION
                    //if(!patched) {
                        result = patch_process_memory(hProcess, pBuffer, "\x90\x90\x90\x90\x90\x90\x90\x90\x90", 9);
                        if(result) {
                            //patched = TRUE;
                            printf("SnowRunner timer has been stopped!\n");
                            //message_box("SnowRunner timer has been stopped!", MB_ICONINFORMATION);
                        }
                    //}
                    break;
                case 2:
                    printf("\nCtrl + NumPad/ hotkey press has been detected!\n");
                    //RESTORE MEMORY
                    //if(patched) {
                        result = patch_process_memory(hProcess, pBuffer, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9);
                        if(result) {
                            //patched = FALSE;
                            printf("SnowRunner timer has been started!\n");
                            //message_box("SnowRunner timer has been started!", MB_ICONINFORMATION);
                        }
                    //}
                    break;
                case 3:
                    printf("\nCtrl + NumPad- hotkey press has been detected!\n");
                    break;
                case 4:
                    printf("\nCtrl + NumPad+ hotkey press has been detected!\n");
                    break;
                case 5:
                    printf("\nCtrl + NumPad0 hotkey press has been detected!\n");
                    //WRITE TO NEW MEMORY REGION
                    float new_time = 0.0f;
                    //print_hex(&new_time, 4);
                    //printf("\n");
                    SIZE_T bytes_written;
                    char buf1[50];
                    memset(buf1, '\x90', 50);
                    memcpy(buf1, "\x41\xC7\x85\x38\x01\x00\x00\x00\x00\x00\x00", 11); //mov [r13+00000138],(float)new_time
                    //memcpy(buf1, "\x41\x81\x85\x38\x01\x00\x00\x00\x00\x80\x3F", 11); //add [r13+00000138], 1
                    memcpy(buf1 + 45, "\xE9", 1); //jmp opcode
                    memcpy(buf1 + 7, &new_time, 4); //new_time
                    memcpy(buf1 + 46, &jmp_return_offset, 4); //jmp offset
                    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, buf1, 50, &bytes_written);
                    //INJECT JMP TO NEW MEMORY REGION
                    char buf2[9] = "\xE9"; //jmp opcode
                    memcpy(buf2 + 1, &jmp_offset, 4); //jmp offset
                    memcpy(buf2 + 5, "\x90\x90\x90\x90", 4); //4 nop
                    result = patch_process_memory(hProcess, pBuffer, buf2, 9);
                    break;
            }
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    //RESTORE ALL
    patch_process_memory(hProcess, pBuffer, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9);
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
            #define MESSAGE "\n\n\nPlease do not close this application while the game is running!\n\n\n\nCtrl + NumPad *    Stop game timer\n\nCtrl + NumPad /    Start game timer"
            TextField = CreateWindow("STATIC", MESSAGE, WS_VISIBLE | WS_CHILD | SS_CENTER, 10, 10, 474, 252, hwnd, NULL, NULL, NULL);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
