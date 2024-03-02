#include "opengl_renderer.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>

void OpenGlRenderer::init_settings()
{
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void OpenGlRenderer::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset all lights. Assumes a maximum of eight lights - the minimum supported
    // for OpenGL
    for (int i = 0; i < 8; i++)
        glDisable(GL_LIGHT0 + i);
}

static void set_camera(const Camera& camera)
{
    glMatrixMode(GL_PROJECTION);
    Mat4 proj_mat = camera.projection_matrix();
    glLoadMatrixf((float*)&proj_mat);

    glMatrixMode(GL_MODELVIEW);
    Mat4 view_mat = camera.view_matrix();
    glLoadMatrixf((float*)&view_mat);
}

static void set_lights(const Scene& scene)
{
    int light_count = scene.get_point_lights().size();

    // Assumes a maximum of eight lights - the minimum supported
    // for OpenGL
    if (light_count > 8) {
        std::cerr << "Too many lights! Capping to 8..." << std::endl;
        light_count = 8;
    }

    // Let OpenGL know about the lights in the scene
    for (int i = 0; i < light_count; i++) {

        const auto& light = scene.get_point_lights().at(i);
        int id = GL_LIGHT0 + i;
        glEnable(id);

        glLightfv(id, GL_AMBIENT, light.colour().float_ptr());
        glLightfv(id, GL_DIFFUSE, light.colour().float_ptr());
        glLightfv(id, GL_SPECULAR, light.colour().float_ptr());
        glLightf(id, GL_QUADRATIC_ATTENUATION, light.get_attenuation());

        Vec4 light_pos = Vec4(light.position().x(), light.position().y(), light.position().z(), 1.0f);
        glLightfv(id, GL_POSITION, (float*)&light_pos);
    }
}

static void draw_objects(const Scene& scene)
{
    // Copy whatever's on the top of the stack
    glPushMatrix();

    // The global transform should apply to everything in the scene, so we put it on
    // the stack a little earlier
    Mat4 scene_transform_mat = scene.global_transform().matrix();
    glMultMatrixf((float*)&scene_transform_mat);

    for (auto instance : scene.get_instances()) {
        // Copy the top element again
        glPushMatrix();

        // Multiply by the tranform of the object being drawn
        Mat4 model_mat = instance.transform.matrix();
        glMultMatrixf((float*)&model_mat);

        // Set material
        glMaterialfv(GL_FRONT, GL_AMBIENT, instance.material.ambient().float_ptr());
        glMaterialfv(GL_FRONT, GL_DIFFUSE, instance.material.diffuse().float_ptr());
        glMaterialfv(GL_FRONT, GL_SPECULAR, instance.material.specular().float_ptr());
        glMaterialf(GL_FRONT, GL_SHININESS, instance.material.shininess());

        const auto& mesh = scene.get_meshes().at(instance.mesh_index);

        // Draw!
        glVertexPointer(3, GL_FLOAT, 0, mesh.get_positions().data());
        glNormalPointer(GL_FLOAT, 0, mesh.get_normals().data());
        glDrawArrays(GL_TRIANGLES, 0, mesh.get_positions().size());

        // Remember to pop
        glPopMatrix();
    }

    // Remember to pop
    glPopMatrix();
}

static void prepare_shader(const Scene& scene, const ShaderProgram& shader)
{
    shader.use();

    shader.set_uniform("u_num_lights", scene.get_point_lights().size());
}

void OpenGlRenderer::render(const Scene& scene, const ShaderProgram& shader)
{
    prepare_shader(scene, shader);
    set_camera(scene.camera());
    set_lights(scene);
    draw_objects(scene);
    ShaderProgram::unuse();
}