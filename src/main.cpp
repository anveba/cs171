#include <GL/glut.h>

#include <iostream>

#include "animator.h"
#include "ibar.h"
#include "image.h"
#include "io/animation_format.h"
#include "io/ioutil.h"
#include "io/obj_format.h"
#include "io/scene_format.h"
#include "opengl_renderer.h"
#include "phong_shader.h"
#include "quaternion.h"
#include "shader_program.h"
#include "texture2d.h"
#include "triangle_renderer.h"
#include "wireframe_renderer.h"

enum class SoftwareRenderMode
{
    Wireframe,
    Gouraud,
    Phong
};

enum class HardwareRenderMode
{
    Gouraud,
    Phong
};

static ShaderProgram shader;
static OpenGlRenderer gl_renderer;

static Scene current_scene;

static Quaternion current_arcball_rotation;

struct MouseState
{
    float ndc_x, ndc_y;
    bool left_down;
};

static MouseState last_mouse_state;
static MouseState current_mouse_state;

static int viewport_width, viewport_height;

static void check_for_opengl_errors()
{
    GLenum code;
    while (true) {
        const GLubyte* string;
        code = glGetError();
        if (code == GL_NO_ERROR)
            break;
        string = gluErrorString(code);
        std::cerr << "OpenGL error: " << string << std::endl;
    }
}

static void draw()
{
    gl_renderer.clear();

    gl_renderer.render(current_scene, shader);

    check_for_opengl_errors();

    glutSwapBuffers();
}

static void reshape(int width, int height)
{
    viewport_width = (width == 0) ? 1 : width;
    viewport_height = (height == 0) ? 1 : height;

    glViewport(0, 0, viewport_width, viewport_height);

    glutPostRedisplay();
}

// Hardcoded scene with a quad in it used for the texturing demo
static Scene quad_scene()
{
    std::vector<Vec3> positions = {
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f),
        Vec3(1.0f, 0.0f, 0.0f),
        Vec3(1.0f, 1.0f, 0.0f),
    };

    std::vector<Vec3> normals = {
        Vec3(0.0f, 0.0f, 1.0f),
        Vec3(0.0f, 0.0f, 1.0f),
        Vec3(0.0f, 0.0f, 1.0f),
        Vec3(0.0f, 0.0f, 1.0f),
    };
    std::vector<IndexedTriangle> tris = { { 0, 2, 1 }, { 2, 3, 1 } };

    std::vector<MeshBuffers> bufs = { MeshBuffers("quad", Mesh(positions, normals, tris)) };
    std::vector<Instance> instances = { Instance(0,
                                                 translation(Vec3(-0.5f, -0.5f, 0.0f)),
                                                 PhongMaterial(Colour(0.15f), Colour(0.7f), Colour(0.2f), 5.0f)) };
    std::vector<PointLight> lights = { PointLight(Vec3(-3.0f, 4.0f, 4.0f), Colour(1.0f), 0.0f) };
    Camera cam(translation(Vec3(0.0f, 0.0f, 4.0f)),
               rotation(Vec3(0.0f, 1.0f, 0.0f), 0.0f),
               projection(3.0f, 10.0f, -0.5f, 0.5f, 0.5f, -0.5f));
    return Scene(bufs, instances, lights, cam);
}

static Vec3 point_on_arcball(float ndc_x, float ndc_y)
{
    float norm_sq = ndc_x * ndc_x + ndc_y * ndc_y;
    float z;
    if (norm_sq <= 1.0f)
        z = sqrtf(1 - norm_sq);
    else
        z = 0;
    return Vec3(ndc_x, ndc_y, z);
}

static void do_arcball_rotation()
{
    // Get points on arcball
    Vec3 p_last = point_on_arcball(last_mouse_state.ndc_x, last_mouse_state.ndc_y);
    Vec3 p_current = point_on_arcball(current_mouse_state.ndc_x, current_mouse_state.ndc_y);

    // Calculate rotation
    float theta = acosf(std::min(1.0f, p_last.dot(p_current) / (p_last.norm() * p_current.norm())));
    Vec3 axis = p_last.cross(p_current);
    if (axis.squaredNorm() < 1e-6)
        return;

    // Perform quaternion magic and update rotation
    auto rotation = Quaternion::from_rotation(axis, theta);
    current_arcball_rotation = rotation * current_arcball_rotation;

    current_scene.global_transform().matrix() = current_arcball_rotation.to_rotation_matrix();
}

static void mouse_pressed(int button, int state, int x, int y)
{
    // Update the mouse state
    last_mouse_state = current_mouse_state;

    current_mouse_state.ndc_x = (float(x) / viewport_width) * 2.0f - 1.0f;
    current_mouse_state.ndc_y = (1.0f - float(y) / viewport_height) * 2.0f - 1.0f;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        current_mouse_state.left_down = true;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        current_mouse_state.left_down = false;
    }
}

static void mouse_moved(int x, int y)
{
    // Update mouse state and do arcball rotation if applicable
    last_mouse_state = current_mouse_state;

    current_mouse_state.ndc_x = (float(x) / viewport_width) * 2.0f - 1.0f;
    current_mouse_state.ndc_y = (1.0f - float(y) / viewport_height) * 2.0f - 1.0f;

    if (current_mouse_state.left_down) {
        do_arcball_rotation();
        glutPostRedisplay();
    }
}

static void key_pressed(unsigned char key, int x, int y)
{
    if (key == 'q') {
        exit(0);
    } else if (key >= '0' && key <= '9') {

        float smooth_amount = 0.001f * (1 << (key - '0'));
        std::cout << "Smoothing by " << smooth_amount << "..." << std::endl;

        for (auto& scene_mesh : current_scene.get_meshes()) {
            scene_mesh.get_mesh().implicit_fairing(smooth_amount);
            scene_mesh.update_buffers();
        }
        std::cout << "Done." << std::endl;
        glutPostRedisplay();
    }
}

static void begin_loop()
{
    glutMainLoop();
}

static void init_window(int w_win, int h_win, const std::string& name)
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(w_win, h_win);

    int screen_width = glutGet(GLUT_SCREEN_WIDTH);
    int screen_height = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition((screen_width - w_win) / 2.0f, (screen_height - h_win) / 2.0f);

    glutCreateWindow(name.c_str());
}

static void init_glut(int argc, char** argv)
{
    glutInit(&argc, argv);
}

static void set_callbacks()
{
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);

    last_mouse_state.ndc_x = last_mouse_state.ndc_y = 0.0f;
    current_mouse_state.ndc_x = current_mouse_state.ndc_y = 0.0f;
    last_mouse_state.left_down = false;
    current_mouse_state.left_down = false;
    glutMouseFunc(mouse_pressed);
    glutMotionFunc(mouse_moved);
    glutPassiveMotionFunc(mouse_moved);
    glutKeyboardFunc(key_pressed);
}

bool is_uinteger(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(),
                                      [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

static void start_opengl_renderer(const std::string& scene_path, HardwareRenderMode mode)
{
    current_scene = read_scene(str_from_file(scene_path), directory_of(scene_path));

    int width = 800;
    int height = 800;

    init_window(width, height, "OpenGL Window");

    set_callbacks();

    gl_renderer.init_settings();

    // Compile and link shader
    if (mode == HardwareRenderMode::Gouraud) {
        shader = ShaderProgram::link(
            VertexShader::from_source(
#include "glsl/gouraud_vert.glsl"
                ),
            FragmentShader::from_source(
#include "glsl/gouraud_frag.glsl"
                ));
    } else {
        shader = ShaderProgram::link(
            VertexShader::from_source(
#include "glsl/phong_vert.glsl"
                ),
            FragmentShader::from_source(
#include "glsl/phong_frag.glsl"
                ));
    }

    begin_loop();
}

static void parse_opengl_renderer(int argc, char** argv)
{
    if (argc == 4) {

        std::string scene_path(argv[2]);
        std::string mode_str(argv[3]);

        HardwareRenderMode mode;
        if (mode_str == "gouraud") {
            mode = HardwareRenderMode::Gouraud;
        } else if (mode_str == "phong") {
            mode = HardwareRenderMode::Phong;
        } else {
            std::cout << "Mode was not gouraud, phong, or wireframe." << std::endl;
        }

        init_glut(argc, argv);

        start_opengl_renderer(scene_path, mode);

    } else {
        std::cout << "Invalid argument count. Usage is:\n"
                  << "opengl SCENE_PATH gouraud|phong" << std::endl;
    }
}

void start_software_renderer(const std::string& scene_path, int width, int height, SoftwareRenderMode mode)
{
    Scene scene = read_scene(str_from_file(scene_path), directory_of(scene_path));

    Image image(width, height);

    if (mode == SoftwareRenderMode::Wireframe) {
        WireframeRenderer renderer;
        renderer.render(image, scene);
    } else {
        TriangleRenderer renderer;
        PhongShader shader(mode == SoftwareRenderMode::Phong);
        renderer.render(&shader, image, scene);
    }

    std::cout << image.to_ppm() << std::endl;
}

static void parse_software_renderer(int argc, char** argv)
{
    if (argc == 6) {
        std::string scene_path(argv[2]);
        std::string width_str(argv[3]);
        int width, height;
        if (is_uinteger(width_str) && (width = std::stoi(width_str)) > 0) {
            std::string height_str(argv[4]);
            if (is_uinteger(height_str) && (height = std::stoi(height_str)) > 0) {
                int height = std::stoi(height_str);
                std::string mode_str(argv[5]);
                SoftwareRenderMode mode;
                if (mode_str == "gouraud") {
                    mode = SoftwareRenderMode::Gouraud;
                } else if (mode_str == "phong") {
                    mode = SoftwareRenderMode::Phong;
                } else if (mode_str == "wireframe") {
                    mode = SoftwareRenderMode::Wireframe;
                } else {
                    std::cout << "Mode was not gouraud, phong, or wireframe." << std::endl;
                }
                try {
                    start_software_renderer(scene_path, width, height, mode);
                } catch (const std::exception& e) {
                    std::cerr << e.what() << '\n';
                }
            } else {
                std::cout << "Height was not a positive integer." << std::endl;
            }
        } else {
            std::cout << "Width was not a positive integer." << std::endl;
        }
    } else {
        std::cout << "Invalid argument count. Usage is:\n"
                  << "software SCENE_PATH WIDTH HEIGHT gouraud|phong|wireframe" << std::endl;
    }
}

static void start_texturing_demo(const std::string& diffuse_path, const std::string& normal_path)
{
    current_scene = quad_scene();

    int width = 800;
    int height = 800;

    init_window(width, height, "OpenGL Window");

    set_callbacks();

    gl_renderer.init_settings();

    // Load textures
    Texture2D colour_tex = Texture2D::from_file(diffuse_path);
    Texture2D normal_map = Texture2D::from_file(normal_path);

    // Compile and link shader
    shader = ShaderProgram::link(
        VertexShader::from_source(
#include "glsl/texturing_vert.glsl"
            ),
        FragmentShader::from_source(
#include "glsl/texturing_frag.glsl"
            ));

    // Set uniforms
    shader.use();

    Texture2D::set_active(0);
    Texture2D::bind(colour_tex);
    shader.set_uniform("u_colour_tex", 0);

    Texture2D::set_active(1);
    Texture2D::bind(normal_map);
    shader.set_uniform("u_normal_map", 1);

    shader.set_uniform("u_tangent", Vec3(-1.0f, 0.0f, 0.0f));

    begin_loop();
}

static void parse_texturing_demo(int argc, char** argv)
{
    if (argc == 4) {
        init_glut(argc, argv);

        std::string diffuse_path(argv[2]);
        std::string normal_path(argv[3]);

        start_texturing_demo(diffuse_path, normal_path);
    } else {
        std::cout << "Invalid argument count. Usage is:\n"
                  << "texture DIFFUSE_MAP_PATH NORMAL_MAP_PATH" << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Not enough arguments. Run with argument 'help'"
                  << " for more information." << std::endl;
    } else {
        std::string arg(argv[1]);
        if (arg == "opengl") {
            parse_opengl_renderer(argc, argv);
        } else if (arg == "software") {
            parse_software_renderer(argc, argv);
        } else if (arg == "texture") {
            parse_texturing_demo(argc, argv);
        } else if (arg == "help") {
            std::cout << "Usage:\n"
                      << "opengl SCENE_PATH\n"
                      << "  * Renders an interactive scene using OpenGL. The number keys may\n"
                      << "    be pressed to smooth the meshes in the scene.\n"
                      << "software SCENE_PATH WIDTH HEIGHT gouraud|phong|wireframe\n"
                      << "  * Renders the scene using the CPU (ppm format to stdout).\n"
                      << "texture DIFFUSE_MAP_PATH NORMAL_MAP_PATH\n"
                      << "  * Starts an interactive demo scene of normal mapping." << std::endl;
        }
    }
}
