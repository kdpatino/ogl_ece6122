#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

// Include GLM
#include "final_project.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

unsigned int RenderObject::seedGenerator_ = 0; // Initialize the static member variable

RenderObject::RenderObject(const glm::vec3 &initialPosition, float radius, const glm::vec3 &boxMin,
                           const glm::vec3 &boxMax)
    : position_(initialPosition), objectRadius_(radius), boxMin_(boxMin), boxMax_(boxMax)
{
    // Seed the random number generator with a unique seed
    std::srand(static_cast<unsigned>(std::time(nullptr)) + seedGenerator_);
    std::cout << "Initila Position: (" << position_.x << ", " << position_.y << ", " << position_.z << ")" << std::endl;
    // Increment the seed for the next object
    id_ = seedGenerator_;
    ++seedGenerator_;

    rotationAngle_ = id_ * 90.0f;
    if (rotationAngle_ == 270.0f)
    {
        rotationAngle_ = -90.0f;
    }

    rotationVector_ = glm::vec3(0.0, 1.0, 0.0);

    // Generate random values for x, y, z directions
    float randomX = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f; // Random value between -1 and 1
    float randomY = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f; // Random value between -1 and 1
    float randomZ = static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f; // Random value between -1 and 1

    // Create a random direction vector
    movementDirection_ = glm::normalize(glm::vec3(randomX, randomY, randomZ));
}

void RenderObject::moveRandomly(float step)
{
    glm::vec3 nextPosition = position_ + movementDirection_ * step;
    if (isPositionInsideBox(nextPosition))
    {
        position_ = nextPosition;
    }
    else
    {
        glm::vec3 normal = calculateCollisionNormal(nextPosition);
        movementDirection_ = glm::reflect(movementDirection_, normal);
        position_ += movementDirection_ * (step);
    }
    rotationAngle_ += 10;
    rotationAngle_ = fmod(rotationAngle_, 360.0f);

    // Adjust to the desired range (-180 to 180)
    if (rotationAngle_ > 180.0f)
    {
        rotationAngle_ -= 360.0f;
    }
    else if (rotationAngle_ < -180.0f)
    {
        rotationAngle_ += 360.0f;
    }
    // Apply the movement in the specified direction to the initial position
}

glm::vec3 RenderObject::getFixPosition() const
{
    glm::vec3 position;
    position.x = position_.x;
    position.y = position_.z;
    position.z = position_.y;
    return position;
}

glm::vec3 RenderObject::getPosition() const
{
    return position_;
}

glm::vec3 RenderObject::getMovementDirection() const
{
    return movementDirection_;
}

void RenderObject::setMovementDirection(const glm::vec3 &newDirection)
{
    movementDirection_ = newDirection;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    glm::vec3 rotationVector(dis(gen), dis(gen), dis(gen));

    // Normalize the vector to make it a unit vector
    rotationVector_ = glm::normalize(rotationVector);
}

bool RenderObject::isPositionInsideBox(glm::vec3 &nextPosition)
{
    // Calculate the results of individual subtractions
    float xResult = nextPosition.x - objectRadius_;
    float yResult = nextPosition.y - objectRadius_;
    float zResult = nextPosition.z - objectRadius_;

    // Calculate the results of individual additions
    float xResultAdd = nextPosition.x + objectRadius_;
    float yResultAdd = nextPosition.y + objectRadius_;
    float zResultAdd = nextPosition.z + objectRadius_;

    bool insideBox = (xResult >= boxMin_.x && xResultAdd <= boxMax_.x && yResult >= boxMin_.y &&
                      yResultAdd <= boxMax_.y && zResult >= boxMin_.z && zResultAdd <= boxMax_.z);

    return insideBox;
}

glm::vec3 RenderObject::calculateCollisionNormal(const glm::vec3 &collisionPoint)
{
    if (collisionPoint.x <= boxMin_.x)
    {
        return glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    else if (collisionPoint.x >= boxMax_.x)
    {
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
    else if (collisionPoint.y <= boxMin_.y)
    {
        return glm::vec3(0.0f, -1.0f, 0.0f);
    }
    else if (collisionPoint.y >= boxMax_.y)
    {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }
    else if (collisionPoint.z <= boxMin_.z)
    {
        return glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }
}

void RenderObject::handleCollision(RenderObject &otherObject)
{
    glm::vec3 collisionNormal = glm::normalize(otherObject.getPosition() - getPosition());

    // Calculate the relative velocity
    glm::vec3 relativeVelocity = otherObject.getMovementDirection() - getMovementDirection();

    // Calculate the relative speed along the collision normal
    float relativeSpeed = glm::dot(relativeVelocity, collisionNormal);

    // Check if the objects are moving towards each other
    if (relativeSpeed < 0.0f)
    {
        // Reflect the movement direction of both objects
        setMovementDirection(glm::reflect(getMovementDirection(), collisionNormal));
        otherObject.setMovementDirection(glm::reflect(otherObject.getMovementDirection(), collisionNormal));
    }
}

bool RenderObject::isCollidingWith(const RenderObject &otherObject) const
{
    glm::vec3 centerOffset = otherObject.position_ - position_;
    float distance = glm::length(centerOffset);
    return distance < (objectRadius_ + otherObject.objectRadius_);
}

float RenderObject::getRotationAngle() const
{
    return rotationAngle_;
}

glm::vec3 RenderObject::getRotationVector() const
{
    return rotationVector_;
}