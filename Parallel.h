#ifndef PARALLEL_H
#define PARALLEL_H
#include <string>
#include <vector>
#include <chrono>
#include <atomic>
#include <utility>
#include <filesystem>

enum class Status
{
	ready,
	not_ready,
	failed,
	max_status,
};

class Parallel
{
	bool m_api{ false };
	std::string m_arguments{};
	std::int64_t m_count{ -1 };
	std::int64_t m_threads{ -1 };
	std::filesystem::path m_input{};
	std::filesystem::path m_output{};
	std::filesystem::path m_extension{};
	Status m_status{ Status::not_ready };
	std::filesystem::path m_colordPath{};
	std::atomic<std::int64_t> m_pathIndex{ 0 };
	std::vector<std::filesystem::path> m_directories{};
	std::chrono::high_resolution_clock::time_point m_decompression_end{};
	std::chrono::high_resolution_clock::time_point m_decompression_start{};

	void handleDecompression();
	void systemDecompression(const std::filesystem::path& path, std::int64_t current);
	//void API_colordDecompression(const std::filesystem::path& path, std::int64_t current);

public:
	Parallel() = default;

	Status parseArguments(const int argc, char** argv);

	void getFilesToDecomp();

	void decompress();

	[[nodiscard]] Status getStatus() const
	{
		return m_status;
	}

	void generateOutput();
};

#endif