#pragma once

struct PlayerInputs
{
	float pitch;
	float yaw;
	float roll;
	float throttle; //-1 for decreasing, 1 for increasing, 0 for idle
};