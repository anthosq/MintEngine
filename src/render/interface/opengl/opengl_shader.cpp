#include "opengl_shader.h"
#include <string>
#include <vector>
#include "log_system.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

#include "render/render_system.h"
#include "render/interface/opengl/opengl_utils.h"

namespace Mint {
    OpenGLShader::OpenGLShader(const std::filesystem::path& filepath)
        : m_asset_path(filepath.string()) {
        m_name = filepath.filename().string();
        ReadShaderFromFile(filepath);
        RenderSystem::Submit([this]() {
            CompileAndUploadShader();
            Reflect();
        });
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

            for (auto id : shader_renderer_ids) {
                glDeleteShader(id);
            }

            LOG_ERROR("Shader link failure!");
            LOG_ERROR(fmt::format("{0}", infoLog.data()));
            // later use assert here
            return;
        }

        // Always detach shaders after a successful link.
        for (auto id : shader_renderer_ids) {
            glDetachShader(program, id);
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

    void OpenGLShader::Reflect() {
        if (m_renderer_id ==0) return;

        m_uniforms.clear();

        unsigned int numUniforms = 0;
        glGetProgramiv(m_renderer_id, GL_ACTIVE_UNIFORMS, (GLint*)&numUniforms);

        // unsured
        unsigned int maxNameLength = 0;
        glGetProgramiv(m_renderer_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, (GLint*)&maxNameLength);
        
        std::vector<char> nameBuffer(maxNameLength);

        uint32_t offset = 0;

        LOG_INFO(fmt::format("Shader '{0}' Reflection:", m_name));
        for (unsigned int i = 0; i < numUniforms; i++) {
            GLsizei nameLength = 0;
            GLint size = 0;
            GLenum type = 0;

            glGetActiveUniform(m_renderer_id, i, maxNameLength, &nameLength, &size, &type, nameBuffer.data());
            std::string uniformName(nameBuffer.data(), nameLength);

            ShaderUniformType uniformType = Utils::GLTypeToShaderUniformType(type);
            uint32_t uniformSize = Utils::GetShaderUniformSize(uniformType);

            m_uniforms[uniformName] = ShaderUniform(uniformName, uniformType, uniformSize, offset);

            offset += uniformSize;

            LOG_INFO(fmt::format("  Name: {0}, Type: {1}, Size: {2}, Offset: {3}", 
                uniformName, ShaderUniform::UniformTypeToString(uniformType), uniformSize, offset));
        }
    }

    OpenGLShader::~OpenGLShader() {
        RenderSystem::Submit([this]() {
            glDeleteProgram(m_renderer_id);
        });
    }

    void OpenGLShader::Bind() const {
        RenderSystem::Submit([this]() {
            glUseProgram(m_renderer_id);
        });
    }

    void OpenGLShader::Unbind() const {
        RenderSystem::Submit([]() {
            glUseProgram(0);
        });
    }

    // wrapper functions
    void OpenGLShader::SetInt(const std::string& name, int value) {
        RenderSystem::Submit([=]() {
            UploadUniformInt(name, value);
        });
    }

    void OpenGLShader::SetFloat(const std::string& name, float value) {
        RenderSystem::Submit([=]() {
            UploadUniformFloat(name, value);
        });
    }

    void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& vector) {
        RenderSystem::Submit([=]() {
            UploadUniformFloat2(name, vector);
        });
    }

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& vector) {
        RenderSystem::Submit([=]() {
            UploadUniformFloat3(name, vector);
        });
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& vector) {
        RenderSystem::Submit([=]() {
            UploadUniformFloat4(name, vector);
        });
    }
    void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix) {
        RenderSystem::Submit([=]() {
            UploadUniformMat3(name, matrix);
        });
    }
    
    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix) {
        RenderSystem::Submit([=]() {
            UploadUniformMat4(name, matrix);
        });
    }


    // 后续整合在一起, 在ResolveAndSetUniform中调用
    void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
        GLint Location = glGetUniformLocation(m_renderer_id, name.c_str());
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