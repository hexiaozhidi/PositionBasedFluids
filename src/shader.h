#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int program;

    Shader(const char *vertexPath, const char *fragmentPath) {
        std::string vertexCode, fragmentCode;
        std::ifstream vShaderFile, fShaderFile;

        // ensure that ifstream can throw exceptions
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        // load shader source code files
        try {
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            vShaderFile.close();
            fShaderFile.close();
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure &e) {
            std::cout << "ERROR: Failed to load shader source code files" << std::endl;
        }
        const char *vShaderCode = vertexCode.c_str();
        const char *fShaderCode = fragmentCode.c_str();

        // compile the vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        checkCompileErrors(vertexShader, "VERTEX");

        // compile the fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        checkCompileErrors(fragmentShader, "FRAGMENT");

        // link the shader program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        checkCompileErrors(program, "PROGRAM");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

    }

    void use() const {
        glUseProgram(program);
    }

    void clear() {
        glDeleteProgram(program);
    }

    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(program, name.c_str()), static_cast<int>(value));
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(program, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(program, name.c_str()), value);
    }

    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }

    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(glGetUniformLocation(program, name.c_str()), x, y);
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }

    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(program, name.c_str()), x, y, z);
    }

    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(program, name.c_str()), 1, &value[0]);
    }

    void setVec4(const std::string &name, float x, float y, float z, float w) const {
        glUniform4f(glGetUniformLocation(program, name.c_str()), x, y, z, w);
    }

    void setMat2(const std::string &name, const glm::mat2 &mat)  {
        glUniformMatrix2fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string &name, const glm::mat3 &mat)  {
        glUniformMatrix3fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void checkCompileErrors(GLuint ID, const std::string &type) {
        GLint success;
        GLchar infoLog[1024];
        if (type == "VERTEX") {
            glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(ID, 1024, NULL, infoLog);
                std::cout << "ERROR£ºFailed to compile the vertex shader." << std::endl;
                std::cout << infoLog << std::endl;
            }
        } else if (type == "FRAGMENT") {
            glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(ID, 1024, NULL, infoLog);
                std::cout << "ERROR£ºFailed to compile the fragment shader." << std::endl;
                std::cout << infoLog << std::endl;
            }
        } else {
            glGetProgramiv(ID, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(ID, 1024, NULL, infoLog);
                std::cout << "ERROR£ºFailed to link the shader program." << std::endl;
                std::cout << infoLog << std::endl;
            }
        }
    }
};
#endif