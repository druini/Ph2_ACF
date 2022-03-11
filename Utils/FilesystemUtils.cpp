#include "FilesystemUtils.h"
#include <regex>
#include <boost/range/iterator_range.hpp>


namespace FSUtils {
    std::string getAvailablePath(const boost::filesystem::path& path) {
        if (!boost::filesystem::exists(path)) {
            if (!path.parent_path().empty())
                boost::filesystem::create_directories(path.parent_path());
            return path.string();
        }
        else {
            std::regex runNumberRegex(path.stem().string() + "\\(([0-9]+)\\)\\" + path.extension().string());
            std::vector<size_t> existingRunNumbers{{0}};
            for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(path.parent_path()), {})) {
                if (boost::filesystem::is_regular_file(entry.status())) {
                    std::string filename = entry.path().filename().string();
                    std::smatch m;
                    if (std::regex_match(filename, m, runNumberRegex) && m.size() > 1) {
                        std::cout << "filename: " << filename << std::endl;
                        existingRunNumbers.push_back(std::stoul(m[1]));
                    }
                }
            }
            std::sort(existingRunNumbers.begin(), existingRunNumbers.end());
            existingRunNumbers.erase(std::unique(existingRunNumbers.begin(), existingRunNumbers.end()), existingRunNumbers.end());
            std::cout << "existingRunNumbers:" << std::endl;
            for (const auto& runNo : existingRunNumbers)
                std::cout << runNo << std::endl;
            auto it = std::adjacent_find(existingRunNumbers.begin(), existingRunNumbers.end(), [] (auto a, auto b) { return b != a + 1; });
            size_t runNumber;
            if (it == existingRunNumbers.end())
                runNumber = existingRunNumbers.back() + 1;
            else
                runNumber = *it + 1;
            std::stringstream ss;
            ss << path.stem().string() << '(' << runNumber << ')' << path.extension().string();
            return (path.parent_path() / ss.str()).string();
        }
    }
}