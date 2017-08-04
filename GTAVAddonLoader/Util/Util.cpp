#define NOMINMAX
#include "../../../ScriptHookV_SDK/inc/natives.h"
#include "Util.hpp"
#include <algorithm>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void showText(float x, float y, float scale, const char* text, int font, const Color &rgba, bool outline) {
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(scale, scale);
	UI::SET_TEXT_COLOUR(rgba.R, rgba.G, rgba.B, rgba.A);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(0);
	if (outline) UI::SET_TEXT_OUTLINE();
	UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(text));
	UI::END_TEXT_COMMAND_DISPLAY_TEXT(x, y);
}

void showNotification(const char* message, int *prevNotification) {
	if (prevNotification != nullptr && *prevNotification != 0) {
		UI::_REMOVE_NOTIFICATION(*prevNotification);
	}
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");

	UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message));
	
	int id = UI::_DRAW_NOTIFICATION(false, false);
	if (prevNotification != nullptr) {
		*prevNotification = id;
	}
}

// gracefully borrowed from FiveM <3
void showSubtitle(std::string message, int duration) {
	UI::BEGIN_TEXT_COMMAND_PRINT("CELL_EMAIL_BCON");

	const int maxStringLength = 99;

	for (int i = 0; i < message.size(); i += maxStringLength) {
		int npos = std::min(maxStringLength, static_cast<int>(message.size()) - i);
		UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message.substr(i, npos).c_str()));
	}

	UI::END_TEXT_COMMAND_PRINT(duration, 1);
}

GameSound::GameSound(char *sound, char *soundSet): m_prevNotification(0) {
	Active = false;
	m_sound = sound;
	m_soundSet = soundSet;
	m_soundID = -1;
}

GameSound::~GameSound() {
	if (m_soundID == -1 || !Active) return;
	AUDIO::RELEASE_SOUND_ID(m_soundID);
}

void GameSound::Load(char *audioBank) {
	AUDIO::REQUEST_SCRIPT_AUDIO_BANK(audioBank, false);
}

void GameSound::Play(Entity ent) {
	if (Active) return;
	m_soundID = AUDIO::GET_SOUND_ID();
	//showNotification(("New soundID: " + std::to_string(m_soundID)).c_str(), nullptr);
	AUDIO::PLAY_SOUND_FROM_ENTITY(m_soundID, m_sound, ent, m_soundSet, 0, 0);
	Active = true;
}

void GameSound::Stop() {
	if (m_soundID == -1 || !Active) return;
	AUDIO::STOP_SOUND(m_soundID);
	Active = false;
}

// https://codereview.stackexchange.com/questions/149717/implementation-of-c-standard-library-function-ntohl
uint32_t my_ntohl(uint32_t const net) {
	uint8_t data[4] = {};
	memcpy(&data, &net, sizeof(data));

	return ((uint32_t)data[3] << 0)
		| ((uint32_t)data[2] << 8)
		| ((uint32_t)data[1] << 16)
		| ((uint32_t)data[0] << 24);
}

bool GetIMGDimensions(std::string file, int *width, int *height) {
	auto ext = fs::path(file).extension();
	if (ext == ".png" || ext == ".PNG")
		return GetPNGDimensions(file, width, height);
	if (ext == ".jpg" || ext == ".JPG" || ext == ".jpeg" || ext == ".JPEG")
		return GetJPGDimensions(file, width, height);
	return false;
}

bool GetPNGDimensions(std::string file, int *width, int *height) {
	std::ifstream in(file);
	int _width, _height;

	in.seekg(16);
	in.read((char *)&_width, 4);
	in.read((char *)&_height, 4);

	*width = my_ntohl(_width);
	*height = my_ntohl(_height);

	return true;
}

//https://stackoverflow.com/questions/17847171/c-library-for-getting-the-jpeg-image-size
bool GetJPGDimensions(std::string file, int *width, int *height) {
	FILE *image = nullptr;

	errno_t err = fopen_s(&image, file.c_str(), "rb");  // open JPEG image file
	if (!image || err) {
		return false;
	}
	fseek(image, 0, SEEK_END);
	int size = ftell(image);
	fseek(image, 0, SEEK_SET);
	unsigned char *data = (unsigned char *)malloc(size);
	fread(data, 1, size, image);
	/* verify valid JPEG header */
	int i = 0;
	if (data[i] == 0xFF && data[i + 1] == 0xD8 && data[i + 2] == 0xFF && data[i + 3] == 0xE0) {
		i += 4;
		/* Check for null terminated JFIF */
		if (data[i + 2] == 'J' && data[i + 3] == 'F' && data[i + 4] == 'I' && data[i + 5] == 'F' && data[i + 6] == 0x00) {
			while (i < size) {
				i++;
				if (data[i] == 0xFF) {
					if (data[i + 1] == 0xC0) {
						*height = data[i + 5] * 256 + data[i + 6];
						*width = data[i + 7] * 256 + data[i + 8];
						break;
					}
				}
			}
		}
		else {
			fclose(image);
			return false;
		}
	}
	else {
		fclose(image);
		return false;
	}
	fclose(image);
	return true;
}
