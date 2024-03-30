#include "Parallel.h"

#include "Timer.h"
#include <string>
#include <filesystem>
#include <unordered_set>

Status Parallel::parseArguments(const int argc, char** argv)
{
	const std::unordered_set<std::string> setOfOneParam{ "-G", "--reference-genome", "-s","-v","--verbose","-h", "--help" };
	for (int i{ 0 }; i < argc; ++i)
	{
		if (std::string param{ argv[i] }; param == "--input")
		{
			m_input = argv[++i];
			if (!std::filesystem::directory_entry(m_input).exists())
			{
				m_status = Status::not_ready;

				return getStatus();
			}
		}
		else if (param == "--output")
		{
			m_output = argv[++i];
			m_output.remove_filename();
		}
		else if(param == "--extension")
		{
			m_extension = argv[++i];
		}
		else if (param == "--count")
		{
			try
			{
				m_maxNumberOfFilesDecomp = std::stoul(argv[++i]);
			}
			catch (...)
			{
				m_status = Status::failed;

				return getStatus();
			}
		}
		else if (param == "--colord")
		{
			m_path = argv[++i];
		}
		else if (setOfOneParam.contains(param))
		{
			m_arguments.append(" " + param);
		}
	}
	m_arguments.append(" ");
	if (m_path.empty() && m_maxNumberOfFilesDecomp == 0)
	{
		m_status = Status::not_ready;

		return getStatus();
	}

	/**getInputStream().open(getInput());

	std::filesystem::create_directory(getOutput());
	if (!getInputStream())
	{
		m_status = Status::failed;

		return getStatus();
	}
	*/



	m_status = Status::ready;

	return getStatus();
}

void Parallel::getFilesToDecomp()
{
	for(const auto& entry: std::filesystem::directory_iterator(m_input))
	{
		if(entry.path().extension() == m_extension)
		{
			m_directories.emplace_back(entry);
		}
	}
}


//kazdy watek bedzie to wywowylal
void Parallel::decompress()
{
	for(const auto& path : m_directories)
	{
		handleDecompression(path);
	}
}

void Parallel::handleDecompression(const std::filesystem::path& path)
{
	static int current{0};
	std::filesystem::path tempOutput(m_output);
	tempOutput.append(std::to_string(current++));
	const std::string temp{ " " + path.string() + " decompress " + path.string() + tempOutput.string()};
	std::system(temp.c_str());
}
