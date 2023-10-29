#include "Vector.h"

Vector getMutualProjectionVector(Vector a, Vector b) {
    double a2 = a.x * a.x + a.y * a.y;
    double b2 = b.x * b.x + b.y * b.y;
    double denom = a.x * b.y - a.y * b.x;
    if (denom == 0.0) {
        return (Vector) {0.0, 0.0 };
    }
    double frac = 1.0 / denom;
    return (Vector) {
        (b.y * a2 - a.y * b2) * frac,
        (a.x * b2 - b.x * a2) * frac
    };
}

int hasProjectionOnVector(Vector a, Vector b) {
    double b2 = b.x * b.x + b.y * b.y;
    double dot = a.x * b.x + a.y * b.y;
    return dot >= 0.0 && dot <= b2;
}

double dotProduct(Vector a, Vector b) {
    return a.x * b.x + a.y * b.y;
}

double magnitudeSquared(Vector a) {
    return a.x * a.x + a.y * a.y;
}

double magnitude(Vector a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

Vector normalized(Vector a) {
    double magnitude = sqrt(a.x * a.x + a.y * a.y);
    double oneOverMagnitude = 1.0 / magnitude;
    return (Vector) {
        a.x * oneOverMagnitude,
        a.y * oneOverMagnitude
    };
}

void sortVectorsByY(Vector vectors[], int indices[], int count) {
    for (int i = 0; i < count; i++) {
        indices[i] = i;
    }
    sortVectorsByYRecurse(vectors, indices, 0, count);
}

void sortVectorsByYRecurse(Vector vectors[], int indices[], 
    int start, int count) {
        
    if (count < 2)
        return;
    
    int end = start + count;
    double pivot = vectors[(start + end) / 2].y;
    
    int i = start;
    int j = end - 1;
    while (1) {
        while (vectors[i].y < pivot) i++;
        while (vectors[j].y > pivot) j--;
        
        if (i >= j) break;
        
        Vector temp1 = vectors[i];
        vectors[i] = vectors[j];
        vectors[j] = temp1;
        int temp2 = indices[i];
        indices[i] = indices[j];
        indices[j] = temp2;
        i++;
        j--;
    }
    
    sortVectorsByYRecurse(vectors, indices, start, i - start);
    sortVectorsByYRecurse(vectors, indices, i, end - i);
}