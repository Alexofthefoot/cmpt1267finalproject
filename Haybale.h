#pragma once
#include "SDL/SDL.h"
#include <string>

struct Vector2
{
	float x;
	float y;
};

class Haybale
{
public:
	void Initialize();
	int GetXPosition();
	int GetYPosition();
	bool UpdatePosition();
	bool IsClose(Vector2);

	Vector2 mHayBalePos;
	Vector2 mHayBaleDir;
	int size;

};

