#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <math.h>
#include "time_control.h"

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

BOOL PatchEx(HANDLE hProcess, LPVOID dst, LPCVOID src, SIZE_T size, SIZE_T *BytesWritten) {
    DWORD oldprotect;
    VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    BOOL result = WriteProcessMemory(hProcess, dst, src, size, BytesWritten);
    //VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
    return result;
}

void print_hex(char *str, int len) {
    for(int i=0; i<len; i++)
        printf("%02x", (unsigned char)str[i]);
}

void print_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, SIZE_T size) {
    SIZE_T bytes_read = 0;
    char local_buffer[size];
    ReadProcessMemory(hProcess, (void*)pBuffer, &local_buffer, size, &bytes_read);
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

BOOL patch_process_memory(HANDLE hProcess, DWORD_PTR pBuffer, char* new_buffer, SIZE_T size) {
    SIZE_T bytes_written;
    printf("Memory before injection = ");
    print_process_memory(hProcess, pBuffer, size);
    BOOL result = PatchEx(hProcess, (LPVOID)pBuffer, new_buffer, size, &bytes_written);
    DWORD last_error = GetLastError();
    printf("CODE INJECTION:\n");
    printf("Result code = %d\n", result);
    printf("Bytes written = %d\n", bytes_written);
    printf("Last error = %d\n", last_error);
    printf("Memory after injection = ");
    print_process_memory(hProcess, pBuffer, size);
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

BOOL get_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion, float *time) {
    SIZE_T bytes_read = 0;
    BOOL result = ReadProcessMemory(hProcess, (void*)pNewMemoryRegion + 0x32, time, 4, &bytes_read); //get the game timer value
    return result;
}

BOOL set_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion, float *time) {
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

BOOL start_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion) {
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[45];
    memset(new_mem_buf, '\x90', 45); //45 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    memcpy(new_mem_buf + 8, "\xF3\x41\x0F\x11\x95\x38\x01\x00\x00", 9); //MOVSS dword ptr [r13+0x138],xmm2
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 45, &bytes_written);
    return result;
}

BOOL stop_time(HANDLE hProcess, DWORD_PTR pNewMemoryRegion) {
    BOOL result = FALSE;
    SIZE_T bytes_written = 0;
    char new_mem_buf[45];
    memset(new_mem_buf, '\x90', 45); //45 x NOP
    memcpy(new_mem_buf, "\xF3\x0F\x11\x15\x2A\x00\x00\x00", 8); //MOVSS dword ptr [offset 0x2A], xmm2 - Save game time
    result = WriteProcessMemory(hProcess, (LPVOID)pNewMemoryRegion, new_mem_buf, 45, &bytes_written);
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
