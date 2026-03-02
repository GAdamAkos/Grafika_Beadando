#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>

typedef struct {
    float x, y, z;      // Position
    float M;            // Height (eye level)
    float pitch, yaw;   // Rotation angles
    float speed;
    float sensitivity;
} Camera;

// Initialize the camera
void init_camera(Camera* camera);

// Update camera position based on input
void update_camera(Camera* camera, double delta_time);

// Apply the camera transformation to the OpenGL view
void apply_camera(Camera* camera);

#endif