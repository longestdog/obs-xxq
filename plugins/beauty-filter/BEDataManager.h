#ifndef BEDATAMANAGER_H
#define BEDATAMANAGER_H

#include "BEResourceModel.h"
#include <map>

class BEDataManager
{
public:
    BEDataManager();
    ~BEDataManager();

    void init(BEEffectType effectType);
    void clear();

    BEComposerNodeModel* getComposerNodeModel(int64_t type);

public:
    void setEffectEnable(bool v);
    bool isEffectEnable();

    void setLocalLanguage(int localType);
    std::map<int64_t, float>& getDefaultEffects();

private:
    void initDefaultEffects();
    void initBeautyFaceModel();
    void initBeautyReshapeModel();
    void initBeautyBodyModel();
    void initMakeupModel();

    void initMakeupLipModel();
    void initMakeupBlusherModel();
    void initMakeupEyelashModel();
    void initMakeupPupilModel();
    void initMakeupHairModel();
    void initMakeupEyeshadowModel();
    void initMakeupEyebrowModel();
    void initMakeupFacialModel();
    void initMakeupEyeLightModel();
    void initMakeupEyePlumpModel();

    void initQualityModel();
    void initBlackTechModel();

    void initStyleMakeupModel();

    void initBackgroundBlurModel();

    void registerComposerNode(int64_t mainId, int64_t subId, const std::string& nodePath, std::vector<TagInfo> tagArray);

    std::string getBeautyPath();
    std::string getReshapePath();


private:
    BEEffectType m_effectType;
    std::map<int64_t, BEComposerNodeModel*> m_mapComposerNode;

    bool m_bEffectEnable{true};

    std::map<int64_t, float> m_liteDefaultEffects;
    std::map<int64_t, float> m_standardDefaultEffects;

    int m_localLanguage;  // 当前系统语言, 0 中文 1 英文
};

#endif // BEDATAMANAGER_H
