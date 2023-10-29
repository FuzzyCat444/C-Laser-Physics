
# This script converts Inkscape Plain SVG exports to C source code for the
# program to construct the level from. This script is not perfect and will fail
# if the format is different from what's expected. Inkscape should be set to
# export absolute coordinates and can only handle polygons. Also, any
# SVG commands besides M at the beginning of coordinate lists or Z at the end
# will cause problems.

levelFile = open("level.svg", "r")
outputFile = open("level.txt", "w")

svgStr = levelFile.read()

pathIndex = -1
i = 1

while True:
    pathIndex = svgStr.find("<path", pathIndex + 1)

    if pathIndex == -1:
        break
        
    endPathIndex = svgStr.find("/>", pathIndex)
    
    idIndex = svgStr.find("id=", pathIndex, endPathIndex)
    dIndex = svgStr.find("d=", pathIndex, endPathIndex)
    if idIndex < dIndex:
        dIndex = svgStr.find("d=", idIndex + 3, endPathIndex)
    
    coordsIndex = dIndex + 5
    endCoordsIndex = svgStr.find(" Z\"", coordsIndex, endPathIndex)
    
    coordsStr = svgStr[coordsIndex : endCoordsIndex]
    
    pairsStrList = coordsStr.split(" ")
    coords = []
    for pairStr in pairsStrList:
        pairList = pairStr.split(",")
        coords.append((float(pairList[0]), float(pairList[1])))
    
    polygonName = "poly{}".format(i)
    
    outputFile.write("Polygon* {} = Polygon_Create();\n".format(polygonName))
    for x, y in coords:
        outputFile.write("Polygon_PushPoint({}, (Vector) {{ {}, {} }});\n"
            .format(polygonName, x, y))
    outputFile.write("App_AddPolygon(app, {});\n".format(polygonName))
    
    i += 1

outputFile.close()
levelFile.close()