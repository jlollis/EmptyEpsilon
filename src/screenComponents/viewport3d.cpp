#include <GL/glad.h>

#include "main.h"
#include "playerInfo.h"
#include "gameGlobalInfo.h"
#include "viewport3d.h"

#include "particleEffect.h"
#include "glObjects.h"
#include "shaderRegistry.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SFML/stb/dds-ktx.h"

GuiViewport3D::GuiViewport3D(GuiContainer* owner, string id)
: GuiElement(owner, id)
{
    show_callsigns = false;
    show_headings = false;
    show_spacedust = false;

    // Load up our starbox into a cubemap.
#if FEATURE_3D_RENDERING
    if (gl::isAvailable())
    {
        // Setup shader.
        starbox_shader = ShaderManager::getShader("shaders/starbox");
        starbox_uniforms[static_cast<size_t>(Uniforms::Projection)] = glGetUniformLocation(starbox_shader->getNativeHandle(), "projection");
        starbox_uniforms[static_cast<size_t>(Uniforms::ModelView)] = glGetUniformLocation(starbox_shader->getNativeHandle(), "view");

        starbox_vertex_attributes[static_cast<size_t>(VertexAttributes::Position)] = glGetAttribLocation(starbox_shader->getNativeHandle(), "position");

        // Load up the cube texture.
        // Face setup
        std::array<std::tuple<const char*, uint32_t>, 6> faces{
            std::make_tuple("StarsRight", GL_TEXTURE_CUBE_MAP_POSITIVE_X),
            std::make_tuple("StarsLeft", GL_TEXTURE_CUBE_MAP_NEGATIVE_X),
            std::make_tuple("StarsTop", GL_TEXTURE_CUBE_MAP_POSITIVE_Y),
            std::make_tuple("StarsBottom", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y),
            std::make_tuple("StarsFront", GL_TEXTURE_CUBE_MAP_POSITIVE_Z),
            std::make_tuple("StarsBack", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z),
        };

        // Upload
        glBindTexture(GL_TEXTURE_CUBE_MAP, starbox_texture[0]);
        sf::Image image;

        for (const auto& face : faces)
        {
            P<ResourceStream> stream;
            const string basename{ std::get<0>(face) };
            // Attempt to load compressed textures first.
            if (GLAD_GL_EXT_texture_compression_s3tc || GLAD_GL_KHR_texture_compression_astc_ldr)
            {
                if (GLAD_GL_KHR_texture_compression_astc_ldr)
                    stream = getResourceStream(basename + "-astc.ktx");
                if (!stream && GLAD_GL_EXT_texture_compression_s3tc)
                    stream = getResourceStream(basename + "-dxt.ktx");

                if (stream && image.loadFromStream(**stream))
                {
                    ddsktx_texture_info info{};
                    if (ddsktx_parse(&info, image.data(), image.getByteSize()))
                    {
                        auto gl_format = [ddsktx_format = info.format]()
                        {
                            switch (ddsktx_format)
                            {
                            case DDSKTX_FORMAT_BC1:
                                return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                            case DDSKTX_FORMAT_BC3:
                                return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                            case DDSKTX_FORMAT_ASTC4x4:
                                return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
                            case DDSKTX_FORMAT_ASTC8x6:
                                return GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
                            }

                            SDL_assert_paranoid(false);
                            return GL_NONE;
                        }();

                        if (gl_format != GL_NONE)
                        {
                            ddsktx_sub_data sub_data{};
                            for (auto mip = 0; mip < 1; ++mip)
                            {
                                ddsktx_get_sub(&info, &sub_data, image.data(), image.getByteSize(), 0, 0, mip);
                                glCompressedTexImage2D(std::get<1>(face), mip, gl_format, sub_data.width, sub_data.height, 0, sub_data.size_bytes, sub_data.buff);
                            }
                        }
                    }
                }
            }

            if (!stream)
            {
                auto stream = getResourceStream(basename + ".png");
                if (!stream || !image.loadFromStream(**stream))
                {
                    LOG(WARNING) << "Failed to load texture: " << std::get<0>(face);
                    image.create(8, 8, sf::Color(255, 0, 255, 128));
                }

                glTexImage2D(std::get<1>(face), 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
            }
        }

        // Make it pretty.
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        for (auto wrap_axis : { GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T })
            glTexParameteri(GL_TEXTURE_CUBE_MAP, wrap_axis, GL_CLAMP_TO_EDGE);
        if (GLAD_GL_ES_VERSION_2_0)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);

        // Load up the ebo and vbo for the cube.
        /*   
               .2------6
             .' |    .'|
            3---+--7'  |
            |   |  |   |
            |  .0--+---4
            |.'    | .'
            1------5'
        */
        std::array<sf::Vector3f, 8> positions{
            // Left face
            sf::Vector3f{-1.f, -1.f, -1.f}, // 0
            sf::Vector3f{-1.f, -1.f, 1.f},  // 1
            sf::Vector3f{-1.f, 1.f, -1.f},  // 2
            sf::Vector3f{-1.f, 1.f, 1.f},   // 3

            // Right face
            sf::Vector3f{1.f, -1.f, -1.f},  // 4
            sf::Vector3f{1.f, -1.f, 1.f},   // 5
            sf::Vector3f{1.f, 1.f, -1.f},   // 6
            sf::Vector3f{1.f, 1.f, 1.f},    // 7
        };

        constexpr std::array<uint8_t, 6 * 6> elements{
            2, 6, 4, 4, 0, 2, // Back
            3, 2, 0, 0, 1, 3, // Left
            6, 7, 5, 5, 4, 6, // Right
            7, 3, 1, 1, 5, 7, // Front
            6, 2, 3, 3, 7, 6, // Top
            0, 4, 5, 5, 1, 0, // Bottom
        };

        // Upload to GPU.
        glBindBuffer(GL_ARRAY_BUFFER, starbox_buffers[static_cast<size_t>(Buffers::Vertex)]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starbox_buffers[static_cast<size_t>(Buffers::Element)]);

        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(sf::Vector3f), positions.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(uint8_t), elements.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
        // Setup spacedust
        spacedust_shader = ShaderManager::getShader("shaders/spacedust");
        spacedust_uniforms[static_cast<size_t>(Uniforms::Projection)] = glGetUniformLocation(spacedust_shader->getNativeHandle(), "projection");
        spacedust_uniforms[static_cast<size_t>(Uniforms::ModelView)] = glGetUniformLocation(spacedust_shader->getNativeHandle(), "view");
        spacedust_uniforms[static_cast<size_t>(Uniforms::Rotation)] = glGetUniformLocation(spacedust_shader->getNativeHandle(), "rotation");

        spacedust_vertex_attributes[static_cast<size_t>(VertexAttributes::Position)] = glGetAttribLocation(spacedust_shader->getNativeHandle(), "position");
        spacedust_vertex_attributes[static_cast<size_t>(VertexAttributes::Sign)] = glGetAttribLocation(spacedust_shader->getNativeHandle(), "sign_value");

        // Reserve our GPU buffer.
        // Each dust particle consist of:
        // - a worldpace position (Vector3f)
        // - a sign value (single byte, passed as float).
        // Both "arrays" are maintained separate:
        // the signs are stable (they just tell us which "end" of the line we're on)
        // The positions will get updated more frequently.
        // It means each particle occupies 2*16B (assuming tight packing)
        glBindBuffer(GL_ARRAY_BUFFER, spacedust_buffer[0]);
        glBufferData(GL_ARRAY_BUFFER, 2 * spacedust_particle_count * (sizeof(sf::Vector3f) + sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

        // Generate and update the alternating vertices signs.
        std::array<float, 2 * spacedust_particle_count> signs;
        
        for (auto n = 0; n < signs.size(); n += 2)
        {
            signs[n] = -1.f;
            signs[n + 1] = 1.f;
        }

        // Update sign parts.
        glBufferSubData(GL_ARRAY_BUFFER, 2 * spacedust_particle_count * sizeof(sf::Vector3f), signs.size() * sizeof(float), signs.data());
        {
            // zero out positions.
            const std::vector<sf::Vector3f> zeroed_positions(2 * spacedust_particle_count);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * spacedust_particle_count * sizeof(sf::Vector3f), zeroed_positions.data());
        }
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        
    }
#endif // FEATURE_3D_RENDERING
}

void GuiViewport3D::onDraw(sf::RenderTarget& window)
{
#if FEATURE_3D_RENDERING
    if (my_spaceship)
        soundManager->setListenerPosition(my_spaceship->getPosition(), my_spaceship->getRotation());
    else
        soundManager->setListenerPosition(sf::Vector2f(camera_position.x, camera_position.y), camera_yaw);
    
    glActiveTexture(GL_TEXTURE0);

    float camera_fov = 60.0f;
    {
        // Translate our rect from view coordinates to window.
        const auto& view = window.getView();
        const auto& view_size = view.getSize();

        const auto& relative_viewport = view.getViewport();

        // View's viewport in target coordinate system (= pixels)
        const auto& window_viewport = window.getViewport(view);

        // Get the scaling factor - from logical size to pixels.
        const sf::Vector2f view_to_window{ window_viewport.width / view_size.x, window_viewport.height / view_size.y };
        
        // Compute rect, applying logical -> pixel scaling.
        const sf::IntRect window_rect{
            static_cast<int32_t>(.5f + rect.left * view_to_window.x),
            static_cast<int32_t>(.5f + rect.top * view_to_window.y),
            static_cast<int32_t>(.5f + rect.width * view_to_window.x),
            static_cast<int32_t>(.5f + rect.height * view_to_window.y)
        };

        // Apply current viewport translation.
        // (top / bottom is flipped around)
        auto left = view_size.x * relative_viewport.left + window_rect.left;
        auto top = view_size.y * (view_to_window.y + relative_viewport.top) - (window_rect.top + window_rect.height);

        // Setup 3D viewport.
        glViewport(left, top, window_rect.width, window_rect.height);
    }
    glClearDepthf(1.f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_CULL_FACE);

    projection_matrix = glm::perspective(glm::radians(camera_fov), rect.width / rect.height, 1.f, 25000.f);
    view_matrix = glm::rotate(glm::identity<glm::mat4>(), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    view_matrix = glm::scale(view_matrix, glm::vec3(1.f, 1.f, -1.f));
    view_matrix = glm::rotate(view_matrix, glm::radians(-camera_pitch), glm::vec3(1.f, 0.f, 0.f));
    view_matrix = glm::rotate(view_matrix, glm::radians(-camera_yaw - 90.f), glm::vec3(0.f, 0.f, 1.f));
    view_matrix = glm::translate(view_matrix, -glm::vec3(camera_position.x, camera_position.y, camera_position.z));
    
    glGetFloatv(GL_VIEWPORT, glm::value_ptr(viewport));

    // Draw starbox.
    glDepthMask(GL_FALSE);
    {
        
        sf::Shader::bind(starbox_shader);
        starbox_shader->setUniform("scale", 100.f);

        // Setup shared state (uniforms)
        glBindTexture(GL_TEXTURE_CUBE_MAP, starbox_texture[0]);
        
        // Uniform
        // Upload matrices
        glUniformMatrix4fv(starbox_uniforms[static_cast<size_t>(Uniforms::Projection)], 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(starbox_uniforms[static_cast<size_t>(Uniforms::ModelView)], 1, GL_FALSE, glm::value_ptr(view_matrix));
        
        // Bind our cube
        {
            gl::ScopedVertexAttribArray positions(starbox_vertex_attributes[static_cast<size_t>(VertexAttributes::Position)]);
            glBindBuffer(GL_ARRAY_BUFFER, starbox_buffers[static_cast<size_t>(Buffers::Vertex)]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, starbox_buffers[static_cast<size_t>(Buffers::Element)]);

            // Vertex attributes.
            glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), (GLvoid*)0);


            glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_BYTE, (GLvoid*)0);

            // Cleanup
            glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
        sf::Shader::bind(nullptr);
    }
    glDepthMask(GL_TRUE);

    sf::Texture::bind(NULL);

    class RenderInfo
    {
    public:
        RenderInfo(SpaceObject* obj, float d)
        : object(obj), depth(d)
        {}

        SpaceObject* object;
        float depth;
    };
    std::vector<std::vector<RenderInfo>> render_lists;

    sf::Vector2f viewVector = sf::vector2FromAngle(camera_yaw);
    float depth_cutoff_back = camera_position.z * -tanf((90+camera_pitch + camera_fov/2.0) / 180.0f * M_PI);
    float depth_cutoff_front = camera_position.z * -tanf((90+camera_pitch - camera_fov/2.0) / 180.0f * M_PI);
    if (camera_pitch - camera_fov/2.0 <= 0.0)
        depth_cutoff_front = std::numeric_limits<float>::infinity();
    if (camera_pitch + camera_fov/2.0 >= 180.0)
        depth_cutoff_back = -std::numeric_limits<float>::infinity();
    foreach(SpaceObject, obj, space_object_list)
    {
        float depth = sf::dot(viewVector, obj->getPosition() - sf::Vector2f(camera_position.x, camera_position.y));
        if (depth + obj->getRadius() < depth_cutoff_back)
            continue;
        if (depth - obj->getRadius() > depth_cutoff_front)
            continue;
        if (depth > 0 && obj->getRadius() / depth < 1.0 / 500)
            continue;
        int render_list_index = std::max(0, int((depth + obj->getRadius()) / 25000));
        while(render_list_index >= int(render_lists.size()))
            render_lists.emplace_back();
        render_lists[render_list_index].emplace_back(*obj, depth);
    }

    for (auto i = 0; i < ShaderRegistry::Shaders_t(ShaderRegistry::Shaders::Count); ++i)
    {
        const auto& shader = ShaderRegistry::get(ShaderRegistry::Shaders(i));
        if (shader.uniform(ShaderRegistry::Uniforms::View) != -1)
        {
            glUseProgram(shader.get()->getNativeHandle());
            glUniformMatrix4fv(shader.uniform(ShaderRegistry::Uniforms::View), 1, GL_FALSE, glm::value_ptr(view_matrix));
        }
    }

    for(int n=render_lists.size() - 1; n >= 0; n--)
    {
        auto& render_list = render_lists[n];
        std::sort(render_list.begin(), render_list.end(), [](const RenderInfo& a, const RenderInfo& b) { return a.depth > b.depth; });

        auto projection = glm::perspective(glm::radians(camera_fov), rect.width / rect.height, 1.f, 25000.f * (n + 1));
        // Update projection matrix in shaders.
        for (auto i = 0; i < ShaderRegistry::Shaders_t(ShaderRegistry::Shaders::Count); ++i)
        {
            const auto& shader = ShaderRegistry::get(ShaderRegistry::Shaders(i));
            if (shader.uniform(ShaderRegistry::Uniforms::Projection) != -1)
            {
                glUseProgram(shader.get()->getNativeHandle());
                glUniformMatrix4fv(shader.uniform(ShaderRegistry::Uniforms::Projection), 1, GL_FALSE, glm::value_ptr(projection));
            }
        }
        glUseProgram(GL_NONE);

        glDepthMask(true);
        glClear(GL_DEPTH_BUFFER_BIT);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        
        for(const auto &info : render_list)
        {
            SpaceObject* obj = info.object;
            obj->draw3D();
        }
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDisable(GL_CULL_FACE);
        glDepthMask(false);
        for(const auto &info : render_list)
        {
            SpaceObject* obj = info.object;
            obj->draw3DTransparent();
        }
    }
    
    ParticleEngine::render(projection_matrix, view_matrix);

    if (show_spacedust && my_spaceship)
    {
        static std::vector<sf::Vector3f> space_dust(2 * spacedust_particle_count);
        
        sf::Vector2f dust_vector = my_spaceship->getVelocity() / 100.f;
        sf::Vector3f dust_center = sf::Vector3f(my_spaceship->getPosition().x, my_spaceship->getPosition().y, 0.f); 

        constexpr float maxDustDist = 500.f;
        constexpr float minDustDist = 100.f;
        
        bool update_required = false; // Do we need to update the GPU buffer?

        for (auto n = 0; n < space_dust.size(); n += 2)
        {
            //
            auto delta = space_dust[n] - dust_center;
            if (delta > maxDustDist || delta < minDustDist)
            {
                update_required = true;
                space_dust[n] = dust_center + sf::Vector3f(random(-maxDustDist, maxDustDist), random(-maxDustDist, maxDustDist), random(-maxDustDist, maxDustDist));
                space_dust[n + 1] = space_dust[n];
            }
        }

        sf::Shader::bind(spacedust_shader);

        // Upload matrices (only float 4x4 supported in es2)
        glUniformMatrix4fv(spacedust_uniforms[static_cast<size_t>(Uniforms::Projection)], 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(spacedust_uniforms[static_cast<size_t>(Uniforms::ModelView)], 1, GL_FALSE, glm::value_ptr(view_matrix));

        // Ship information for flying particles
        spacedust_shader->setUniform("velocity", dust_vector);
        
        {
            gl::ScopedVertexAttribArray positions(spacedust_vertex_attributes[static_cast<size_t>(VertexAttributes::Position)]);
            gl::ScopedVertexAttribArray signs(spacedust_vertex_attributes[static_cast<size_t>(VertexAttributes::Sign)]);
            glBindBuffer(GL_ARRAY_BUFFER, spacedust_buffer[0]);
            
            if (update_required)
            {
                glBufferSubData(GL_ARRAY_BUFFER, 0, space_dust.size() * sizeof(sf::Vector3f), space_dust.data());
            }
            glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), (GLvoid*)0);
            glVertexAttribPointer(signs.get(), 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(2 * spacedust_particle_count * sizeof(sf::Vector3f)));
            
            glDrawArrays(GL_LINES, 0, 2 * spacedust_particle_count);
            glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        }
        sf::Shader::bind(nullptr);
    }
    
    if (my_spaceship && my_spaceship->getTarget())
    {
        ShaderRegistry::ScopedShader billboard(ShaderRegistry::Shaders::Billboard);

        P<SpaceObject> target = my_spaceship->getTarget();
        glDisable(GL_DEPTH_TEST);

        auto model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(target->getPosition().x, target->getPosition().y, 0.f));

        glBindTexture(GL_TEXTURE_2D, textureManager.getTexture("redicule2.png")->getNativeHandle());
        glUniform4f(billboard.get().uniform(ShaderRegistry::Uniforms::Color), .5f, .5f, .5f, target->getRadius() * 2.5f);
        glUniformMatrix4fv(billboard.get().uniform(ShaderRegistry::Uniforms::Model), 1, GL_FALSE, glm::value_ptr(model_matrix));
        {
            gl::ScopedVertexAttribArray positions(billboard.get().attribute(ShaderRegistry::Attributes::Position));
            gl::ScopedVertexAttribArray texcoords(billboard.get().attribute(ShaderRegistry::Attributes::Texcoords));
            auto vertices = {
                0.f, 0.f, 0.f,
                0.f, 0.f, 0.f,
                0.f, 0.f, 0.f,
                0.f, 0.f, 0.f
            };
            glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)vertices.begin());
            auto coords = {
                0.f, 0.f,
                1.f, 0.f,
                1.f, 1.f,
                0.f, 1.f
            };
            glVertexAttribPointer(texcoords.get(), 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)coords.begin());
            std::initializer_list<uint8_t> indices{ 0, 1, 2, 2, 3, 0 };
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, std::begin(indices));
        }
    }

    glDepthMask(true);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

#ifdef DEBUG
    glDisable(GL_DEPTH_TEST);
    
    {
        ShaderRegistry::ScopedShader debug_shader(ShaderRegistry::Shaders::BasicColor);
        // Common state: color, projection matrix.
        glUniform4f(debug_shader.get().uniform(ShaderRegistry::Uniforms::Color), 1.f, 1.f, 1.f, 1.f);

        glUniformMatrix4fv(debug_shader.get().uniform(ShaderRegistry::Uniforms::Projection), 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(debug_shader.get().uniform(ShaderRegistry::Uniforms::View), 1, GL_FALSE, glm::value_ptr(view_matrix));

        std::vector<sf::Vector3f> points;
        gl::ScopedVertexAttribArray positions(debug_shader.get().attribute(ShaderRegistry::Attributes::Position));

        foreach(SpaceObject, obj, space_object_list)
        {
            auto model_matrix = glm::translate(glm::identity<glm::mat4>(), glm::vec3(obj->getPosition().x, obj->getPosition().y, 0.f));
            model_matrix = glm::rotate(model_matrix, glm::radians(obj->getRotation()), glm::vec3(0.f, 0.f, 1.f));

            glUniformMatrix4fv(debug_shader.get().uniform(ShaderRegistry::Uniforms::Model), 1, GL_FALSE, glm::value_ptr(model_matrix));

            std::vector<sf::Vector2f> collisionShape = obj->getCollisionShape();

            if (collisionShape.size() > points.size())
            {
                points.resize(collisionShape.size());
                glVertexAttribPointer(positions.get(), 3, GL_FLOAT, GL_FALSE, sizeof(sf::Vector3f), points.data());
            }

            for (unsigned int n = 0; n < collisionShape.size(); n++)
                points[n] = sf::Vector3f(collisionShape[n].x, collisionShape[n].y, 0.f);
            
            glDrawArrays(GL_LINE_LOOP, 0, collisionShape.size());
        }
    }
#endif

    window.resetGLStates();

    if (show_callsigns && !render_lists.empty())
    {
        for(const auto& info : render_lists[0])
        {
            SpaceObject* obj = info.object;
            if (!obj->canBeTargetedBy(my_spaceship) || obj == *my_spaceship)
                continue;
            string call_sign = obj->getCallSign();
            if (call_sign == "")
                continue;

            sf::Vector3f screen_position = worldToScreen(window, sf::Vector3f(obj->getPosition().x, obj->getPosition().y, obj->getRadius()));
            if (screen_position.z < 0)
                continue;
            if (screen_position.z > 10000.0)
                continue;
            float distance_factor = 1.0f - (screen_position.z / 10000.0f);
            drawText(window, sf::FloatRect(screen_position.x, screen_position.y, 0, 0), call_sign, ACenter, 20 * distance_factor, bold_font, sf::Color(255, 255, 255, 128 * distance_factor));
        }
    }

    if (show_headings && my_spaceship)
    {
        float distance = 2500.f;

        for(int angle = 0; angle < 360; angle += 30)
        {
            sf::Vector2f world_pos = my_spaceship->getPosition() + sf::vector2FromAngle(float(angle - 90)) * distance;
            sf::Vector3f screen_pos = worldToScreen(window, sf::Vector3f(world_pos.x, world_pos.y, 0.0f));
            if (screen_pos.z > 0.0f)
                drawText(window, sf::FloatRect(screen_pos.x, screen_pos.y, 0, 0), string(angle), ACenter, 30, bold_font, sf::Color(255, 255, 255, 128));
        }
    }
#endif//FEATURE_3D_RENDERING
}

sf::Vector3f GuiViewport3D::worldToScreen(const sf::RenderTarget& window, const sf::Vector3f& world)
{
    auto view_pos = view_matrix * glm::vec4(world.x, world.y, world.z, 1.f);
    auto pos = projection_matrix * view_pos;
    
    // Perspective division
    pos /= pos.w;

    //Window coordinates
    //Map x, y to range 0-1
    sf::Vector3f ret;
    ret.x = (pos.x * .5f + .5f) * viewport.z + viewport.x;
    ret.y = (pos.y * .5f + .5f) * viewport.w + viewport.y;
    //This is only correct when glDepthRange(0.0, 1.0)
    //ret.z = (1.0+fTempo[6])*0.5;  //Between 0 and 1
    //Set Z to distance into the screen (negative is behind the screen)
    ret.z = -view_pos.z;

    ret.x = ret.x * window.getView().getSize().x / window.getSize().x;
    ret.y = ret.y * window.getView().getSize().y / window.getSize().y;
    ret.y = window.getView().getSize().y - ret.y;
    return ret;  
}
