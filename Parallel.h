#ifndef PARALLEL_H
#define PARALLEL_H

#include <utility>
#include <string>
#include <vector>
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
	Status m_status{ Status::not_ready };
	std::filesystem::path m_path{}; 
	std::filesystem::path m_input{};
	std::filesystem::path m_output{};
	std::int64_t m_maxNumberOfFilesDecomp{ 0 };
	std::filesystem::path m_extension{};
	std::vector<std::filesystem::path> m_directories{};

private:
	void handleDecompression(const std::filesystem::path& path);
public:
	Parallel() = default;

	Status parseArguments(const int argc, char** argv);

	void getFilesToDecomp();

	void decompress();

	[[nodiscard]] Status getStatus() const
	{
		return m_status;
	}
};

#endif