#ifndef STRING_H
#define STRING_H

#include <stdint.h>
#include <stddef.h>

int memcmp(const void* ptr1, const void* ptr2, size_t size);
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* destination, const void* source, size_t size);
void* memmove(void* destination, const void* source, size_t size);

int strcmp(const char* str1, const char* str2);
char* strchr(char* str, int ch);
char* strcpy(char* dest, const char* source);
char* strtok(char* str, char sep);
char* strcat(char* dest, const char* src);
size_t strlen(const char* str);
int strncmp(const char* str1, const char* str2, size_t n);
char* strncpy(char* dest, const char* source, size_t num);

#endif
