#pragma once
#include "DrawableGameObject.h"

#define		SEPARATION_OFFSET		1.0f
#define		ALIGNMENT_OFFSET		1.0f
#define		COHESION_OFFSET			1.0f
#define		PREDATOR_OFFSET			1.0f

#define		PRED_STAM_MAX				200.0f

class Boid :
	public DrawableGameObject
{
public:
	Boid(float SpeedMax, float ForceMax, bool Predator);
	~Boid();

	XMFLOAT3*					    getDirection() { return &m_direction; }

	void									    checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj, vecBoid* boidList);
	void									    update(float t, vecBoid* drawList);

protected:
	void								        setDirection(XMFLOAT3 direction);
	void								        createRandomDirection();

	vecBoid								nearbyBoids(vecBoid* boidList);

	//Calculate Flocking
	XMFLOAT3							calculateSeparationVector(vecBoid* drawList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* drawList);
	XMFLOAT3							calculateCohesionVector(vecBoid* drawList);
	XMFLOAT3							calculatePredatorAvoidance(vecBoid* drawList);
	XMFLOAT3							calculateTarget(vecBoid* drawList);

	//Predator Behaviours
	void										predatorSprint();
	void										checkCaughtFish();
	Boid*									target;
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
	
	//Traits
	float										speed =  75.0f;
	float										maxSpeed = 100.0f;
	float										stamina = 10.0f;
	float										BOID_FOV = 180.0f;

	bool										m_offscreen;
	bool										predator;
	bool										p_chase = false;
	bool										reflecting = false;
	bool										recovery = false;

	float										p_Stamina = 200.0f;
	float										b_Stamina = 250.0f;
	//unsigned int*						m_nearbyDrawables;

};

