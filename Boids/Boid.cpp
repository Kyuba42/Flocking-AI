#include "Boid.h"

//ImGui Globals
vector<Boid*>	g_deadBoids;

bool					alignmentActive;
bool					cohesionActive;
bool					seperationActive;
bool					avoidanceActive;

int					alignmentOffsetMultiplyer = 1;
int					cohesionOffsetMultiplyer = 1;
int					separationOffsetMultiplyer = 1;
int					avoidanceOffsetMultiplyer = 1;

Boid::Boid(float SpeedMax, float ForceMax,bool Predator)
{
	this->predator = Predator;

	if (predator)
	{
		DrawableGameObject::m_scale = 2.f;
		createRandomDirection();
		speed = 60.f;
		target = nullptr;
		BOID_FOV = 180.f;
		nearbyDistance = 100.f;
	}
	else
	{
		alignmentActive = true;
		cohesionActive = true;
		seperationActive = true;
		avoidanceActive = true;

		DrawableGameObject::m_scale = 1.0f;
		createRandomDirection();

		b_maxStam = 250;
		b_Stamina = 250;// = 250.0f;

		speed = 70;// = 75.0f;
		BOID_FOV = 180;// = 180.0f;
		nearbyDistance =100;// = 100.0f;	// how far boids can see
	}
}

Boid::~Boid()
{
}

void Boid::StartTest(int x, int y)
{
	alignmentActive = true;
	cohesionActive = true;
	seperationActive = true;
	avoidanceActive = true;

	alignmentOffsetMultiplyer = 1;
	cohesionOffsetMultiplyer = 1;
	separationOffsetMultiplyer = 1;
	avoidanceOffsetMultiplyer = 1;

	createRandomDirection();

	b_maxStam = rand() % 200 + 1;
	speed = rand() % 100 + 1;
	BOID_FOV = rand() % 280 + 10;
	while (b_Stamina > b_maxStam)
		b_Stamina = rand() % 200 + 1;
	nearbyDistance = rand() % 300 + 1;

	setPosition(XMFLOAT3(x + rand() % 50, y + rand() % 50, 0));
	m_dead = false;
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

		vAlignment = multiplyFloat3(vAlignment,		(ALIGNMENT_OFFSET	* alignmentOffsetMultiplyer));
		vCohesion = multiplyFloat3(vCohesion,			(COHESION_OFFSET		* cohesionOffsetMultiplyer));
		vSeparation = multiplyFloat3(vSeparation,	(SEPARATION_OFFSET * separationOffsetMultiplyer));
		vAvoidance = multiplyFloat3(vAvoidance,		(PREDATOR_OFFSET      * avoidanceOffsetMultiplyer));
		XMFLOAT3 steerTotal = XMFLOAT3(0,0,0);


		//check if forces are active
		if (seperationActive)
			steerTotal = addFloat3(steerTotal, vSeparation);
		if (cohesionActive)
			steerTotal = addFloat3(steerTotal, vCohesion);
		if (alignmentActive)
			steerTotal = addFloat3(steerTotal, vAlignment);
		if(avoidanceActive)
			steerTotal = addFloat3(steerTotal, vAvoidance);
		
		steerTotal = normaliseFloat3(steerTotal);

		if (magnitudeFloat3(steerTotal) > 0.9)
		{
			steerTotal = multiplyFloat3(steerTotal, 0.1);
			m_direction = addFloat3(steerTotal, m_direction);
			m_direction = normaliseFloat3(m_direction);
		}

		boidBehaviour();
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
		predatorBehaviour();
		
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

	vecBoid nBoidList = nearbyBoids(boidList);

	if (nBoidList.size() > 0)
	{
		for (Boid* boid : nBoidList)
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

	vecBoid nBoidList = nearbyBoids(boidList);

	if (nBoidList.size() > 0)
	{
		for (Boid* boid : nBoidList)
		{
			if (boid->predator)
				continue;

			nearbyVelocity = addFloat3(nearbyVelocity, boid->m_direction);
		}
	}

	nearbyVelocity = divideFloat3(nearbyVelocity, nBoidList.size());
	//	nearbyVelocity = divideFloat3(nearbyVelocity, 8);
	return normaliseFloat3(nearbyVelocity); // return the normalised (average) direction of nearby drawables
}

XMFLOAT3 Boid::calculateCohesionVector(vecBoid* boidList)
{
	XMFLOAT3 nearbyCentre = XMFLOAT3(0, 0, 0);

	if (boidList == nullptr)
		return nearbyCentre;

	vecBoid nBoidList = nearbyBoids(boidList);

	if (nBoidList.size() > 0)
	{
		for (Boid* boid : nBoidList)
		{
			nearbyCentre = addFloat3(nearbyCentre, boid->m_position);

			if (boid->predator)
				nearbyCentre = multiplyFloat3(nearbyCentre, -1);
		}
	}

	nearbyCentre = divideFloat3(nearbyCentre, nBoidList.size());
	nearbyCentre = subtractFloat3(nearbyCentre, this->m_position);
	//nearbyCentre = divideFloat3(nearbyCentre, 100);

	return normaliseFloat3(nearbyCentre); // nearby is the direction to where the other drawables are
}

XMFLOAT3 Boid::calculatePredatorAvoidance(vecBoid* boidList)
{
	XMFLOAT3 avoidance = XMFLOAT3(0, 0, 0);
	if (boidList == nullptr)
		return avoidance;

		vecBoid nBoidList = nearbyBoids(boidList);
		if (nBoidList.size() > 0)
		{
			for (Boid* boid : nBoidList)
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
	Boid* prevTarg = nullptr;

	if (target != nullptr)
		prevTarg = target;

	vecBoid nBoidList = nearbyBoids(boidList);

	float closest = INFINITY;
	float current = 0.0f;
	XMFLOAT3 targetPos = XMFLOAT3(0,0,0);

	if (nBoidList.size() > 0)
	{
		for (Boid* boid : nBoidList)
		{
			if (boid->predator)
				continue;
			
			if (boid->b_chased)
				continue;

			current = distanceFloat3(this->m_position, boid->m_position);

			if (current < closest)
			{
				closest = current;
				targetPos = subtractFloat3(boid->m_position, this->m_position);
				
				if (target != nullptr)
				{
					target->b_chased = false;
				}
				target = boid;
			}
		}
	}
	else
	{
		target = nullptr;
	}

	if (closest < 25.0f)
	{
		p_chase = true;
		
		if (target != nullptr)
			target->b_chased = true;
	}
	else
	{
		p_chase = false;

		if (target != nullptr)	
			target->b_chased = false;
	}


	return normaliseFloat3(targetPos); // nearby is the direction to where the other drawables are
}

void Boid::predatorSprint()
{
	if (p_Stamina > 0.0f && !recovery)
	{
		if (speed < p_speedMax)
			speed += 0.4f;

		p_Stamina -= 1.0f;
	}
	else
		p_chase = false;
}

void Boid::predatorBehaviour()
{
	if (!p_chase && !recovery)
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

		while (speed > 20.0f)
			speed--;

		recovery = true;
	}

	if (recovery)
	{
		p_Stamina += 0.5f;

		if (p_Stamina >= p_maxStam)
		{
			p_Stamina = p_maxStam;
			speed = 60.0f;
			recovery = false;
		}

	}

}

void Boid::checkCaughtFish()
{
	if (target != nullptr)
	{
		if (distanceFloat3(this->m_position, target->m_position) < 2.0f)
		{
			target->m_position.x = INFINITY;
			target->m_position.y = INFINITY;
			p_chase = false;
			m_dead = true;
			
			g_deadBoids.push_back(target);
		}
	}
}

void Boid::boidSprint()
{
	if (b_Stamina > 0.0f)
	{
		if (speed < b_speedMax)
			speed += 0.2f;

		b_Stamina -= 1.0f;
	}
	else
	{
		b_prevSpeed = speed;
		b_chased = false;
	}
		
}

void Boid::boidBehaviour()
{
	if (!b_chased && !recovery)
	{
		b_Stamina += 0.1f;
	}
	if (b_chased)
	{
		boidSprint();
	}
	if (b_Stamina <= 0)
	{
		b_Stamina = 0;

		while (speed > 10.0f)
			speed--;

		recovery = true;
	}
	if (recovery)
	{
		b_Stamina += 0.5f;

		if (b_Stamina >= b_maxStam)
		{
			b_Stamina = b_maxStam;
			speed = b_prevSpeed;
			b_prevSpeed = 0.f;
			recovery = false;
		}
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
	return sqrt(pow(f2.x - f1.x, 2) +pow(f2.y - f1.y, 2));
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
		if (l < nearbyDistance) {
			nearBoids.push_back(boid);
		}

		XMFLOAT3 toOtherBoid = subtractFloat3(boid->m_position, this->m_position);

		if (magnitudeFloat3(toOtherBoid) >= nearbyDistance)
			continue;

		// Is the boid in our field of view?
		float angle = abs(atan2(m_velocity.x, m_velocity.y) - atan2(toOtherBoid.x, toOtherBoid.y));
		angle *= 90 / 3.14;

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

		if (v.x < -1 || v.x > 1) 
		{
			v4.x = -v4.x + (fOffset * v.x);
		}
		else if (v.y < -1 || v.y > 1) 
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