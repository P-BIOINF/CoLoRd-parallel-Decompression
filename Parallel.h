﻿#ifndef PARALLEL_H
#define PARALLEL_H
#include <utility>
#include <string>
#include <vector>
#include <filesystem>
#include <atomic>

enum class Status
{
	ready,
	not_ready,
	failed,
	max_status,
};

class Parallel
{
	int m_count{ -1 };
	int m_threads{ -1 };
	bool m_api{ false };
	std::string m_arguments{};
	std::filesystem::path m_input{};
	std::filesystem::path m_output{};
	std::atomic<int> m_pathIndex{ 0 };
	std::filesystem::path m_extension{};
	Status m_status{ Status::not_ready };
	std::filesystem::path m_colordPath{};
	std::vector<std::filesystem::path> m_directories{};

	void handleDecompression();
	void systemDecompression(const std::filesystem::path& path, int current);
	void API_colordDecompression(const std::filesystem::path& path, int current);

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