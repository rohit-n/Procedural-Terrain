/*
Joseph Dombrowski - 1073257
Rohit Nirmal - 0848815

Computer Graphics
*/
#include "camera.h"
const float TO_RADS = 3.141592654f / 180.0f; // The value of 1 degree in radians
Camera::Camera()
{
    camXRot = 0.0f;
    camYRot = 0.0f;
    //camZRot = 0.0f;

    // Camera position
    camXPos = 0.0f;
    camYPos = 0.0f;
    camZPos = 0.0f;

    // Camera movement speed
    camXSpeed = 0.0f;
    camYSpeed = 0.0f;
    camZSpeed = 0.0f;

    // How fast we move (higher values mean we move and strafe faster)
    movementSpeedFactor = 0.5f;

    // Hoding any keys down?
    holdingForward     = false;
    holdingBackward    = false;
    holdingLeftStrafe  = false;
    holdingRightStrafe = false;
    holdingUp          = false;
}

void Camera::moveCamera()
{
    camXPos += camXSpeed;
    camYPos += camYSpeed;
    camZPos += camZSpeed;
}

void Camera::calculateCameraMovement()
{
    // Break up our movement into components along the X, Y and Z axis
    float camMovementXComponent = 0.0f;
    float camMovementYComponent = 0.0f;
    float camMovementZComponent = 0.0f;

    if (holdingForward == true)
    {
        // Control X-Axis movement
        float pitchFactor = cos(toRads(camXRot));
        camMovementXComponent += ( movementSpeedFactor * float(sin(toRads(camYRot))) ) * pitchFactor;

        // Control Y-Axis movement
        camMovementYComponent += movementSpeedFactor * float(sin(toRads(camXRot))) * -1.0f;

        // Control Z-Axis movement
        float yawFactor = float(cos(toRads(camXRot)));
        camMovementZComponent += ( movementSpeedFactor * float(cos(toRads(camYRot))) * -1.0f ) * yawFactor;
    }

    if (holdingBackward == true)
    {
        // Control X-Axis movement
        float pitchFactor = cos(toRads(camXRot));
        camMovementXComponent += ( movementSpeedFactor * float(sin(toRads(camYRot))) * -1.0f) * pitchFactor;

        // Control Y-Axis movement
        camMovementYComponent += movementSpeedFactor * float(sin(toRads(camXRot)));

        // Control Z-Axis movement
        float yawFactor = float(cos(toRads(camXRot)));
        camMovementZComponent += ( movementSpeedFactor * float(cos(toRads(camYRot))) ) * yawFactor;
    }

    if (holdingLeftStrafe == true)
    {
        // Calculate our Y-Axis rotation in radians once here because we use it twice
        float yRotRad = toRads(camYRot);

        camMovementXComponent += -movementSpeedFactor * float(cos(yRotRad));
        camMovementZComponent += -movementSpeedFactor * float(sin(yRotRad));
    }

    if (holdingRightStrafe == true)
    {
        // Calculate our Y-Axis rotation in radians once here because we use it twice
        float yRotRad = toRads(camYRot);

        camMovementXComponent += movementSpeedFactor * float(cos(yRotRad));
        camMovementZComponent += movementSpeedFactor * float(sin(yRotRad));
    }
    if (holdingUp == true)
    {
        camMovementYComponent++;
    }

    // After combining our movements for any & all keys pressed, assign them to our camera speed along the given axis
    camXSpeed = camMovementXComponent;
    camYSpeed = camMovementYComponent;
    camZSpeed = camMovementZComponent;

    // Cap the speeds to our movementSpeedFactor (otherwise going forward and strafing at an angle is twice as fast as just going forward!)
    // X Speed cap
    if (camXSpeed > movementSpeedFactor)
    {
        //cout << "high capping X speed to: " << movementSpeedFactor << endl;
        camXSpeed = movementSpeedFactor;
    }
    if (camXSpeed < -movementSpeedFactor)
    {
        //cout << "low capping X speed to: " << movementSpeedFactor << endl;
        camXSpeed = -movementSpeedFactor;
    }

    // Y Speed cap
    if (camYSpeed > movementSpeedFactor)
    {
        //cout << "low capping Y speed to: " << movementSpeedFactor << endl;
        camYSpeed = movementSpeedFactor;
    }
    if (camYSpeed < -movementSpeedFactor)
    {
        //cout << "high capping Y speed to: " << movementSpeedFactor << endl;
        camYSpeed = -movementSpeedFactor;
    }

    // Z Speed cap
    if (camZSpeed > movementSpeedFactor)
    {
        //cout << "high capping Z speed to: " << movementSpeedFactor << endl;
        camZSpeed = movementSpeedFactor;
    }
    if (camZSpeed < -movementSpeedFactor)
    {
        //cout << "low capping Z speed to: " << movementSpeedFactor << endl;
        camZSpeed = -movementSpeedFactor;
    }
}
float Camera::toRads(const float &theAngleInDegrees)
{
    return theAngleInDegrees * TO_RADS;
}

