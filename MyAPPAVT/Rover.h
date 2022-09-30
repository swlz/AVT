#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;
class Rover
{

public:
	std::vector<struct MyMesh> body;
	std::vector<struct MyMesh> wheels;
	float position_body[3] = { 0.0f, 0.0f, 0.0f };
	float position_wheel1[3] = { -0.15f, 0.3f, 0.0f };
	float position_wheel2[3] = { 1.0f, 0.3f, 0.0f };
	float position_wheel3[3] = { 1.0f, 0.3f, 1.0f };
	float position_wheel4[3] = { -0.15f, 0.3f, 1.0f };
	float direction_angle = 0.0f;
	float speed = 0.0f;
};