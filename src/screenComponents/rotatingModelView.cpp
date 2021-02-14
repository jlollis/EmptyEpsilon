#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include <glm/ext/matrix_transform.hpp>

#include "featureDefs.h"
#include "rotatingModelView.h"

#include "glObjects.h"

#include <array>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GuiRotatingModelView::GuiRotatingModelView(GuiContainer* owner, string id, P<ModelData> model)
: GuiElement(owner, id), model(model)
{
}

void GuiRotatingModelView::onDraw(sf::RenderTarget& window)
{
#if FEATURE_3D_RENDERING
    if (rect.height <= 0) return;
    if (rect.width <= 0) return;
    if (!model) return;

    window.popGLStates();

    float camera_fov = 60.0f;
    float sx = window.getSize().x * window.getView().getViewport().width / window.getView().getSize().x;
    float sy = window.getSize().y * window.getView().getViewport().height / window.getView().getSize().y;
    glViewport(rect.left * sx, (float(window.getView().getSize().y) - rect.height - rect.top) * sx, rect.width * sx, rect.height * sy);

    glClearDepth(1.f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    auto& shader = model->getShader();
    sf::Shader::bind(&shader);
    auto projection = glm::perspective(glm::radians(camera_fov), rect.width / rect.height, 1.f, 25000.f);
    glUniformMatrix4fv(
        glGetUniformLocation(shader.getNativeHandle(), "projection"), 1, GL_FALSE,
        glm::value_ptr(projection)
    );

    auto view = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    view = glm::scale(view, glm::vec3(1.f, 1.f, -1.f));
    view = glm::translate(view, glm::vec3(0.f, -200.f, 0.f));
    view = glm::rotate(view, glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f));
    view = glm::rotate(view, glm::radians(engine->getElapsedTime() * 360.0f / 10.0f), glm::vec3(0.f, 0.f, 1.f));

    glUniformMatrix4fv(glGetUniformLocation(shader.getNativeHandle(), "view"), 1, GL_FALSE, glm::value_ptr(view));

    glColor4f(1,1,1,1);
    glDisable(GL_BLEND);
    sf::Texture::bind(NULL);
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    {
        float scale = 100.0f / model->getRadius();
        auto model_matrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(scale));
        model->render(model_matrix);
#ifdef DEBUG
        auto debug_shader = ShaderManager::getShader("shaders/basicColor");
        sf::Shader::bind(debug_shader);
        {
            // Common state - matrices.
            glUniformMatrix4fv(glGetUniformLocation(debug_shader->getNativeHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(debug_shader->getNativeHandle(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(debug_shader->getNativeHandle(), "model"), 1, GL_FALSE, glm::value_ptr(model_matrix));

            // Vertex attrib
            gl::ScopedVertexAttribArray positions(glGetAttribLocation(debug_shader->getNativeHandle(), "position"));

            for (const EngineEmitterData& ee : model->engine_emitters)
            {
                sf::Vector3f offset = ee.position * model->scale;
                float r = model->scale * ee.scale * 0.5;

                debug_shader->setUniform("color", sf::Glsl::Vec4(ee.color.x, ee.color.y, ee.color.z, 1.f));
                auto vertices = {
                    sf::Vector3f{offset.x + r, offset.y, offset.z},
                    sf::Vector3f{offset.x - r, offset.y, offset.z},
                    sf::Vector3f{offset.x, offset.y + r, offset.z},
                    sf::Vector3f{offset.x, offset.y - r, offset.z},
                    sf::Vector3f{offset.x, offset.y, offset.z + r},
                    sf::Vector3f{offset.x, offset.y, offset.z - r}
                };
                glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), std::begin(vertices));
                glDrawArrays(GL_LINES, 0, vertices.size());
            }
            float r = model->getRadius() * 0.1f;
            debug_shader->setUniform("color", sf::Glsl::Vec4(sf::Color::White));

            for (const sf::Vector3f& position : model->beam_position)
            {
                sf::Vector3f offset = position * model->scale;

                auto vertices = {
                    sf::Vector3f{offset.x + r, offset.y, offset.z},
                    sf::Vector3f{offset.x - r, offset.y, offset.z},
                    sf::Vector3f{offset.x, offset.y + r, offset.z},
                    sf::Vector3f{offset.x, offset.y - r, offset.z},
                    sf::Vector3f{offset.x, offset.y, offset.z + r},
                    sf::Vector3f{offset.x, offset.y, offset.z - r}
                };
                glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), std::begin(vertices));
                glDrawArrays(GL_LINES, 0, vertices.size());
            }
            
            for (const sf::Vector3f& position : model->tube_position)
            {
                sf::Vector3f offset = position * model->scale;

                auto vertices = {
                    sf::Vector3f{offset.x + r * 3, offset.y, offset.z},
                    sf::Vector3f{offset.x - r, offset.y, offset.z},
                    sf::Vector3f{offset.x, offset.y + r, offset.z},
                    sf::Vector3f{offset.x, offset.y - r, offset.z},
                    sf::Vector3f{offset.x, offset.y, offset.z + r},
                    sf::Vector3f{offset.x, offset.y, offset.z - r}
                };
                glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), std::begin(vertices));
                glDrawArrays(GL_LINES, 0, vertices.size());
            }
        }
#endif
    }

    sf::Shader::bind(NULL);
    glDisable(GL_DEPTH_TEST);

    window.pushGLStates();
#endif//FEATURE_3D_RENDERING
}
