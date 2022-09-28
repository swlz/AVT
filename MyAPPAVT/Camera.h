#pragma once
class Camera
{
public:
	float camPos[3] = { 0.0f, 0.0f, 0.0f };
	float camTarget[3] = { 0.0f, 0.0f, 0.0f };
	int type = 0; //0:perspective, 1:orthographic
};

