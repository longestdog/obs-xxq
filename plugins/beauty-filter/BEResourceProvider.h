#ifndef BERESOURCEPROVIDER_H
#define BERESOURCEPROVIDER_H

#include <string>


class BEResourceProvider
{
public:
    BEResourceProvider();
    ~BEResourceProvider() {}

    static BEResourceProvider& getInstance();
    void initialize(const std::string& appDir);
public:
    std::string& licenseDirPath();
    std::string& modelDirPath();
    std::string& filterPath();
    std::string& stickerPath();
    std::string& composerNodePath();
    std::string& getFaceCacheDir();

    void setResourceDir(const std::string& resDir);

private:
    std::string m_modelDir;
    std::string m_licenseDir;
    std::string m_filterDir;
    std::string m_composerNodeDir;
    std::string m_stickerDir;
    std::string m_resourceDir;
    std::string m_faceCacheDir;
};

#endif // BERESOURCEPROVIDER_H
