/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#ifndef terrain_h
#define terrain_h

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include "glVector3.h"
#include <math.h>

using namespace std;

#define R 0	// Index R in color type
#define G 1	// Index G in color type
#define B 2	// Index B in color type
#define A 3	// Index A in color type
#define _PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679 // Yeah, it's overkill, but why not.

struct terrainType // Used to store main terrain type settings
{
    string name;
    int minHt;
    int maxHt;
    int entropy;
    int percent;
    int likelihood;
    vector<int> connectTo;
};

struct regionType // Used in the region list to stoe the diffrent regions and their attributes
{
    int type;
    int entropy;
    int percent;
    int size;
    int height;
};

struct tPixel // Used in the terrain map, the basic unit of terrain.
{
    double height;
    double color[4]; // Access with indices R, G, B, A
    int texture;
    int region;
    glVector3 normal;
    bool enabled;
};

struct point // Stores the location of a point
{
    point()
    {
        x = 0;
        y = 0;
    }
    point(int X, int Y)
    {
        x = X;
        y = Y;
    }
    int x;
    int y;
};

class terrain
{
public:
    // Main public function
    void generateTerrain(int width, int height, int minRegions, int maxRegions, string terrainType, bool step, vector<vector<tPixel> >& terrain);

    // Print functions
    void printRegions(vector<vector<tPixel> >& terrain);
    void printHeight(vector<vector<tPixel> >& terrain);
    void printType(vector<vector<tPixel> >& terrain);

    // Data access functions
    int getType(int regionNumber);
    int getMaxHt(vector<vector<tPixel> >& terrain);
    int getMinHt(vector<vector<tPixel> >& terrain);

    // Reset "step by step"
    void resetStep();

private:
    // Member variables
    double maxPosHeight;
    int numRegions;
    int stepper;
    vector<regionType> regionList;			// A list of specific regions and their settings
    vector<vector<point> > points;			// Used for keeping track of points in region generation
    vector<vector<bool> > links;			// Used to store region links
    vector<terrainType> tSettings;			// The generic settings

    // Basic helper functions
    double fMin(double a, double b);
    double fMax(double a, double b);
    int randomInt(int min, int max);
    double randomDouble(double min, double max);
    string nextToken(string &line);
    void splitStr(string theString, vector<int>& result, char token);
    void splitStr(string theString, vector<string>& result, char token);

    // Main functions
    void getSettings(string type, vector<terrainType>& settingsVector);
    void getRegions(int hSize, int vSize, vector<vector<tPixel> >& terrain);
    void setRegionType(vector<vector<tPixel> >& terrain);
    void setRegionHeight(vector<vector<tPixel> >& terrain);
    void setColors(bool step, vector<vector<tPixel> >& terrain);

    // Specialized helper functions
    bool isValid(point thePoint, vector<vector<tPixel> >& terrain);
    bool hasRegion(point thePoint, vector<vector<tPixel> >& terrain);
    int getRegion(point thePoint, vector<vector<tPixel> >& terrain);
    void setNeighbors(point thePoint, int theRegion, vector<vector<tPixel> >& terrain);
    double pointMaxSlope(point thePoint, vector<vector<tPixel> >& terrain);
    double getValue(int X, int Y, vector<vector<tPixel> >& tArray);

    bool setPoint(point thePoint, int theRegion, vector<vector<tPixel> >& terrain);
    void linkRegions(vector<vector<tPixel> >& terrain);

    void smoothPoint(point thePoint, int size, vector<vector<tPixel> >& terrain);
    void smoothEntropy(int size, vector<vector<tPixel> >& terrain);
    void smoothColors(int size, vector<vector<tPixel> >& terrain);
    void smoothMap(int size, vector<vector<tPixel> >& tArray);
};

#endif

