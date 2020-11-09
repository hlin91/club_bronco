#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION
#include <cmath>
#include <vector>
#include "olcPixelGameEngine/olcPixelGameEngine.h"

struct Character
{
	olc::vf2d pos; // The character's designated position
	olc::vf2d currPos; // The character's current position
	float theta; // Angle in radians from horizontal axis of line from current position to designated position

	Character()
	{
		pos = {0, 0};
		currPos = {0, 0};
		theta = 0;
	}

	Character(olc::vf2d &p, olc::vf2d &cp, float t)
	{
		pos = p;
		currPos = cp;
		theta = t;
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
	std::vector<Character> others; // List of other players
	float walkSpeed; // The walk speed of the player in pixels per second
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		player.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
		player.currPos = player.pos;
		float playerTheta = 0;
		walkSpeed = 100;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame
		if (GetMouse(0).bPressed) // Move player on mouse click
			player.move(GetMouseX(), GetMouseY());
		// Gradually move the player towards the designated position
		if (sqrt(pow(player.currPos.x - player.pos.x, 2) + pow(player.currPos.y - player.pos.y, 2)) > walkSpeed * fElapsedTime)
		{
			// Because of the way the coordinate system works in the window, we have to subtract instead of add
			player.currPos.x -= walkSpeed * cos(player.theta) * fElapsedTime;
			player.currPos.y -= walkSpeed * sin(player.theta) * fElapsedTime;
		}
		for (auto c : others) // Move the other players
		{
			if (sqrt(pow(c.currPos.x - c.pos.x, 2) + pow(c.currPos.y - c.pos.y, 2)) > walkSpeed * fElapsedTime)
			{
				c.currPos.x -= walkSpeed * cos(c.theta) * fElapsedTime;
				c.currPos.y -= walkSpeed * sin(c.theta) * fElapsedTime;
			}
		}
		Clear(olc::BLACK); // Erase the previous frame
		FillCircle(player.currPos, 5, olc::WHITE); // Draw the player
		for (auto c : others) // Draw the other players
			FillCircle(c.currPos, 5, olc::RED);
		return true;
	}
};


int main()
{
	ClubBronco cb;
	if (cb.Construct(640, 480, 2, 2))
		cb.Start();

	return 0;
}