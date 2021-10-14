#include "Boid.h"


#define NEARBY_DISTANCE		100.0f	// how far boids can see

Boid::Boid(float SpeedMax, float ForceMax,bool Predator)
{
	//this->m_position = XMFLOAT3(0, 0, 0);
	this->predator = Predator;
	this->m_offscreen = false;
	
	if (predator)
	{

	}
	else
	{
		createRandomDirection();
	}
}

Boid::~Boid()
{
}

void Boid::createRandomDirection()
{
	float x = (float)(rand() % 10-10);
	float y = (float)(rand() % 10-10);
	float z = 0;
	setDirection(XMFLOAT3(x, y, z));
}

void Boid::setDirection(XMFLOAT3 direction)
{
	XMVECTOR v = XMLoadFloat3(&direction);
	v = XMVector3Normalize(v);
	XMStoreFloat3(&m_direction, v);
}

void Boid::update(float t, vecBoid* boidList)
{
	// create a list of nearby boids
	vecBoid nearBoids = nearbyBoids(boidList);

	// NOTE these functions should always return a normalised vector
	XMFLOAT3  vAlignment = calculateAlignmentVector(&nearBoids);
	XMFLOAT3  vCohesion = calculateCohesionVector(&nearBoids);
	XMFLOAT3  vSeparation = calculateSeparationVector(&nearBoids);

	

	if(m_offscreen)
	{	
		XMFLOAT3 centre = XMFLOAT3(0, 0, 0);
		XMFLOAT3 toCentre = (subtractFloat3(centre, m_position));
		setDirection(toCentre);
	}
	else
	{
		setDirection(addFloat3(m_direction, vSeparation));
		setDirection(addFloat3(m_direction, vAlignment));
		setDirection(addFloat3(m_direction, vCohesion));
	}

	setPosition(addFloat3(m_position, m_direction));
	DrawableGameObject::update(t);
}

XMFLOAT3 Boid::calculateSeparationVector(vecBoid* boidList)
{
	XMFLOAT3 seperation = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return seperation;

	vecBoid NEARBY = nearbyBoids(boidList);

	if (NEARBY.size() > 0)
	{
		for (Boid* boid : NEARBY)
		{
			XMFLOAT3 temp = subtractFloat3(boid->m_position, this->m_position);
			seperation = subtractFloat3(seperation, temp);
		}
	}
	return normaliseFloat3(seperation);
}

XMFLOAT3 Boid::calculateAlignmentVector(vecBoid* boidList)
{
	XMFLOAT3 nearbyVelocity = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return nearbyVelocity;

	vecBoid NEARBY = nearbyBoids(boidList);

	if (NEARBY.size() > 0)
	{
		for (Boid* boid : NEARBY)
		{
			if (boid->predator)
				continue;

			nearbyVelocity = addFloat3(nearbyVelocity, boid->m_direction);
		}
	}

	nearbyVelocity = divideFloat3(nearbyVelocity, NEARBY.size());
	nearbyVelocity = divideFloat3(nearbyVelocity, 8);
	return normaliseFloat3(nearbyVelocity); // return the normalised (average) direction of nearby drawables
}

XMFLOAT3 Boid::calculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearbyCentre = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr)
		return nearbyCentre;

	vecBoid NEARBY = nearbyBoids(boidList);

	if (NEARBY.size() > 0)
	{
		for (Boid* boid : NEARBY)
		{
			if (boid->predator)
				continue;

			nearbyCentre = addFloat3(nearbyCentre, boid->m_position);
		}
	}

	nearbyCentre = divideFloat3(nearbyCentre, NEARBY.size()-1);
	nearbyCentre = subtractFloat3(nearbyCentre, this->m_position);
	nearbyCentre = divideFloat3(nearbyCentre, 100);

	return normaliseFloat3(nearbyCentre); // nearby is the direction to where the other drawables are
}

// use but don't alter the methods below

XMFLOAT3 Boid::addFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x + f2.x;
	out.y = f1.y + f2.y;
	out.z = f1.z + f2.z;

	return out;
}

XMFLOAT3 Boid::subtractFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	XMFLOAT3 out;
	out.x = f1.x - f2.x;
	out.y = f1.y - f2.y;
	out.z = f1.z - f2.z;

	return out;
}

XMFLOAT3 Boid::multiplyFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x * scalar;
	out.y = f1.y * scalar;
	out.z = f1.z * scalar;

	return out;
}

XMFLOAT3 Boid::divideFloat3(XMFLOAT3& f1, const float scalar)
{
	XMFLOAT3 out;
	out.x = f1.x / scalar;
	out.y = f1.y / scalar;
	out.z = f1.z / scalar;

	return out;
}

float Boid::magnitudeFloat3(XMFLOAT3& f1)
{
	return sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
}

XMFLOAT3 Boid::normaliseFloat3(XMFLOAT3& f1)
{
	float length = sqrt((f1.x * f1.x) + (f1.y * f1.y) + (f1.z * f1.z));
	if (length == 0) return XMFLOAT3(0, 0, 0);

	f1.x /= length;
	f1.y /= length;
	f1.z /= length;

	return f1;
}

vecBoid Boid::nearbyBoids(vecBoid* boidList)
{
	vecBoid nearBoids;
	if (boidList->size() == 0)
		return nearBoids;

	for (Boid* boid : *boidList) {
		// ignore self
		if (boid == this)
			continue;

		// get the distance between the two
		/*XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vB);
		float l = magnitudeFloat3(vDiff);
		if (l < NEARBY_DISTANCE) {
			nearBoids.push_back(boid);
		}*/

		XMFLOAT3 toOtherBoid = subtractFloat3(boid->m_position, this->m_position);

		if (magnitudeFloat3(toOtherBoid) >= NEARBY_DISTANCE)
			continue;

		// Is the boid in our field of view?
		float angle = abs(atan2(m_velocity.x, m_velocity.y) - atan2(toOtherBoid.x, toOtherBoid.y));
		angle *= 80 / 3.14;

		if (angle > BOID_FOV)
			continue;

		nearBoids.push_back(boid);
	}

	return nearBoids;
}

void Boid::checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj)
{
	XMFLOAT4 v4;
	v4.x = m_position.x;
	v4.y = m_position.y;
	v4.z = m_position.z;
	v4.w = 1.0f;

	XMVECTOR vScreenSpace = XMLoadFloat4(&v4);
	XMVECTOR vScreenSpace2 = XMVector4Transform(vScreenSpace, view);
	XMVECTOR vScreenSpace3 = XMVector4Transform(vScreenSpace2, proj);

	XMFLOAT4 v;
	XMStoreFloat4(&v, vScreenSpace3);
	v.x /= v.w;
	v.y /= v.w;
	v.z /= v.w;
	v.w /= v.w;

	float fOffset = 0; // a suitable distance to rectify position within clip space
	if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	{
		if (v.x < -1 || v.x > 1) {
			v4.x = -v4.x + (fOffset * v.x);
		}
		else if (v.y < -1 || v.y > 1) {
			v4.y = -v4.y + (fOffset * v.y);
		}
		// throw a bit of randomness into the mix
		createRandomDirection();
	}

	// method 1 - appear on the other side
	this->m_position.x = v4.x;
	this->m_position.y = v4.y;
	this->m_position.z = v4.z;
	
	// method2 - bounce off sides and head to centre
	//if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	//{
		//m_offscreen = true;
		//m_direction = multiplyFloat3(m_direction, -1);;
		//m_direction = normaliseFloat3(m_direction);
	//}
	//else if (magnitudeFloat3(m_position) < 50)
	//{
		//m_offscreen = false;
	//}

	return;
}