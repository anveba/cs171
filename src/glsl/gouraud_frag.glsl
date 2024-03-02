R"(

varying vec3 colour;

void main() {
    float gamma = 1.0;
    gl_FragColor = vec4(pow(colour, vec3(gamma)), 1.0);
}

)"