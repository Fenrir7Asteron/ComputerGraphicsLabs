#define NOMINMAX
#include "PongGame.h"
#include "PongRacket.h"
#include "PongWall.h"
#include <iostream>
#include <cmath>

PongGame::PongGame(LPCWSTR applicationName) : GameFramework::GameFramework(applicationName)
{

}

void PongGame::Init(int windowWidth, int windowHeight)
{
	GameFramework::Init(windowWidth, windowHeight);

	pongBall_ = new PongBall(this, { 0.0f, 0.0f, 0.0f }, 0.03f, 32, 0.6f, 0.1f);

	PongRacket* playerRacket = new PongRacket(this, PhysicalLayer::Player, { -0.85f, 0.0f, 0.0f }, 0.05f, 0.3f, 1.75f, 60.0f, false, pongBall_);
	PongRacket* enemyRacket = new PongRacket(this, PhysicalLayer::Enemy, { 0.85f, 0.0f, 0.0f }, 0.05f, 0.3f, 0.9f, 60.0f, true, pongBall_);
	rackets_.push_back(playerRacket);
	rackets_.push_back(enemyRacket);

	AddComponent(playerRacket);
	AddComponent(enemyRacket);
	AddComponent(new PongWall(this, PhysicalLayer::Default, { 0.0f, 1.225f, 0.0f }, 0.5f, 10000.0f, { 0.0f, -1.0f, 0.0f }, 0.1f));
	AddComponent(new PongWall(this, PhysicalLayer::Default, { 0.0f, -1.225f, 0.0f }, 0.5f, 10000.0f, { 0.0f, 1.0f, 0.0f }, 0.1f));

	
	AddComponent(pongBall_);
	pongBall_->BallEnteredGoal.AddRaw(this, &PongGame::ScoredABall);

	score.playerScore = 0;
	score.enemyScore = 0;
	goalScoreAnimationDuration = 3.0f;
}

void PongGame::Update()
{
	GameFramework::Update();

	if (goalAnimationInProgress) 
	{
		if (currentGoalDelayTime < goalScoreAnimationDuration)
		{
			currentGoalDelayTime += deltaTime;

			double intpart;
			float fractpart = std::modf(currentGoalDelayTime, &intpart);

			if (fractpart > 0.5f)
				fractpart = 1.0f - fractpart;

			if (playerScoredAGoal)
				pongBall_->color = {0.0f, fractpart, 0.0f, 1.0f};
			else
				pongBall_->color = { fractpart, 0.0f, 0.0f, 1.0f };
			return;
		}

		goalAnimationInProgress = false;

		pongBall_->enabled = true;

		pongBall_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
		pongBall_->positionOffset = { 0.0f, 0.0f, 0.0f };
		pongBall_->boundingBox.Center = pongBall_->positionOffset;

		if (playerScoredAGoal)
			pongBall_->currentSpeed = { pongBall_->startSpeed, 0.0f, 0.0f };
		else
			pongBall_->currentSpeed = { -pongBall_->startSpeed, 0.0f, 0.0f };
	}
}

void PongGame::ScoredABall(bool playerScoredAGoal)
{
	pongBall_->enabled = false;

	if (playerScoredAGoal) {
		std::cout << "Player scored a goal!" << std::endl;
		score.playerScore++;
	}
	else {
		std::cout << "Enemy scored a goal!" << std::endl;
		score.enemyScore++;
	}

	printf("SCORE %d:%d\n", score.playerScore, score.enemyScore);

	pongBall_->positionOffset.x = std::min(pongBall_->positionOffset.x, 0.95f);
	pongBall_->positionOffset.x = std::max(pongBall_->positionOffset.x, -0.95f);
	
	currentGoalDelayTime = 0.0f;
	goalAnimationInProgress = true;
	this->playerScoredAGoal = playerScoredAGoal;
}
