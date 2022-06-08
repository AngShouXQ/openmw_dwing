#ifndef COMPONENTS_FILES_ANDROIDPATH_H
#define COMPONENTS_FILES_ANDROIDPATH_H

#if defined(__ANDROID__)

#include <filesystem>
/**
 * \namespace Files
 */


namespace Files
{

struct AndroidPath
{
    AndroidPath(const std::string& application_name);
    

    /**
     * \brief Return path to the user directory.
     */
    std::filesystem::path getUserConfigPath() const;

    std::filesystem::path getUserDataPath() const;

    /**
     * \brief Return path to the global (system) directory where config files can be placed.
     */
    std::filesystem::path getGlobalConfigPath() const;

    /**
     * \brief Return path to the runtime configuration directory which is the
     * place where an application was started.
     */
    std::filesystem::path getLocalPath() const;

    /**
     * \brief Return path to the global (system) directory where game files can be placed.
     */
    std::filesystem::path getGlobalDataPath() const;

    /**
     * \brief
     */
    std::filesystem::path getCachePath() const;

    std::filesystem::path getInstallPath() const;
};

} /* namespace Files */

#endif /* defined(__Android__) */

#endif /* COMPONENTS_FILES_ANDROIDPATH_H */
