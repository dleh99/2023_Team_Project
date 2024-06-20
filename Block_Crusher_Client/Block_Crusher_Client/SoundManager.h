#pragma once
#include "stdafx.h"
#include "Singleton.h"

//constexpr int EFFECT_SOUND_NUM = 1;
constexpr int CHANNEL_NUM = 20;

enum SOUND { GUN, DEATH, COUNT };

class SoundManager : public Singleton<SoundManager>
{
private:
	// fmod ¸â¹ö
	FMOD_SYSTEM* m_SoundSystem;

	FMOD_SOUND* m_Stage_soundFile;
	FMOD_SOUND* m_Battle_soundFile;
	FMOD_SOUND* m_effect_sound[SOUND::COUNT];
	FMOD_CHANNEL* m_sound_channel[CHANNEL_NUM];

public:
	SoundManager();
	void StartStageSound();
	void StartBattleSound();
	void EffectSound(SOUND tag);
};