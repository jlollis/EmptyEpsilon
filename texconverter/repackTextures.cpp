#include "astcenc.h"

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_TGA
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

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
#else
#define bswap32 __builtin_bswap32
#endif

#include <bitset>
enum OutputFlag : size_t
{
	OutputASTC = 0,
	OutputDXT,
	OutputCount
};

static constexpr uint32_t GL_COMPRESSED_RGB_S3TC_DXT1_EXT = 0x83F0;
static constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
static constexpr uint32_t GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;
static constexpr uint32_t GL_COMPRESSED_RGBA_ASTC_4x4_KHR = 0x93B0;

static constexpr uint32_t GL_RGB = 0x1907;
static constexpr uint32_t GL_RGBA = 0x1908;


using file_ptr = std::unique_ptr<FILE, decltype(&fclose)>;
using astcenc_context_ptr = std::unique_ptr<astcenc_context, decltype(&astcenc_context_free)>;

using image_ptr = std::unique_ptr<uint8_t, decltype(&stbi_image_free)>;

file_ptr open_file(const char* filename, const char* mode) noexcept
{
	return {fopen(filename, mode), &fclose};
}

astcenc_context_ptr create_context(astcenc_config& config, uint32_t threads)
{
	astcenc_context* context{};
	auto status = astcenc_context_alloc(&config, threads, &context);
	if (status != ASTCENC_SUCCESS)
	{
		fprintf(stderr, "astcenc context creation: %s" LF, astcenc_get_error_string(status));
	}

	return { context, &astcenc_context_free };
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

struct KtxHeader
{
	uint8_t magic[12]{ 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
	uint32_t endianness{ 0x04030201 };
	uint32_t type{}; 
	uint32_t type_size{ 1 };
	uint32_t format{};
	uint32_t internal_format{};
	uint32_t base_internal_format{ GL_RGBA };
	uint32_t pixel_width{};
	uint32_t pixel_height{};
	uint32_t pixel_depth{};
	uint32_t array_elements{};
	uint32_t faces{ 1 };
	uint32_t mips{ 1 };
	uint32_t kv_bytes{};
};

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

static constexpr std::array suffixes{ "-astc", "-dxt" };

void write_ktx(uint32_t format, uint32_t width, uint32_t height, const void* image, uint32_t image_length, const file_ptr& dst)
{
	KtxHeader header;
	header.internal_format = format;
	header.base_internal_format = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? GL_RGB : GL_RGBA;
	header.pixel_width = width;
	header.pixel_height = height;
	fwrite(&header, sizeof(header), 1, dst.get());
	fwrite(&image_length, sizeof(image_length), 1, dst.get());
	fwrite(image, image_length, 1, dst.get());
}

size_t compress_dxt(const image_ptr& image_data, uint32_t channels, uint32_t width, uint32_t height, std::vector<uint8_t>& compressed)
{
	auto block_size = channels % 2 == 0 ? 16 : 8;
	auto blocks_x = (width + 4 - 1) / 4;
	auto blocks_y = (height + 4 - 1) / 4;
	auto compressed_size = static_cast<size_t>(block_size * blocks_x * blocks_y);
	compressed.resize(std::max(compressed.size(), compressed_size));

	auto pixels = reinterpret_cast<const uint32_t*>(image_data.get());
	auto output = compressed.data();

	// Traverse each 4x4 block from the source image.
	for (auto by = 0; by < blocks_y; ++by)
	{
		auto start_y = 4 * by;
		for (auto bx = 0; bx < blocks_x; ++bx)
		{
			// Each block is 4x4, with 4 components (RGBA).
			std::array<uint32_t, 4 * 4> block{};
			// Load up the block.
			for (auto y = 0; y < 4; ++y)
				for (auto x = 0; x < 4; ++x)
				{
					auto pixel_offset = (start_y + y) * width + 4 * bx + x;
					if (pixel_offset < width * height)
						block[y * 4 + x] = pixels[pixel_offset];
				}


			stb_compress_dxt_block(output, reinterpret_cast<const uint8_t*>(block.data()), channels % 2 == 0 ? 1 : 0, STB_DXT_HIGHQUAL);
			output += block_size;
		}
	}

	return compressed_size;
}

struct AstcDetails
{
	astcenc_config config{};

	std::vector<std::thread> workers;
	std::vector<astcenc_error> errors;
	
	
	astcenc_context_ptr context{nullptr, &astcenc_context_free};
	
	explicit AstcDetails(size_t thread_count)
		:workers(thread_count - 1)
		, errors(thread_count)
	{
		for (auto& error : errors)
			error = ASTCENC_SUCCESS;
	}

	~AstcDetails() = default;
	AstcDetails(const AstcDetails&) = delete;
	AstcDetails& operator=(const AstcDetails&) = delete;
	AstcDetails(AstcDetails&&) = default;
	AstcDetails& operator=(AstcDetails&&) = default;
};

AstcDetails setup_astc_compression(bool initialize)
{
	AstcDetails details{ std::thread::hardware_concurrency() };
	
	if (initialize)
	{
		auto status = astcenc_config_init(ASTCENC_PRF_LDR, 4, 4, 1, ASTCENC_PRE_MEDIUM, ASTCENC_FLG_SELF_DECOMPRESS_ONLY, &details.config);
		if (status == ASTCENC_SUCCESS)
		{
			details.context = create_context(details.config, details.errors.size());

		}
		else
		{
			fprintf(stderr, "astcenc config init: %s" LF, astcenc_get_error_string(status));
		}
	}

	return details;
}

size_t compress_astc(const image_ptr& image_data, uint32_t width, uint32_t height, std::vector<uint8_t>& compressed, AstcDetails& details)
{
	std::array<void*, 1> data{ image_data.get() };
	astcenc_image raw
	{
		static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u,
		ASTCENC_TYPE_U8, data.data()
	};

	size_t compressed_size = ((raw.dim_x + 4 - 1) / 4) * (raw.dim_y + 4 - 1) / 4 * 16;

	compressed.resize(std::max(compressed.size(), compressed_size));

	auto worker_function = [&raw, &compressed, compressed_size, &details](auto worker)
	{
		astcenc_swizzle swizzle{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A };
		details.errors[worker] = astcenc_compress_image(details.context.get(), &raw, swizzle, compressed.data(), compressed_size, worker);
	};

	for (auto worker = 0; worker < details.workers.size(); ++worker)
	{
		details.workers[worker] = std::thread(worker_function, worker + 1);
	}

	worker_function(0);

	for (auto& worker : details.workers)
		worker.join();

	astcenc_compress_reset(details.context.get());
	return compressed_size;
}

astcenc_error astc_check_errors(const AstcDetails& details, std::string_view name)
{
	for (auto worker = 0; worker < details.errors.size(); ++worker)
	{
		if (details.errors[worker] != ASTCENC_SUCCESS)
		{
			fprintf(stderr, "[%s][%d]: %s" LF, name.data(), worker, astcenc_get_error_string(details.errors[worker]));
			return details.errors[worker];
		}
	}

	return ASTCENC_SUCCESS;
}

int process_pack(std::string_view src_name, file_ptr& src_pack, std::bitset<OutputCount> outputs)
{
	auto version = read_int32(src_pack);

	if (version != 0)
	{
		fprintf(stderr, "version unsupported %d." LF, version);
		return -1;
	}
	
	std::array<file_ptr, 2> dst_packs{ file_ptr{nullptr, &fclose}, file_ptr{nullptr, &fclose} };
	{
		const auto basename = src_name.substr(0, src_name.find_last_of('.'));
		
		for (auto output = 0; output < OutputCount; ++output)
		{
			if (outputs[output])
			{
				auto dst_pack = std::string{ basename } + suffixes[output] + ".pack";
				dst_packs[output] = open_file(dst_pack.data(), "wb");

				if (!dst_packs[output])
				{
					fprintf(stderr, "Failed to open destination pack %s" LF, dst_pack.data());
					return -1;
				}
			}
		}
	}	

	std::vector<Entry> entries;
	auto file_count = read_int32(src_pack);
	entries.reserve(file_count);
	
	constexpr std::array<std::string_view, 4> valid_extensions{ ".png", ".jpg", ".bmp", ".tga" };
	for (auto i = 0; i < file_count; ++i)
	{
		Entry entry{ src_pack };
		{
			auto found = entry.name.find_last_of('.');

			if (found != std::string_view::npos && (entry.name.size() - found) == 5)
			{
				std::string_view ext{ entry.name.data() + found, 4 };
				auto candidate = std::find(std::begin(valid_extensions), std::end(valid_extensions), ext);
				if (candidate != std::end(valid_extensions))
				{
					entries.emplace_back(std::move(entry));
				}
			}
		}
	}

	for (auto& dst_pack: dst_packs)
	{
		if (dst_pack)
		{
			write_int32(dst_pack, 0); // version
			write_int32(dst_pack, static_cast<int32_t>(entries.size())); // filecount
		}
	}

	// ASTC 'context'.
	auto details = setup_astc_compression(outputs[OutputASTC]);

	if (outputs[OutputASTC] && !details.context)
		return -1;
	
	// Compute header size.
	std::array<size_t, OutputCount> base_offsets{};
	for (auto& base_offset: base_offsets)
		base_offset = 2 * sizeof(int32_t);

	for (const auto& entry: entries)
	{
		for (auto& base_offset: base_offsets)
			base_offset += sizeof(int8_t) + entry.name.size() + 2 * sizeof(int32_t);
	}

	// reserve header space.
	for (auto& dst_pack: dst_packs)
	{
		if (dst_pack)
			fseek(dst_pack.get(), base_offsets[0], SEEK_SET);
	}
	
	// offset for each destination type.
	std::array<std::vector<std::tuple<int32_t, int32_t>>, 2> dst_offsets;
	for (auto output = 0; output < OutputCount; ++output)
	{
		if (outputs[output])
		{
			dst_offsets[output].resize(entries.size());
		}
	}

	{
		std::vector<uint8_t> png_data;
		std::vector<uint8_t> compressed;
		for (auto i = 0; i < entries.size(); ++i)
		{
			const auto& entry = entries[i];
			fseek(src_pack.get(), entry.position, SEEK_SET);
			png_data.resize(std::max(static_cast<size_t>(entry.size), png_data.size()));
			fread(png_data.data(), entry.size, 1, src_pack.get());
			
			int width{}, height{}, channels{};
			image_ptr image_data{ stbi_load_from_memory(png_data.data(), entry.size, &width, &height, &channels, STBI_rgb_alpha), &stbi_image_free };
			if (!image_data)
			{
				fprintf(stderr, "[%s][loading]: %s" LF, entry.name.data(), stbi_failure_reason());
				exit(-1);
			}

			if (outputs[OutputDXT])
			{
				auto compressed_size = compress_dxt(image_data, channels, width, height, compressed);

				// Write the KTX texture.
				auto& dst_pack = dst_packs[OutputDXT];
				write_ktx(
					channels % 2 == 0 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
					width, height, compressed.data(), compressed_size, dst_pack
				);

				// Update entry information (replace extension, fixup size, position)
				auto position = static_cast<int32_t>(base_offsets[OutputDXT]);
				auto size = static_cast<int32_t>(compressed_size + sizeof(KtxHeader) + sizeof(uint32_t));
				dst_offsets[OutputDXT][i] = std::make_tuple(position, size);
				base_offsets[OutputDXT] += size;
			}

			if (outputs[OutputASTC])
			{
				auto compressed_size = compress_astc(image_data, width, height, compressed, details);
				if (astc_check_errors(details, entry.name) != ASTCENC_SUCCESS)
					return -1;

				// Write the KTX texture.
				auto& dst_pack = dst_packs[OutputASTC];
				write_ktx(GL_COMPRESSED_RGBA_ASTC_4x4_KHR, width, height, compressed.data(), compressed_size, dst_pack);

				// Update entry information (replace extension, fixup size, position)
				auto position = static_cast<int32_t>(base_offsets[OutputASTC]);
				auto size = static_cast<int32_t>(compressed_size + sizeof(KtxHeader) + sizeof(uint32_t));
				dst_offsets[OutputASTC][i] = std::make_tuple(position, size);
				base_offsets[OutputASTC] += size;
			}
		}

		// Second pass: write the header information
		for (auto i = 0; i < OutputCount; ++i)
		{
			if (outputs[i])
			{
				fseek(dst_packs[i].get(), 2 * sizeof(int32_t), SEEK_SET);
				for (auto e = 0; e < entries.size(); ++e)
				{
					auto entry_name = entries[e].name.substr(0, entries[e].name.find_last_of('.')) + suffixes[i] + ".ktx";
					auto&& [position, size] = dst_offsets[i][e];
					write_string(dst_packs[i], entry_name);
					write_int32(dst_packs[i], position);
					write_int32(dst_packs[i], size);
				}
			}
		}
	}

	return 0;
}

int process_image(std::string_view src_name, const file_ptr& src_file, std::bitset<OutputCount> outputs)
{
	int width{}, height{}, channels{};
	image_ptr image_data{ stbi_load_from_file(src_file.get(), &width, &height, &channels, STBI_rgb_alpha), &stbi_image_free };
	if (!image_data)
	{
		fprintf(stderr, "[%s][loading]: %s" LF, src_name.data(), stbi_failure_reason());
		return -1;
	}

	auto basename = src_name.substr(0, src_name.find_last_of('.'));

	std::vector<uint8_t> compressed;

	if (outputs[OutputDXT])
	{
		auto compressed_size = compress_dxt(image_data, channels, width, height, compressed);

		// Write the KTX texture.
		auto dxt_name = std::string{ basename } + suffixes[OutputDXT] + ".ktx";
		auto dxt_ktx = open_file(dxt_name.data(), "wb");

		write_ktx(
			channels % 2 == 0 ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
			width, height, compressed.data(), compressed_size, dxt_ktx
		);
	}

	if (outputs[OutputASTC])
	{
		// ASTC 'context'.
		auto details = setup_astc_compression(outputs[OutputASTC]);

		if (!details.context)
			return -1;

		auto compressed_size = compress_astc(image_data, width, height, compressed, details);
		if (astc_check_errors(details, src_name) != ASTCENC_SUCCESS)
			return -1;

		// Write the KTX texture.
		auto astc_name = std::string{ basename } + suffixes[OutputASTC] + ".ktx";
		auto astc_ktx = open_file(astc_name.data(), "wb");
		write_ktx(GL_COMPRESSED_RGBA_ASTC_4x4_KHR, width, height, compressed.data(), compressed_size, astc_ktx);
	}

	return 0;
}

// <single|pack> <src> [astc] [dxt]
int main(int argc, char* argv[])
{
	if (!(argc == 4 || argc == 5))
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

	std::bitset<OutputCount> outputs{};

	for (auto i = 3; i < argc; ++i)
	{
		const std::string_view desired{argv[i]};
		if (desired == "astc")
		{
			outputs.set(OutputASTC, true);
		}
		else if (desired == "dxt")
		{
			outputs.set(OutputDXT, true);
		}
		else
		{
			fprintf(stderr, "Unknown output type %s" LF, desired.data());
			return -1;
		}
	}

    return run_type == "pack" ? process_pack(src, src_file, outputs) : process_image(src, src_file, outputs);
}