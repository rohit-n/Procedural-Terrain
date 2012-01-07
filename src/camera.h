/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include <SDL/SDL_opengl.h>
#include <math.h>
class Camera
{
public:

    void moveCamera();
    void calculateCameraMovement();
    float toRads(const float &theAngleInDegrees);
    Camera();

    // Camera rotation
    GLfloat camXRot;
    GLfloat camYRot;
    //GLfloat camZRot;

    // Camera position
    GLfloat camXPos;
    GLfloat camYPos;
    GLfloat camZPos;

    // Camera movement speed
    GLfloat camXSpeed;
    GLfloat camYSpeed;
    GLfloat camZSpeed;

    // How fast we move (higher values mean we move and strafe faster)
    GLfloat movementSpeedFactor;

    // Hoding any keys down?
    bool holdingForward;
    bool holdingBackward;
    bool holdingLeftStrafe;
    bool holdingRightStrafe;
    bool holdingUp;
};
