#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <boost/filesystem.hpp>

namespace FSUtils {
    std::string getAvailablePath(const boost::filesystem::path& path);
}

#endif