#include "string.h"

int memcmp(const void* ptr1, const void* ptr2, size_t size)
{
    const unsigned char* a = (const unsigned char*) ptr1;
    const unsigned char* b = (const unsigned char*) ptr2;
    for(size_t i=0; i < size; i++)
    {
        if (a[i] < b[i])
            return -1;
        else if (b[i] < a[i])
            return 1;
    }
    return 0;
}

void* memset(void* ptr, int value, size_t size)
{
    uint8_t* buf = (uint8_t*) ptr;
    for(size_t i=0; i<size; i++)
        buf[i] = (uint8_t) value;
    return ptr;
}

void* memcpy(void* destination, const void* source, size_t size)
{
    uint8_t* dst = (uint8_t*) destination;
    const uint8_t* src = (uint8_t*) source;
    for(size_t i=0; i<size; i++)
        dst[i] = src[i];
    return destination;
}

void* memmove(void* destination, const void* source, size_t size)
{
    uint8_t* dst = (uint8_t*) destination;
    const uint8_t* src = (uint8_t*) source;
    if (dst < src) {
        for (size_t i=0;i<size;i++)
            dst[i] = src[i];
    } else {
        for (size_t i=size;i!=0;i--)
            dst[i-1] = src[i-1];
    }
    return destination;
}

int strcmp(const char* str1, const char* str2)
{
    while(*str1 != '\0')
    {
        if(*str1 != *str2)
            break;
        
        str1++;
        str2++;
    }

    return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n)
{
    while(n && *str1 && (*str1 == *str2))
    {
        ++str1;
        ++str2;
        --n;
    }
    if (n == 0)
        return 0;
    else
        return *(const unsigned char*)str1 - *(const unsigned char*)str2;
}

char* strchr(char* str, int ch)
{
    while (*str != (char)ch)
        if (!*str++)
            return 0;

    return (char*)str;
}

char* strcpy(char* dest, const char* source)
{
    char* d = dest;
    while (*source != '\0')
        *d++ = *source++;

    *d = *source;
        
    return dest;
}

static char* s;

char* strtok(char* str, char sep)
{
    if(str == NULL)
        str = s;
    
    char* p = str;
    while(*str != '\0')
    {
        if(*str == sep)
        {
            *str = '\0';
            s = str + 1;
            return p;
        }
        str++;
    }
    s = NULL;
    return p;
}

char* strcat(char* dest, const char* src)
{
    char* d = dest;
    while(*dest != '\0') dest++;

    strcpy(dest, src);
    return d;
}

size_t strlen(const char* str)
{
    size_t i = 0;
    while(*str++ != '\0') i++;

    return i;
}

char* strncpy(char* dest, const char* source, size_t num)
{
    size_t j = 0;
    while(j < num && source[j] != '\0')
    {
        dest[j] = source[j];
        j++;
    }
    while(j < num)
        dest[j++] = 0;

    return dest;
}