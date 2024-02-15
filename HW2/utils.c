/*******************************************************************************
 * Name        : utils.c
 * Author      : Justin Chen
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System
 ******************************************************************************/
#include "utils.h"

/**
 * Compares the values inside two data pointers
 * @param dp1 Void pointer to first data pointer
 * @param dp2 Void pointer to second data pointer
 * @returns 1 if dp1 > dp2, -1 if dp2 > dp1, and 0 if they're equal
*/
int cmpr_int(void* dp1, void* dp2)
{
    // Cast and reference the two integers because we can't dereference void*
    int int1 = *((int*)(dp1));
    int int2 = *((int*)(dp2));

    if (int1 > int2)
    {
        return 1;
    }

    if (int2 > int1)
    {
        return -1;
    }
    
    return 0;
}

/**
 * Compares the values inside two data pointers, but for floats
 * @param dp1 Void pointer to first data pointer
 * @param dp2 Void pointer to second data pointer
 * @returns 1 if dp1 > dp2, -1 if dp2 > dp1, and 0 if they're equal
*/
int cmpr_float(void* dp1, void* dp2)
{
    // Similar to cmpr_int, we need to cast and dereference the floats because we can't do it to void*
    float float1 = *((float*)(dp1));
    float float2 = *((float*)(dp2));

    if (float1 > float2)
    {
        return 1;
    }

    if (float2 > float1) // Float 1 < float 2
    {
        return -1;
    }
    
    return 0;
}

/**
 * Print the value of an integer inside a data pointer.
 * @param data_pointer The data_pointer to print the value of
*/
void print_int(void* data_pointer)
{
    // Cast and dereference the data pointer
    int num = *((int*)(data_pointer));

    printf("%d ", num);
}

/**
 * Print the value of a float inside a data pointer
 * @param data_pointer The data_pointer to print the value of
*/
void print_float(void* data_pointer)
{
    // Cast and dereference the data pointer
    float num = *((float*)(data_pointer));

    printf("%f ", num);
}