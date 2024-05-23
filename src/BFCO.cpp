#include "BFCO.h"

#define _USE_MATH_DEFINES
#include <math.h>

// https://blackdoor.github.io/blog/thumbstick-controls/

double GetAngle(RE::NiPoint2 a_vec)
{
	//Normally atan2 takes Y,X, not X,Y.  We switch these around since we want 0
	// degrees to be straight up, not to the right like the unit circle;
	double angleInRadians = std::atan2(a_vec.x, a_vec.y);

	//atan2 gives us a negative value for angles in the 3rd and 4th quadrants.
	// We want a full 360 degrees, so we will add 2 PI to negative values.
	if (angleInRadians < 0.0f)
		angleInRadians += (M_PI * 2.0f);

	//Convert the radians to degrees.  Degrees are easier to visualize.
	double angleInDegrees = (180.0f * angleInRadians / M_PI);

	return angleInDegrees;
}

BFCO::Direction BFCO::GetDirection(RE::NiPoint2 a_vec, bool a_gamepad)
{
	if (a_vec.Length() > (a_gamepad ? 0.25f : 0.0f)) {
		//We have 4 sectors, so get the size of each in degrees.
		constexpr double sectorSize = 360.0f / 4;

		//We also need the size of half a sector
		constexpr double halfSectorSize = sectorSize / 2.0f;

		//First, get the angle using the function above
		double angle = GetAngle(a_vec);

		//Next, rotate our angle to match the offset of our sectors.
		double convertedAngle = angle + halfSectorSize;

		//Finally, we get the current direction by dividing the angle
		// by the size of the sectors
		Direction direction = (Direction)(std::floor(convertedAngle / sectorSize) + 1);

		//the result directions map as follows:
		// 0 = kForward, 1 = kStrafeRight, 2 = kBack 3 = kStrafeLeft.
		return direction;
	}
	DEBUG("kNeutral");
	return Direction::kNeutral;
}

BFCO::DirectionOcto BFCO::GetDirectionOcto(RE::NiPoint2 a_vec, bool a_gamepad)
{
	if (a_vec.Length() > (a_gamepad ? 0.25f : 0.0f)) {
		//We have 8 sectors, so get the size of each in degrees.
		constexpr double sectorSize = 360.0f / 8;

		//We also need the size of half a sector
		constexpr double halfSectorSize = sectorSize / 2.0f;

		//First, get the angle using the function above
		double angle = GetAngle(a_vec);

		//Next, rotate our angle to match the offset of our sectors.
		double convertedAngle = angle + halfSectorSize;

		//Finally, we get the current direction by dividing the angle
		// by the size of the sectors
		DirectionOcto direction = (DirectionOcto)(std::floor(convertedAngle / sectorSize) + 1);

		//the result directions map as follows:
		// 0 = kForward, 1 = kForwardRight, 2 = kStrafeRight ... 7 = kForwardLeft.
		return direction;
	}
	DEBUG("kNeutral");
	return DirectionOcto::kNeutral;
}

void BFCO::ProcessMovement(RE::PlayerControlsData* a_data, bool a_gamepad)
{
	if (auto player = RE::PlayerCharacter::GetSingleton()) {
		if (GetDirectionOcto(a_data->moveInputVec, a_gamepad) != DirectionOcto::kNeutral) {
			DEBUG("BFCO_MoveStart");
			player->NotifyAnimationGraph("BFCO_MoveStart");
		}
	}
}