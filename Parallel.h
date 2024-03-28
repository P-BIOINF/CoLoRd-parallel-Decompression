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
	std::string m_arguments{ "colord decompress"};
	std::filesystem::path m_path{}; // filesystem? czm blad mi tu wyskakuje
	std::filesystem::path m_input{};
	std::filesystem::path m_output{};
	std::int64_t m_maxNumberOfFilesDecomp{ 0 };
	std::filesystem::path m_extension{};
public:
	Parallel() = default;

	/**
	 * \brief parses command arguments
	 * \param argc self-explanatory
	 * \param argv self-explanatory
	 * \return returns status. Status::ready if the program is ready to use
	 */
	Status parseArguments(const int argc, char** argv);

	[[nodiscard]] Status getStatus() const
	{
		return m_status;
	}
};

#endif