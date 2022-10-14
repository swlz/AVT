#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;
class Rover
{

public:
    // first cube, then 4 wheels
	std::vector<struct MyMesh> body;
	std::vector<struct MyMesh> wheels;
	// 1st ... pos_cube, rest ... pos_wheel
	float position[5][3] = { { 0.0f, 0.5f, 0.0f },
							{ 0.5f, 0.2f, 0.5f },
									{ -0.5f, 0.2f, 0.5f },
									{ 0.5f, 0.2f, -0.5f },
									{ -0.5f, 0.2f, -0.5f } };
	float direction[3] = { 0.0f, 0.0f, 1.0f };
	float angle = 0.0f;
	float speed = 0.0f;
	float max_pos[3];
	float min_pos[3];
};