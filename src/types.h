#pragma once
#ifndef RAYTRACE_TYPES_H_
#define	RAYTRACE_TYPES_H_
#include <cstdint>
#include <iostream>
#include <vector>
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

template <typename T> struct Tuple3 {
	T x;
	T y;
	T z;
	Tuple3() :x(0), y(0), z(0) {  }
	Tuple3(T x_, T y_, T z_) :x(x_), y(y_), z(z_) {};
	Tuple3 operator+(Tuple3 const& obj) {
		Tuple3 res;
		res.x = x + obj.x;
		res.y = y + obj.y;
		res.z = z + obj.z;
		return res;
	}
	Tuple3 operator-() {
		Tuple3 res;
		res.x = x * -1.0f;
		res.y = y * -1.0f;
		res.z = z * -1.0f;
		return res;
	}
	Tuple3 operator-(Tuple3 const& obj) {
		Tuple3 res;
		res.x = x - obj.x;
		res.y = y - obj.y;
		res.z = z - obj.z;
		return res;
	}
	Tuple3 operator*(float const& f) {
		Tuple3 res;
		res.x = x * f;
		res.y = y * f;
		res.z = z * f;
		return res;
	}
	friend Tuple3 operator*(const float& s, Tuple3& t) {
		return t * s;
	}
	
	Tuple3 operator/(float const& f) {
		Tuple3 res;
		res.x = x / f;
		res.y = y / f;
		res.z = z / f;
		return res;
	}
	
	T dot(Tuple3 const& obj){
		return x * obj.x + y * obj.y + z * obj.z;
	}
	Tuple3 Normalized() {
		return *this / Length(*this);
	}
	static float Length(Tuple3 t) {
		return sqrt(t.dot(t));
	}
};
template <typename T> struct Tuple4 {
	T x;
	T y;
	T z;
	T w;
	Tuple4(T x_, T y_, T z_, T w_) :x(x_), y(y_), z(z_), w(w_) {};
	Tuple4(T x_, T y_, T z_) :x(x_), y(y_), z(z_), w(1.0f) {};
	Tuple4(Tuple3<float> v) : x(v.x), y(v.y), z(v.z), w(1.0f) {};
	Tuple4(Tuple3<float> v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {};

};
template <typename T> struct Tuple2 {
	T x;
	T y;
	Tuple2(T x_, T y_) :x(x_), y(y_) {};
};

using vec2 = Tuple2<float>;
using vec3 = Tuple3<float>;
using vec4 = Tuple4<float>;

class Mat4 {
public:
	Mat4(float values[4][4]){
		std::memcpy(values_, values, (u64)4*(u64)4*sizeof(float));
	}
	
	void Print() const{
		std::cout << "=========================================\n";
		for (int i = 0; i < 4; i++) {
			std::cout << '|';
			for (int j = 0; j < 4; j++) {
				std::cout << '\t' << values_[i][j];
				if (j != 3) {
					std::cout << ',';
				}
			}
			std::cout << "\t|\n";
		}
		std::cout << "=========================================\n";

	}
	vec3 operator*(const vec3& v) {
		vec3 res;
		res.x = v.x * values_[0][0] + v.y * values_[0][1] + v.z * values_[0][2];
		res.y = v.x * values_[1][0] + v.y * values_[1][1] + v.z * values_[1][2];
		res.z = v.x * values_[2][0] + v.y * values_[2][1] + v.z * values_[2][2];
		return res;
	}
	float values_[4][4];
};
enum class LightType {
	kAmbient = 0,
	kPoint,
	kDirectional
};
struct Light {
	vec3 position; // Used for point lights
	vec3 direction; // Used for directional lights
	LightType type;
	float intensity;
	Light() = delete;
	const static int LIGHT_SIZE_STD140 =
		sizeof(vec4) // position			- offset - 0
		+ sizeof(vec4) // direction			- offset - 16
		+ sizeof(type) //
		+ sizeof(intensity) // 
		+ 8 // bring offset to 48, getting to base alignment 4N (???????????? maybe not needed)????
		;
	static void WriteUniformBuffer(u8* buffer_start, std::vector<std::shared_ptr<Light>> lights) { // Caller is responsible for buffer size
		int offset = 0;
		// num_entries
		int num_lights = (int)lights.size();
		memcpy(buffer_start, &num_lights, sizeof(int));
		offset += 16; // pad to 16, requirement of struct
		// lights
		for (int i = 0; i < lights.size(); i++) {
			(*lights[i]).std140_serialize(buffer_start + offset + i * Light::LIGHT_SIZE_STD140);
		}
	}
	static Light AmbientLight(float intensity) {
		return Light(LightType::kAmbient, intensity, vec3(0, 0, 0), vec3(0, 0, 0));
	}
	static Light PointLight(float intensity, vec3 position) {
		return Light(LightType::kPoint, intensity, position, vec3(0, 0, 0));
	}
	static Light DirectionalLight(float intensity, vec3 direction) {
		return Light(LightType::kDirectional, intensity, vec3(0, 0, 0), direction);
	}
	void std140_serialize(u8* dst) {
		int offset = 0;

		// Position
		vec4 position4(position);
		memcpy(dst + offset, &position4.x, sizeof(position4.x));
		offset += sizeof(position4.x);
		memcpy(dst + offset, &position4.y, sizeof(position4.y));
		offset += sizeof(position4.y);
		memcpy(dst + offset, &position4.z, sizeof(position4.z));
		offset += sizeof(position4.z);
		memcpy(dst + offset, &position4.w, sizeof(position4.w));
		offset += sizeof(position4.w);

		// Direction
		vec4 direction4(direction);
		memcpy(dst + offset, &direction4.x, sizeof(direction4.x));
		offset += sizeof(direction4.x);
		memcpy(dst + offset, &direction4.y, sizeof(direction4.y));
		offset += sizeof(direction4.y);
		memcpy(dst + offset, &direction4.z, sizeof(direction4.z));
		offset += sizeof(direction4.z);
		memcpy(dst + offset, &direction4.w, sizeof(direction4.w));
		offset += sizeof(direction4.w);

		// Light type
		memcpy(dst + offset, &type, sizeof(type));
		offset += sizeof(type);

		// Intensity
		memcpy(dst + offset, &intensity, sizeof(intensity));
		offset += sizeof(intensity);
	}

private:
	Light(LightType type, float intensity, vec3 position, vec3 direction) :type(type), intensity(intensity), position(position), direction(direction) {}
};
struct Color {
	u8 r;
	u8 g;
	u8 b;
	u8 a;

	Color(u8 red, u8 green, u8 blue, u8 alpha) :r(red), g(green), b(blue), a(alpha) {};
	Color(u8 red, u8 green, u8 blue) :r(red), g(green), b(blue), a(0xFF) {};
	u8 Clamp(int val) {
		return (u8)std::max(0, std::min(val, 255));
	}
	Color operator*(float const& f) {
		u8 nr = Clamp((int)((float)r * f));
		u8 ng = Clamp((int)((float)g * f));
		u8 nb = Clamp((int)((float)b * f));
		u8 na = a;

		return Color((u8)nr, (u8)ng, (u8)nb, (u8)na);
	}
	Color operator+(Color const& c) {

		return Color(r + c.r, g + c.g, b + c.b, a);
	}
	vec4 ToFloat() {
		return vec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f);
	}
	u32 xrgb_pixel = ((u32)a << 24) | ((u32)r << 16) | ((u32)g << 8) | ((u32)b);
};
#endif