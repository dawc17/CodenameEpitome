#pragma once

#include "raylib.h"
#include "raymath.h"
#include <random>
#include <string>

namespace Utils {
    // Random number generation
    inline std::mt19937& GetRNG() {
        static std::mt19937 rng(std::random_device{}());
        return rng;
    }
    
    inline void SeedRNG(unsigned int seed) {
        GetRNG().seed(seed);
    }
    
    inline int RandomInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(GetRNG());
    }
    
    inline float RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(GetRNG());
    }
    
    // Vector helpers
    inline Vector2 RandomDirection() {
        float angle = RandomFloat(0.0f, 2.0f * PI);
        return { cosf(angle), sinf(angle) };
    }
    
    inline Vector2 DirectionFromAngle(float angleDegrees) {
        float rad = angleDegrees * DEG2RAD;
        return { cosf(rad), sinf(rad) };
    }
    
    inline float AngleFromDirection(Vector2 dir) {
        return atan2f(dir.y, dir.x) * RAD2DEG;
    }
    
    inline Vector2 RotateVector(Vector2 v, float angleDegrees) {
        float rad = angleDegrees * DEG2RAD;
        float c = cosf(rad);
        float s = sinf(rad);
        return { v.x * c - v.y * s, v.x * s + v.y * c };
    }
    
    // Lerp helpers
    inline float Lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    
    inline Vector2 LerpV(Vector2 a, Vector2 b, float t) {
        return { Lerp(a.x, b.x, t), Lerp(a.y, b.y, t) };
    }
    
    inline float SmoothDamp(float current, float target, float smoothTime, float dt) {
        float t = 2.0f / smoothTime;
        float x = t * dt;
        float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
        return Lerp(current, target, 1.0f - exp);
    }
    
    // Collision helpers
    inline bool CircleRectCollision(Vector2 circlePos, float radius, Rectangle rect) {
        float closestX = Clamp(circlePos.x, rect.x, rect.x + rect.width);
        float closestY = Clamp(circlePos.y, rect.y, rect.y + rect.height);
        float distX = circlePos.x - closestX;
        float distY = circlePos.y - closestY;
        return (distX * distX + distY * distY) < (radius * radius);
    }
    
    // Screen shake
    struct ScreenShake {
        float duration = 0.0f;
        float intensity = 0.0f;
        Vector2 offset = {0, 0};
        
        void Trigger(float dur, float inten) {
            duration = dur;
            intensity = inten;
        }
        
        void Update(float dt) {
            if (duration > 0) {
                duration -= dt;
                offset = { RandomFloat(-intensity, intensity), 
                          RandomFloat(-intensity, intensity) };
            } else {
                offset = {0, 0};
            }
        }
    };
}
