#ifndef ARRAY_H
#define ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 10

typedef struct {
    const char **array;  // Change to an array of const char*
    size_t size;
    size_t capacity;
} Array_s;

Array_s *init_array() {
    Array_s *arr = malloc(sizeof(Array_s));
    if (arr == NULL) {
        perror("Failed memory allocation for Array_s.\n");
        exit(EXIT_FAILURE);
    }

    arr->array = malloc(INITIAL_SIZE * sizeof(const char*));
    if (arr->array == NULL) {
        perror("Failed to allocate memory for (Array_s*)->array.\n");
        free(arr);
        exit(EXIT_FAILURE);
    }

    arr->size = 0;
    arr->capacity = INITIAL_SIZE;

    return arr;
}

void push_s(Array_s *arr, const char *val) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2;
        arr->array = realloc(arr->array, arr->capacity * sizeof(const char*));
        if (arr->array == NULL) {
            perror("Failed to reallocate memory for array in DynamicArray");
            free(arr);
            exit(EXIT_FAILURE);
        }
    }

    // Allocate space for the string and copy it
    arr->array[arr->size] = strdup(val);
    if (arr->array[arr->size] == NULL) {
        perror("Failed to duplicate string.\n");
        free(arr);
        exit(EXIT_FAILURE);
    }

    arr->size++;
}

void free_s(Array_s *arr) {
    for (size_t i = 0; i < arr->size; i++) {
        free((void*)arr->array[i]);  // Free each duplicated string
    }
    free(arr->array);
    free(arr);
}

#endif // ARRAY_H


