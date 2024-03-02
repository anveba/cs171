#include <optional>
#include <stdexcept>

#include "animation.h"
#include "ioutil.h"
#include "obj_format.h"
#include "parseutil.h"
#include "token_stream.h"

// A struct that is passed around during the parsing process.
struct AnimationParserState
{
    AnimationParserState(size_t frame_count)
        : frame_count(frame_count)
    {
    }

    ~AnimationParserState()
    {
    }

    AnimationParserState(AnimationParserState const&) = delete;
    void operator=(AnimationParserState const&) = delete;

    std::vector<KeyFrame> frames;
    size_t frame_count;
};

// Looks for the first token that is the frame count and
// consumes it
static size_t parse_frame_count(TokenStream& lines)
{
    while (!lines.done()) {
        TokenStream tokens(lines.next(), ' ');
        if (tokens.size() == 0)
            continue;
        else if (tokens.size() != 1)
            throw std::runtime_error("Frame count not found");
        return std::stoull(tokens.next());
    }
    throw std::runtime_error("Frame count not found");
}

// Parses a line and updates the parser's state accordingly
static void parse_line(const std::string& line,
                       AnimationParserState& state)
{
    TokenStream tokens(line, ' ');

    if (tokens.done())
        return;
    const std::string& tok = tokens.next();

    if (tok == "Frame") {
        state.frames.push_back(KeyFrame(std::stoull(tokens.next()), Frame()));
    } else if (tok == "translation") {
        if (state.frames.size() == 0)
            throw std::runtime_error("No frame to translate");
        state.frames[state.frames.size() - 1].get_frame().position = parse_vector(tokens);
    } else if (tok == "rotation") {
        if (state.frames.size() == 0)
            throw std::runtime_error("No frame to rotate");
        Vec3 axis = parse_vector(tokens);
        float angle = std::stof(tokens.next());
        state.frames[state.frames.size() - 1].get_frame().rotation =
            Quaternion::from_rotation(axis, angle / 180.0f * 3.1415f);
    } else if (tok == "scale") {
        if (state.frames.size() == 0)
            throw std::runtime_error("No frame to scale");
        state.frames[state.frames.size() - 1].get_frame().scale = parse_vector(tokens);
    } else {
        throw std::runtime_error("Unrecognised token " + tok);
    }
}

Animation read_animation(const std::string& raw)
{
    std::string chars_to_remove = "\r\t";
    std::string filtered = raw;
    filter_string(filtered, chars_to_remove);

    TokenStream lines(filtered, '\n');

    AnimationParserState state(parse_frame_count(lines));

    while (!lines.done())
        parse_line(lines.next(), state);

    return Animation(state.frame_count, state.frames);
}