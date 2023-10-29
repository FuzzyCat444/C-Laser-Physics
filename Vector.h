#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

typedef struct Vector {
    double x;
    double y;
} Vector;

Vector getMutualProjectionVector(Vector a, Vector b);

int hasProjectionOnVector(Vector a, Vector b);

double dotProduct(Vector a, Vector b);

double magnitudeSquared(Vector a);

double magnitude(Vector a);

Vector normalized(Vector a);

void sortVectorsByY(Vector vectors[], int indices[], int count);

void sortVectorsByYRecurse(Vector vectors[], int indices[], 
    int start, int count);

#endif