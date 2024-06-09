#include "BFCO.h"

#define _USE_MATH_DEFINES
#include <math.h>

// https://blackdoor.github.io/blog/thumbstick-controls/

static inline double GetAngle(RE::NiPoint2 a_vec)
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

static inline RE::hkbClipGenerator* ToClipGenerator(RE::hkbNode* a_node)
{
	constexpr char CLASS_NAME[] = "hkbClipGenerator";

	if (a_node && a_node->GetClassType()) {
		if (_strcmpi(a_node->GetClassType()->name, CLASS_NAME) == 0)
			return skyrim_cast<RE::hkbClipGenerator*>(a_node);
	}

	return nullptr;
}

static inline bool HasSCARComboEvent(RE::Actor* a_actor)
{
	if (!a_actor)
		return false;

	RE::BSAnimationGraphManagerPtr graphMgr;
	if (a_actor->GetAnimationGraphManager(graphMgr) && graphMgr) {
		auto behaviourGraph = graphMgr->graphs[0] ? graphMgr->graphs[0]->behaviorGraph : nullptr;
		auto activeNodes = behaviourGraph ? behaviourGraph->activeNodes : nullptr;
		if (activeNodes) {
			for (auto nodeInfo : *activeNodes) {
				auto nodeClone = nodeInfo.nodeClone;
				if (nodeClone && nodeClone->GetClassType()) {
					auto clipGenrator = ToClipGenerator(nodeClone);
					if (clipGenrator) {
						auto binding = clipGenrator->binding;
						auto animation = binding ? binding->animation : nullptr;
						if (animation && !animation->annotationTracks.empty()) {
							for (auto anno : animation->annotationTracks[0].annotations) {
								std::string_view text{ anno.text.c_str() };
								if (text.starts_with("SCAR_ComboStart"))
									return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
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

void BFCO::ProcessAttackWinStart(RE::Actor* a_actor)
{
	if (!a_actor || a_actor->IsPlayerRef() || !a_actor->IsAttacking() || (scarPlugin && HasSCARComboEvent(a_actor))) {
		return;
	}

	auto attackState = a_actor->GetAttackState();
	if (attackState > RE::ATTACK_STATE_ENUM::kNone && attackState < RE::ATTACK_STATE_ENUM::kNextAttack) {
		a_actor->actorState1.meleeAttackState = RE::ATTACK_STATE_ENUM::kNextAttack;
	}
}
