#pragma once
#include "DrawableGameObject.h"

#define		SEPARATION_OFFSET		1.0f
#define		ALIGNMENT_OFFSET		1.0f
#define		COHESION_OFFSET			1.0f
#define		PREDATOR_OFFSET		1.0f


class Boid :
	public DrawableGameObject
{
public:
	Boid(float SpeedMax, float ForceMax, bool Predator);
	~Boid();

	// boid traits
	//make randomised
	//Traits ALL should be randomised
	float										p_maxStam = 250.0f;
	float										b_maxStam = 0;

	float										speed;// = 75.0f;
	float										b_prevSpeed = 0;
	float										p_speedMax = 90;
	float										b_speedMax = 0;

	float										BOID_FOV = 0;// = 180.0f;
	float										b_Stamina = 0;// = 250.0f;
	float										nearbyDistance = 0;// = 100.0f;	// how far boids can see

	bool										recovery = false;

	float										p_Stamina = 200.0f;
	bool										m_dead = false;

	XMFLOAT3*							getDirection() { return &m_direction; }

	void									    checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void									    update(float t, vecBoid* drawList);

	void										StartTest(int x, int y);
	bool										predator;

protected:
	void								        setDirection(XMFLOAT3 direction);
	void								        createRandomDirection();

	vecBoid									nearbyBoids(vecBoid* boidList);

	//Calculate Flocking
	XMFLOAT3							calculateSeparationVector(vecBoid* drawList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* drawList);
	XMFLOAT3							calculateCohesionVector(vecBoid* drawList);
	XMFLOAT3							calculatePredatorAvoidance(vecBoid* drawList);
	XMFLOAT3							calculateTarget(vecBoid* drawList);

	//Predator Behaviours
	void										predatorSprint();
	void										predatorBehaviour();
	void										checkCaughtFish();
	Boid*									target;
	bool										p_chase = false;
	bool										b_chased = false;

	//Boid Behaviorus
	void										boidSprint();
	void										boidBehaviour();
	
	//Float Mathematics
	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);

	float								        magnitudeFloat3(XMFLOAT3& f1);
	float										distanceFloat3(XMFLOAT3& f1, XMFLOAT3& f2);

	XMFLOAT3							m_direction;
	XMFLOAT3							m_velocity;
	XMFLOAT3							m_acceleration;
	
};

