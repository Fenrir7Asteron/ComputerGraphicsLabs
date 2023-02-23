#pragma once
#include "GameFramework.h"
#include "PongBall.h"
#include "PongScore.h"

class PongGame : public GameFramework
{
public:
	PongGame(LPCWSTR applicationName);
	void Init(int windowWidth, int windowHeight) override;
	GAMEFRAMEWORK_API virtual void Update();

private:
	void ScoredABall(bool playerScoredAGoal);
	PongBall* pongBall_;
	std::vector<PongRacket*> rackets_;
	PongScore score;
	float goalScoreAnimationDuration;
	float currentGoalDelayTime;
	bool goalAnimationInProgress;
	bool playerScoredAGoal;
};

