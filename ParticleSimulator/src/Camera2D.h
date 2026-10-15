
#pragma once

struct Camera2D
{
    float x = 0.0f;
    float y = 0.0f;
    float zoom = 1.0f;

    float moveSpeed = 400.0f;
    float zoomSpeed = 0.1f;

    void Reset()
    {
        x = 0.0f;
        y = 0.0f;
        zoom = 1.0f;
    }
};