#pragma once

class BFCO
{
public:
	static BFCO* GetSingleton()
	{
		static BFCO singleton;
		return &singleton;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	enum class Direction {
		kNeutral = 0,
		kForward,
		kStrafeRight,
		kBack,
		kStrafeLeft,
	};

	enum class DirectionOcto {
		kNeutral = 0,
		kForward,
		kForwardRight,
		kStrafeRight,
		kBackRight,
		kBack,
		kBackLeft,
		kStrafeLeft,
		kForwardLeft
	};

	Direction GetDirection(RE::NiPoint2 a_vec, bool a_gamepad);
	DirectionOcto GetDirectionOcto(RE::NiPoint2 a_vec, bool a_gamepad);

	void ProcessMovement(RE::PlayerControlsData* a_data, bool a_gamepad);

	struct Hooks
	{
		struct MovementHandler_ProcessThumbstick
		{
			static void thunk(RE::MovementHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data)
			{
				func(a_this, a_event, a_data);
				GetSingleton()->ProcessMovement(a_data, true);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct MovementHandler_ProcessButton
		{
			static void thunk(RE::MovementHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
			{
				func(a_this, a_event, a_data);
				GetSingleton()->ProcessMovement(a_data, false);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_vfunc<0x2, MovementHandler_ProcessThumbstick>(RE::VTABLE_MovementHandler[0]);
			stl::write_vfunc<0x4, MovementHandler_ProcessButton>(RE::VTABLE_MovementHandler[0]);
		}
	};
private:
	BFCO()
	{
	}

	BFCO(const BFCO&) = delete;
	BFCO(BFCO&&) = delete;

	~BFCO() = default;

	BFCO& operator=(const BFCO&) = delete;
	BFCO& operator=(BFCO&&) = delete;
};
