#ifndef MESH_H
#define MESH_H

#include <SFML/System.hpp>
#include "stringImproved.h"

struct MeshVertex
{
    float position[3]{};
    float normal[3]{};
    float uv[2]{};
};

class Mesh : public sf::NonCopyable
{
    std::vector<MeshVertex> vertices;
    uint32_t vbo{}, ibo{};
    uint32_t element_type{};
    uint32_t face_count{};
public:
    Mesh(std::vector<MeshVertex>&& vertices, std::vector<uint8_t>&& indices);
    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) = delete;

    ~Mesh();

    void render(int32_t position_attrib, int32_t texcoords_attrib, int32_t normal_attrib);
    sf::Vector3f randomPoint();

    static Mesh* getMesh(const string& filename);
};

#endif//MESH_H
