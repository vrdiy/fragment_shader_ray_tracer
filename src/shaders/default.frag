#version 330 core
#define FLT_MAX 3.402823466e+38

uniform float u_Time;
uniform mat4 u_Camera_Rotation_Matrix_X;
uniform mat4 u_Camera_Rotation_Matrix_Y;
uniform vec4 u_Camera_Position;


in vec3 posColor;
// ================================================================================
struct Sphere{ // 44 bytes, padded to 48 to be multiple of vec4
	vec4 center; // 16 bytes 
	vec4 color; // 16
	float radius; // 4
	int specular; // 4 
	float reflective; // 4
};
struct Light{
	vec4 position;
	vec4 direction;
	int type;
	float intensity;
};
// ================================================================================
layout (row_major,std140) uniform ubo_Spheres
{
	int num_spheres;
	Sphere spheres_[100]; // starts at offset 16
};
layout (row_major,std140) uniform ubo_Lights
{
	int num_lights;
	Light lights_[100]; // starts at offset 16
};
// ================================================================================
const Sphere NULL_SPHERE = Sphere(vec4(0,0,0,0), vec4(1.0f,0.0f,1.0f,1.0f),0.0f, 500, 0.2f);
const vec3 BACKGROUND_COLOR = vec3(0.0f,0.0f,0.0f);
// ================================================================================
vec3 Vec3FromVec4(vec4 val){
	return vec3(val.x,val.y,val.z);
}
vec3 ReflectRay(vec3 ray_to_reflect, vec3 normal_to_reflect_over) {
	return normal_to_reflect_over * (2.0f * dot(normal_to_reflect_over,ray_to_reflect)) - ray_to_reflect;
}
vec2 IntersectRaySphere(vec3 ray_origin, vec3 direction, Sphere sphere){
	float r = sphere.radius;
	vec3 CO = ray_origin - Vec3FromVec4(sphere.center);

	// Set up quadratic formula
	float a = dot(direction, direction);
	float b = 2 * dot(CO, direction);
	float c = dot(CO,CO) - r * r;

	// The discriminant determines how many solutions to the sphere intersection there are
	// discriminat > 0, two real roots
	// discriminant == 0, one repeated root
	// discrimant < 0, no real roots
	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0) {
		return vec2(FLT_MAX, FLT_MAX);
	}
	// Solve quadratic equation
	float t1 = (-b + sqrt(discriminant)) / (2 * a);
	float t2 = (-b - sqrt(discriminant)) / (2 * a);

	return vec2(t1, t2);
}
// ================================================================================
void ClosestIntersection(vec3 ray_origin, vec3 direction, float t_min, float t_max, out float out_closest_t, out Sphere out_closest_sphere){
	float closest_t = FLT_MAX;
	Sphere closest_sphere = NULL_SPHERE;

	for (int i = 0; i < num_spheres; i++){
		vec2 intersects = IntersectRaySphere(ray_origin, direction, spheres_[i]);

		// Check for closer intersections 
		if (((intersects.x > t_min) && (intersects.x < t_max)) && intersects.x < closest_t) {
			closest_t = intersects.x;
			closest_sphere = spheres_[i];
		}
		if (((intersects.y > t_min) && (intersects.y < t_max)) && intersects.y < closest_t) {
			closest_t = intersects.y;
			closest_sphere = spheres_[i];
		}
	}
	out_closest_sphere = closest_sphere;
	out_closest_t = closest_t;
}
// s is specular
float ComputeLighting(vec3 point, vec3 normal, vec3 vec_to_camera, int s){
	float intensity = 0.0f;
	for (int i = 0; i < num_lights; i++){
		Light light = lights_[i];
		if(light.type == 0){ // Ambient
			intensity += light.intensity;
		}
		else{
			vec3 light_vec;
			if (light.type == 1){ // point
				light_vec = Vec3FromVec4(light.position) - point;
			}
			else{ // directional
				light_vec = Vec3FromVec4(light.direction);
			}

			// Check for shadow
			float closest_t = FLT_MAX;
			Sphere closest_sphere = NULL_SPHERE;
			ClosestIntersection(point,light_vec,0.01f,FLT_MAX,closest_t,closest_sphere);
			if (closest_sphere.center.w != 0){ // Hit another sphere, obscured
				continue;
			}

			// Diffuse
			float n_dot_l = dot(normal,light_vec);
			if (n_dot_l > 0.0f){
				intensity += light.intensity * n_dot_l / (length(normal)*length(light_vec));
			}

			// Specular
			if (s != -1) {
				vec3 reflection = ReflectRay(light_vec, normal);
				float r_dot_v = dot(reflection,vec_to_camera);
				
				// Reflection is facing the camera.
				// angle between reflection and camera is less than 90 degrees
				if (r_dot_v > 0.05f) { 
					intensity += light.intensity * pow(r_dot_v / (length(reflection) * length(vec_to_camera)), s);
				}
			}
		}
	}
	return intensity;

}
vec3 TraceRay(vec3 ray_origin, vec3 direction, float t_min, float t_max, int recursion_depth, out bool hit, out vec3 hit_point, out Sphere hit_sphere){
	float closest_t = FLT_MAX;
	Sphere closest_sphere = NULL_SPHERE;

	ClosestIntersection(ray_origin,direction,t_min,t_max,closest_t,closest_sphere);
	if (closest_sphere.center.w == 0){ // Got NULL_SPHERE, no hits
		hit = false;
		hit_sphere = NULL_SPHERE;
		return BACKGROUND_COLOR;
	}
	hit = true;
	vec3 point = ray_origin + direction * closest_t;
	hit_point = point;
	hit_sphere = closest_sphere;
	vec3 point_normal = point - Vec3FromVec4(closest_sphere.center);
	point_normal = point_normal/length(point_normal);
	vec3 local_color = Vec3FromVec4(closest_sphere.color) * ComputeLighting(point, point_normal, -direction, closest_sphere.specular);
	return local_color;
	
}
vec2 CoordConversion(){
	float x = gl_FragCoord.x;
	float y = gl_FragCoord.y;
	x -= 250.0f;
	y -= 250.0f;
	x/=500.0f;
	y/=500.0f;
	return vec2(x,y);
}
void main()
{
	
	vec3 origin = Vec3FromVec4(u_Camera_Position);
	int recursion_depth = 1;
	float dist_to_canvas = 0.5f;
	vec3 direction = vec3(CoordConversion(),dist_to_canvas);
	direction = Vec3FromVec4(vec4(direction,1.0f) * u_Camera_Rotation_Matrix_X * u_Camera_Rotation_Matrix_Y);
	
	bool hit = false;
	vec3 point_hit;
	Sphere hit_sphere;

	vec3 local_color = TraceRay(origin, direction, 1.0f, FLT_MAX, recursion_depth,hit,point_hit,hit_sphere);
	if (hit == true) // hit sphere
	{ 
		hit = false;
		if(hit_sphere.reflective <= 0.0f){ // this sphere is not reflective
			gl_FragColor = vec4(local_color,1.0f);
		}
		else{ // sphere is reflective, so trace reflection
			vec3 point1_normal = point_hit - Vec3FromVec4(hit_sphere.center);
			point1_normal = point1_normal/length(point1_normal);
			vec3 reflected_ray = ReflectRay(-direction,point1_normal);
			float old_reflective = hit_sphere.reflective;
			vec3 reflected_color = TraceRay(point_hit, reflected_ray, 0.1f, FLT_MAX, recursion_depth,hit,point_hit,hit_sphere);

			// Update color 
			vec3 current_color = (local_color * (1.0f - old_reflective)) + reflected_color * old_reflective;

			if (hit == true){ // reflection got hit, so trace reflection
				hit = false;
				if(hit_sphere.reflective <= 0.0f){ // sphere is not reflective
					gl_FragColor = vec4(current_color,1.0f);
				}
				else
				{
					vec3 point2_normal = point_hit - Vec3FromVec4(hit_sphere.center);
					point2_normal = point2_normal/length(point2_normal);
					vec3 reflected_ray2 = ReflectRay(-reflected_ray, point2_normal);
					float old_reflective2 = hit_sphere.reflective;

					vec3 color3 = TraceRay(point_hit, reflected_ray2, 0.1f, FLT_MAX, recursion_depth,hit,point_hit,hit_sphere);
					vec3 sub_color = (reflected_color * (1.0f - old_reflective2)) + color3 * old_reflective2;
					current_color = (local_color * (1.0f - old_reflective)) + sub_color * old_reflective;
					gl_FragColor = vec4(current_color,1.0f);
				}

			}
			else // reflection yeilded no hits
			{ 
				gl_FragColor = vec4(current_color,1.0f);
			}
		}
	}
	else
	{
		gl_FragColor = vec4(local_color,1.0f);
	}
	
}