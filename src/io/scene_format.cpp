#include <iostream>
#include <optional>
#include <stdexcept>

#include "colour.h"
#include "ioutil.h"
#include "light.h"
#include "obj_format.h"
#include "parseutil.h"
#include "scene_format.h"
#include "token_stream.h"

// A state that is passed around when parsing.
struct SceneParserState
{
    SceneParserState(const std::string data_dir)
        : data_dir(data_dir)
    {
        current_object = std::nullopt;
        current_transform = Mat4::Identity();
        meshes = std::vector<MeshBuffers>();
        mesh_indices = std::unordered_map<std::string, size_t>();
        instances = std::vector<Instance>();
        lights = std::vector<PointLight>();
        camera = std::nullopt;
    }

    SceneParserState(SceneParserState const&) = delete;
    void operator=(SceneParserState const&) = delete;

    std::string data_dir;

    std::optional<std::string> current_object;

    Mat4 current_transform;
    PhongMaterial current_material;

    std::vector<MeshBuffers> meshes;
    std::unordered_map<std::string, size_t> mesh_indices;

    std::vector<Instance> instances;

    std::vector<PointLight> lights;

    std::optional<Camera> camera;
};

static void parse_light(TokenStream& tokens,
                        SceneParserState& state)
{
    if (tokens.size() - tokens.current_index() != 9)
        throw std::runtime_error("Invalid light syntax");

    Vec3 pos = parse_vector(tokens);

    if (tokens.next() != ",")
        throw std::runtime_error("Invalid light syntax");

    Colour col = parse_colour(tokens);

    if (tokens.next() != ",")
        throw std::runtime_error("Invalid light syntax");

    float attenuation = std::stof(tokens.next());

    state.lights.push_back(PointLight(pos, col, attenuation));
}

// Assumes identifier is consumed in stream.
static void parse_identifier(const std::string& identifier,
                             TokenStream& tokens,
                             SceneParserState& state)
{
    // Identifier alone on a line means a new instance is being created.
    if (tokens.done()) {
        if (!state.mesh_indices.count(identifier))
            throw std::runtime_error("Object " + identifier + " not recognised");

        // If this is not the first instance in the file, we
        // finialise the previous instance
        if (state.current_object.has_value()) {
            Instance instance(state.mesh_indices[state.current_object.value()],
                              state.current_transform,
                              state.current_material);
            state.instances.push_back(instance);
        }

        // Reset state for new instance
        state.current_object = identifier;
        state.current_transform = Mat4::Identity();
        state.current_material = PhongMaterial();

    } else {
        // Load new mesh from obj with the identifier as the name.
        MeshBuffers m(identifier, read_obj(str_from_file(state.data_dir + tokens.next())));
        state.mesh_indices[identifier] = state.meshes.size();
        state.meshes.push_back(m);
    }
}

static bool is_section_label(const std::string& token)
{
    return token[token.size() - 1] == ':';
}

// Parses the camera section
static void parse_camera(TokenStream& lines, SceneParserState& state)
{
    // Default camera values
    Mat4 position = translation(Vec3::Zero());
    Mat4 orientation = rotation(Vec3(0.0f, 0.0f, -1.0f), 0.0f);
    float near = 1.0f, far = 100.0f;
    float left = -1.0f, right = 1.0f, top = 1.0f, bottom = -1.0f;

    while (!lines.done()) {
        TokenStream tokens(lines.next(), ' ');

        if (tokens.done())
            continue;
        const std::string& tok = tokens.next();

        if (tok == "position") {
            position = parse_translation(tokens);
        } else if (tok == "orientation") {
            orientation = parse_rotation(tokens);
        } else if (tok == "near") {
            near = std::stof(tokens.next());
        } else if (tok == "far") {
            far = std::stof(tokens.next());
        } else if (tok == "left") {
            left = std::stof(tokens.next());
        } else if (tok == "right") {
            right = std::stof(tokens.next());
        } else if (tok == "top") {
            top = std::stof(tokens.next());
        } else if (tok == "bottom") {
            bottom = std::stof(tokens.next());
        } else if (tok == "light") {
            parse_light(tokens, state);
        } else if (is_section_label(tok)) {
            // New section has started, so we exit.
            lines.rollback(1);
            break;
        } else {
            throw std::runtime_error("Unrecognised token in camera section " + tok);
        }
    }

    state.camera = Camera(
        position,
        orientation,
        projection(near, far, left, right, top, bottom));
}

// Parses the objects section
static void parse_objects(TokenStream& lines, SceneParserState& state)
{
    while (!lines.done()) {
        TokenStream tokens(lines.next(), ' ');

        if (tokens.done())
            continue;
        const std::string& tok = tokens.next();

        if (tok == "t") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to translate");
            state.current_transform = parse_translation(tokens) * state.current_transform;
        } else if (tok == "r") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to rotate");
            state.current_transform = parse_rotation(tokens) * state.current_transform;
        } else if (tok == "s") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to scale");
            state.current_transform = parse_scaling(tokens) * state.current_transform;
        } else if (tok == "ambient") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to apply ambient parameters to");
            state.current_material.ambient() = parse_colour(tokens);
        } else if (tok == "diffuse") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to apply diffuse parameters to");
            state.current_material.diffuse() = parse_colour(tokens);
        } else if (tok == "specular") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to apply specular parameters to");
            state.current_material.specular() = parse_colour(tokens);
        } else if (tok == "shininess") {
            if (!state.current_object.has_value())
                throw std::runtime_error("No instance to apply shininess parameter to");
            state.current_material.shininess() = std::stof(tokens.next());
        } else if (tok == "light") {
            parse_light(tokens, state);
        } else if (is_section_label(tok)) {
            // New section has started, so we exit.
            lines.rollback(1);
            break;
        } else {
            parse_identifier(tok, tokens, state);
        }
    }

    // Since we only add an instance to the scene when it is made no longer
    // relevant by another instance, the last instance won't be added.
    // The following code corrects for that.
    if (state.current_object.has_value()) {
        Instance instance(state.mesh_indices[state.current_object.value()],
                          state.current_transform,
                          state.current_material);
        state.instances.push_back(instance);
    }
}

// Parses on the section level (camera/objects)
static void parse_sections(TokenStream& lines, SceneParserState& state)
{
    while (!lines.done()) {
        TokenStream tokens(lines.next(), ' ');
        if (tokens.done())
            continue;
        const std::string& tok = tokens.next();
        if (tok == "camera:") {
            parse_camera(lines, state);
        } else if (tok == "objects:") {
            parse_objects(lines, state);
        } else {
            throw std::runtime_error("Unrecognised token " + tok);
        }
    }
}

Scene read_scene(const std::string& raw, const std::string& data_dir)
{
    std::string chars_to_remove = "\r\t";
    std::string filtered = raw;
    filter_string(filtered, chars_to_remove);

    TokenStream lines(filtered, '\n');

    SceneParserState state(directory_of(data_dir));

    parse_sections(lines, state);

    if (!state.camera.has_value())
        throw std::runtime_error("No camera section in scene description");
    return Scene(state.meshes, state.instances, state.lights, state.camera.value());
}