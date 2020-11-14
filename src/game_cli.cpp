#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION

#include <cmath>
#include <vector>
#include <deque>
#include <string>
#include "olcPixelGameEngine/olcPixelGameEngine.h"

struct Character
{
    static const int MAX_NAME_LENGTH = 20;
    olc::vf2d pos; // The character's designated position
    olc::vf2d currPos; // The character's current position
    float theta; // Angle in radians from horizontal axis of line from current position to designated position
    std::string name; // Player name
    bool dancing; // Dancing flag
    double danceAngle; // Current angle of rotation in the dance
    bool inputing; // Is the player currently inputing

    Character()
    {
        pos = {0, 0};
        currPos = {0, 0};
        theta = 0;
        name = "Player";
        dancing = false;
        danceAngle = 0;
        inputing = false;
    }

    Character(std::string &n, olc::vf2d p={0, 0}, olc::vf2d cp={0, 0})
    {
        pos = p;
        currPos = cp;
        theta = 0;
        name = n.substr(0, MAX_NAME_LENGTH);
        dancing = false;
        danceAngle = 0;
        inputing = false;
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
    float danceSpeed; // The speed at which the player dances in radians per second
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
    olc::vf2d origin; // The 0,0 position
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
    // For testing
    unsigned long long itr = 0;

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
            else
                DrawDecal(c.currPos, dpAvatarFlip.get());
        }
        else
        {
            if (c.dancing)
                DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatar.get(), sin(c.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(c.danceAngle * 2) / 3 + 1, sin(c.danceAngle * 2) / 3 + 1));
            else
                DrawDecal(c.currPos, dpAvatar.get());
        }
        if (c.inputing) // Draw the chat bubble
        {
            olc::vf2d drawPos = {float(c.currPos.x + pAvatar->width / 2.0 - chatBubble[0]->width / 2.0), float(c.currPos.y - chatBubble[0]->height - arrowSpace)};
            DrawDecal(drawPos, dchatBubble[int(globalTimer / 0.5) % 3].get()); // Determine the frame of the animation based on elapsed time
        }
    }

    void moveCharacter(Character &c, float fElapsedTime) // Update the position and dance angle of the character
    {
        if (sqrt(pow(c.currPos.x - c.pos.x, 2) + pow(c.currPos.y - c.pos.y, 2)) > walkSpeed * fElapsedTime)
        {
            c.currPos.x -= walkSpeed * cos(c.theta) * fElapsedTime;
            c.currPos.y -= walkSpeed * sin(c.theta) * fElapsedTime;
        }
        if (c.dancing)
            c.danceAngle += danceSpeed * fElapsedTime;
        else
            c.danceAngle = 0;
    }

public:
    // These variables will need to be updated by slave threads
    std::vector<Character> others; // List of other players
    std::deque<std::string> messages; // List of messages

    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        // TODO: Create slave thread to listen for updates from HTTP server
        origin = {0, 0};
        player.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
        player.currPos = player.pos;
        float playerTheta = 0;
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
        DrawSprite(origin, bg.get());
        // For testing
        input = "";
        for (unsigned int i = 0; i < MAX_MESSAGES; ++i)
            messages.push_back("Test message");
        for (unsigned int i = 1; i <= 30; ++i)
        {
            others.push_back(player);
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
            if (player.inputing) // Done with current input string
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
            player.inputing = !(player.inputing);
            // TODO: Send server updated input state
        }
        if (player.inputing) // Listen for alphanumeric key presses
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
            player.move(GetMouseX() - (pAvatar->width / 2.0), GetMouseY() - (pAvatar->height / 2.0));
            // TODO: Send server updated position
        }
        if (GetKey(olc::Key::DOWN).bPressed) // Toggle dancing for player
        {
            player.dancing = !(player.dancing);
            // TODO: Send server updated dancing state
        }
        // Gradually move the player towards the designated position
        moveCharacter(player, fElapsedTime);
        for (auto c : others) // Move and dance the other players
            moveCharacter(c, fElapsedTime);
        arrowPos = {float(player.currPos.x + pAvatar->width / 2.0 - arrow->width / 2.0), float(player.currPos.y - arrow->height - arrowSpace)};

        for (auto c : others) // Draw the other players
            drawCharacter(c);
        drawCharacter(player); // Draw the player
        if (!player.inputing)
            DrawDecal(arrowPos, darrow.get()); // Draw the arrow above player
        DrawDecal(mBoxPos, dmbox.get()); // Draw the message box
        DrawDecal(inputBoxPos, dinputBox.get()); // Draw the input box
        DrawDecal(nameBoxPos, dnameBox.get()); // Draw the name box
        DrawStringDecal(namePos, player.name.substr(0, 12), olc::WHITE); // Draw the first 12 characters of player name in the name box
         // Draw the input line
         if (player.inputing)
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
