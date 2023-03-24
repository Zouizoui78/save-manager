#include "wintools.hpp"

#include <shlobj.h>
#include <shlwapi.h>

namespace fs = std::filesystem;

fs::path get_documents_path() {
    std::string path;

    PWSTR pPath = NULL;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &pPath) == S_OK)
    {
        int wlen = lstrlenW(pPath);
        int len = WideCharToMultiByte(CP_ACP, 0, pPath, wlen, NULL, 0, NULL, NULL);
        if (len > 0)
        {
            path.resize(len+1);
            WideCharToMultiByte(CP_ACP, 0, pPath, wlen, &path[0], len, NULL, NULL);
            path[len] = '\\';
        }
        CoTaskMemFree(pPath);
    }

    return fs::path(path);
}