/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include "terrain.h"
#include <math.h>
#include <vector>
#include <climits>
#include <fstream>
void terrain::generateTerrain(int width, int height, int minRegions, int maxRegions, string terrainType, bool step, vector<vector<tPixel> >& terrain)
{
    int i;
    if(!step || (step && stepper == 0)) // All of these statements are used to only enable certain parts of the rendering in 'step-by-step' mode
    {
        // Clear out the old settings
        for (i = 0; i < terrain.size(); i++)
            terrain[i].clear();

        terrain.clear();
        for(i=0; i< regionList.size(); i++)
            links[i].clear();
        links.clear();


        for (i = 0; i < points.size(); i++)
            points[i].clear();

        points.clear();
        regionList.clear();
        tSettings.clear();

        // Load the settings from the file
        getSettings(terrainType, tSettings);

        // Figure out how many regions
        srand(time(0));
        numRegions = randomInt(minRegions, maxRegions);

        // Make Region List from settings
        getRegions(width, height, terrain);
        points.resize(numRegions);
        for(i=0; i<numRegions; i++)
            points[i].resize(numRegions);
        // Link regions
        linkRegions(terrain);

        // Set Region types
        setRegionType(terrain);

        // Set Region Heights
        setRegionHeight(terrain);
    }

    if(step) // Increment the step
        stepper = (stepper+1)%8;

    // Smooth regions to make basic heights
    if(!step || (step && stepper == 2))
    {
        /*
        To fully smooth a region (eliminate flat areas), it must undergo enough smoothing operations to reach the center.
        In other words, the first iteration smooths the point on the edge.  The next one smooths the next point, and so on.
        Therefore, we must smooth a number of iterations such that a regions center of mass is affected. The arrangement of
        points with the longest possible distance from outside to center of mass is a circle, so the number of iterations is
        equal to the radius of a circle with an area equal to our largest region.
        */

        // Find largest region
        int area = 0;
        for(int i=0; i<numRegions; i++)
            if(regionList[i].size > area)
                area = regionList[i].size;

        // Find radius if the area of a circle is (pi*r)^2
        int radius = sqrt(area / _PI);

        // Smooth regions that many times
        cout << endl << "Smoothing progress _________________________________________________________________________________" << endl;

        double percentBar = (double)radius/100.0; // Used to make progress bar out of 100%
        double currentPercent = 0;
        int percentCount = 0;

        for(int i=0; i<(int)(radius); i++)
        {
            while(i > currentPercent)
            {
                cout << (char)219;
                currentPercent+=percentBar;
                percentCount++;
            }
            smoothMap(3,terrain);
        }

        while(percentCount < 100) // Fill in remaining progress bar.  (Usually 1-2 steps)
        {
            percentCount++;
            cout << (char)219;
        }
        cout << endl;

        // Change height

        /*
        Because there are significantly more land regions than water, and because of the extreme smoothing,
        once the terrain is generated, it needs to be shifted such that there are more water based areas.
        */

        // Find average lake depth
        double avgHeight = 0;
        int numLakePoints = 0;

        for(int y=0; y<terrain[0].size(); y++) // Find the average water height
        {
            for(int x=0; x<terrain.size(); x++)
            {
                if(regionList[terrain[x][y].region].type == 4 || regionList[terrain[x][y].region].type == 5)
                {
                    avgHeight += terrain[x][y].height;
                    numLakePoints++;
                }
            }
        }

        //avgHeight = (int)((double)avgHeight/(double)(terrain.size()*terrain[0].size()));

        //double shift = (int)((int)(maxPosHeight*0.10) - avgHeight);

        if(numLakePoints > 0) // Only execute if there are water points
        {
            // Calculate shift
            avgHeight = avgHeight/numLakePoints;
            double target = -maxPosHeight*0.075;

            double shift = target - avgHeight;

            cout << endl << "Shifting terrain height: " << endl;
            cout << "Total size of water: " << numLakePoints << ", Average lake height: " << avgHeight << ", Shift amount: ";

            // Only shift downward, never up!
            if(shift < 0)
            {
                cout << shift << endl;
                // Shift all heights
                for(int y=0; y<terrain[0].size(); y++)
                {
                    for(int x=0; x<terrain.size(); x++)
                    {
                        terrain[x][y].height += shift;
                    }
                }
            }
            else
                cout << "0" << endl;
        }
    }

    // Add entropy.
    if(!step || (step && stepper == 3))
    {
        /*
        Entropy is based on two values: Entropy and Percent.

        Percent is the percentage of points in a region that will be affected.

        Entropy is also a percent, but is the percentage of the current height that can be
        added or subtracted from the point. In other words, at height 100 and entropy 50,
        the points resulting height can be 100 +- 50, or from 50 to 150.

        After the entropy is added, the entire map is smoothed again to round it out and create the features.
        */

        cout << endl << "Adding Entropy _____________________________________________________________________________________" << endl;

        // Push all points of a region into a list
        for(int y=0; y<terrain[0].size(); y++)
            for(int x=0; x<terrain.size(); x++)
                points[terrain[x][y].region].push_back(point(x,y));
        double percentBar = (double)numRegions/100.5;
        double currentPercent = 0;
        int percentCount = 0;

        // Add entropy to each region
        int i;
        for(i=0; i<numRegions; i++)
        {
            while(i > currentPercent)
            {
                cout << (char)219;
                currentPercent+=percentBar;
                percentCount++;
            }

            double percent = ((double)points[i].size() * ((double)regionList[i].percent / 100.0));
            for(int j=0; j<percent; j++)
            {

                // Pick a random point from the list
                int index = randomInt(0,points[i].size()-1);
                point thePoint = points[i].at( index );

                // Modify height
                double modBy = (double)randomInt(-regionList[i].entropy,regionList[i].entropy)/100.0;
                terrain[thePoint.x][thePoint.y].height += (modBy*terrain[thePoint.x][thePoint.y].height);

                // Pop point from list
                points[i].erase(points[i].begin() + index);
            }


            // Empty completed region list
            points[i].clear();
        }


        while(percentCount < 100)
        {
            percentCount++;
            cout << (char)219;
        }
        cout << endl;
    }

    // Smooth Entropy
    if(!step || (step && stepper == 4))
        smoothEntropy(terrain.size()/(terrain.size()/512*64+64),terrain);

    // Set colors
    if(!step || (step && stepper >= 5))
        setColors(step, terrain);
    cout << endl;
}

// Creates a map of which regions touch each other
void terrain::linkRegions(vector<vector<tPixel> >& terrain)
{
    // Resize links
    links.resize(regionList.size());
    for(int i=0; i< regionList.size(); i++)
        links[i].resize(regionList.size());

    // Initialize
    for(int y=0; y<regionList.size(); y++)
    {
        for(int x=0; x<regionList.size(); x++)
        {
            links[x][y] = false;
        }
    }

    // Make links. If a point touches another region in 8 space, the connection is made. The links matrix will be
    // false when the region index on the x axis is not connected to the region index on the y axis
    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            // Top
            if(terrain[x][y].region != getRegion(point(x,y-1),terrain) && getRegion(point(x,y-1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x,y-1),terrain)] = true;

            // Top Right
            if(terrain[x][y].region != getRegion(point(x+1,y-1),terrain) && getRegion(point(x+1,y-1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x+1,y-1),terrain)] = true;

            // Right
            if(terrain[x][y].region != getRegion(point(x+1,y),terrain) && getRegion(point(x+1,y),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x+1,y),terrain)] = true;

            // Bottom Right
            if(terrain[x][y].region != getRegion(point(x+1,y+1),terrain) && getRegion(point(x+1,y+1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x+1,y+1),terrain)] = true;

            // Bottom
            if(terrain[x][y].region != getRegion(point(x,y+1),terrain) && getRegion(point(x,y+1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x,y+1),terrain)] = true;

            // Bottom Left
            if(terrain[x][y].region != getRegion(point(x-1,y+1),terrain) && getRegion(point(x-1,y+1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x-1,y+1),terrain)] = true;

            // Left
            if(terrain[x][y].region != getRegion(point(x-1,y),terrain) && getRegion(point(x-1,y),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x-1,y),terrain)] = true;

            // Top Left
            if(terrain[x][y].region != getRegion(point(x-1,y-1),terrain) && getRegion(point(x-1,y-1),terrain) != -1)
                links[terrain[x][y].region][getRegion(point(x-1,y-1),terrain)] = true;
        }
    }
}

// Procedurally sets the type of each region based on a random selection from the most likely regions
void terrain::setRegionType(vector<vector<tPixel> >& terrain)
{
    // Make sure no types are set
    for(int i=0; i<regionList.size(); i++)
        regionList[i].type = -1;

    // Set up a vector of terrain types. The type int is pushed based on the 'liklihood' parameter in settings.
    vector<int> terrainTypes;
    for(int i=0; i<tSettings.size(); i++)
        for(int j=0; j<tSettings[i].likelihood; j++)
            terrainTypes.push_back(i);

    // Randomly pick a region type for region 0
    regionList[0].type = terrainTypes[randomInt(0,terrainTypes.size()-1)];

    // Use a boolean array to keep track of what I can do next
    vector<bool> nextStep;
    nextStep.resize(regionList.size());

    for(int i=0; i<regionList.size(); i++)
        nextStep[i] = false;

    // Now set the array to true for each region it touches
    for(int i=0; i<links[0].size(); i++)
        if(links[0][i])
            nextStep[i] = true;

    // Now, we loop. Find the first 'true' region, and repeat the above process
    bool more;
    int curRegion;
    do
    {
        more = false;
        curRegion = 0;
        while(nextStep[curRegion] == false)
            curRegion++;

        // Push all the possible region types onto a list
        vector<int> possibilities;
        possibilities.clear();
        possibilities.resize(0);

        for(int i=0; i<links.size(); i++)
        {
            if(links[curRegion][i])
            {
                // If it's connected to a region that's already set, take it's possibilities into account
                if(regionList[i].type != -1)
                {
                    for(int j=0; j<tSettings[regionList[i].type].connectTo.size(); j++)
                        possibilities.push_back(tSettings[regionList[i].type].connectTo[j]);
                }
                // If the region is NOT already set, set nextStep to true
                else
                    nextStep[i] = true;
            }
        }

        // Pick a region from the list randomly and set the type.
        regionList[curRegion].type = possibilities[randomInt(0,possibilities.size()-1)];

        // Set this region to false on nextStep
        nextStep[curRegion] = false;

        // Are there any more?
        for(int i=0; i<nextStep.size(); i++)
            if(nextStep[i])
                more = true;
    }
    while(more);

}

// Returns a random integer between min and max, inclusive
int terrain::randomInt(int min, int max)
{
    if(max < min)
    {
        int temp = max;
        max = min;
        min = temp;
    }
    return (rand() % ((max+1)-min)) + min;
}

// Returns the absolute value of a
double absolute(double a)
{
    if (a > 0) return a;
    else return 0 - a;
}

// Returns the next token from 'line', using whitespace of any length to separate tokens
string terrain::nextToken(string &line)
{
    int space, tab, pos;
    string result;

    // Remove leading whitespace
    while(line.at(0) == ' ' || line.at(0) == '\t')
        line = line.substr(1);

    // Find next whitespace
    space = line.find_first_of(' ');
    tab = line.find_first_of('\t');
    if (space < tab && space != -1)
        pos = space;
    else
        pos = tab;

    // If string is empty or does not contain whitespace, return line
    if(pos == -1)
        return line;

    // Set result to the next token
    result = line.substr(0,pos);

    // Remove token from string
    line = line.substr(pos);

    return result;
}

// Splits string into ints based on 'token' and fills 'result'
void terrain::splitStr(string theString, vector<int>& result, char token)
{
    result.clear();
    result.resize(1);
    int index;

    while(theString.find_first_of(token) != string::npos)
    {
        index = theString.find_first_of(token);
        result.push_back(atoi(theString.substr(0,index).c_str()));
        theString = theString.substr(index+1);
    }
}

// Splits string into smaller strings based on 'token' and fills 'result'
void terrain::splitStr(string theString, vector<string>& result, char token)
{
    result.clear();
    result.resize(1);
    int index;

    while(index = theString.find_first_of(token) != -1)
        result.push_back(theString.substr(0,index));
}

// Loads the region setting file.
void terrain::getSettings(string type, vector<terrainType>& settingsVector)
{
    string line;
    string terList[] = {"General", "Highlands", "Lowlands", "Plains", "Swamp", "Water"};
    string filename;
    bool found = false;
    int index = randomInt(0,5);

    // If type is specified, use that file
    for(int i=0; i<6; i++)
        if(type.compare(terList[i]) == 0)
        {
            filename = "terrain" + type + ".txt";
            index = i;
        }

    // If random is specified, load a random type
    if(type.compare("Random") == 0)
        filename = "terrain" + terList[index] + ".txt";
    cout << "Generating terrain of type: " << terList[index] << endl << endl;

    ifstream settings (filename.c_str());
    cout<<"size of tSettings is "<<tSettings.size()<<endl;
    if (settings.is_open())
    {
        // Ignore line with column titles
        getline (settings,line);

        while ( settings.good() )
        {
            getline (settings,line);
            // Save settings
            terrainType curTerrain;
            curTerrain.name = nextToken(line);
            curTerrain.minHt = atoi(nextToken(line).c_str());
            curTerrain.maxHt = atoi(nextToken(line).c_str());
            curTerrain.entropy = atoi(nextToken(line).c_str());
            curTerrain.percent = atoi(nextToken(line).c_str());
            curTerrain.likelihood = strtod(nextToken(line).c_str(),NULL);
            splitStr(nextToken(line),curTerrain.connectTo,',');

            tSettings.push_back(curTerrain);
        }
        settings.close();
    }
    else
        cout << "The file is not opened." << endl;
}

// Returns true if the point specified is within the map bounds
bool terrain::isValid(point thePoint, vector<vector<tPixel> >& terrain)
{
    if(thePoint.x < 0 || thePoint.x >= terrain.size() || thePoint.y < 0 || thePoint.y >= terrain[0].size())
        return false;
    return true;
}

// Returns true if the specified point has a region already set
bool terrain::hasRegion(point thePoint, vector<vector<tPixel> >& terrain)
{
    if(!isValid(thePoint, terrain))
        return true; // Returns true if it is out of bounds so caller doesn't try to use it
    else if(terrain[thePoint.x][thePoint.y].region == -1)
        return false; // If point doesn't have a region, say so
    return true;
}

// Returns the region type of the specified point
int terrain::getRegion(point thePoint, vector<vector<tPixel> >& terrain)
{
    if(!isValid(thePoint, terrain))
        return -1;
    else
        return terrain[thePoint.x][thePoint.y].region;
}

// Initialize a single point
bool terrain::setPoint(point thePoint, int theRegion, vector<vector<tPixel> >& terrain)
{
    if(!hasRegion(point(thePoint.x,thePoint.y),terrain))
    {
        terrain[thePoint.x][thePoint.y].region = theRegion;
        points[theRegion].push_back(point(thePoint.x,thePoint.y));
        terrain[thePoint.x][thePoint.y].height = (terrain.size()/6) * (regionList[theRegion].height/100.0);
        regionList[theRegion].size++;
        return true;
    }
    return false;
}

// Initialize all the valid neighboring points
void terrain::setNeighbors(point thePoint, int theRegion, vector<vector<tPixel> >& terrain)
{
    // Above
    setPoint(point(thePoint.x,thePoint.y-1),theRegion,terrain);

    // Right
    setPoint(point(thePoint.x+1,thePoint.y),theRegion,terrain);

    // Below
    setPoint(point(thePoint.x,thePoint.y+1),theRegion,terrain);

    // Left
    setPoint(point(thePoint.x-1,thePoint.y),theRegion,terrain);
}

// Recursively grows the regions to fill the map
void terrain::getRegions(int hSize, int vSize, vector<vector<tPixel> >& terrain)
{
    cout << "Creating regions..." << endl;

    int xMax = hSize-1, yMax = vSize-1;
    int i;
    // Resize to proper dimensions
    regionList.resize(numRegions);
    points.resize(numRegions);
    for(i=0; i<numRegions; i++)
        points[i].resize(numRegions);
    terrain.resize(hSize);
    for(i=0; i<hSize; i++)
        terrain[i].resize(vSize);

    // Initialize terrain
    for(int y=0; y<yMax+1; y++)
    {
        for(int x=0; x<xMax+1; x++)
        {
            terrain[x][y].region = -1;
            terrain[x][y].enabled = false;
        }
    }

    // Initialize region list
    for(i=0; i<numRegions; i++)
    {
        regionList[i].type = -1;
        regionList[i].size = 0;
    }

    // Set region centers
    point currentCenter;
    for(i=0; i<numRegions; i++)
    {

        currentCenter.x = randomInt(0,xMax);
        currentCenter.y = randomInt(0,yMax);

        points[i].push_back(currentCenter);
    }

    // Grow regions
    int counter = 0;
    do
    {
        int theRegion = counter%points.size();
        int index;
        if(points[theRegion].size() > 0)
        {
            // Randomly pick a point from the current region
            index = randomInt(0,points[theRegion].size()-1);

            // All empty 4-space neighbors set to region and pushed onto the queue
            point currentPoint = points[theRegion].at(index);			// Get point
            points[theRegion].erase(points[theRegion].begin() + index);	// Pop point
            setNeighbors(currentPoint,theRegion,terrain);				// Set all valid neighbors
        }

        // If current region is empty, remove it
        if(points[counter%points.size()].size() <= 0)
            points.erase(points.begin()+counter%points.size());

        // Otherwise, increment the counter
        else
            counter++;
    }
    while(points.size() > 0);
}

// Sets the height of every point in the map to its region height
void terrain::setRegionHeight(vector<vector<tPixel> >& terrain)
{
    cout << endl << "Setting initial region height..." << endl;
    int xMax = terrain.size(), yMax = terrain[0].size();
    maxPosHeight = (int)((double)xMax/pow(((double)xMax/2.0),(1.0/3.25)));

    // Fill region list
    int minHt, maxHt;
    for(int i=0; i<numRegions; i++)
    {
        regionList[i].entropy = tSettings[regionList[i].type].entropy;
        regionList[i].percent = tSettings[regionList[i].type].percent;

        minHt = ((double)tSettings[regionList[i].type].minHt/100.0) * maxPosHeight;
        maxHt = ((double)tSettings[regionList[i].type].maxHt/100.0) * maxPosHeight;

        regionList[i].height = randomInt(minHt, maxHt);
    }

    for(int y=0; y< yMax; y++)
    {
        for(int x=0; x<xMax; x++)
        {
            regionList[terrain[x][y].region].size++;
            terrain[x][y].height = regionList[terrain[x][y].region].height;
            terrain[x][y].color[R] = 255;
            terrain[x][y].color[G] = 255;
            terrain[x][y].color[B] = 255;
            terrain[x][y].color[A] = 255;
        }
    }

    // Output formatting
    string top = "                            Region list                            ";
    top[0] = (char)218;
    for(int i=1; i<27; i++)
        top[i] = (char)196;
    for(int i=40; i<66; i++)
        top[i] = (char)196;
    top[66] = (char)191;

    string middle = "                                                                   ";
    for(int i=0; i<middle.size(); i++)
        middle[i] = (char)196;
    middle[0] = (char)195;
    middle[66] = (char)180;

    string bottom = "                                                                   ";
    for(int i=0; i<bottom.size(); i++)
        bottom[i] = (char)196;
    bottom[0] = (char)192;
    bottom[66] = (char)217;


    // Print Regions
    cout << endl << top << endl;
    cout << (char)179 <<  "  Region       Type     Height       Size    Entropy     Pecent  " << (char)179 << endl;
    cout  <<  middle << endl;
    for(int i=0; i<numRegions; i++)
        cout << (char)179 << "  " << setfill(' ') << setw(6) << i << setw(11) << regionList[i].type << setw(11) << regionList[i].height << setw(11) << regionList[i].size << setw(11) << regionList[i].entropy << setw(10) << regionList[i].percent << "%  " << (char)179 << endl;
    cout << bottom << endl;
}

// Sets the specified point to the average of all values in the window, specified by size.
void terrain::smoothPoint(point thePoint, int size, vector<vector<tPixel> >& terrain)
{
    double value = 0;
    int num = 0;

    for(int i=thePoint.x-size; i<=thePoint.x+size; i++)
    {
        for(int j=thePoint.y-size; j<=thePoint.y+size; j++)
        {
            if(isValid(point(i,j),terrain))
            {
                value += getValue(i,j,terrain);
                num++;
            }
        }
    }

    value = value/num;
    terrain[thePoint.x][thePoint.y].height = value;
}

// Color the map
void terrain::setColors(bool step, vector<vector<tPixel> >& terrain)
{
    // First, we set colors based solely on elevation.
    /*
    -100% to -25%  : dark blue to light blue
    -25%  to  0%   : light blue to sand
     0%   to  10%  : sand to green
     10%  to  30%  : green
     30%  to  40%  : green to grey
     40%  to  80%  : grey
     80%  to  85%  : grey to white
     85%  to  100% : white
    */

    // Define ground type colors
    double _WATER_LOW[]     = { 0, 0, 127.5 };
    double _WATER_HIGH[]    = { 128, 128, 204 };
    double _SAND[]     = { 235, 230, 199 };
    double _GRASS[]     = { 0, 127.5, 0 };
    double _ROCK[]     = { 127.5, 127.5, 127.5};
    double _DIRT[]     = { 171, 125, 36 };
    double _SNOW[]     = { 255, 255, 255 };

    // Define elevation changes
    double _DEEP_WATER = 1;
    double _SHALLOW_WATER = 0.1;
    double _WATER_LINE = 0;
    double _BEGIN_GRASS = 0.1;
    double _END_GRASS = 0.5;
    double _BEGIN_MOUNTAIN = 0.6;
    double _END_MOUNTAIN = 0.77;
    double _SNOW_LINE = 0.85;

    // Min and max heights
    double minimum = getMinHt(terrain);
    double maximum = maxPosHeight;
    if(maximum < getMaxHt(terrain))
        maximum = getMaxHt(terrain);

    double thisHt;
    double percent;

    if(!step || (step && stepper == 5))
    {
        std::cout << endl << "Setting color based on height..." << endl;

        for(int y=0; y<terrain[0].size(); y++)
        {
            for(int x=0; x<terrain.size(); x++)
            {
                // Find percent
                thisHt = terrain[x][y].height;

                if(thisHt < _WATER_LINE) // Below water
                {
                    percent = (double)thisHt/(double)minimum;
                    if(percent > _SHALLOW_WATER && percent <= _DEEP_WATER) // Deep water
                    {
                        percent = (percent-_SHALLOW_WATER)*(1.0/(1.0-_SHALLOW_WATER));

                        terrain[x][y].color[R] = _WATER_LOW[R] + (_WATER_HIGH[R] - _WATER_LOW[R])*(1-percent);
                        terrain[x][y].color[G] = _WATER_LOW[G] + (_WATER_HIGH[G] - _WATER_LOW[G])*(1-percent);
                        terrain[x][y].color[B] = _WATER_LOW[B] + (_WATER_HIGH[B] - _WATER_LOW[B])*(1-percent);
                    }
                    else // Shallow water
                    {
                        percent /= _SHALLOW_WATER;
                        terrain[x][y].color[R] = _WATER_HIGH[R] + (_SAND[R]-_WATER_HIGH[R])*(1-percent);
                        terrain[x][y].color[G] = _WATER_HIGH[G] + (_SAND[G]-_WATER_HIGH[G])*(1-percent);
                        terrain[x][y].color[B] = _WATER_HIGH[B] + (_SAND[B]-_WATER_HIGH[B])*(1-percent);
                    }
                }
                else // Above water
                {
                    percent = (double)thisHt/(double)maximum;
                    if(percent < _BEGIN_GRASS) // Beach
                    {
                        percent = (percent-_WATER_LINE)*(1.0/(_BEGIN_GRASS - _WATER_LINE));

                        terrain[x][y].color[R] = _SAND[R] + (_GRASS[R]-_SAND[R])*(percent);
                        terrain[x][y].color[G] = _SAND[G] + (_GRASS[G]-_SAND[G])*(percent);
                        terrain[x][y].color[B] = _SAND[B] + (_GRASS[B]-_SAND[B])*(percent);
                    }
                    else if(percent < _END_GRASS) // Grass
                    {
                        percent = (percent-_BEGIN_GRASS)*(1.0/(_END_GRASS - _BEGIN_GRASS));
                        terrain[x][y].color[R] = _GRASS[R];
                        terrain[x][y].color[G] = _GRASS[G];
                        terrain[x][y].color[B] = _GRASS[B];
                    }
                    else if(percent < _BEGIN_MOUNTAIN) // Mountain
                    {
                        percent = (percent-_END_GRASS)*(1.0/(_BEGIN_MOUNTAIN - _END_GRASS));

                        terrain[x][y].color[R] = _GRASS[R] + (_ROCK[R]-_GRASS[R])*(percent);
                        terrain[x][y].color[G] = _GRASS[G] + (_ROCK[G]-_GRASS[G])*(percent);
                        terrain[x][y].color[B] = _GRASS[B] + (_ROCK[B]-_GRASS[B])*(percent);
                    }
                    else if(percent < _END_MOUNTAIN) // Mountain to snow transition
                    {
                        percent = (percent-_BEGIN_MOUNTAIN)*(1.0/(_END_MOUNTAIN - _BEGIN_MOUNTAIN));

                        terrain[x][y].color[R] = _ROCK[R] + (_SNOW[R]-_ROCK[R])*(percent);
                        terrain[x][y].color[G] = _ROCK[G] + (_SNOW[G]-_ROCK[G])*(percent);
                        terrain[x][y].color[B] = _ROCK[B] + (_SNOW[B]-_ROCK[B])*(percent);
                    }
                    else if(percent < _SNOW_LINE) // Snow
                    {
                        percent = (percent-_END_MOUNTAIN)*(1.0/(_SNOW_LINE - _END_MOUNTAIN));

                        terrain[x][y].color[R] = _SNOW[R] + (_SNOW[R]-_SNOW[R])*(percent);
                        terrain[x][y].color[G] = _SNOW[G] + (_SNOW[G]-_SNOW[G])*(percent);
                        terrain[x][y].color[B] = _SNOW[B] + (_SNOW[B]-_SNOW[B])*(percent);
                    }
                    else // Should never get here!
                    {
                        terrain[x][y].color[R] = _SNOW[R];
                        terrain[x][y].color[G] = _SNOW[G];
                        terrain[x][y].color[B] = _SNOW[B];
                    }
                }
            }
        }
    }

    // Next, we use the slope to add grey to steep hills
    if(!step || (step && stepper == 6))
    {
        cout << endl << "Setting color based on slope..." << endl;

        double minS=INT_MAX, maxS=INT_MIN;
        double thisSlope = 0;
        double thisHeight = 0;
        double avgS = 0;
        double avgH = 0;
        int numPoints = 0;

        for(int y=0; y<terrain[0].size(); y++)
        {
            for(int x=0; x<terrain.size(); x++)
            {
                avgS += pointMaxSlope(point(x,y),terrain);
                avgH += terrain[x][y].height;
                numPoints++;
            }
        }

        avgS = avgS/numPoints; // Finds the average slope of all points
        avgH = avgH/numPoints; // Finds the average height of all points

        double theSlope, percentS, theHeight, mH = getMinHt(terrain), pH;

        for(int y=0; y<terrain[0].size(); y++)
        {
            for(int x=0; x<terrain.size(); x++)
            {
                theSlope = pointMaxSlope(point(x,y),terrain);

                percentS = (theSlope-(avgS*0.15))/(avgS);

                theHeight = terrain[x][y].height;

                if(percentS > 1)
                    percentS = 1;
                if(percentS < 0)
                    percentS = 0;

                if(theHeight < 0)
                {
                    pH = 5*theHeight/mH;
                    percentS = percentS * (1-pH);
                }
                if(theHeight > mH+_SNOW_LINE*(maximum-mH))
                {
                    percentS = 0;
                }

                if(percentS > 1)
                    percentS = 1;
                if(percentS < 0)
                    percentS = 0;

                terrain[x][y].color[R] = terrain[x][y].color[R]*(1-percentS) + _ROCK[R]*percentS;
                terrain[x][y].color[G] = terrain[x][y].color[G]*(1-percentS) + _ROCK[G]*percentS;
                terrain[x][y].color[B] = terrain[x][y].color[B]*(1-percentS) + _ROCK[B]*percentS;
            }
        }
    }

    // Finally, we smooth the colors.
    if(!step || (step && stepper == 7))
        smoothColors(1,terrain);

}

// Returns the maximum slope at specified point
double terrain::pointMaxSlope(point thePoint, vector<vector<tPixel> >& terrain)
{
    double maxSlope = 0, thisSlope = 0;
    double thisHt = terrain[thePoint.x][thePoint.y].height;
    // Top
    if(isValid(point(thePoint.x,thePoint.y-1),terrain))
    {
        thisSlope = absolute(thisHt-terrain[thePoint.x][thePoint.y-1].height);
        if(thisSlope > maxSlope)
            maxSlope = thisSlope;
    }

    // Right
    if(isValid(point(thePoint.x+1,thePoint.y),terrain))
    {
        thisSlope = absolute(thisHt-terrain[thePoint.x+1][thePoint.y].height);
        if(thisSlope > maxSlope)
            maxSlope = thisSlope;
    }

    // Bottom
    if(isValid(point(thePoint.x,thePoint.y+1),terrain))
    {
        thisSlope = absolute(thisHt-terrain[thePoint.x][thePoint.y+1].height);
        if(thisSlope > maxSlope)
            maxSlope = thisSlope;
    }

    // Left
    if(isValid(point(thePoint.x-1,thePoint.y),terrain))
    {
        thisSlope = absolute(thisHt-terrain[thePoint.x-1][thePoint.y].height);
        if(thisSlope > maxSlope)
            maxSlope = thisSlope;
    }
    return maxSlope;

}

// Uses a window of size (2*size)+1 to smooth map
void terrain::smoothMap(int size, vector<vector<tPixel> >& terrain)
{
    int xMax = terrain.size();
    int yMax = terrain[0].size();

    double value = 0;
    double num = 0;

    int wSize = size;

    for(int y=0; y<yMax; y++)
    {
        for(int x=0; x<xMax; x++)
        {
            if(regionList[terrain[x][y].region].type != 3 && regionList[terrain[x][y].region].type != 1) // Plateaus and mountains
                smoothPoint(point(x,y),size,terrain);
            else if(regionList[terrain[x][y].region].type == 3) // Plateaus
                smoothPoint(point(x,y),1,terrain);
            else
                smoothPoint(point(x,y),size*2,terrain); // Mountains
        }
    }
}

// Smooths map without special handling for plateaus and mountains
void terrain::smoothEntropy(int size, vector<vector<tPixel> >& terrain)
{
    int xMax = terrain.size();
    int yMax = terrain[0].size();

    double value = 0;
    double num = 0;

    int wSize = size;

    cout << endl << "Smoothing Entropy __________________________________________________________________________________" << endl;

    double percentBar = (double)terrain[0].size()/100.0;
    double currentPercent = 0;
    int percentCount = 0;

    for(int y=0; y<yMax; y++)
    {
        while(y > currentPercent)
        {
            cout << (char)219;
            currentPercent+=percentBar;
            percentCount++;
        }
        for(int x=0; x<xMax; x++)
            smoothPoint(point(x,y),size,terrain);
    }

    while(percentCount < 100)
    {
        percentCount++;
        cout << (char)219;
    }

    cout << endl;
}

// Smooths the specified point's color based on its neighbors
void terrain::smoothColors(int size, vector<vector<tPixel> >& terrain)
{
    int xMax = terrain.size();
    int yMax = terrain[0].size();

    double allR = 0, allG = 0, allB = 0;
    double num = 0;

    int wSize = size;

    cout << endl << "Smoothing Colors ___________________________________________________________________________________" << endl;
    double percentBar = (double)terrain[0].size()/100.0;
    double currentPercent = 0;
    int percentCount = 0;

    for(int y=0; y<yMax; y++)
    {
        while(y > currentPercent)
        {
            cout << (char)219;
            currentPercent+=percentBar;
            percentCount++;
        }

        for(int x=0; x<xMax; x++)
        {
            allR = 0;
            allG = 0;
            allB = 0;
            int num = 0;

            for(int i=x-size; i<=x+size; i++)
            {
                for(int j=y-size; j<=y+size; j++)
                {
                    if(isValid(point(i,j),terrain))
                    {
                        allR += terrain[i][j].color[R];
                        allG += terrain[i][j].color[G];
                        allB += terrain[i][j].color[B];
                        num++;
                    }
                }
            }

            allR = allR/num;
            allG = allG/num;
            allB = allB/num;
            terrain[x][y].color[R] = allR;
            terrain[x][y].color[G] = allG;
            terrain[x][y].color[B] = allB;
        }
    }

    while(percentCount < 100)
    {
        percentCount++;
        char thing = 219;
        cout << thing;
    }

    cout << endl;
}

// Returns the value at point X,Y
double terrain::getValue(int X, int Y, vector<vector<tPixel> >& tArray)
{
    if(X<0 || Y<0 || X>=tArray.size() || Y>=tArray[0].size())
        return 0;
    else
        return tArray[X][Y].height;
}

// Prints a text matrix of the regions
void terrain::printRegions(vector<vector<tPixel> >& terrain)
{
    cout << "---------------------------------------- Regions ----------------------------------------" << endl;
    for(int x=0; x<regionList.size(); x++)
        cout << x << ":  Height: " << regionList[x].height << ".  Size: "<< regionList[x].size << ".  Entropy: " << regionList[x].entropy << endl;
    cout << endl;

    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            cout << setfill(' ') << setw(5) << terrain[x][y].region;
        }
        cout << endl;
    }
    cout << endl << "-----------------------------------------------------------------------------------------" << endl << endl;
}

// Prints a text matrix of the height map
void terrain::printHeight(vector<vector<tPixel> >& terrain)
{
    cout << "---------------------------------------- Height -----------------------------------------" << endl;

    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            cout << setfill(' ') << setw(7) << terrain[x][y].height;
        }
        cout << endl;
    }
    cout << endl << "-----------------------------------------------------------------------------------------" << endl << endl;
}

// Prints a text matrix of the type
void terrain::printType(vector<vector<tPixel> >& terrain)
{
    cout << "----------------------------------------- Type ------------------------------------------" << endl;

    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            cout << setfill(' ') << setw(5) << regionList[terrain[x][y].region].type;
        }
        cout << endl;
    }
    cout << endl << "-----------------------------------------------------------------------------------------" << endl << endl;
}

// Returns the type of the region at regionNumber
int terrain::getType(int regionNumber)
{
    return regionList[regionNumber].type;
}

// Returns the lowest elevation on the map
int terrain::getMinHt(vector<vector<tPixel> >& terrain)
{
    int minimum = INT_MAX;
    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            if(terrain[x][y].height < minimum)
                minimum = terrain[x][y].height;
        }
    }
    return minimum;
}

// Returns the highest elevation on the map
int terrain::getMaxHt(vector<vector<tPixel> >& terrain)
{
    int maximum = INT_MIN;
    for(int y=0; y<terrain[0].size(); y++)
    {
        for(int x=0; x<terrain.size(); x++)
        {
            if(terrain[x][y].height > maximum)
                maximum = terrain[x][y].height;
        }
    }
    return maximum;
}

// Reset the current step to 0
void terrain::resetStep()
{
    stepper = 0;
}