#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION

#include <cmath>
#include <deque>
#include <string>
#include <unordered_map>
#include "olcPixelGameEngine/olcPixelGameEngine.h"
#include "Polygon.h"

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
    unsigned long long id; // The unique ID associated with the character

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

    Character(std::string &n, unsigned long long i=0, const olc::vf2d &p={0, 0}, const olc::vf2d &cp={0, 0})
    {
        pos = p;
        currPos = cp;
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


class ClubBronco : public olc::PixelGameEngine
{
public:
    ClubBronco()
    {
        sAppName = "Club Bronco";
    }

private:
    Character player; // The player character
    std::string input; // The input string. Should limit to around 40 characters
    float walkSpeed; // The walk speed of the player in pixels per second
    float danceSpeed; // The speed at which the player dances in radians per second. The walk animation speed also depends on this
    std::unique_ptr<olc::Sprite> pAvatar; // The avatar of the player character
    std::unique_ptr<olc::Sprite> pAvatarFlip; // The flipped avatar of the player character
    std::unique_ptr<olc::Decal> dpAvatar; // Decal version of player avatar
    std::unique_ptr<olc::Decal> dpAvatarFlip; // Flipped decal of player avatar
    std::unique_ptr<olc::Sprite> bg; // The background image
    std::unique_ptr<olc::Decal> dbg; // Decal version of background image
    std::unique_ptr<olc::Sprite> arrow; // Arrow above the player
    std::unique_ptr<olc::Decal> darrow;
    std::unique_ptr<olc::Sprite> mbox; // Message box
    std::unique_ptr<olc::Decal> dmbox;
    std::unique_ptr<olc::Sprite> inputBox; // Input box
    std::unique_ptr<olc::Decal> dinputBox;
    std::unique_ptr<olc::Sprite> nameBox; // Name box
    std::unique_ptr<olc::Decal> dnameBox;
    std::unique_ptr<olc::Sprite> chatBubble[3]; // The chat bubble animation
    std::unique_ptr<olc::Decal> dchatBubble[3];
    olc::vf2d playerCenter; // Center of player sprite
    olc::vf2d arrowPos; // Position of arrow above player character
    olc::vf2d nameBoxPos; // Position of name box
    olc::vf2d mBoxPos; // Position of message box
    olc::vf2d inputBoxPos; // Position of input box
    float arrowSpace; // Space between bottom of arrow and top of player avatar
    olc::vf2d inputPos; // Position of input text
    olc::vf2d messagePos; // Position of message text
    olc::vf2d namePos; // Position of player names in name box
    static const int MAX_MESSAGES = 21; // Max number of displayed messages
    static const int MAX_INPUT_LENGTH = 40; // Max length of input string
    static const int MBOX_CHAR_WIDTH = 45; // Approximate width of message box in characters
    float processDelay; // For limiting processing of held key
    double globalTimer; // Total amount of time passed in seconds
    harv::Polygon bounds; // Polygonal boundary of the walkable area
    harv::Polygon rectBounds; // Rectilinear approximation of the walkable area to reduce computational complexity
    
    char processAlnum() // Process an textual keystroke as character input
    {
        char c = 0;
        if (GetKey(olc::Key::SPACE).bPressed) // Handle space
            return ' ';
        if (GetKey(olc::Key::BACK).bPressed) // Handle backspace
            return 8;
        for (unsigned long long i = olc::Key::A; i <= olc::Key::K9; ++i)
        {
            if (GetKey(olc::Key(i)).bPressed)
            {
                if (i >= olc::Key::K0) // Numeric key pressed
                    c = '0' + i - olc::Key::K0;
                else // Alphabetical key pressed
                {
                    c = 'a' + i - olc::Key::A;
                    if (GetKey(olc::Key::SHIFT).bHeld)
                        c -= 32; // Convert to upper case
                }
            }
        }
        return c;
    }

    char processHeld(float fElapsedTime) // Process textual keystroke that has been held
    {
        char c = 0;
        processDelay += fElapsedTime;
        if (processDelay > 0.1)
        {
            processDelay -= 0.1;
            if (GetKey(olc::Key::SPACE).bHeld) // Handle space
            return ' ';
            if (GetKey(olc::Key::BACK).bHeld) // Handle backspace
                return 8;
            for (unsigned long long i = olc::Key::A; i <= olc::Key::K9; ++i)
            {
                if (GetKey(olc::Key(i)).bHeld)
                {
                    if (i >= olc::Key::K0) // Numeric key pressed
                        c = '0' + i - olc::Key::K0;
                    else // Alphabetical key pressed
                    {
                        c = 'a' + i - olc::Key::A;
                        if (GetKey(olc::Key::SHIFT).bHeld)
                            c -= 32; // Convert to upper case
                    }
                }
            }
        }
        return c;
    }

    void addMessage(std::string &m) // Add message to message box
    {
        if (messages.size() == MAX_MESSAGES)
            messages.pop_front();
        messages.push_back(m);
    }

    void drawCharacter(Character &c) // Draw the character on screen
    {
        if (-(walkSpeed * cos(c.theta)) < 0) // Determine if we need to flip the player
        {
            if (c.dancing) // Draw dancing player
                DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatarFlip.get(), sin(c.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(c.danceAngle * 2) / 3 + 1, sin(c.danceAngle * 2) / 3 + 1));
            else if (c.moving)
                DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatarFlip.get(), sin(c.moveAngle) * (1.57 / 4), playerCenter);
            else
                DrawDecal(olc::vf2d(c.currPos.x, c.currPos.y), dpAvatarFlip.get());
        }
        else
        {
            if (c.dancing)
                DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatar.get(), sin(c.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(c.danceAngle * 2) / 3 + 1, sin(c.danceAngle * 2) / 3 + 1));
            else if (c.moving)
                DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatar.get(), sin(c.moveAngle) * (1.57 / 4), playerCenter);
            else
                DrawDecal(olc::vf2d(c.currPos.x, c.currPos.y), dpAvatar.get());
        }
        if (c.inputting) // Draw the chat bubble
        {
            olc::vf2d drawPos = {float(c.currPos.x + playerCenter.x - chatBubble[0]->width / 2.0), float(c.currPos.y - chatBubble[0]->height - arrowSpace)};
            DrawDecal(drawPos, dchatBubble[int(globalTimer / 0.5) % 3].get()); // Determine the frame of the animation based on elapsed time
        }
    }

    void moveCharacter(Character &c, float fElapsedTime) // Update the position, move angle, and dance angle of the character
    {
        if (sqrt(pow(c.currPos.x - c.pos.x, 2) + pow(c.currPos.y - c.pos.y, 2)) > walkSpeed * fElapsedTime)
        {
            c.moveAngle += danceSpeed * 3 * fElapsedTime;
            c.moving = true;
            c.currPos.x -= walkSpeed * cos(c.theta) * fElapsedTime;
            c.currPos.y -= walkSpeed * sin(c.theta) * fElapsedTime;
        }
        else
        {
            c.moving = false;
            c.moveAngle = 0;
        }
        if (c.dancing)
            c.danceAngle += danceSpeed * fElapsedTime;
        else
            c.danceAngle = 0;
    }

    // Check if the coord is inside the boundary and return the intersection with vector from currPos to coord if not
    bool insideBounds(float x, float y, harv::Coord &intersect)
    {
        bool found = false;
        std::vector<harv::Coord> intrs; // List of found intersections
        // Draw vector from current position to designated position
        harv::Edge e;
        e.v1 = harv::Coord(player.currPos.x, player.currPos.y);
        e.v2 = harv::Coord(x, y);
        // Special case for when the current position coincides with a boundary vertex
        for (auto &v : bounds.v)
        {
            if (abs(e.v1.x - v.x) < 10 && abs(e.v1.y - v.y) < 10)
            {
                // Don't move
                intersect = e.v1;
                return false;
            }
        }
        // If the vector to the designated position is within the boundary, we will find no intersection
        // Else, we will find at least one intersection
        for (unsigned int i = 0; i < bounds.size(); ++i)
        {
            harv::Edge edge = bounds.edge(i);
            found = harv::intersection(e, edge, intersect);
            if (found)
            {
                // Offset the intersection by a bit to avoid clipping
                intersect.x -= 10 * cos(edge.theta() + PI / 2);
                intersect.y -= 10 * sin(edge.theta() + PI / 2);
                intrs.push_back(intersect);
            }
        }
        if (intrs.size() == 0)
            return true;
        else if (intrs.size() == 1)
            intersect = intrs[0];
        else // Return the intersection closest to current position
        {
            double minDist = harv::distance(e.v1, intrs[0]);
            intersect = intrs[0];
            for (unsigned int i = 0; i < intrs.size(); ++i)
            {
                if (harv::distance(e.v1, intrs[i]) < minDist)
                {
                    minDist = harv::distance(e.v1, intrs[i]);
                    intersect = intrs[i];
                }
            }
        }
        return false;
    }

public:
    // These variables will need to be updated by slave threads
    std::unordered_map<unsigned long long, Character> others; // List of other players
    std::deque<std::string> messages; // List of messages
    bool gameOver; // Used to tell slave threads that the game has ended

    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        // TODO: Create slave thread to listen for updates from HTTP server
        player.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
        player.currPos = player.pos;
        walkSpeed = 100;
        danceSpeed = 1;
        processDelay = 0;
        globalTimer = 0;
        pAvatar = std::make_unique<olc::Sprite>("./imgs/player.png");
        pAvatarFlip = std::make_unique<olc::Sprite>("./imgs/player_flip.png");
        playerCenter = {float(pAvatar->width / 2.0), float(pAvatar->height / 2.0)};
        dpAvatar = std::make_unique<olc::Decal>(pAvatar.get());
        dpAvatarFlip = std::make_unique<olc::Decal>(pAvatarFlip.get());
        bg = std::make_unique<olc::Sprite>("./imgs/bg.png");
        dbg = std::make_unique<olc::Decal>(bg.get());
        arrow = std::make_unique<olc::Sprite>("./imgs/arrow.png");
        darrow = std::make_unique<olc::Decal>(arrow.get());
        mbox = std::make_unique<olc::Sprite>("./imgs/message_box.png");
        dmbox = std::make_unique<olc::Decal>(mbox.get());
        inputBox = std::make_unique<olc::Sprite>("./imgs/input_box.png");
        dinputBox = std::make_unique<olc::Decal>(inputBox.get());
        nameBox = std::make_unique<olc::Sprite>("./imgs/name_box.png");
        dnameBox = std::make_unique<olc::Decal>(nameBox.get());
        chatBubble[0] = std::make_unique<olc::Sprite>("./imgs/chat1.png");
        chatBubble[1] = std::make_unique<olc::Sprite>("./imgs/chat2.png");
        chatBubble[2] = std::make_unique<olc::Sprite>("./imgs/chat3.png");
        dchatBubble[0] = std::make_unique<olc::Decal>(chatBubble[0].get());
        dchatBubble[1] = std::make_unique<olc::Decal>(chatBubble[1].get());
        dchatBubble[2] = std::make_unique<olc::Decal>(chatBubble[2].get());
        arrowPos = {0, 0};
        arrowSpace = 5;
        mBoxPos = {20.0, float(ScreenHeight() - 290.0)};
        inputBoxPos = {20.0, float(ScreenHeight() - 50.0)};
        inputPos = {inputBoxPos.x + 12, float(inputBoxPos.y + inputBox->height / 2.0 - 3)};
        messagePos = {mBoxPos.x + 12, mBoxPos.y + 12};
        nameBoxPos = {20.0, mBoxPos.y - 24 - nameBox->height};
        namePos = {nameBoxPos.x + 12, nameBoxPos.y + 12};
        DrawSprite({0, 0}, bg.get());
        // Define rectilinear approximation of boundary
        rectBounds.addVert(harv::Coord(175, 150));
        rectBounds.addVert(harv::Coord(180, 460));
        rectBounds.addVert(harv::Coord(1090, 460));
        rectBounds.addVert(harv::Coord(1090, 150));
        // Define high resolution boundary
        bounds.addVert(harv::Coord(292, 5));
        bounds.addVert(harv::Coord(169, 136));
        bounds.addVert(harv::Coord(169,212));
        bounds.addVert(harv::Coord(102, 306));
        bounds.addVert(harv::Coord(90, 382));
        bounds.addVert(harv::Coord(75, 498));
        bounds.addVert(harv::Coord(197, 544));
        bounds.addVert(harv::Coord(262, 473));
        bounds.addVert(harv::Coord(384, 573));
        bounds.addVert(harv::Coord(951, 574));
        bounds.addVert(harv::Coord(1189, 529));
        bounds.addVert(harv::Coord(1126, 392));
        bounds.addVert(harv::Coord(1164, 359));
        bounds.addVert(harv::Coord(1077, 273));
        bounds.addVert(harv::Coord(1166, 259));
        bounds.addVert(harv::Coord(1159, 247));
        bounds.addVert(harv::Coord(1071, 201));
        bounds.addVert(harv::Coord(1114, 178));
        bounds.addVert(harv::Coord(973, 96));
        bounds.addVert(harv::Coord(793, 44));
        bounds.addVert(harv::Coord(647, 32));
        bounds.addVert(harv::Coord(512, 43));
        bounds.addVert(harv::Coord(432, 5));
        gameOver = false;
        // TODO: For testing
        input = "";
        for (unsigned int i = 0; i < 5; ++i)
        {
            Character p;
            p.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
            p.pos.x += rand() % 100;
            p.pos.y += rand() % 100;
            p.currPos = p.pos;
            others.insert(std::pair(i, p));
        }
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // Called once per frame
        globalTimer += fElapsedTime;
        if (GetKey(olc::Key::ESCAPE).bPressed) // Quit with escape
        {
            // TODO: Tell server that player is leaving
            return false;
        }
        if (GetKey(olc::Key::ENTER).bPressed) // Enter toggles input state
        {
            if (player.inputting) // Done with current input string
            {
                if (input.size())
                {
                    std::string m = player.name + ": " + input;
                    addMessage(m.substr(0, 45));
                    if (m.size() > MBOX_CHAR_WIDTH) // If the player name plus the message exceeds message box width
                        addMessage("    " + m.substr(45)); // Print the rest on a indented new line
                    // TODO: Send the input string to the server
                }
                input.clear();
            }
            player.inputting = !(player.inputting);
            // TODO: Send server updated input state
        }
        if (player.inputting) // Listen for alphanumeric key presses
        {
            // Handle pressing of new key
            char c = processAlnum();
            if (c == 8) // Backspace
            {
                if (input.size())
                    input.pop_back();
                processDelay = -.5; // Delay the handling of held keys by .5s
            }	
            else if (c)
            {
                if (input.size() < MAX_INPUT_LENGTH)
                input.push_back(c);
                processDelay = -.5;
            }
            else
            {
                // Handle holding down of key
                c = processHeld(fElapsedTime);
                if (c == 8) // Backspace
                {
                    if (input.size())
                        input.pop_back();
                }	
                else if (c)
                    if (input.size() < MAX_INPUT_LENGTH)
                        input.push_back(c);
            }
        }
        if (GetMouse(0).bPressed) // Move player on mouse click
        {
            float x = GetMouseX();
            float y = GetMouseY();
            // Check against the simple boundary first
            if ((x < rectBounds.v[0].x || x > rectBounds.v[2].x) || (y < rectBounds.v[0].y || y > rectBounds.v[2].y)) // If the position is outside the simple boundary
            {
                // Do a check against the high resolution boundary
                harv::Coord intersection;
                if (!insideBounds(x, y, intersection)) // If outside the boundary
                {
                    // Move to the intersection
                    x = intersection.x;
                    y = intersection.y;
                }
            }
            player.move(x, y);
            // TODO: Send server updated position
        }
        if (GetKey(olc::Key::DOWN).bPressed) // Toggle dancing for player
        {
            player.dancing = !(player.dancing);
            // TODO: Send server updated dancing state
        }
        // Gradually move the player towards the designated position
        moveCharacter(player, fElapsedTime);
        for (auto &c : others) // Move and dance the other players
            moveCharacter(c.second, fElapsedTime);
        arrowPos = {float(player.currPos.x + playerCenter.x - arrow->width / 2.0), float(player.currPos.y - arrow->height - arrowSpace)};
        // Handle drawing
        for (auto &c : others) // Draw the other players
            drawCharacter(c.second);
        drawCharacter(player); // Draw the player
        if (!player.inputting)
            DrawDecal(arrowPos, darrow.get()); // Draw the arrow above player
        DrawDecal(mBoxPos, dmbox.get()); // Draw the message box
        DrawDecal(inputBoxPos, dinputBox.get()); // Draw the input box
        DrawDecal(nameBoxPos, dnameBox.get()); // Draw the name box
        DrawStringDecal(namePos, player.name.substr(0, 12), olc::WHITE); // Draw the first 12 characters of player name in the name box
        // Draw the input line
        if (player.inputting)
            DrawStringDecal(inputPos, input + '_', olc::WHITE);
        else
            DrawStringDecal(inputPos, input, olc::WHITE);
        // Draw messages
        for (unsigned int i = 0; i < messages.size(); ++i)
        {
            olc::vf2d drawPos = {messagePos.x, messagePos.y + i * 10};
            DrawStringDecal(drawPos, messages[i], olc::WHITE);
        }
        // Draw the other player names
        for (unsigned int i = 0; i < others.size(); ++i)
        {
            if (i > 28) // Only draw up to 29 other player names
                break;
            olc::vf2d drawPos = {namePos.x, namePos.y + (i + 1) * 12};
            DrawStringDecal(drawPos, others[i].name.substr(0, 12), olc::WHITE); // Only draw the first 12 characters of name
        }
        if (others.size() > 28) // If there are more than 28 other players in the room
        {
            // Indicate that there are undrawn names
            olc::vf2d drawPos = {namePos.x, namePos.y + 30 * 12};
            DrawStringDecal(drawPos, "...", olc::WHITE);
        }
        return true;
    }
};

int main()
{
    ClubBronco cb;
    if (cb.Construct(1280, 720, 1, 1))
        cb.Start();

    return 0;
}
