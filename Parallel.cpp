#include "Parallel.h"

#include <execution>

#include "Timer.h"
#include <iostream>
#include <string>
#include <fstream>
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
			m_extension = std::filesystem::path(m_input).extension();
		}
		else if (param == "--output")
		{
			m_output = argv[++i];
			m_output.remove_filename();
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