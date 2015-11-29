#import "../constants.h"
#import "../unit.h"

namespace signum::env
{

class ASR : public Unit
{
	public:
		ASR(float attack = 0.1, float sustain = 0.5, float release = 0.1, UnitRef clock = nullptr);

		float attack;
		float sustain;
		float release;
		float phase;
		float clock_last;

		UnitRef clock = nullptr;

		virtual void trigger();
		virtual void next(sample **out, int num_frames);
};

REGISTER(ASR, "env-asr");

}
