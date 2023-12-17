#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <math.h>
#include "time_control.h"

//GLOBAL VARS
HANDLE hProcess;
DWORD_PTR pSignature;
DWORD_PTR pNewMemoryRegion;
BOOL time_stopped = FALSE;
BOOL custom_time_rate = FALSE;
float time = 12.0f;

DWORD get_PID(CHAR *PrName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if(Process32First(snapshot, &entry)) {
        while(Process32Next(snapshot, &entry)) {
            if(strcmp(entry.szExeFile, PrName) == 0)
                return entry.th32ProcessID;
        }
    }
    CloseHandle(snapshot);
    return NULL;
}

DWORD_PTR GetModuleBase(char *lpModuleName, DWORD dwProcessId) {
    MODULEENTRY32 lpModuleEntry = { 0 };
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
    if(!hSnapShot) return NULL;
    lpModuleEntry.dwSize = sizeof(lpModuleEntry);
    BOOL bModule = Module32First(hSnapShot, &lpModuleEntry);
    while(bModule) {
        if(!strcmp(lpModuleEntry.szModule, lpModuleName)) {
            CloseHandle(hSnapShot);
            return (DWORD_PTR)lpModuleEntry.modBaseAddr;
        }
        bModule = Module32Next(hSnapShot, &lpModuleEntry);
    }
    CloseHandle(hSnapShot);
    return NULL;
}

BOOL PatchEx(HANDLE hProcess, LPVOID dst_addr, LPCVOID src_addr, SIZE_T size, SIZE_T *bytes_written) {
    DWORD old_protect;
    VirtualProtectEx(hProcess, dst_addr, size, PAGE_EXECUTE_READWRITE, &old_protect);
    BOOL result = WriteProcessMemory(hProcess, dst_addr, src_addr, size, bytes_written);
    //VirtualProtectEx(hProcess, dst_addr, size, old_protect, &old_protect);
    return result;
}

void print_hex(char *str, int len) {
    for(int i=0; i<len; i++)
        printf("%02x", (unsigned char)str[i]);
}

void print_process_memory(HANDLE hProcess, DWORD_PTR addr, SIZE_T size) {
    SIZE_T bytes_read = 0;
    char local_buffer[size];
    ReadProcessMemory(hProcess, (void*)addr, &local_buffer, size, &bytes_read);
    print_hex(local_buffer, size);
    printf("\nBytes read = %d\n", bytes_read);
}

DWORD_PTR search_process_memory(HANDLE hProcess, DWORD_PTR StartAddress, char *bytes, SIZE_T size) {
    const int READ_SIZE = 102400*size; //Read 1000 Kbytes (in our case, size of the search sequence = 10 bytes)
    char local_buffer[READ_SIZE];
    SIZE_T bytes_read = 0;
    DWORD_PTR Address = StartAddress;
    while(ReadProcessMemory(hProcess, (void*)Address, &local_buffer, READ_SIZE, &bytes_read)) {
        BOOL found = TRUE;
        for(int i=0; i<bytes_read-size+1; i++) {
            found = TRUE;
            for(int j=0; j<size; j++) {
                if(bytes[j]!=local_buffer[i+j]) {
                    found = FALSE;
                    break;
                }
            }
            if(found) return Address + i;
        }
        Address += READ_SIZE;
    }
    return 0;
}

BOOL patch_process_memory(HANDLE hProcess, DWORD_PTR dst_addr, char* src_addr, SIZE_T size) {
    SIZE_T bytes_written = 0;
    printf("Memory before injection = ");
    print_process_memory(hProcess, dst_addr, size);
    BOOL result = PatchEx(hProcess, (LPVOID)dst_addr, src_addr, size, &bytes_written);
    DWORD last_error = GetLastError();
    printf("CODE INJECTION:\n");
    printf("Result code = %d\n", result);
    printf("Bytes written = %d\n", bytes_written);
    printf("Last error = %d\n", last_error);
    printf("Memory after injection = ");
    print_process_memory(hProcess, dst_addr, size);
    return result==1 && bytes_written==size && last_error==0;
}

void message_box(char* message, UINT uType) {
    MessageBox(NULL, message, "SnowRunner time control", uType);
}

void inc_time(float *curr_time, float step, BOOL h_round) {
    //printf("\nTIME BEFORE INC= %f\n", *curr_time);
    *curr_time+=step;
    //printf("\nTIME AFTER STEP= %f\n", *curr_time);
    if(h_round) *curr_time = roundf(*curr_time);
    //*curr_time = rintf(*curr_time);
    //printf("\nTIME AFTER ROUND= %f\n", *curr_time);
    if(*curr_time < 0)
        *curr_time = 24.0f + *curr_time;
    if(*curr_time >= 24.0f)
        *curr_time = *curr_time - 24.0f;
    //printf("\nTIME AFTER ALL= %f\n\n", *curr_time);
}

BOOL get_time(float *time) {
    SIZE_T bytes_read = 0;
    BOOL result = ReadProcessMemory(hProcess, (void*)pNewMemoryRegion + 0x32, time, 4, &bytes_read); //get the game timer value
    return result;
}

BOOL set_time(float *time) {
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[45];
    memset(new_mem_buf, '\x90', 45); //45 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    memcpy(new_mem_buf + 8, "\x41\xC7\x85\x38\x01\x00\x00\x00\x00\x00\x00", 11); //MOV dword ptr [r13+0x138],(float)time
    memcpy(new_mem_buf + 15, time, 4); //new time value
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 45, &bytes_written);
    return result;
}

BOOL start_time() {
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[45];
    memset(new_mem_buf, '\x90', 45); //45 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    memcpy(new_mem_buf + 8, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9); //MOVSS dword ptr [r13+0x138],xmm2
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 45, &bytes_written);
    return result;
}

BOOL stop_time() {
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[45];
    memset(new_mem_buf, '\x90', 45); //45 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 45, &bytes_written);
    return result;
}

BOOL shift_time(float *time, float step) {
    BOOL result = FALSE;
    result = get_time(time);
    inc_time(time, step, FALSE);
    result = set_time(time);
    if(!time_stopped && !custom_time_rate) {
        Sleep(10);
        result = start_time();
    }
    return result;
}

BOOL set_time_rate(HWND hWnd, float *time, unsigned char rate_factor, BOOL sync_real_time) {
    BOOL result = FALSE;
    if(sync_real_time)
        *time = get_local_time(); //sync with OS local time
    else
        get_time(time);
    result = set_time(time);
    if(result) {
        SetTimer(hWnd, IDT_TIMER, 60000/rate_factor, (TIMERPROC) NULL);
        time_stopped = FALSE;
        custom_time_rate = TRUE;
    }
    return result;
}

float get_local_time() {
    SYSTEMTIME local_time;
    GetLocalTime(&local_time);
    float result_time = local_time.wHour + local_time.wMinute / 60.0;
    //printf("\nLOCAL_TIME = %d:%d\n", local_time.wHour, local_time.wMinute);
    //printf("\nLOCAL_TIME (float) = %f\n", result_time);
    return result_time;
}

int init_memory() {
    char *PrName = "SnowRunner.exe";
    DWORD PID;
    DWORD_PTR BaseAddress;
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[50];
    char inj_pnt_buf[9];

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
    pSignature = search_process_memory(hProcess, BaseAddress, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00\xF3", 10);
    if(pSignature != 0)
        printf("Found pattern address = %llx\n", pSignature);
    else {
        printf("Pattern not found in memory!\n\n");
        message_box("Pattern not found in memory!\n\nProcess SnowRunner.exe already patched or this app may not be compatible with your version of the game.", MB_ICONERROR);
        return -1;
    }

    //ALLOCATE NEW MEMORY REGION
    pNewMemoryRegion = (DWORD_PTR)VirtualAllocEx(hProcess, BaseAddress-0x1000, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    printf("New memory region address = %llx\n", pNewMemoryRegion);
    DWORD jmp_offset = pNewMemoryRegion - (pSignature + 5); // 5 - length of JMP operation for return to injection point
    DWORD jmp_return_offset = (pSignature + 5 + 4) - (pNewMemoryRegion + 50);
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
    time_stopped = result;

    //INJECT JMP
    memcpy(inj_pnt_buf, "\xE9", 1); //JMP opcode
    memcpy(inj_pnt_buf + 1, &jmp_offset, 4); //JMP offset
    memcpy(inj_pnt_buf + 5, "\x90\x90\x90\x90", 4); //4 x NOP
    result = patch_process_memory(hProcess, pSignature, inj_pnt_buf, 9);
    return 0;
}

void restore_memory() {
    patch_process_memory(hProcess, pSignature, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9); //MOVSS dword ptr [r13+0x138],xmm2
    VirtualFreeEx(hProcess, (LPVOID)pNewMemoryRegion, 0, MEM_RELEASE);
}
