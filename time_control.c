#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
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

void inc_time(float *curr_time, float step) {
    //printf("\nTIME BEFORE INC= %f\n", *curr_time);
    *curr_time+=step;
    //printf("\nTIME AFTER STEP= %f\n", *curr_time);
    *curr_time = roundf(*curr_time);
    //*curr_time = rintf(*curr_time);
    //printf("\nTIME AFTER ROUND= %f\n", *curr_time);
    if(*curr_time < 0)
        *curr_time = 24.0f + *curr_time;
    if(*curr_time >= 24.0f)
        *curr_time = *curr_time - 24.0f;
    //printf("\nTIME AFTER ALL= %f\n\n", *curr_time);
}
