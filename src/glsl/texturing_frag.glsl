R"(

uniform int u_num_lights;
uniform sampler2D u_colour_tex, u_normal_map;

varying vec3 normal, tangent, view_pos;

void main() {

    // Calculate surface space orthonormal basis
    vec3 unit_normal = normalize(normal);
    vec3 unit_tangent = normalize(tangent);
    vec3 unit_bitangent = cross(unit_tangent, unit_normal);

    // Change normals to view space
    mat3 tbn = mat3(unit_tangent, unit_bitangent, unit_normal);
    unit_normal = tbn * normalize(texture2D(u_normal_map, gl_TexCoord[0].st).xyz * 2.0 - 1.0);

    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    // Calculate lighting for each light
    for (int i = 0; i < u_num_lights; i++) {
        vec3 light_to_point = vec3(gl_LightSource[i].position) + view_pos;
        float light_dist = length(light_to_point);
        vec3 unit_light_dir = light_to_point / light_dist;

        vec3 light_diffuse = vec3(gl_LightSource[i].diffuse) / (1.0 + gl_LightSource[i].quadraticAttenuation * light_dist * light_dist);
        diffuse += light_diffuse * max(0.0, dot(unit_light_dir, unit_normal));
        
        vec3 light_specular = vec3(gl_LightSource[i].specular) / (1.0 + gl_LightSource[i].quadraticAttenuation * light_dist * light_dist);
        specular += light_specular * pow(
                max(0.0,
                    dot(unit_normal, normalize(unit_light_dir + normalize(vec3(view_pos))))), 
                    gl_FrontMaterial.shininess
            );
    }

    // Calculate colour
    vec3 map_col = texture2D(u_colour_tex, gl_TexCoord[0].st).rgb;
    gl_FragColor = vec4(
        map_col * vec3(gl_FrontMaterial.ambient) +
        map_col * diffuse * vec3(gl_FrontMaterial.diffuse) +
        map_col * specular * vec3(gl_FrontMaterial.specular), 1.0);

    float gamma = 1.0;
    gl_FragColor = vec4(pow(gl_FragColor.rgb, vec3(gamma)), 1.0);
}



)"