#ifndef FILESYSTEMUTILS_H
#define FILESYSTEMUTILS_H

#include <boost/filesystem.hpp>

namespace FSUtils {
    boost::filesystem::path getAvailableFilePath(const boost::filesystem::path& path);
    boost::filesystem::path getAvailableDirectoryPath(const boost::filesystem::path& path);

    namespace detail {
        boost::filesystem::path getAvailablePath(const boost::filesystem::path& path);
    }
}

#endif