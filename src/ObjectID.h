#ifndef OBJECTID_H
#define OBJECTID_H

typedef struct ObjectID {
    int polygonIndex;
    int lineVertexIndex;
} ObjectID;

inline int sameID(ObjectID id1, ObjectID id2) {
    return id1.polygonIndex == id2.polygonIndex && 
           id1.lineVertexIndex == id2.lineVertexIndex &&
           id1.polygonIndex != -1;
}

#endif