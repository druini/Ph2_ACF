#include "FilesystemUtils.h"
#include <regex>
#include <boost/range/iterator_range.hpp>

namespace fs = boost::filesystem;

namespace FSUtils {
    fs::path getAvailableFilePath(const fs::path& path) {
        auto availablePath = detail::getAvailablePath(path);
        if (availablePath.has_parent_path())
            fs::create_directories(availablePath.parent_path());
        return availablePath;
    }
    
    fs::path getAvailableDirectoryPath(const fs::path& path) {
        auto availablePath = detail::getAvailablePath(path);
        fs::create_directories(availablePath);
        return availablePath;
    }

    namespace detail {
        fs::path getAvailablePath(const fs::path& path) {
            if (!fs::exists(path))
                return path;
            else {
                std::ostringstream runNumberRegexStream;
                runNumberRegexStream << path.stem().string() << "\\(([0-9]+)\\)";
                if (path.has_extension())
                    runNumberRegexStream << "\\" << path.extension().string();
                std::regex runNumberRegex(runNumberRegexStream.str());
                std::vector<size_t> existingRunNumbers{{0}};
                auto parent_path = path.has_parent_path() ? path.parent_path() : fs::path(".");
                for (auto& entry : boost::make_iterator_range(fs::directory_iterator(parent_path), {})) {
                    std::string filename = entry.path().filename().string();
                    std::smatch m;
                    if (std::regex_match(filename, m, runNumberRegex) && m.size() > 1) {
                        existingRunNumbers.push_back(std::stoul(m[1]));
                    }
                }
                std::sort(existingRunNumbers.begin(), existingRunNumbers.end());
                existingRunNumbers.erase(std::unique(existingRunNumbers.begin(), existingRunNumbers.end()), existingRunNumbers.end());
                auto it = std::adjacent_find(existingRunNumbers.begin(), existingRunNumbers.end(), [] (auto a, auto b) { return b != a + 1; });
                size_t runNumber;
                if (it == existingRunNumbers.end())
                    runNumber = existingRunNumbers.back() + 1;
                else
                    runNumber = *it + 1;
                std::stringstream ss;
                ss << path.stem().string() << '(' << runNumber << ')' << path.extension().string();
                if (path.has_parent_path())
                    return (parent_path / ss.str());
                else
                    return ss.str();
            }
        }
    }
}