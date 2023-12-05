#pragma once

#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
struct CollisionInfo
{
    int object1Index;
    int object2Index;
};

class RenderObject
{
  public:
    RenderObject(const glm::vec3 &initialPosition, float radius, const glm::vec3 &boxMin, const glm::vec3 &boxMax);
    void moveRandomly(float step);
    glm::vec3 getFixPosition() const;
    glm::vec3 getPosition() const;
    float getRotationAngle() const;
    glm::vec3 getRotationVector() const;
    glm::vec3 getMovementDirection() const;
    bool isPositionInsideBox(glm::vec3 &nextPosition);
    glm::vec3 calculateCollisionNormal(const glm::vec3 &collisionPoint);
    void handleCollision(RenderObject& otherObject);
    void setMovementDirection(const glm::vec3& newDirection);
    bool isCollidingWith(const RenderObject &otherObject) const;

  private:
    glm::vec3 position_;
    float rotationAngle_;
    glm::vec3 rotationVector_;
    glm::vec3 movementDirection_;
    glm::vec3 boxMin_;
    glm::vec3 boxMax_;
    float objectRadius_;
    int id_;
    static unsigned int seedGenerator_;
};
