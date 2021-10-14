#pragma once
#include "DrawableGameObject.h"
//#define MAX_ACCELERATION 2.0f
#define ACCELERATION_SCALER 
class Boid :
	public DrawableGameObject
{
public:
	Boid(float SpeedMax, float ForceMax, bool Predator);
	~Boid();

	XMFLOAT3*					    getDirection() { return &m_direction; }
	void									    checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj);
	void									    update(float t, vecBoid* drawList);

protected:
	void								        setDirection(XMFLOAT3 direction);

	vecBoid								    nearbyBoids(vecBoid* boidList);
	XMFLOAT3							calculateSeparationVector(vecBoid* drawList);
	XMFLOAT3							calculateAlignmentVector(vecBoid* drawList);
	XMFLOAT3							calculateCohesionVector(vecBoid* drawList);
	void								        createRandomDirection();

	XMFLOAT3							addFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2);
	XMFLOAT3							normaliseFloat3(XMFLOAT3& f1);
	float								        magnitudeFloat3(XMFLOAT3& f1);

	XMFLOAT3							multiplyFloat3(XMFLOAT3& f1, const float scalar);
	XMFLOAT3							divideFloat3(XMFLOAT3& f1, const float scalar);

	XMFLOAT3							m_direction;
	XMFLOAT3							m_velocity;
	XMFLOAT3							m_acceleration;
	bool										m_offscreen;

	float									    speedMax;
	float										forceMax;
	bool										predator;

	float										BOID_FOV = 90.0f;
	//unsigned int*						m_nearbyDrawables;

};

