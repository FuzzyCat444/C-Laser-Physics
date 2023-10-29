#include "Blur.h"

void blur(double* rasterA, double* rasterB, int width, int height, 
    int size, BlurType type) {
    
    int kernelSize = 2 * size + 1;
    double weights[100];
    if (type == BOX_BLUR) {
        double weight = 1.0 / kernelSize;
        for (int i = 0; i < kernelSize; i++) {
            weights[i] = weight;
        }
    } else {
        double scaleFactor = type == GAUSSIAN_BLUR ? 2.3 : 1.0;
        double weightSum = 0.0;
        for (int i = 0; i < kernelSize; i++) {
            double x = (double) i / size - 1.0;
            double exponent = scaleFactor * x;
            exponent *= exponent;
            double weight = 1.0 / exp(exponent);
            weightSum += weight;
            weights[i] = weight;
        }
        double oneOverWeightSum = 1.0 / weightSum;
        for (int i = 0; i < kernelSize; i++) {
            weights[i] *= oneOverWeightSum;
        }
    }
    
    // Horizontal pass
    int rowInc = width;
    int colInc = 1 - rowInc * height;
    int idxB = 0;
    for (int x = 0; x < width; x++) {
        int startWeightI = 0;
        int startX = x - size;
        if (startX < 0) {
            startWeightI += -startX;
            startX = 0;
        }
        int endWeightI = kernelSize;
        int endX = x + size + 1;
        if (endX > width) {
            endWeightI -= endX - width;
            endX = width;
        }
        int idxA = startX;
        for (int y = 0; y < height; y++) {
            double v = 0.0;
            
            int idxA_ = idxA;
            for (int i = startWeightI; i < endWeightI; i++) {
                double w = weights[i];
                v += w * rasterA[idxA_ + 0];
                
                idxA_++;
            }
            
            rasterB[idxB] = v;
            
            idxA += rowInc;
            idxB += rowInc;
        }
        idxB += colInc;
    }
    
    double* tmp = rasterA;
    rasterA = rasterB;
    rasterB = tmp;
    
    // Vertical pass
    idxB = 0;
    for (int y = 0; y < height; y++) {
        int startWeightI = 0;
        int startY = y - size;
        if (startY < 0) {
            startWeightI += -startY;
            startY = 0;
        }
        int endWeightI = kernelSize;
        int endY = y + size + 1;
        if (endY > height) {
            endWeightI -= endY - height;
            endY = height;
        }
        int idxA = startY * rowInc;
        for (int x = 0; x < width; x++) {
            double v = 0.0;
            
            int idxA_ = idxA;
            for (int i = startWeightI; i < endWeightI; i++) {
                double w = weights[i];
                v += w * rasterA[idxA_];
                
                idxA_ += rowInc;
            }
            
            rasterB[idxB] = v;
            
            idxA++;
            idxB++;
        }
    }
}