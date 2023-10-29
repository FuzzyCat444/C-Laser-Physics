#ifndef VIEW_H
#define VIEW_H

#include <stdlib.h>
#include <math.h>

#include "Vector.h"

typedef struct View {
    Vector offset;
    Vector scale;
} View;

View View_Create(Vector windowSize, Vector position, double zoom);
    
// view1 applied first, then view2
View View_Combine(const View* view1, const View* view2);

#endif