#include "meshoptimizer.h"

#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

#ifdef _WIN32
#define LF "\r\n"
#else
#define LF "\n"
#endif

#ifdef _MSC_VER
#include <cstdlib>
#define bswap32 _byteswap_ulong
#define bswap16 _byteswap_ushort
#else
#define bswap32 __builtin_bswap32
#define bswap16 __builtin_bswap16
#endif

#include <bitset>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_WIN32)
constexpr uint32_t to_little(uint32_t value)
{
	return value;
}

constexpr uint16_t to_little(uint16_t value)
{
	return value;
}

int32_t from_big(int32_t value)
{
	return bswap32(value);
}
#else
uint32_t to_little(uint32_t value)
{
	return bswap32(value);
}

uint16_t to_little(uint16_t value)
{
	return bswap16(value);
}

int32_t from_big(int32_t value)
{
	return value;
}
#endif

using file_ptr = std::unique_ptr<FILE, decltype(&fclose)>;

file_ptr open_file(const char* filename, const char* mode) noexcept
{
	return {fopen(filename, mode), &fclose};
}

int32_t read_int32(const file_ptr& file) noexcept
{
    int32_t value{};
    fread(&value, sizeof(int32_t), 1, file.get());
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_WIN32)
	return bswap32(value);
#endif
    return value;
}

void write_int32(file_ptr& file, int32_t value) noexcept
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || defined(_WIN32)
	value = bswap32(value);
#endif
	fwrite(&value, sizeof(int32_t), 1, file.get());
}

std::string read_string(const file_ptr& file) noexcept
{
	int8_t length{};
	fread(&length, sizeof(int8_t), 1, file.get());

	std::string data(length + 1, '\0');
	fread(data.data(), length, 1, file.get());
	return data;
}

void write_string(file_ptr& file, std::string_view str)
{
	auto length = static_cast<int8_t>(str.size());
	fwrite(&length, sizeof(int8_t), 1, file.get());
	fwrite(str.data(), length, 1, file.get());
}

struct Entry
{
	std::string name{};
	int32_t position{};
	int32_t size{};

	explicit Entry(const file_ptr& file)
		: name{ read_string(file) }
		, position{ read_int32(file) }
		, size{ read_int32(file) }
	{
	}

	~Entry() = default;
	Entry(const Entry&) = default;
	Entry(Entry&&) = default;

	Entry& operator=(const Entry&) = default;
	Entry& operator=(Entry&&) = default;
};

using obj_ptr = std::unique_ptr<fastObjMesh, decltype(&fast_obj_destroy)>;

namespace obj_vector
{
	struct holder
	{
		const std::vector<uint8_t>& data;
		size_t consumed{};
	};

	void* open(const char*, void* user_data)
	{
		return user_data;
	}

	void close(void* file, void* user_data)
	{
		delete static_cast<holder*>(file);
	}

	size_t read(void* file, void* dst, size_t bytes, void* user_data)
	{
		auto vector = static_cast<holder*>(file);
		bytes = std::min(vector->data.size() - vector->consumed, bytes);
		memcpy(dst, vector->data.data() + vector->consumed, bytes);
		vector->consumed += bytes;
		return bytes;
	}

	unsigned long size(void* file, void* user_data)
	{
		return static_cast<holder*>(file)->data.size();
	}

}

obj_ptr open_obj(const std::vector<uint8_t>& mesh)
{
	fastObjCallbacks callbacks{
		&obj_vector::open, &obj_vector::close, &obj_vector::read, &obj_vector::size
	};

	return { fast_obj_read_with_callbacks("", &callbacks, new obj_vector::holder{mesh}), &fast_obj_destroy };
}

struct Vertex
{
	float position[3]{}; // 12 B
	float normal[3]{};   // 12 B
	float uv[2]{};       //  8 B
};                       // 32 B
static_assert(sizeof(Vertex) == 32, "Padding to handle!");

size_t optimize_vertices(const Vertex* unindexed_vertices, size_t unindexed_count, std::vector<uint8_t>& optimized)
{
	// Recreate index buffer.
	std::vector<uint32_t> remap(unindexed_count); // allocate temporary memory for the remap table
	size_t vertex_count = meshopt_generateVertexRemap(remap.data(), nullptr, unindexed_count, unindexed_vertices, unindexed_count, sizeof(Vertex));

	// Recreate index buffer.
	std::vector<uint32_t> indices(unindexed_count);
	meshopt_remapIndexBuffer(indices.data(), nullptr, indices.size(), remap.data());

	// Recreate vertex buffer (without duplications)
	std::vector<Vertex> vertices(vertex_count);
	meshopt_remapVertexBuffer(vertices.data(), unindexed_vertices, unindexed_count, sizeof(Vertex), remap.data());

	// Optimization passes
	meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());
	meshopt_optimizeOverdraw(indices.data(), indices.data(), indices.size(), vertices.data()->position, vertices.size(), sizeof(Vertex), 1.05f);
	meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), vertices.data(), vertices.size(), sizeof(Vertex));

	// model2: All little endian.
	// Header is <vertex_count:u32><indices_count:u32>
	// Indice type is based on the vertex count:
	// - u8 if <= 256
	// - u16 if <= 65536
	// - u32 otherwise.
	// Then indices, [indice_type] * indice_count bytes.
	// Followed by Vertex * vertex_count (8 ieee floats per)
	size_t element_size = [](size_t vertex_count)
	{
		if (vertex_count <= 256)
			return sizeof(uint8_t);

		if (vertex_count <= 65536)
			return sizeof(uint16_t);

		return sizeof(uint32_t);
	}(vertices.size());

	auto optimized_size = 2 * sizeof(uint32_t) + element_size * indices.size() + vertices.size() * sizeof(Vertex);

	optimized.resize(std::max(optimized.size(), optimized_size));

	uint8_t* dst = optimized.data();
	*reinterpret_cast<uint32_t*>(dst) = to_little(static_cast<uint32_t>(vertices.size()));
	dst += sizeof(uint32_t);
	*reinterpret_cast<uint32_t*>(dst) = to_little(static_cast<uint32_t>(indices.size()));
	dst += sizeof(uint32_t);

	switch (element_size)
	{
		case sizeof(uint8_t):
		{
			auto dst_u8 = dst;
			for (auto idx : indices)
				*dst_u8++ = static_cast<uint8_t>(idx);
		}
		break;
		case sizeof(uint16_t):
		{
			auto dst_u16 = reinterpret_cast<uint16_t*>(dst);

			for (auto idx : indices)
				*dst_u16++ = to_little(static_cast<uint16_t>(idx));
		}
		break;
		default:
		{
			auto dst_u32 = reinterpret_cast<uint32_t*>(dst);

			for (auto idx : indices)
				*dst_u32++ = to_little(static_cast<uint32_t>(idx));
		}
	}

	dst += element_size * indices.size();

	// plaster vertices.
	memcpy(dst, vertices.data(), vertices.size() * sizeof(Vertex));

	return optimized_size;
}

size_t optimize_obj(const std::vector<uint8_t>& mesh, std::vector<uint8_t>& optimized)
{
	auto obj = open_obj(mesh);
	if (!obj)
		return 0;

	// Move everything into one buffer,
	// triangularize
	size_t index_count{};
	// 3 index per face (one triangle)
	// Each additional vertex creates one new triangle.
	for (auto face = 0; face < obj->face_count; ++face)
		index_count += (1 + (obj->face_vertices[face] - 3)) * 3;
	
	std::vector<Vertex> unindexed_vertices(index_count);

	auto copy_vertex = [&unindexed_vertices, &obj](size_t i, const auto& index)
	{
		// EE swaps z/y.
		unindexed_vertices[i].position[0] = obj->positions[3 * index.p + 0];
		unindexed_vertices[i].position[1] = obj->positions[3 * index.p + 2];
		unindexed_vertices[i].position[2] = obj->positions[3 * index.p + 1];

		unindexed_vertices[i].normal[0] = obj->normals[3 * index.n + 0];
		unindexed_vertices[i].normal[1] = obj->normals[3 * index.n + 2];
		unindexed_vertices[i].normal[2] = obj->normals[3 * index.n + 1];

		unindexed_vertices[i].uv[0] = obj->texcoords[2 * index.t + 0];
		
		// Make OpenGL happy by flipping the y coordinate.
		unindexed_vertices[i].uv[1] = 1.f - obj->texcoords[2 * index.t + 1];

	};

	size_t base_index{};
	size_t current_vertex{};
	// Process each face, triangularize.
	for (auto face = 0; face < obj->face_count; ++face)
	{
		auto indices = obj->indices + base_index;
		for (auto v = 2; v < obj->face_vertices[face]; ++v)
		{
			copy_vertex(current_vertex, indices[0]);
			copy_vertex(current_vertex + 2, indices[v]); // 2
			copy_vertex(current_vertex + 1, indices[v - 1]); // 1

			current_vertex += 3;
		}

		base_index += obj->face_vertices[face];
	}

	return optimize_vertices(unindexed_vertices.data(), unindexed_vertices.size(), optimized);
}

size_t optimize_model(const std::vector<uint8_t>& mesh, std::vector<uint8_t>& optimized)
{
	auto vertex_count = from_big(*reinterpret_cast<const int32_t*>(mesh.data()));

	std::vector<Vertex> vertices(vertex_count);

	memcpy(vertices.data(), mesh.data() + sizeof(int32_t), vertex_count * sizeof(Vertex));

	return optimize_vertices(vertices.data(), vertices.size(), optimized);
}

int process_pack(std::string_view src_name, file_ptr& src_pack)
{
	auto version = read_int32(src_pack);

	if (version != 0)
	{
		fprintf(stderr, "version unsupported %d." LF, version);
		return -1;
	}
	
	auto dst_name = std::string{ src_name.substr(0, src_name.find_last_of('.')) } + "-model2.pack";
	auto dst_pack = open_file(dst_name.data(), "wb");
	
	if (!dst_pack)
	{
		fprintf(stderr, "Failed to open destination pack %s" LF, dst_name.data());
		return -1;
	}

	std::vector<Entry> entries;
	auto file_count = read_int32(src_pack);
	entries.reserve(file_count);
	
	constexpr std::array<std::string_view, 4> valid_extensions{ ".model", ".obj" };
	for (auto i = 0; i < file_count; ++i)
	{
		Entry entry{ src_pack };
		{
			auto found = entry.name.find_last_of('.');

			if (found != std::string_view::npos && (entry.name.size() - found) >= 5)
			{
				std::string_view ext{ entry.name.data() + found };
				auto candidate = std::find(std::begin(valid_extensions), std::end(valid_extensions), ext);
				if (candidate != std::end(valid_extensions))
				{
					entries.emplace_back(std::move(entry));
				}
			}
		}
	}

	write_int32(dst_pack, 0); // version
	write_int32(dst_pack, static_cast<int32_t>(entries.size())); // filecount
	
	// Compute header size.
	auto base_offset = 2 * sizeof(int32_t);
	constexpr std::string_view model2{ ".model2" };
	for (const auto& entry: entries)
	{
		base_offset += sizeof(int8_t) + entry.name.find_last_of('.') + model2.size() + 2 * sizeof(int32_t);
	}

	// reserve header space.
	fseek(dst_pack.get(), base_offset, SEEK_SET);
	
	// offset for each destination type.
	std::vector<std::tuple<int32_t, int32_t>> dst_offset;
	dst_offset.resize(entries.size());
		
	{
		std::vector<uint8_t> mesh_data;
		std::vector<uint8_t> optimized;
		for (auto i = 0; i < entries.size(); ++i)
		{
			const auto& entry = entries[i];
			fseek(src_pack.get(), entry.position, SEEK_SET);
			mesh_data.resize(std::max(static_cast<size_t>(entry.size), mesh_data.size()));
			fread(mesh_data.data(), entry.size, 1, src_pack.get());
			
			std::string_view ext{ entry.name.data() + entry.name.find_last_of('.') };
			auto length = ext == ".obj" ? optimize_obj(mesh_data, optimized) : optimize_model(mesh_data, optimized);

			if (!length)
				return -1;

			fwrite(optimized.data(), length, 1, dst_pack.get());
			// Update entry information (replace extension, fixup size, position)
			auto position = static_cast<int32_t>(base_offset);
			dst_offset[i] = std::make_tuple(position, static_cast<int32_t>(length));
			base_offset += length;
		}

		// Second pass: write the header information

		fseek(dst_pack.get(), 2 * sizeof(int32_t), SEEK_SET);
		for (auto e = 0; e < entries.size(); ++e)
		{
			auto entry_name = entries[e].name.substr(0, entries[e].name.find_last_of('.')) + model2.data();
			auto&& [position, size] = dst_offset[e];
			write_string(dst_pack, entry_name);
			write_int32(dst_pack, position);
			write_int32(dst_pack, size);
		}
	}

	return 0;
}

int process_mesh(std::string_view src_name, const file_ptr& src_file)
{
	std::vector<uint8_t> mesh_data;
	fseek(src_file.get(), 0, SEEK_END);
	mesh_data.resize(ftell(src_file.get()));
	fseek(src_file.get(), 0, SEEK_SET);
	fread(mesh_data.data(), mesh_data.size(), 1, src_file.get());

	auto basename = src_name.substr(0, src_name.find_last_of('.'));

	std::vector<uint8_t> optimized;
	auto ext = src_name.substr(src_name.find_last_of('.'));
	auto length = ext == ".obj" ? optimize_obj(mesh_data, optimized) : optimize_model(mesh_data, optimized);
	if (length == 0)
		return -1;


	auto dst_name = std::string{ basename } + ".model2";
	auto dst_mesh = open_file(dst_name.data(), "wb");
	fwrite(optimized.data(), length, 1, dst_mesh.get());

	return 0;
}

// <single|pack> <src>
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fputs("invalid number of arguments" LF, stderr);
		return -1;
	}

	const std::string_view run_type{argv[1]};
	if (!(run_type == "single" || run_type == "pack"))
	{
		fprintf(stderr, "Unknown run type: %s" LF, run_type.data());
		return -1;
	}
	
	const std::string_view src{argv[2]};

	auto src_file = open_file(src.data(), "rb");

	if (!src_file)
	{
		fprintf(stderr, "Failed to open source file %s" LF, src.data());
		return -1;
	}

    return run_type == "pack" ? process_pack(src, src_file) : process_mesh(src, src_file);
}