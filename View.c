#include "View.h"

View View_Create(Vector windowSize, Vector position, double zoom) {
    View view;
    
    double scale = pow(2.0, zoom);
    view.offset = (Vector) {
        0.5 * windowSize.x - position.x * scale,
        0.5 * windowSize.y - position.y * scale
    };
    view.scale = (Vector) {
        scale,
        scale
    };
    
    return view;
}

View View_Combine(const View* view1, const View* view2) {
    Vector o1 = view1->offset;
    Vector s1 = view1->scale;
    Vector o2 = view2->offset;
    Vector s2 = view2->scale;
    
    View view3;
    view3.offset = (Vector) {
        o2.x + s2.x * o1.x,
        o2.y + s2.y * o1.y
    };
    view3.scale = (Vector) {
        s2.x * s1.x,
        s2.y * s1.y,
    };
    return view3;
}