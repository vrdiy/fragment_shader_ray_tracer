#pragma once
#ifndef RAYTRACE_CAMERA_H_
#define	RAYTRACE_CAMERA_H_

#include <vector>

#include <SDL.h>

#include "types.h"
#include "util.h"

namespace raytrace {
	class Camera { // +Z is forward, +Y is up
	public:
		
		Camera(vec3 position, float roll = 0.0f, float pitch = 0.0f, float yaw = 0.0f):position(position),roll(roll), pitch(pitch), yaw(yaw) {};
		vec3 position;

		// Rotations in degrees
		float roll;
		float pitch;
		float yaw;
		float speed = 1.0f;
		vec3 forward = vec3(0.0f, 0.0f, 1.0f); // Based on RayTracer::CanvasToViewport having +Z looking forward
		vec3 up = vec3(0.0f, 1.0f, 0.0f); // unused currently
		vec3 right = vec3(-1.0f, 0.0f, 0.0f);
		Mat4 RotationX() {
			float cam_rot[4][4] = {
				{1, 0, 0, 0},
				{0, cos(Radians(pitch)), -sin(Radians(pitch)), 0},
				{0, sin(Radians(pitch)), cos(Radians(pitch)), 0},
				{0, 0, 0, 1}
			};
			return Mat4(cam_rot);
		}
		Mat4 RotationY() {
			float cam_rot[4][4] = {
				{cos(Radians(yaw)), 0, -sin(Radians(yaw)), 0},
				{0, 1, 0, 0},
				{sin(Radians(yaw)), 0, cos(Radians(yaw)), 0},
				{0, 0, 0, 1}
			};
			return Mat4(cam_rot);
		}
		Mat4 RotationZ() {
			float cam_rot[4][4] = {
				{cos(Radians(roll)), -sin(Radians(roll)), 0, 0},
				{sin(Radians(roll)), cos(Radians(roll)), 0, 0},
				{0, 0, 1, 0},
				{0, 0, 0, 1}
			};
			return Mat4(cam_rot);
		}
		void MoveForward(float distance) {
			vec3 facing = RotationX() * (RotationY() * forward);
			position = position + (facing * speed * distance);
		}
		void MoveRight(float distance) {
			vec3 facing_right = (RotationY() * right);
			position = position + (facing_right * speed * distance);
		}

	private:

	};
} // namespace raytrace
#endif // RAYTRACE_CAMERA_H_
