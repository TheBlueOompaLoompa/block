#include "shader.hpp"
#include <string>
#include <glm/glm.hpp>
#include "geometry.hpp"

Shader::Shader(const char* path) {
    std::string v_path = std::string(path);
    std::string f_path = std::string(path);

    v_path.append(".v.glsl");
    f_path.append(".f.glsl");

    m_program = m_create_program(v_path.c_str(), f_path.c_str());

    bind_attrib("coord3d");
    bind_attrib("texcoord");
    bind_attrib("normal");
}

void Shader::use() {
    glUseProgram(m_program);
}

void Shader::use(GLuint vbo_vertices, GLuint normal) {
    use();

    glEnableVertexAttribArray(get_attrib("coord3d"));
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glVertexAttribPointer(
        get_attrib("coord3d"),
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        0 // offset of the first element
    );

    glEnableVertexAttribArray(get_attrib("texcoord"));
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glVertexAttribPointer(
        get_attrib("texcoord"), // attribute
        2,                  // number of elements per vertex, here (x,y)
        GL_FLOAT,           // the type of each element
        GL_FALSE,           // take our values as-is
        sizeof(Vertex),                  // no extra data between each position
        (GLvoid*) (3 * sizeof(GLfloat))                   // offset of first element
    );

    glEnableVertexAttribArray(get_attrib("normal"));
    glBindBuffer(GL_ARRAY_BUFFER, normal);
    glVertexAttribPointer(
        get_attrib("normal"),
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (GLvoid*) 0
    );
}

void Shader::destroy() {
    glDeleteProgram(m_program);
}

bool Shader::bind_attrib(const char* attribute_name) {
    m_va_arrays[attribute_name] = glGetAttribLocation(m_program, attribute_name);
    if (m_va_arrays[attribute_name]  == -1) {
        std::cerr << "Could not bind attribute " << attribute_name << std::endl;
        return false;
    }

    return true;
}

GLuint Shader::get_attrib(const char* attribute_name) {
    return m_va_arrays[attribute_name];
}

GLuint Shader::get_program_id() {
    return m_program;
}

void Shader::m_print_log(GLuint object) {
    GLint log_length = 0;
    if (glIsShader(object)) {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    } else if (glIsProgram(object)) {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    } else {
        std::cerr << "printlog: Not a shader or a program" << std::endl;
        return;
    }

    char* log = (char*)malloc(log_length);
    
    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);
    
    std::cerr << log;
    free(log);
}

GLuint Shader::m_create_program(const char* vert_path, const char* frag_path) {
    GLint link_ok = GL_FALSE;
    
    GLuint vs, fs;
    GLuint program;

    if(
        (vs = m_create_shader(vert_path, GL_VERTEX_SHADER)) == 0 || 
        (fs = m_create_shader(frag_path, GL_FRAGMENT_SHADER)) == 0
    ) { program = 0; }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        std::cerr << "Error in glLinkProgram" << std::endl;
        program = 0;
    }

    return program;
}

GLuint Shader::m_create_shader(const char* filename, GLenum type) {
    const GLchar* source = file_read(filename);
    if (source == NULL) {
        std::cerr << "Error opening " << filename << ": " << SDL_GetError() << std::endl;
        return 0;
    }

    auto new_source = m_parse_includes(source, filename);
    free((void*)source);

    GLuint res = glCreateShader(type);

    const char* const new_new_source = new_source.c_str();
    glShaderSource(res, 1, &new_new_source, NULL);
    
    printf("Compiling shader %s\n", filename);
    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        printf("%s\n", new_new_source);
        std::cerr << filename << ":";
        m_print_log(res);
        glDeleteShader(res);
        return 0;
    }
    
    return res;
}

std::string Shader::m_parse_includes(const GLchar* source, const char* filename) {
    std::string sauce = std::string(source);
    auto file_path = std::filesystem::path(filename);

    // Hold current search window
    char include_start_buf[10] = "        ";

    for(int i = 0; i < sauce.length(); i++) {
        for(int x = 0; x < sizeof(include_start_buf) - 1; x++) {
            include_start_buf[x] = include_start_buf[x + 1];
        }

        include_start_buf[sizeof(include_start_buf)-2] = sauce[i];

        if(strcmp(include_start_buf, "\n#include") == 0) {
            int start = i - 7;
            while(sauce[i - 1] != '"') { i++; }
            
            std::string path = "";
            while(sauce[i] != '"') {
                path += sauce[i];
                i++;
            }

            file_path.replace_filename(path);
            
            const GLchar* include_source = file_read(file_path.c_str());
            if (include_source == NULL) {
                std::cerr << "Error opening " << file_path << " included in " << filename << ": " << SDL_GetError() << std::endl;
                return sauce;
            }

            sauce.replace(start, i - start + 1, include_source);
        }
    }

    return sauce;
}