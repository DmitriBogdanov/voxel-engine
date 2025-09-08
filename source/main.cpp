#include "ratlib/log/logger.hpp"

#include "SFML/Window.hpp"


#include <cassert>

// ==============
// --- OpenGL ---
// ==============

#define GL_GLEXT_PROTOTYPES // enabled OpenGL extensions
#include "SFML/OpenGL.hpp"

constexpr const char* vertex_shader_source = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
)";
// this here is the simplest "identity" shader that does no transformations
// whatsoever and simply forwards the coordinates
//
// 'layout (location = 0) in vec3' defines how to interpret the input buffer data
// 'layout (location = 0)' => first value is at the beginning of buffer
// 'in vec3'               => our buffer is a set of vec3 triplets
//
// 'gl_Position' is a pre-defined 'vec4' variable

constexpr const char* fragment_shader_source = R"(
    #version 330 core
    out vec4 FragColor;

    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
)";
// fragment shader requires one 'vec4' output variable that defines the color output

using gl_buffer_id    = GLuint;  // unsigned int
using gl_shader_id    = GLuint;  // unsigned int
using gl_program_id   = GLuint;  // unsigned int
using gl_attribute_id = GLuint;  // unsigned int
using gl_size         = GLsizei; // int
using gl_type         = GLenum;  // unsigned int

bool gl_check_compile_errors(gl_shader_id shader) {
    // Retrieve success
    int success{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success) return true;

    // Retrieve error message (if any)
    std::array<char, 512> message{};
    glGetShaderInfoLog(shader, message.size(), nullptr, message.data());
    rl::log::error("GLSL compilation error:\n{0}\n", message.data());

    return false;
}

bool gl_check_link_errors(gl_program_id program) {
    // Retrieve success
    int success{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (success) return true;

    // Retrieve error message (if any)
    std::array<char, 512> message{};
    glGetProgramInfoLog(program, message.size(), nullptr, message.data());
    rl::log::error("GLSL compilation error:\n{0}\n", message.data());

    return false;
}

#define ASSERT_NO_GL_ERRORS ASSERT(!glGetError())

#define ASSERT_NO_GL_COMPILE_ERRORS(shader_) ASSERT(gl_check_compile_errors(shader_))
#define ASSERT_NO_GL_LINK_ERRORS(program_) ASSERT(gl_check_link_errors(program_))

// ============
// --- Main ---
// ============

struct InitResult {
    gl_buffer_id  VAO;
    gl_buffer_id  VBO;
    gl_program_id program;
};

InitResult renderer_init() {
    // --- Build and compile shader program ---
    // -----------------------------------------
    
    // Vertex shader
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    ASSERT_NO_GL_COMPILE_ERRORS(vertex_shader);
    
    // Fragment shader
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    ASSERT_NO_GL_COMPILE_ERRORS(fragment_shader);
    
    // Shader program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    ASSERT_NO_GL_LINK_ERRORS(program);
    
    // Cleanup
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // --- Set up vertices, buffers & attributes ----
    // ----------------------------------------------
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f,  -0.5f, 0.0f, // right
        0.0f,  0.5f,  0.0f  // top
    };

    // Create buffers
    gl_buffer_id VAO; // VAO -> Vertex Array Object
    glGenVertexArrays(1, &VAO); 
    
    gl_buffer_id VBO; // VBO -> Vertex Buffer Object
    glGenBuffers(1, &VBO);
    
    // 1. Bind the VAO first
    glBindVertexArray(VAO);
    // 2. Set and bind the VBO(s)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    // 3. Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    
    // After 'glVertexAttribPointer' VBO can be safely unbound
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // VAO can also be unbound for additional safety
    glBindVertexArray(0);

    return {VAO, VBO, program};
}

void renderer_body(gl_buffer_id VAO, gl_program_id program) {
    // Render background
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Render a triangle
    glUseProgram(program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_cleanup(gl_buffer_id VAO, gl_buffer_id VBO, gl_program_id program) {
    // Cleanup allocated resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);
}

int main() {
    rl::log::info("Hello {0}", 15);

    // Create window
    sf::ContextSettings settings;
    settings.depthBits         = 24;
    settings.antiAliasingLevel = 4;

    sf::Window window(sf::VideoMode({1280, 720}), "Voxel Engine", sf::Style::Default, sf::State::Windowed, settings);

    ASSERT(window.setActive(true));

    // Main loop
    bool running = true;

    auto renderer = renderer_init();

    while (running) {
        // Poll events
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                running = false;
            } else if (const auto* resized = event->getIf<sf::Event::Resized>()) {
                glViewport(0, 0, resized->size.x, resized->size.y);
                ASSERT_NO_GL_ERRORS;
            }
        }

        // --- Render initialization ---
        // -----------------------------

        renderer_body(renderer.VAO, renderer.program);

        // --- End frame ---
        // -----------------
        window.display(); // internally swaps front & back buffers
    }

    renderer_cleanup(renderer.VAO, renderer.VBO, renderer.program);
}