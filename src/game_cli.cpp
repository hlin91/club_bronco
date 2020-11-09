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
	std::unique_ptr<olc::Sprite> pAvatar; // The avatar of the player character
	std::unique_ptr<olc::Decal> dpAvatar; // Decal version of player avatar
	std::unique_ptr<olc::Sprite> bg; // The background image
	std::unique_ptr<olc::Decal> dbg; // Decal version of background image
	olc::vf2d origin; // The 0,0 position
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		origin = {0, 0};
		player.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
		player.currPos = player.pos;
		float playerTheta = 0;
		walkSpeed = 100;
		pAvatar = std::make_unique<olc::Sprite>("./imgs/player.png");
		dpAvatar = std::make_unique<olc::Decal>(pAvatar.get());
		bg = std::make_unique<olc::Sprite>("./imgs/bg.png");
		dbg = std::make_unique<olc::Decal>(bg.get());
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame
		if (GetMouse(0).bPressed) // Move player on mouse click
			player.move(GetMouseX() - (pAvatar->width / 2.0), GetMouseY() - (pAvatar->height / 2.0));
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
		DrawDecal(origin, dbg.get()); // Draw the background
		DrawDecal(player.currPos, dpAvatar.get()); // Draw the player
		for (auto c : others) // Draw the other players
			DrawDecal(c.currPos, dpAvatar.get());
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