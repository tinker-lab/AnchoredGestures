#version 400

in layout (location = 0) vec3 vertex_position;
in layout (location = 1) vec3 vertex_normal;
uniform mat4 projection_mat, view_mat, model_mat;
uniform vec3 eye_world;
out vec3 position_world, normal_world;

void main () {
	position_world = vec3 (model_mat * vec4 (vertex_position, 1.0));
	normal_world = normalize(vec3 ( model_mat * vec4 (vertex_normal, 0.0)));
	gl_Position = projection_mat * view_mat * vec4 (position_world, 1.0);
}