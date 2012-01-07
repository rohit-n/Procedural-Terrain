/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "camera.h"
#include <cmath>
#include "terrain.h"
#include <time.h>
#include <vector>

using namespace std;

int terrainDim = 128;
int minRegions = 35;
int maxRegions = 50;
string terrainType = "random";
int windowX=800, windowY=600, bpp=32;
bool fullscreen = false;
Uint32 lastSecond = 0;
int frameCount = 0;
Camera cam;
GLuint list;
bool wireframe = false;
char fps[5];
vector<vector<tPixel> > terrainMatrix;
terrain theTerrain;
int worldScale = 1;
float error = 0.2; //used for checking if vertices should be enabled

void computeNormals() //thank you, VideoTutorialsRock.
{
    glVector3** normals2 = new glVector3*[terrainMatrix.size()];
    for(int i = 0; i < terrainMatrix.size(); i++)
    {
        normals2[i] = new glVector3[terrainMatrix[0].size()];
    }

    for(int z = 0; z < terrainMatrix.size(); z++)
    {
        for(int x = 0; x < terrainMatrix[0].size(); x++)
        {
            glVector3 sum(0.0f, 0.0f, 0.0f);

            glVector3 out;
            if (z > 0)
            {
                out = glVector3(0.0f, terrainMatrix[z-1][x].height - terrainMatrix[z][x].height, -1.0f);
            }
            glVector3 in;
            if (z < terrainMatrix.size() - 1)
            {
                in = glVector3(0.0f, terrainMatrix[z+1][x].height - terrainMatrix[z][x].height, 1.0f);
            }
            glVector3 left;
            if (x > 0)
            {
                left = glVector3(-1.0f, terrainMatrix[z][x-1].height - terrainMatrix[z][x].height, 0.0f);
            }
            glVector3 right;
            if (x < terrainMatrix[0].size() - 1)
            {
                right = glVector3(1.0f, terrainMatrix[z][x+1].height - terrainMatrix[z][x].height, 0.0f);
            }

            if (x > 0 && z > 0)
            {
                sum += out.cross(left).normalize();
            }
            if (x > 0 && z < terrainMatrix.size() - 1)
            {
                sum += left.cross(in).normalize();
            }
            if (x < terrainMatrix[0].size() - 1 && z < terrainMatrix.size() - 1)
            {
                sum += in.cross(right).normalize();
            }
            if (x < terrainMatrix[0].size() - 1 && z > 0)
            {
                sum += right.cross(out).normalize();
            }

            normals2[z][x] = sum;
        }
    }

    const float FALLOUT_RATIO = 0.5f;
    for(int z = 0; z < terrainMatrix.size(); z++)
    {
        for(int x = 0; x < terrainMatrix[0].size(); x++)
        {
            glVector3 sum = normals2[z][x];

            if (x > 0)
            {
                sum += normals2[z][x - 1] * FALLOUT_RATIO;
            }
            if (x < terrainMatrix[0].size() - 1)
            {
                sum += normals2[z][x + 1] * FALLOUT_RATIO;
            }
            if (z > 0)
            {
                sum += normals2[z - 1][x] * FALLOUT_RATIO;
            }
            if (z < terrainMatrix.size() - 1)
            {
                sum += normals2[z + 1][x] * FALLOUT_RATIO;
            }

            if (sum.magnitude() == 0)
            {
                sum = glVector3(0.0f, 1.0f, 0.0f);
            }
            terrainMatrix[z][x].normal = sum.normalize();
        }
    }

    for(int i = 0; i < terrainMatrix.size(); i++)
    {
        delete[] normals2[i];
    }
    delete[] normals2;
}

int isPowerOfTwo (int x) //needed to see if terrain can be represented as a quadtree
{
    while (((x % 2) == 0) && x > 1)
        x /= 2;
    return (x == 1);
}

bool shouldDivide(int x1, int x2, int z1, int z2)
{
    int xMid = (x1 + x2) / 2;
    int zMid = (z1 + z2) / 2;
    double height;
    double disabledHeight;
    //test top point
    height = terrainMatrix[xMid][z1].height;
    disabledHeight = (terrainMatrix[x1][z1].height + terrainMatrix[x2][z1].height) / 2;
    if (abs(height - disabledHeight) > error)
    {
        return true;
    }
    // test right point
    height = terrainMatrix[x2][zMid].height;
    disabledHeight = (terrainMatrix[x2][z1].height + terrainMatrix[x2][z2].height) / 2;
    if (abs(height - disabledHeight) > error)
    {
        return true;
    }
    // test bottom point
    height = terrainMatrix[xMid][z2].height;
    disabledHeight = (terrainMatrix[x1][z2].height + terrainMatrix[x2][z2].height) / 2;
    if (abs(height - disabledHeight) > error)
    {
        return true;
    }
    // test left point
    height = terrainMatrix[x1][zMid].height;
    disabledHeight = (terrainMatrix[x1][z2].height + terrainMatrix[x1][z1].height) / 2;
    if (abs(height - disabledHeight) > error)
    {
        return true;
    }
    return false;
}
//			A quadtree block.

//			(x1, z1)		(xMid, z1)			(x2, z1)
//			*-----------------------------------*
//			 |       		 					|
//			 |       		 					|
//			 |       		 					|
//			 |       		 					|
//(x1, zMid) |		 		(xMid, zMid) 		| (x2, zMid)
//			 |		 		 					|
//			 |		 		 					|
//			 |		 		 					|
//			 |		 		 					|
//			 |		 		 					|
//			*-----------------------------------*
//			(x1, z2)		 (zMid, z2)			(x2, z2)

void divideBlock(int x1, int x2, int z1, int z2)
{
    //enable the block
    int xMid = (x1 + x2) / 2;
    int zMid = (z1 + z2) / 2;

    terrainMatrix[xMid][zMid].enabled = true;
    terrainMatrix[x1][z1].enabled = true;
    terrainMatrix[x1][z2].enabled = true;
    terrainMatrix[x2][z1].enabled = true;
    terrainMatrix[x2][z2].enabled = true;

    //check if needing to enable edge points
    {
        double height;
        double disabledHeight;
        //test top point
        height = terrainMatrix[xMid][z1].height;
        disabledHeight = (terrainMatrix[x1][z1].height + terrainMatrix[x2][z1].height) / 2;
        if (abs(height - disabledHeight) > error)
        {
            terrainMatrix[xMid][z1].enabled = true;
        }
        // test right point
        height = terrainMatrix[x2][zMid].height;
        disabledHeight = (terrainMatrix[x2][z1].height + terrainMatrix[x2][z2].height) / 2;
        if (abs(height - disabledHeight) > error)
        {
            terrainMatrix[x2][zMid].enabled = true;
        }
        // test bottom point
        height = terrainMatrix[xMid][z2].height;
        disabledHeight = (terrainMatrix[x1][z2].height + terrainMatrix[x2][z2].height) / 2;
        if (abs(height - disabledHeight) > error)
        {
            terrainMatrix[xMid][z2].enabled = true;
        }
        // test left point
        height = terrainMatrix[x1][zMid].height;
        disabledHeight = (terrainMatrix[x1][z2].height + terrainMatrix[x1][z1].height) / 2;
        if (abs(height - disabledHeight) > error)
        {
            terrainMatrix[x1][zMid].enabled = true;
        }
    }

    if (x2 - x1 > 2) //block can be divided more - need to decide which ones to
    {
        //test if upper left should be divided
        if (shouldDivide(x1, xMid, z1, zMid))
        {
            divideBlock(x1, xMid, z1, zMid);
        }
        //test if upper right should be divided
        if (shouldDivide(xMid, x2, z1, zMid))
        {
            divideBlock(xMid, x2, z1, zMid);
        }
        //test if bottom right should be divided
        if (shouldDivide(xMid, x2, zMid, z2))
        {
            divideBlock(xMid, x2, zMid, z2);
        }
        //test if bottom left should be divided
        if (shouldDivide(x1, xMid, zMid, z2))
        {
            divideBlock(x1, xMid, zMid, z2);
        }
    }
}

bool quadPointAt(int r, int c, bool ignore)
{
    if (terrainMatrix[r][c].enabled || ignore)
    {
        glColor3f(terrainMatrix[r][c].color[R]/255.0, terrainMatrix[r][c].color[G]/255.0, terrainMatrix[r][c].color[B]/255.0);
        glVector3 normal = terrainMatrix[r][c].normal;
        glNormal3f(normal[0], normal[1], normal[2]);
        glVertex3f(c * worldScale, terrainMatrix[r][c].height * worldScale, r * worldScale);
        return true;
    }
    return false;
}

void renderBlock(int x1, int x2, int z1, int z2, int block)
{
    int xMid = (x1 + x2) / 2;
    int zMid = (z1 + z2) / 2;
    bool twoByTwo = (x2 - x1) == 2;

    if (!twoByTwo)
    {
        if (terrainMatrix[xMid][zMid].enabled)
        {
            renderBlock(x1, xMid, z1, zMid, 0);
            renderBlock(xMid, x2, z1, zMid, 1);
            renderBlock(xMid, x2, zMid, z2, 2);
            renderBlock(x1, xMid, zMid, z2, 3);
        }
        else
        {
            glBegin(GL_TRIANGLE_STRIP);
            if (block == 0 || block == 2)
            {
                quadPointAt(x2, z1, true);
                quadPointAt(x2, z2, true);
                quadPointAt(x1, z1, true);
                quadPointAt(x1, z2, true);
            }
            else if (block == 1 || block == 3)
            {
                quadPointAt(x2, z2, true);
                quadPointAt(x2, z1, true);
                quadPointAt(x1, z2, true);
                quadPointAt(x1, z1, true);
            }
            glEnd();
        }
    }
    else
    {
        if (!terrainMatrix[xMid][zMid].enabled)
        {
            glBegin(GL_TRIANGLE_STRIP);
            if (block == 0 || block == 2)
            {
                quadPointAt(x2, z1, true);
                quadPointAt(x2, z2, true);
                quadPointAt(x1, z1, true);
                quadPointAt(x1, z2, true);
            }
            else if (block == 1 || block == 3)
            {
                quadPointAt(x2, z2, true);
                quadPointAt(x2, z1, true);
                quadPointAt(x1, z2, true);
                quadPointAt(x1, z1, true);
            }
            glEnd();
        }
        else
        {
            glBegin(GL_TRIANGLE_FAN);
            glVector3 normal = terrainMatrix[xMid][zMid].normal;
            glColor3f(terrainMatrix[xMid][zMid].color[R]/255.0, terrainMatrix[xMid][zMid].color[G]/255.0, terrainMatrix[xMid][zMid].color[B]/255.0);
            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(zMid * worldScale, terrainMatrix[xMid][zMid].height * worldScale, xMid * worldScale); //origin
            bool upperLeft = false;
            bool up = false;
            bool upperRight = false;
            bool right = false;
            bool bottomRight = false;
            bool bottom = false;
            bool bottomLeft = false;
            bool left = false;
            if (quadPointAt(x1, z1, false))
                upperLeft = true;
            if (quadPointAt(x1, zMid, false))
                up = true;
            if (quadPointAt(x1, z2, false))
                upperRight = true;
            if (quadPointAt(xMid, z2, false))
                right = true;
            if (quadPointAt(x2, z2, false))
                bottomRight = true;
            if (quadPointAt(x2, zMid, false))
                bottom = true;
            if (quadPointAt(x2, z1, false))
                bottomLeft = true;
            if (quadPointAt(xMid, z1, false))
                left = true;
            if (upperLeft)
                quadPointAt(x1, z1, false);
            else //there's probably a better way of doing this.
            {
                if (up)
                    quadPointAt(x1, zMid, false);
                else
                {
                    if (upperRight)
                        quadPointAt(x1, z2, false);
                    else
                    {
                        if (right)
                            quadPointAt(xMid, z2, false);
                        else
                        {
                            if (bottomRight)
                                quadPointAt(x2, z2, false);
                            else
                            {
                                if (bottom)
                                    quadPointAt(x2, zMid, false);
                                else
                                {
                                    if (bottomLeft)
                                        quadPointAt(x2, z1, false);
                                    else
                                    {
                                        if (left)
                                        {
                                            quadPointAt(xMid, z1, false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        glEnd();
    }
}

void removeCracks(int x1, int x2, int z1, int z2) //not completely working
{
	int xMid = (x1 + x2) / 2;
    int zMid = (z1 + z2) / 2;
	int dim = x2 - x1;
	
	if (!terrainMatrix[xMid][zMid].enabled) return;
	
	//test top point
	if (terrainMatrix[xMid][z1].enabled)
	{
		//is there a block above this one?
		if (zMid - dim >= 0)
		{
			//is the block above this one disabled?
			if (!terrainMatrix[xMid][zMid - dim].enabled)
			{
				//terrainMatrix[xMid][z1].enabled = false;
				//terrainMatrix[xMid][zMid - dim].enabled = true;
				divideBlock(x1, x2, z1 - dim, z1);
			}
		}
	}
	// test right point
	if (terrainMatrix[x2][zMid].enabled)
	{
		//is there a block to the right of this one?
		if (xMid + dim < terrainDim)
		{
			//is the block to the right of this one disabled?
			if (!terrainMatrix[xMid + dim][zMid].enabled)
			{
				//terrainMatrix[x2][zMid].enabled = false;
				//terrainMatrix[xMid + dim][zMid].enabled = true;
				divideBlock(x2, x2 + dim, z1, z2);
			}
		}
	}
	// test bottom point
	if (terrainMatrix[xMid][z2].enabled)
	{
		//is there a block below this one?
		if (zMid + dim < terrainDim)
		{
			//is the block to the right of this one disabled?
			if (!terrainMatrix[xMid][zMid + dim].enabled)
			{
				//terrainMatrix[xMid][z2].enabled = false;
				//terrainMatrix[xMid][zMid + dim].enabled = true;
				divideBlock(x1, x2, z2, z2 + dim);
			}
		}
	}
	// test left point
	if (terrainMatrix[x1][zMid].enabled)
	{
		//is there a block to the left of this one?
		if (xMid - dim >= 0)
		{
			//is the block to the left of this one disabled?
			if (!terrainMatrix[xMid - dim][zMid].enabled)
			{
				//terrainMatrix[x1][zMid].enabled = false;
				//terrainMatrix[xMid - dim][zMid].enabled = true;
				divideBlock(x1 - dim, x1, z1, z2);
			}
		}
	}
	
	if (dim > 2) //block has sub-blocks - check those for cracks
    {
		removeCracks(x1, xMid, z1, zMid);
		removeCracks(xMid, x2, z1, zMid);
		removeCracks(xMid, x2, zMid, z2);
		removeCracks(x1, xMid, zMid, z2);
	}
}

void reloadListQuad() //Use triangle fans and quadtree simplification
{
    glDeleteLists(list, 1);
    list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    renderBlock(0, terrainDim-1, 0, terrainDim-1, 4);
    glEndList();
}

void reloadListQuadEnabledPoints()
{
    glDeleteLists(list, 1);
    list = glGenLists(1);

    glNewList(list, GL_COMPILE);
    glBegin(GL_POINTS);
    for(int z = 0; z < terrainDim; z+=1)
    {
        for(int x = 0; x < terrainDim; x+=1)
        {
            if (terrainMatrix[z][x].enabled)
            {
                glColor3f(1, 1, 1);
                glVertex3f(x * worldScale, terrainMatrix[z][x].height * worldScale, z * worldScale);
            }
        }
    }
    glEnd();
    glEndList();
}

void reloadList() //normal coloring
{
    glDeleteLists(list, 1);
    list = glGenLists(1);

    glNewList(list, GL_COMPILE);
    for(int z = 0; z < terrainMatrix.size() - 1; z+=1)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for(int x = 0; x < terrainMatrix[0].size(); x+=1)
        {
            glVector3 normal = terrainMatrix[z][x].normal;
            glColor3f(terrainMatrix[z][x].color[R]/255.0, terrainMatrix[z][x].color[G]/255.0, terrainMatrix[z][x].color[B]/255.0);
            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z][x].height * worldScale, z * worldScale);

            normal = terrainMatrix[z+1][x].normal;
            glColor3f(terrainMatrix[z+1][x].color[R]/255.0, terrainMatrix[z+1][x].color[G]/255.0, terrainMatrix[z+1][x].color[B]/255.0);
            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z+1][x].height * worldScale, (z + 1) * worldScale);
        }

        glEnd();
    }
    glEndList();
}

void colorRegion()
{
    glDeleteLists(list, 1);
    list = glGenLists(1);

    glNewList(list, GL_COMPILE);

    double red, grn, blu;
    for(int z = 0; z < terrainMatrix.size() - 1; z+=1)
    {
        glBegin(GL_TRIANGLE_STRIP);
        for(int x = 0; x < terrainMatrix[0].size(); x+=1)
        {
            glVector3 normal = terrainMatrix[z][x].normal;

            switch(theTerrain.getType(terrainMatrix[z][x].region))
            {
            case 0:
                red = 0;
                grn = 127.5;
                blu = 0;
                break; // Hills      Dark Green
            case 1:
                red = 127.5;
                grn = 127.5;
                blu = 127.5;
                break; // Mountains  Gray
            case 2:
                red = 0;
                grn = 255;
                blu = 0;
                break; // Plains     Light Green
            case 3:
                red = 171;
                grn = 125;
                blu = 36;
                break; // Plateaus   Brown
            case 4:
                red = 0;
                grn = 0;
                blu = 127.5;
                break; // Lakes		 Dark Blue
            case 5:
                red = 0;
                grn = 0;
                blu = 255;
                break; // Ponds		 Light Blue
            case 6:
                red = 255;
                grn = 255;
                blu = 0;
                break; // Unknown	 Yellow
            }

            glColor3f(red/255.0, grn/255.0, blu/255.0);

            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z][x].height * worldScale, z * worldScale);
            normal = terrainMatrix[z+1][x].normal;

            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z+1][x].height * worldScale, (z + 1) * worldScale);
        }

        glEnd();
    }
    glEndList();
}

void colorElevation()
{
    glDeleteLists(list, 1);
    list = glGenLists(1);

    int minHt = theTerrain.getMinHt(terrainMatrix);
    int maxHt = theTerrain.getMaxHt(terrainMatrix);

    glNewList(list, GL_COMPILE);
    for(int z = 0; z < terrainMatrix.size() - 1; z+=1)
    {
        //Makes OpenGL draw a triangle at every three consecutive vertices
        glBegin(GL_TRIANGLE_STRIP);
        for(int x = 0; x < terrainMatrix[0].size(); x+=1)
        {
            glVector3 normal = terrainMatrix[z][x].normal;

            int thisHt = terrainMatrix[z][x].height;
            double percent;
			
            if(thisHt < 0)
            {
                percent = (double)thisHt/(double)minHt;
                glColor3f(0.5*(1.0-percent), 0.5*(1.0-percent), 1.0);
            }
            else
            {
                percent = (double)thisHt/(double)maxHt;
                glColor3f(1.0*percent, 1.0*(1.0-percent), 0);
            }

            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z][x].height * worldScale, z * worldScale);
            normal = terrainMatrix[z+1][x].normal;

            glNormal3f(normal[0], normal[1], normal[2]);
            glVertex3f(x * worldScale, terrainMatrix[z+1][x].height * worldScale, (z + 1) * worldScale);
        }

        glEnd();
    }
    glEndList();
}

void render() //display list is called here
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(SDL_GetTicks() - lastSecond >= 1000 )
    {
        lastSecond = SDL_GetTicks();
        cout<<"FPS = "<<frameCount<<endl;
        frameCount = 0;
    }
    frameCount++;

    glRotatef(cam.camXRot, 1.0f, 0.0f, 0.0f);
    glRotatef(cam.camYRot, 0.0f, 1.0f, 0.0f);
    glTranslatef(-cam.camXPos,-cam.camYPos,-cam.camZPos);
    glRotatef(135, 0, 1, 0);
    glTranslatef(0, -50, 0);
    glCallList(list);
    glFlush();
    SDL_GL_SwapBuffers();
}

int input()
{
    SDL_Event event;
    GLfloat vertMouseSensitivity  = 10.0f;
    GLfloat horizMouseSensitivity = 10.0f;
    bool static checkForCentered = 0;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)) return 0;
        else if ( event.type == SDL_KEYDOWN )
        {
            switch(event.key.keysym.sym)
            {
            case SDLK_w:
                cam.holdingForward = true;
                break;
            case SDLK_s:
                cam.holdingBackward = true;
                break;
            case SDLK_a:
                cam.holdingLeftStrafe = true;
                break;
            case SDLK_d:
                cam.holdingRightStrafe = true;
                break;
            case SDLK_x:
                if (wireframe == false)
                {
                    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                    glDisable (GL_LIGHTING);
                    glDisable (GL_TEXTURE_2D);
					wireframe = true;
                }
                else
                {
                    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                    glEnable (GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);
                    wireframe = false;
                }
                break;
            case SDLK_r:
                cout << endl << "_________________________________________________________________________________________________________" << endl << endl << endl << endl << endl;
                theTerrain.resetStep();
                theTerrain.generateTerrain(terrainDim, terrainDim, minRegions, maxRegions, terrainType, false, terrainMatrix);
                computeNormals();
                reloadList();
                break;
            case SDLK_n:
                theTerrain.generateTerrain(terrainDim, terrainDim, minRegions, maxRegions, terrainType, true, terrainMatrix);
                computeNormals();
                reloadList();
                break;
            case SDLK_LSHIFT:
                cam.movementSpeedFactor = cam.movementSpeedFactor * 3;
                break;
            case SDLK_SPACE:
                cam.holdingUp = true;
                break;
            case SDLK_1:
                glClearColor(0.390625, 0.58203125, 0.92578125, 1);
                reloadList();
                break;
            case SDLK_2:
                glClearColor(0.390625, 0.58203125, 0.92578125, 1);
                colorRegion();
                break;
            case SDLK_3:
                glClearColor(0.390625, 0.58203125, 0.92578125, 1);
                colorElevation();
                break;
            case SDLK_4:
                if (isPowerOfTwo(terrainDim-1))
                {
					glClearColor(0, 0, 0, 1);
                    divideBlock(0, terrainDim-1, 0, terrainDim-1);
                    reloadListQuadEnabledPoints();
                }
                break;
            case SDLK_5:               
                if (isPowerOfTwo(terrainDim-1))
                {
					glClearColor(0.390625, 0.58203125, 0.92578125, 1);
                    divideBlock(0, terrainDim-1, 0, terrainDim-1);
					removeCracks(0, terrainDim-1, 0, terrainDim-1);
                    reloadListQuad();
					cout<<"Simplified terrain.\n";
                }
                break;
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            switch(event.key.keysym.sym)
            {
            case SDLK_w:
                cam.holdingForward = false;
                break;
            case SDLK_s:
                cam.holdingBackward = false;
                break;
            case SDLK_a:
                cam.holdingLeftStrafe = false;
                break;
            case SDLK_d:
                cam.holdingRightStrafe = false;
                break;
            case SDLK_LSHIFT:
                cam.movementSpeedFactor = cam.movementSpeedFactor / 3;
                break;
            case SDLK_SPACE:
                cam.holdingUp = false;
                break;
            }
        }
        if (event.type == SDL_MOUSEMOTION)
        {
            if(checkForCentered)
            {
                int horizMovement = event.motion.xrel;
                int vertMovement = event.motion.yrel;

                cam.camXRot += vertMovement / vertMouseSensitivity;
                cam.camYRot += horizMovement / horizMouseSensitivity;

                // Control looking up and down with the mouse forward/back movement

                // Limit looking up to vertically up
                if (cam.camXRot < -90.0f) cam.camXRot = -90.0f;

                // Limit looking down to vertically down
                if (cam.camXRot > 90.0f) cam.camXRot = 90.0f;

                // Looking left and right. Keep the angles in the range -180.0f (anticlockwise turn looking behind) to 180.0f (clockwise turn looking behind)
                if (cam.camYRot < -180.0f) cam.camYRot += 360.0f;

                if (cam.camYRot > 180.0f) cam.camYRot -= 360.0f;

                SDL_WarpMouse(windowX/2, windowY/2); //used to keep the mouse in the window if in windowed mode
                checkForCentered = false;
            }
            else
            {
                checkForCentered = true;
            }
        }
    }
    return 1;
}

void initGL()
{
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glViewport(0, 0, windowX, windowY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)windowX / (float)windowY, 0.01, 2000.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.390625, 0.58203125, 0.92578125, 1); //cornflower blue!
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable (GL_LIGHTING);
    glEnable (GL_TEXTURE_2D);
    glEnable(GL_LIGHT0);
    glShadeModel (GL_SMOOTH);
    GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

    GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);

    glColorMaterial ( GL_AMBIENT_AND_DIFFUSE, GL_EMISSION ) ;
}

void getSettings()
{
    string line;
    ifstream settings ("settings.ini");

    if (settings.is_open())
    {
        while ( settings.good() )
        {
            getline (settings,line);

            if (line.find("windowX",0) != string::npos)
            {
                string resX = line.substr(line.find('=')+1);
                windowX = atoi(resX.c_str());
            }
            if (line.find("windowY",0) != string::npos)
            {
                string resY = line.substr(line.find('=')+1);
                windowY = atoi(resY.c_str());
            }
            if (line.find("fullscreen",0) != string::npos)
            {
                string isFullScreen = line.substr(line.find('=')+1);
                fullscreen = atoi(isFullScreen.c_str());
            }
            if (line.find("terrainDim",0) != string::npos)
            {
                string dimension = line.substr(line.find('=')+1);
                terrainDim = atoi(dimension.c_str());
            }
            if (line.find("minRegions",0) != string::npos)
            {
                string min = line.substr(line.find('=')+1);
                minRegions = atoi(min.c_str());
            }
            if (line.find("maxRegions",0) != string::npos)
            {
                string max = line.substr(line.find('=')+1);
                maxRegions = atoi(max.c_str());
            }
            if (line.find("terrainType",0) != string::npos)
            {
                string type = line.substr(line.find('=')+1);
                terrainType = type;
            }
        }
        settings.close();
    }
    else
        cout << "settings.ini could not be opened." << endl;
}

int main(int argc, char *argv[])
{
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );
    getSettings();

    theTerrain.resetStep();
    theTerrain.generateTerrain(terrainDim, terrainDim, minRegions, maxRegions, terrainType, false, terrainMatrix);

    SDL_Surface *surf;
    srand (time(NULL));
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 0;

    if (fullscreen)
    {
        if (!(surf = SDL_SetVideoMode(windowX, windowY, bpp, SDL_OPENGL | SDL_FULLSCREEN ))) return 0;
    }
    else
    {
        if (!(surf = SDL_SetVideoMode(windowX, windowY, bpp, SDL_OPENGL ))) return 0;
    }

    SDL_ShowCursor(SDL_DISABLE);
    initGL();
    computeNormals();
    reloadList();

    for (;;)
    {
        if (!input()) break;
        cam.calculateCameraMovement();
        cam.moveCamera();
        render();
    }

    SDL_FreeSurface(surf);
    SDL_Quit();
    return 0;
}