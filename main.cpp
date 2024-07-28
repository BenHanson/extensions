// Find all file extensions given a path
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <set>
#include <wildcardtl/wildcard.hpp>

namespace fs = std::filesystem;
using string_set = std::set<std::string, std::less<>>;

bool g_recursive = false;
const char* g_path;
std::pair<std::vector<wildcardtl::wildcard>,
	std::vector<wildcardtl::wildcard>> g_exclude;

bool exclude(const fs::path& path)
{
	try
	{
		const auto pathname = path.string();
		bool skip = !g_exclude.second.empty();

		for (const auto& wc : g_exclude.second)
		{
			if (!wc.match(pathname))
			{
				skip = false;
				break;
			}
		}

		if (!skip)
		{
			for (const auto& wc : g_exclude.first)
			{
				if (wc.match(pathname))
				{
					skip = true;
					break;
				}
			}
		}

		return skip;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return true;
	}
}

void insert_ext(const fs::path& path, string_set& exts)
{
	const std::string ext = path.extension().string();

	if (!ext.empty() && !exclude(path))
	{
		exts.insert(ext);
	}
}

[[nodiscard]] string_set find_exts()
{
	string_set exts;

	if (g_recursive)
	{
		for (auto iter = fs::recursive_directory_iterator(g_path,
			fs::directory_options::skip_permission_denied),
			end = fs::recursive_directory_iterator(); iter != end; ++iter)
		{
			insert_ext(iter->path(), exts);
		}
	}
	else
	{
		for (auto iter = fs::directory_iterator(g_path,
			fs::directory_options::skip_permission_denied),
			end = fs::directory_iterator(); iter != end; ++iter)
		{
			insert_ext(iter->path(), exts);
		}
	}

	return exts;
}

bool is_windows()
{
#ifdef WIN32
	return true;
#else
	return false;
#endif
}

std::vector<std::string_view> split(const char* str, const char c)
{
	std::vector<std::string_view> ret;
	const char* first = str;
	std::size_t count = 0;

	for (; *str; ++str)
	{
		if (*str == c)
		{
			count = str - first;

			if (count > 0)
				ret.emplace_back(first, count);

			first = str + 1;
		}
	}

	count = str - first;

	if (count > 0)
		ret.emplace_back(first, count);

	return ret;
}

void process_params(int argc, char* argv[])
{
	for (int i = 0; i < argc; ++i)
	{
		const char* arg = argv[i];

		if (*arg == '-')
		{
			if (strcmp(arg, "-r") == 0 || strcmp(arg, "--recursive") == 0)
				g_recursive = true;
			else if (strcmp(arg, "-x") == 0 || strcmp(arg, "--exclude") == 0)
			{
				++i;

				if (i < argc)
				{
					const auto pathnames = split(argv[i], ';');

					for (const auto& p : pathnames)
					{
						arg = p.data();

						if (*arg == '!')
							g_exclude.second.emplace_back(arg, arg + p.size(),
								is_windows());
						else
							g_exclude.first.emplace_back(arg, arg + p.size(),
								is_windows());
					}
				}
				else
					throw std::runtime_error("Missing wildcard following -exclude.");
			}
			else
				throw std::runtime_error(std::format("Unknown switch: {}", arg));
		}
		else
			g_path = arg;
	}
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 2)
		{
			std::cerr << "USAGE: extensions [--recursive|-r] "
				"[(--exclude|-x) <';' separated list of wildcards>] <path>\n";
			return 1;
		}

		process_params(argc, argv);
		
		for (const auto& ext : find_exts())
		{
			std::cout << ext << '\n';
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << '\n';
		return 1;
	}

	return 0;
}
