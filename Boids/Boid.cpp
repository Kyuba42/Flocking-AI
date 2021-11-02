#include "Boid.h"

#define NEARBY_DISTANCE		100.0f	// how far boids can see

Boid::Boid(float SpeedMax, float ForceMax,bool Predator)
{
	//this->m_position = XMFLOAT3(0, 0, 0);
	this->predator = Predator;
	this->m_offscreen = false;
	
	if (predator)
	{
		DrawableGameObject::m_scale = 2.0f;
		createRandomDirection();
		speed = 60.0f;
	}
	else
	{
		DrawableGameObject::m_scale = 1.0f;
		createRandomDirection();
		speed = 75.0f + (rand() % 20 - 10);
	}
}

Boid::~Boid()
{
}

void Boid::createRandomDirection()
{
	float x = (float)(rand() % 50 - 50);
	float y = (float)(rand() % 50 - 50);
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

	if (!predator)
	{
		// NOTE these functions should always return a normalised vector
		XMFLOAT3  vAlignment =  calculateAlignmentVector(&nearBoids);
		XMFLOAT3  vCohesion =   calculateCohesionVector(&nearBoids);
		XMFLOAT3  vSeparation = calculateSeparationVector(&nearBoids);
		XMFLOAT3  vAvoidance = calculatePredatorAvoidance(&nearBoids);

		vAlignment = multiplyFloat3(vAlignment, ALIGNMENT_OFFSET);
		vCohesion = multiplyFloat3(vCohesion, COHESION_OFFSET);
		vSeparation = multiplyFloat3(vSeparation, SEPARATION_OFFSET);
		vAvoidance = multiplyFloat3(vAvoidance, PREDATOR_OFFSET);

		XMFLOAT3 steerTotal = addFloat3(vSeparation, vCohesion);
		steerTotal = addFloat3(steerTotal, vAlignment);
		steerTotal = addFloat3(steerTotal, vAvoidance);
		steerTotal = normaliseFloat3(steerTotal);

		float totalMag = magnitudeFloat3(steerTotal);

		if (totalMag > 0.9)
		{
			steerTotal = multiplyFloat3(steerTotal, 0.1);
			m_direction = addFloat3(steerTotal, m_direction);
			m_direction = normaliseFloat3(m_direction);
		}
	}
	else
	{
		XMFLOAT3 predatorSteer = calculateTarget(&nearBoids);
		
		float totalMag = magnitudeFloat3(predatorSteer);
		
		if (totalMag > 0.9)
		{
			predatorSteer = multiplyFloat3(predatorSteer, 0.1);
			m_direction = addFloat3(predatorSteer, m_direction);
			m_direction = normaliseFloat3(m_direction);
		}

		//Predator Chasing/Stamina Functions/Management

		if (!p_chase && speed != 0)
		{
			p_Stamina += 0.1f;
		}
		if (p_chase)
		{
			predatorSprint();
			checkCaughtFish();
		}
		if (p_Stamina <= 0)
		{
			p_Stamina = 0;

			if (speed > 20.0f)
				speed--;

			recovery = true;
		}
		if (recovery)
		{
			p_Stamina += 0.5f;

			if (p_Stamina >= PRED_STAM_MAX)
			{
				p_Stamina = PRED_STAM_MAX;
				speed = 75.0f;
				recovery = false;
			}
		}
	}

	XMFLOAT3 m_position_modifier = multiplyFloat3(m_direction, t * speed);
	setPosition(addFloat3(m_position, m_position_modifier));
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


			nearbyCentre = addFloat3(nearbyCentre, boid->m_position);

			if (boid->predator)
				nearbyCentre = multiplyFloat3(nearbyCentre, -1);
		}
	}

	nearbyCentre = divideFloat3(nearbyCentre, NEARBY.size()-1);
	nearbyCentre = subtractFloat3(nearbyCentre, this->m_position);
	nearbyCentre = divideFloat3(nearbyCentre, 100);

	return normaliseFloat3(nearbyCentre); // nearby is the direction to where the other drawables are
}

XMFLOAT3 Boid::calculatePredatorAvoidance(vecBoid* boidList)
{
	XMFLOAT3 avoidance = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return avoidance;

		vecBoid NEARBY = nearbyBoids(boidList);
		if (NEARBY.size() > 0)
		{
			for (Boid* boid : NEARBY)
			{
				if (boid->predator)
				{
					avoidance = subtractFloat3(boid->m_position, this->m_position);

				}
			}
		}

	avoidance = multiplyFloat3(avoidance, -1);
	return normaliseFloat3(avoidance);
}

XMFLOAT3 Boid::calculateTarget(vecBoid* boidList)
{
	vecBoid NEARBY = nearbyBoids(boidList);

	float closest = 10000000000000.0f;
	float current = 0.0f;
	XMFLOAT3 temp = XMFLOAT3(0,0,0);

	if (NEARBY.size() > 0)
	{
		for (Boid* boid : NEARBY)
		{
			if (boid->predator)
				continue;

			current = distanceFloat3(this->m_position, boid->m_position);

			if (current < closest)
			{
				closest = distanceFloat3(this->m_position, boid->m_position);
				temp = subtractFloat3(boid->m_position, this->m_position);
				target = boid;
			}
		}
	}

	if (closest < 25.0f)
	{
		p_chase = true;
	}

	return normaliseFloat3(temp); // nearby is the direction to where the other drawables are
}

void Boid::predatorSprint()
{
	if (p_Stamina > 0.0f)
	{
		if (speed < maxSpeed)
			speed += 0.2f;

		p_Stamina -= 1.0f;
	}
	else
		p_chase = false;
}

void Boid::checkCaughtFish()
{
	if(distanceFloat3(this->m_position, target->m_position) < 2.0f)
	{
		target->m_position.x = 10000000000;
		target->m_position.y = 10000000000;
		p_chase = false;
	}
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

float Boid::distanceFloat3(XMFLOAT3& f1, XMFLOAT3& f2)
{
	// Calculating distance
	return sqrt(pow(f2.x - f1.x, 2) +pow(f2.y - f1.y, 2) * 1.0);
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
		XMFLOAT3 vB = *(boid->getPosition());
		XMFLOAT3 vDiff = subtractFloat3(m_position, vB);
		float l = magnitudeFloat3(vDiff);
		if (l < NEARBY_DISTANCE) {
			nearBoids.push_back(boid);
		}

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

void Boid::checkIsOnScreenAndFix(const XMMATRIX&  view, const XMMATRIX&  proj, vecBoid* boidList)
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

		if (v.x < -1 || v.x > 1) 
		{
			v4.x = -v4.x + (fOffset * v.x);
		}

		if (v.y < -1 || v.y > 1) 
		{
			v4.y = -v4.y + (fOffset * v.y);
		}
		// throw a bit of randomness into the mix
		//createRandomDirection();

	// method 1 - appear on the other side
	//m_position.x = v4.x;
	//m_position.y = v4.y;
	//m_position.z = v4.z;
	
	// method2 - bounce off sides and head to centre
	if (v.x < -1 || v.x > 1 || v.y < -1 || v.y > 1)
	{
			setDirection(multiplyFloat3(m_direction ,-1));
			setDirection(normaliseFloat3(m_direction));
	}

	return;
}