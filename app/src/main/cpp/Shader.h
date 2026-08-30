#ifndef ANDROIDGLINVESTIGATIONS_SHADER_H
#define ANDROIDGLINVESTIGATIONS_SHADER_H

#include <string>
#include <GLES3/gl3.h>

class Model;

/*!
 * A class representing a simple shader program. It consists of vertex and fragment components. The
 * input attributes are a position (as a Vector3) and a uv (as a Vector2). It also takes a uniform
 * to be used as the entire model/view/projection matrix. The shader expects a single texture for
 * fragment shading, and does no other lighting calculations (thus no uniforms for lights or normal
 * attributes).
 */
class Shader {
public:
    /*!
     * Loads a shader given the full sourcecode and names for necessary attributes and uniforms to
     * link to. Returns a valid shader on success or null on failure. Shader resources are
     * automatically cleaned up on destruction.
     *
     * @param vertexSource The full source code for your vertex program
     * @param fragmentSource The full source code of your fragment program
     * @param positionAttributeName The name of the position attribute in your vertex program
     * @param uvAttributeName The name of the uv coordinate attribute in your vertex program
     * @param projectionMatrixUniformName The name of your model/view/projection matrix uniform
     * @return a valid Shader on success, otherwise null.
     */
    static Shader *loadShader(
            const std::string &vertexSource,
            const std::string &fragmentSource,
            const std::string &positionAttributeName,
            const std::string &uvAttributeName,
            const std::string &projectionMatrixUniformName,
            const std::string &colorUniformName = "",
            const std::string &useTextureUniformName = "");

    inline ~Shader() {
        if (program_) {
            glDeleteProgram(program_);
            program_ = 0;
        }
    }

    void activate() const;
    void deactivate() const;

    void drawModel(const Model &model) const;
    void setProjectionMatrix(const float *projectionMatrix) const;
    void setColor(float r, float g, float b, float a = 1.0f) const;
    void setUseTexture(bool useTexture) const;

    void drawIndexed(const void *vertices, size_t vertexStride, size_t positionOffset,
                     size_t uvOffset, const uint16_t *indices, size_t indexCount) const;

private:
    static GLuint loadShader(GLenum shaderType, const std::string &shaderSource);

    constexpr Shader(
            GLuint program,
            GLint position,
            GLint uv,
            GLint projectionMatrix,
            GLint colorUniform = -1,
            GLint useTextureUniform = -1)
            : program_(program),
              position_(position),
              uv_(uv),
              projectionMatrix_(projectionMatrix),
              colorUniform_(colorUniform),
              useTextureUniform_(useTextureUniform) {}

    GLuint program_;
    GLint position_;
    GLint uv_;
    GLint projectionMatrix_;
    GLint colorUniform_;
    GLint useTextureUniform_;
};

#endif //ANDROIDGLINVESTIGATIONS_SHADER_H