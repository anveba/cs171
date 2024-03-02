R"(

uniform int u_num_lights;

varying vec3 normal, view_pos;

void main() {
    
    vec3 unit_normal = normalize(normal);

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    for (int i = 0; i < u_num_lights; i++) {
        // Get info on light 
        vec3 light_to_point = vec3(gl_LightSource[i].position) + view_pos;
        float light_dist = length(light_to_point);
        vec3 unit_light_dir = light_to_point / light_dist;

        // Calculate diffuse
        vec3 light_diffuse = vec3(gl_LightSource[i].diffuse) / (1.0 + gl_LightSource[i].quadraticAttenuation * light_dist * light_dist);
        diffuse += light_diffuse * max(0.0, dot(unit_light_dir, unit_normal));
        
        // Calculate specular
        vec3 light_specular = vec3(gl_LightSource[i].specular) / (1.0 + gl_LightSource[i].quadraticAttenuation * light_dist * light_dist);
        specular += light_specular * pow(
                max(0.0,
                    dot(unit_normal, normalize(unit_light_dir + normalize(vec3(view_pos))))), 
                    gl_FrontMaterial.shininess
            );
    }

    // Calculate colour
    gl_FragColor = vec4(vec3(gl_FrontMaterial.ambient) + 
        vec3(gl_FrontMaterial.diffuse) * diffuse +
        vec3(gl_FrontMaterial.specular) * specular, 1.0);

    float gamma = 1.0;
    gl_FragColor = vec4(pow(gl_FragColor.rgb, vec3(gamma)), 1.0);
}

)"