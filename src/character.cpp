#include "olcPixelGameEngine/olcPixelGameEngine.h"
#include <string>


struct Character
{
    static const int MAX_NAME_LENGTH = 20;
    olc::vf2d pos; // The character's designated position
    olc::vf2d currPos; // The character's current position
    float theta; // Angle in radians from horizontal axis of line from current position to designated position
    std::string name; // Player name
    bool dancing; // Dancing flag
    double danceAngle; // Current angle of rotation in the dance
    bool inputting; // Is the player currently inputting
    bool moving; // Is the avatar still moving to a designated position
    float moveAngle; // Current angle of rotation in move animation
    unsigned int id; // The unique ID associated with the character

    Character()
    {
        pos = {0, 0};
        currPos = {0, 0};
        theta = 0;
        name = "Player";
        dancing = false;
        danceAngle = 0;
        inputting = false;
        moving = false;
        moveAngle = 0;
        id = 0;
    }

    Character(std::string &n, unsigned int i=0, float xp=0, float yp=0, float xcp=0, float ycp=0)
    {
        pos = {xp,yp};
        currPos = {xcp,ycp};
        theta = 0;
        name = n.substr(0, MAX_NAME_LENGTH);
        dancing = false;
        danceAngle = 0;
        inputting = false;
        moving = false;
        moveAngle = 0;
        id = i;
    }

    void move(float x, float y) // Move the character to specified position and update theta
    {
        pos = {x, y};
        theta = atan2(currPos.y - pos.y, currPos.x - pos.x);
    }
};