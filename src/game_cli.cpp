#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION
#include <cmath>
#include "olcPixelGameEngine/olcPixelGameEngine.h"

class ClubBronco : public olc::PixelGameEngine
{
public:
	ClubBronco()
	{
		sAppName = "Club Bronco";
	}

private:
	olc::vf2d playerPos; // The player's designated position
	olc::vf2d currPlayerPos; // The current position of the player
	float playerTheta; // Angle in radians from horizontal axis of line from current positon to designated position
	float walkSpeed; // The walk speed of the player in pixels per second
public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		playerPos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
		currPlayerPos = playerPos;
		playerTheta = 0;
		walkSpeed = 100;
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame
		if (GetMouse(0).bPressed) // Move player on mouse click
		{
			playerPos = { float(GetMouseX()), float(GetMouseY()) };
			// Redefine theta
			playerTheta = atan2(currPlayerPos.y - playerPos.y, currPlayerPos.x - playerPos.x);
		}
		// Gradually move the player towards the designated position
		if (sqrt(pow(currPlayerPos.x - playerPos.x, 2) + pow(currPlayerPos.y - playerPos.y, 2)) > walkSpeed * fElapsedTime)
		{
			currPlayerPos.x -= walkSpeed * cos(playerTheta) * fElapsedTime;
			currPlayerPos.y -= walkSpeed * sin(playerTheta) * fElapsedTime;
		}
		// TODO: Do the same for other players in the game
		Clear(olc::BLACK); // Erase the previous frame
		FillCircle(currPlayerPos, 5, olc::WHITE); // Draw the player
		
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