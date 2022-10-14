#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

using namespace std;
class Rocks
{
public:
	vector<struct MyMesh> amesh;
	float position[3] = { 0.0f, 0.0f, 0.0f };
	float direction[3] = { 0.0f, 0.0f, 1.0f };
	float angle = 0.0f;
	float speed = 0.001f;
	float max_pos[3];
	float min_pos[3];
};

