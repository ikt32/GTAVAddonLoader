#pragma once

#include <string>
#include <vector>
#include "inc/types.h"

struct Color {
	int R;
	int G;
	int B;
	int A;
};

const Color solidWhite = {
	255, 255, 255, 255
};

// Natives called
void showText(float x, float y, float scale, const char* text, int font = 0, const Color &rgba = solidWhite, bool outline = false);
void showNotification(const char* message, int *prevNotification = nullptr);
void showSubtitle(std::string message, int duration = 2500);

//https://github.com/CamxxCore/AirSuperiority
class GameSound {
public:
	GameSound(char *sound, char *soundSet);
	~GameSound();
	void Load(char *audioBank);
	void Play(Entity ent);
	void Stop();

	bool Active;

private:
	char *m_soundSet;
	char *m_sound;
	int m_soundID;
	int m_prevNotification;
};

bool GetIMGDimensions(std::string file, int *width, int *height);

bool GetPNGDimensions(std::string file, int *width, int *height);
bool GetJPGDimensions(std::string file, int *width, int *height);
