R"(

varying vec3 normal, view_pos;

void main() {
    view_pos = vec3(-gl_ModelViewMatrix * gl_Vertex);
    normal = gl_NormalMatrix * gl_Normal;
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}



)"