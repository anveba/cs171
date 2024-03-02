R"(

uniform vec3 u_tangent;

varying vec3 normal, tangent, view_pos;

void main() {
    normal = gl_NormalMatrix * gl_Normal;
    tangent = gl_NormalMatrix * u_tangent;
    view_pos = vec3(-gl_ModelViewMatrix * gl_Vertex);

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

    // Quick and dirty way of getting texture coords. The vertex positions are 
    // assumed to be between 0 and 1 in the x and y directions.
    gl_TexCoord[0] = vec4(gl_Vertex.xy, 0.0, 0.0); 
}

)"