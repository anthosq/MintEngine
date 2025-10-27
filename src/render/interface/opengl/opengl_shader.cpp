#include "opengl_shader.h"
#include <string>
#include <vector>
#include "log_system.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>


namespace Mint {
    OpenGLShader::OpenGLShader(const std::filesystem::path& filepath)
        : m_asset_path(filepath.string()) {
        m_name = filepath.filename().string();
        ReadShaderFromFile(filepath);
        CompileAndUploadShader();
    }

    OpenGLShader::OpenGLShader(const std::string& vertex_src, const std::string& fragment_src) {
        // Create an empty vertex shader handle
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

        // Send the vertex shader source code to GL
        // Note that std::string's .c_str is NULL character terminated.
        const GLchar* source = vertex_src.c_str();
        glShaderSource(vertexShader, 1, &source, 0);

        // Compile the vertex shader
        glCompileShader(vertexShader);

        GLint isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            // We don't need the shader anymore.
            glDeleteShader(vertexShader);

            // Use the infoLog as you see fit.
            LOG_ERROR("Vertex shader compilation failure!");
            LOG_ERROR(fmt::format("{0}", infoLog.data()));
        }

        // Create an empty fragment shader handle
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Send the fragment shader source code to GL
        // Note that std::string's .c_str is NULL character terminated.
        source = fragment_src.c_str();
        glShaderSource(fragmentShader, 1, &source, 0);

        // Compile the fragment shader
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            // We don't need the shader anymore.
            glDeleteShader(fragmentShader);
            // Either of them. Don't leak shaders.
            glDeleteShader(vertexShader);

            // Use the infoLog as you see fit.

            // In this simple program, we'll just leave
            LOG_ERROR("Fragment shader compilation failure!");
            LOG_ERROR(fmt::format("{0}", infoLog.data()));
            return;
        }

        // Vertex and fragment shaders are successfully compiled.
        // Now time to link them together into a program.
        // Get a program object.
        GLuint program = glCreateProgram();
        m_renderer_id = program;

        // Attach our shaders to our program
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        // Link our program
        glLinkProgram(program);

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(program);
            // Don't leak shaders either.
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            // Use the infoLog as you see fit.

            LOG_ERROR("Shader link failure!");
            LOG_ERROR(fmt::format("{0}", infoLog.data()));
            return;
        }

        // Always detach shaders after a successful link.
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
    }

    void OpenGLShader::ReadShaderFromFile(const std::filesystem::path& filepath) {
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            // use std:filesystem::file_size(filepath) ?
            m_shader_resource.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&m_shader_resource[0], m_shader_resource.size());
            in.close();
        }
        else
        {
            LOG_ERROR(fmt::format("Could not read shader file {0}", filepath.string()));
        }
    }

    GLenum OpenGLShader::ShaderTypeFromString(const std::string& type) {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment")
            return GL_FRAGMENT_SHADER;

        LOG_ERROR(fmt::format("Unknown shader type specified: {0}", type));
        return 0;
    }

    void OpenGLShader::CompileAndUploadShader() {
        std::unordered_map<GLenum, std::string> shader_sources;

        // extract shader sources
        const char* type_token = "#type";
        size_t type_token_length = strlen(type_token);
        size_t pos = m_shader_resource.find(type_token, 0);
        while (pos != std::string::npos) {
            size_t eol = m_shader_resource.find_first_of("\r\n", pos);
            size_t begin = pos + type_token_length + 1;
            std::string type = m_shader_resource.substr(begin, eol - begin);

            size_t next_line_pos = m_shader_resource.find_first_not_of("\r\n", eol);
            pos = m_shader_resource.find(type_token, next_line_pos);
            shader_sources[ShaderTypeFromString(type)] = m_shader_resource.substr(next_line_pos, pos - (next_line_pos == std::string::npos ? m_shader_resource.size() - 1 : next_line_pos));
        }

        std::vector<GLuint> shader_renderer_ids;

        // Upload and compile each shader source
        GLuint program = glCreateProgram();
        for (auto &[type, source] : shader_sources) {
            GLenum shader_type = type;
            GLuint shader_id = glCreateShader(shader_type);
            const GLchar* source_cstr = source.c_str();
            glShaderSource(shader_id, 1, &source_cstr, nullptr);
            glCompileShader(shader_id);

            // Check for compilation errors
            GLint isCompiled = 0;
            glGetShaderiv(shader_id, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader_id, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader_id);

                LOG_ERROR("Shader compilation failure!");
                LOG_ERROR(fmt::format("{0}", infoLog.data()));
                // later use assert here
                return;
            }

            // Attach the shader to the program
            glAttachShader(program, shader_id);
            shader_renderer_ids.push_back(shader_id);
        }

        glLinkProgram(program);

        // Check for linking errors
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            for (auto shader : shader_renderer_ids) {
                glDeleteShader(shader);
            }

            LOG_ERROR("Shader link failure!");
            LOG_ERROR(fmt::format("{0}", infoLog.data()));
            // later use assert here
            return;
        }

        // Always detach shaders after a successful link.
        for (auto shader : shader_renderer_ids) {
            glDetachShader(program, shader);
        }

        m_renderer_id = program;

        // Bind default texture unit
        // UploadUniformInt("u_AlbedoTexture", 1);
        // UploadUniformInt("u_NormalTexture", 2);
        // UploadUniformInt("u_MetalnessTexture", 3);
        // UploadUniformInt("u_RoughnessTexture", 4);

        // UploadUniformInt("u_EnvRadianceTex", 10);
        // UploadUniformInt("u_EnvIrradianceTex", 11);

        // UploadUniformInt("u_BRDFLUTTexture", 15);
    }

    OpenGLShader::~OpenGLShader() {
        glDeleteProgram(m_renderer_id);
    }

    void OpenGLShader::Bind() const {
        glUseProgram(m_renderer_id);
    }

    void OpenGLShader::Unbind() const {
        glUseProgram(0);
    }

    // wrapper functions
    void OpenGLShader::SetInt(const std::string& name, int value) {
        UploadUniformInt(name, value);
    }

    void OpenGLShader::SetFloat(const std::string& name, float value) {
        UploadUniformFloat(name, value);
    }

    void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& vector) {
        UploadUniformFloat2(name, vector);
    }

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& vector) {
        UploadUniformFloat3(name, vector);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& vector) {
        UploadUniformFloat4(name, vector);
    }
    void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix) {
        UploadUniformMat3(name, matrix);
    }
    
    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix) {
        UploadUniformMat4(name, matrix);
    }


    void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
        GLint Location = glGetUniformLocation(m_renderer_id,  name.c_str());
        glUniform1i(Location, value);
    }

    void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniform1f(Location, value);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& vector) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniform2fv(Location, 1, glm::value_ptr(vector));
    }

    void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& vector) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniform3fv(Location, 1, glm::value_ptr(vector));
    }

    void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& vector) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniform4fv(Location, 1, glm::value_ptr(vector));
    }


    void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniformMatrix3fv(Location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
        glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
}