#include <GL/glad.h>
#include <unordered_map>
#include "engine.h"
#include "mesh.h"
#include "featureDefs.h"

#include "meshoptimizer.h"

#ifdef _MSC_VER
#include <cstdlib>
#define bswap32 _byteswap_ulong
#define bswap16 _byteswap_ushort
#else
#define bswap32 __builtin_bswap32
#define bswap16 __builtin_bswap16
#endif

namespace
{
    inline int32_t readInt(const P<ResourceStream>& stream)
    {
        int32_t ret = 0;
        stream->read(&ret, sizeof(int32_t));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_WIN32)
        return (ret & 0xFF) << 24 | (ret & 0xFF00) << 8 | (ret & 0xFF0000) >> 8 | (ret & 0xFF000000) >> 24;
#endif
        return ret;
    }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_WIN32)
    constexpr bool is_little_endian = true;
    constexpr uint32_t fromLittleEndian(uint32_t value)
    {
        return value;
    }

    constexpr uint16_t fromLittleEndian(uint16_t value)
    {
        return value;
    }
#else
    constexpr bool is_little_endian = false;

    inline uint32_t fromLittleEndian(uint32_t value)
    {
        return bswap32(value);
    }

    inline uint16_t fromLittleEndian(uint16_t value)
    {
        return bswap16(value);
    }
#endif
    constexpr size_t element_size_for_vertex_count(size_t vertex_count)
    {
        if (vertex_count <= 256)
            return sizeof(uint8_t);

        if (vertex_count <= 65536)
            return sizeof(uint16_t);

        return sizeof(uint32_t);
    }

    constexpr uint32_t NO_BUFFER = 0;
    std::unordered_map<string, Mesh*> meshMap;
}
Mesh::Mesh(std::vector<MeshVertex>&& input_vertices, std::vector<uint8_t>&& indices)
    :vbo{NO_BUFFER}, ibo{NO_BUFFER}, face_count{static_cast<uint32_t>(input_vertices.size() / 3)}
{
    if (!input_vertices.empty() && GLAD_GL_ES_VERSION_2_0)
    {
        size_t element_size = sizeof(uint32_t);
        size_t index_count = 0;
        if (!indices.empty())
        {
            vertices = std::move(input_vertices);
            element_size = element_size_for_vertex_count(vertices.size());

            index_count = indices.size() / element_size;
            face_count = index_count / 3;
        }
        else
        {
            index_count = 3 * face_count;
            {
                std::vector<uint32_t> remap(index_count); // allocate temporary memory for the remap table
                vertices.resize(meshopt_generateVertexRemap(remap.data(), nullptr, index_count, input_vertices.data(), index_count, sizeof(MeshVertex)));

                indices.resize(index_count * sizeof(uint32_t));
                meshopt_remapIndexBuffer(reinterpret_cast<uint32_t*>(indices.data()), nullptr, index_count, remap.data());
                meshopt_remapVertexBuffer(vertices.data(), input_vertices.data(), index_count, sizeof(MeshVertex), remap.data());
                input_vertices.clear();
            }

            // Repack the index array 'in-place'.
            auto src = reinterpret_cast<const uint32_t*>(indices.data());
            auto src_end = reinterpret_cast<const uint32_t*>(indices.data() + indices.size());
            element_size = element_size_for_vertex_count(vertices.size());
            switch (element_size)
            {
                case sizeof(uint8_t):
                {
                    auto dst = reinterpret_cast<uint8_t*>(indices.data());
                    while (src != src_end)
                        *dst++ = static_cast<uint8_t>(*src++);
                }
                break;
                case sizeof(uint16_t) :
                {
                    auto dst = reinterpret_cast<uint16_t*>(indices.data());
                    while (src != src_end)
                        *dst++ = static_cast<uint16_t>(*src++);
                }
            }
        }
        

        std::array<uint32_t, 2> buffers{};
        glGenBuffers(buffers.size(), buffers.data());
        vbo = buffers[0];
        ibo = buffers[1];
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * element_size, indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

        element_type = [element_size]()
        {
            if (element_size == sizeof(uint8_t))
                return GL_UNSIGNED_BYTE;
            if (element_size == sizeof(uint16_t))
                return GL_UNSIGNED_SHORT;

            return GL_UNSIGNED_INT;
        }();
    }
}

Mesh::~Mesh()
{
    if (vbo != NO_BUFFER)
    {
        std::array buffers{ vbo, ibo };
        glDeleteBuffers(buffers.size(), buffers.data());
    }
        
}

void Mesh::render(int32_t position_attrib, int32_t texcoords_attrib, int32_t normal_attrib)
{
#if FEATURE_3D_RENDERING
    if (vertices.empty() || vbo == NO_BUFFER)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    if (position_attrib != -1)
        glVertexAttribPointer(position_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, position));
    
    if (normal_attrib != -1)
        glVertexAttribPointer(normal_attrib, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, normal));
    
    if (texcoords_attrib != -1)
        glVertexAttribPointer(texcoords_attrib, 2, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), (void*)offsetof(MeshVertex, uv));

    glDrawElements(GL_TRIANGLES, face_count * 3, element_type, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
#endif//FEATURE_3D_RENDERING
}

sf::Vector3f Mesh::randomPoint()
{
    if (vertices.empty())
        return sf::Vector3f{};

    //int idx = irandom(0, vertexCount-1);
    //return sf::Vector3f(vertices[idx].position[0], vertices[idx].position[1], vertices[idx].position[2]);
    // Pick a face
    int idx = irandom(0, vertices.size() / 3 - 1) * 3; 
    sf::Vector3f v0 = sf::Vector3f(vertices[idx].position[0], vertices[idx].position[1], vertices[idx].position[2]);
    sf::Vector3f v1 = sf::Vector3f(vertices[idx+1].position[0], vertices[idx+1].position[1], vertices[idx+1].position[2]);
    sf::Vector3f v2 = sf::Vector3f(vertices[idx+2].position[0], vertices[idx+2].position[1], vertices[idx+2].position[2]);

    float f1 = random(0.0, 1.0);
    float f2 = random(0.0, 1.0);
    if (f1 + f2 > 1.0f)
    {
        f1 = 1.0f - f1;
        f2 = 1.0f - f2;
    }
    sf::Vector3f v01 = (v0 * f1) + (v1 * (1.0f - f1));
    sf::Vector3f ret = (v01 * f2) + (v2 * (1.0f - f2));
    return ret;
}

struct IndexInfo
{
    int v;
    int t;
    int n;
};

Mesh* Mesh::getMesh(const string& filename)
{
    Mesh* ret = meshMap[filename];
    if (ret)
        return ret;

    std::string_view basename{ filename };
    basename = basename.substr(0, basename.find_last_of('.'));
    P<ResourceStream> stream = getResourceStream(std::string{ basename } + ".model2");
    if (!stream)
    {
        basename = std::string_view{};
        stream = getResourceStream(filename);
    }
    if(!stream)
        return NULL;

    std::vector<MeshVertex> mesh_vertices;
    std::vector<uint8_t> mesh_indices;
    if (!basename.empty())
    {
        uint32_t value{};
        stream->read(&value, sizeof(value));
        mesh_vertices.resize(value);
        auto element_size = element_size_for_vertex_count(mesh_vertices.size());

        stream->read(&value, sizeof(value));
        mesh_indices.resize(value * element_size);
        stream->read(mesh_indices.data(), mesh_indices.size());
        if constexpr (!is_little_endian)
        {
            switch (element_size)
            {
                case sizeof(uint16_t) :
                {
                    auto as_u16 = reinterpret_cast<uint16_t*>(mesh_indices.data());
                    auto end = reinterpret_cast<uint16_t*>(mesh_indices.data() + mesh_indices.size());
                    while (as_u16 != end)
                        *as_u16 = fromLittleEndian(*as_u16);
                }
                break;
                case sizeof(uint32_t) :
                {
                    auto as_u32 = reinterpret_cast<uint32_t*>(mesh_indices.data());
                    auto end = reinterpret_cast<uint32_t*>(mesh_indices.data() + mesh_indices.size());
                    while (as_u32 != end)
                        *as_u32 = fromLittleEndian(*as_u32);
                }
                break;
            }
        }
        
        stream->read(mesh_vertices.data(), sizeof(MeshVertex) * mesh_vertices.size());
    }
    else if (filename.endswith(".obj"))
    {
        std::vector<sf::Vector3f> vertices;
        std::vector<sf::Vector3f> normals;
        std::vector<sf::Vector2f> texCoords;
        std::vector<IndexInfo> indices;

        do
        {
            string line = stream->readLine();
            if (line.length() > 0 && line[0] != '#')
            {
                std::vector<string> parts = line.strip().split();
                if (parts.size() < 1)
                    continue;
                if (parts[0] == "v")
                {
                    vertices.push_back(sf::Vector3f(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat()));
                }else if (parts[0] == "vn")
                {
                    normals.push_back(sf::normalize(sf::Vector3f(parts[1].toFloat(), parts[2].toFloat(), parts[3].toFloat())));
                }else if (parts[0] == "vt")
                {
                    texCoords.push_back(sf::Vector2f(parts[1].toFloat(), parts[2].toFloat()));
                }else if (parts[0] == "f")
                {
                    for(unsigned int n=3; n<parts.size(); n++)
                    {
                        std::vector<string> p0 = parts[1].split("/");
                        std::vector<string> p1 = parts[n].split("/");
                        std::vector<string> p2 = parts[n-1].split("/");

                        IndexInfo info;
                        info.v = p0[0].toInt() - 1;
                        info.t = p0[1].toInt() - 1;
                        info.n = p0[2].toInt() - 1;
                        indices.push_back(info);
                        info.v = p2[0].toInt() - 1;
                        info.t = p2[1].toInt() - 1;
                        info.n = p2[2].toInt() - 1;
                        indices.push_back(info);
                        info.v = p1[0].toInt() - 1;
                        info.t = p1[1].toInt() - 1;
                        info.n = p1[2].toInt() - 1;
                        indices.push_back(info);
                    }
                }else{
                    //printf("%s\n", parts[0].c_str());
                }
            }
        }while(stream->tell() < stream->getSize());

        
        mesh_vertices.resize(indices.size());
        for(unsigned int n=0; n<indices.size(); n++)
        {
            mesh_vertices[n].position[0] = vertices[indices[n].v].x;
            mesh_vertices[n].position[1] = vertices[indices[n].v].z;
            mesh_vertices[n].position[2] = vertices[indices[n].v].y;
            mesh_vertices[n].normal[0] = normals[indices[n].n].x;
            mesh_vertices[n].normal[1] = normals[indices[n].n].z;
            mesh_vertices[n].normal[2] = normals[indices[n].n].y;
            mesh_vertices[n].uv[0] = texCoords[indices[n].t].x;
            mesh_vertices[n].uv[1] = 1.f - texCoords[indices[n].t].y;
        }
    }
    else if (filename.endswith(".model"))
    {
        mesh_vertices.resize(readInt(stream));
        stream->read(mesh_vertices.data(), sizeof(MeshVertex) * mesh_vertices.size());
    }else{
        LOG(ERROR) << "Unknown mesh format: " << filename;
    }

    stream = P<ResourceStream>();

    ret = new Mesh(std::move(mesh_vertices), std::move(mesh_indices));
    meshMap[filename] = ret;
   
    return ret;
}
