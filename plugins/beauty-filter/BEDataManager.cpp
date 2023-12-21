#include "bedatamanager.h"
#include "BEResourceProvider.h"

BEDataManager::BEDataManager()
{
    m_effectType = BEEffectType::BEEffectTypeLite;
}


BEDataManager::~BEDataManager() {

}

void BEDataManager::init(BEEffectType effectType) {
    m_effectType = effectType;
    initBeautyFaceModel();
    initBeautyReshapeModel();
    initBeautyBodyModel();
    initMakeupModel();
    initStyleMakeupModel();
    initQualityModel();
    initBlackTechModel();
    initBackgroundBlurModel();
    initDefaultEffects();
}

void BEDataManager::clear() {
    m_mapComposerNode.clear();
}

void BEDataManager::initDefaultEffects() {
    m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceSmooth, 0.5));
    if (m_localLanguage == 0) {
        m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceWhiten, 0.35));
    }

    m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceSharp, 0.3));
    //m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceClarity, 0.2));
    m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyReshapeFaceOverall, 0.35));
    m_liteDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyReshapeEyeSize, 0.35));

    m_standardDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceSmooth, 0.65));
    if (m_localLanguage == 0) {
        m_standardDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceWhiten, 0.35));
    }
    m_standardDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyFaceSharp, 0.25));
    m_standardDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyReshapeFaceOverall, 0.75));
    m_standardDefaultEffects.insert(std::make_pair((int64_t)BEType::BETypeBeautyReshapeEyeSize, 0.65));
}

void BEDataManager::initBeautyFaceModel() {
    std::string beautyPath = BEResourceProvider::getInstance().composerNodePath() + getBeautyPath();
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceSmooth,
                         beautyPath,
                         std::vector<TagInfo>{TagInfo("smooth", 0.0, 0)});
    
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceSharp,
                         beautyPath,
                         std::vector<TagInfo>{TagInfo("sharp", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceClarity,
                         beautyPath,
                         std::vector<TagInfo>{TagInfo("clear", 0.0, 0)});
    std::string whitenPath, whitenFenbaiPath, whitenLengbaiPath;
    if(m_effectType == BEEffectType::BEEffectTypeLite) {
        whitenPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_lite";
        whitenFenbaiPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_fenbai_lite";
        whitenLengbaiPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_lengbai_lite";
    }
    else {
        whitenPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_standard";
        whitenFenbaiPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_fenbai_standard";
        whitenLengbaiPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_IOS_lengbai_standard";
    }
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceWhiten,
                         whitenPath,
                         std::vector<TagInfo>{TagInfo("whiten", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceWhitenFenbai,
                         whitenFenbaiPath,
                         std::vector<TagInfo>{TagInfo("whiten", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyFace, (int64_t)BEType::BETypeBeautyFaceWhitenLengbai,
                         whitenLengbaiPath,
                         std::vector<TagInfo>{TagInfo("whiten", 0.0, 0)});
}

void BEDataManager::initBeautyReshapeModel() {
    std::string reshapePath = BEResourceProvider::getInstance().composerNodePath() + getReshapePath();
    std::string beauty4ItemsPath = BEResourceProvider::getInstance().composerNodePath() + "beauty_4Items";
    std::string doubleEyePath = BEResourceProvider::getInstance().composerNodePath() + "double_eye_lid/newmoon";
    std::string eyeSizeLitePath = BEResourceProvider::getInstance().composerNodePath() + "reshape_lite_eye_size";
    
    std::string reshapeDefaultFaceLitePath = BEResourceProvider::getInstance().composerNodePath() + "reshape_lite";
    std::string reshapeBoyFaceLitePath = BEResourceProvider::getInstance().composerNodePath() + "reshape_boy";
    std::string reshapeGirlFaceLitePath = BEResourceProvider::getInstance().composerNodePath() + "reshape_girl";
    std::string reshapeNatureFaceLitePath = BEResourceProvider::getInstance().composerNodePath() + "reshape_nature";
    
    std::string reshapeDefaultFaceStandardPath = BEResourceProvider::getInstance().composerNodePath() + "reshape_standard";
    std::string reshapeBoyFaceStandardPath = BEResourceProvider::getInstance().composerNodePath() + "reshape_standard_boy";
    std::string reshapeGirlFaceStandardPath = BEResourceProvider::getInstance().composerNodePath() + "reshape_standard_girl";
    std::string reshapeNatureFaceStandardPath = BEResourceProvider::getInstance().composerNodePath() + "reshape_standard_nature";
    
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        // lite resource
        
        // face
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceOverall,
                             reshapeDefaultFaceLitePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyBoyReshapeFaceOverall,
                             reshapeBoyFaceLitePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyGirlReshapeFaceOverall,
                             reshapeGirlFaceLitePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyNatureReshapeFaceOverall,
                             reshapeNatureFaceLitePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceSmall,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Face", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeShrinkHead,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_SHRINK_HEAD", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceCut,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_CutFace", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeCheek,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Zoom_Cheekbone", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeJaw,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Zoom_Jawbone", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeChin,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Chin", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeForehead,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Forehead", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeRemoveSmileFolds,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_SMILES_FOLDS", 0.0, 0)});

        // eye
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeSize,
                             eyeSizeLitePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeMove,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Eye_Move", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeSpacing,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Eye_Spacing", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeRotate,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_RotateEye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrightenEye,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BRIGHTEN_EYE", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeRemovePouch,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_REMOVE_POUCH", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeSingleToDoubleEyelid,
                             doubleEyePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_EYE_SINGLE_TO_DOUBLE", 0.0, 0)});

        // nose
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseWing,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Nose", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMovNose,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MovNose", 0.0, 1)});


        // mouth
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthZoom,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_ZoomMouth", 0.0, 0)});

        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthMove,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MovMouth", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthSmile,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MouthCorner", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeWhitenTeeth,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_WHITEN_TEETH", 0.0, 0)});
    }
    else {
        // pro resource
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceOverall,
                             reshapeDefaultFaceStandardPath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyBoyReshapeFaceOverall,
                             reshapeBoyFaceStandardPath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyGirlReshapeFaceOverall,
                             reshapeGirlFaceStandardPath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyNatureReshapeFaceOverall,
                             reshapeNatureFaceStandardPath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Overall", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceSmall,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Face", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeShrinkHead,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_SHRINK_HEAD", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceCut,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_CutFace", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeCheek,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Zoom_Cheekbone", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeJaw,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Zoom_Jawbone", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeFaceV,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_VFace", 0.0, 0)});

        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeChin,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Chin", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeForehead,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Forehead", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeRemoveSmileFolds,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_SMILES_FOLDS", 0.0, 0)});

        // eye
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeSize,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Eye", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeHeight,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_EyeHeight", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeWidth,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_EyeWidth", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeMove,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Eye_Move", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeSpacing,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Eye_Spacing", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeLowerEyelid,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_LowerEyelid", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyePupil,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_EyePupil", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeInnerCorner,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_EyeInnerCorner", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeOuterCorner,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_EyeOuterCorner", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeEyeRotate,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_RotateEye", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrightenEye,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BRIGHTEN_EYE", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeRemovePouch,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_REMOVE_POUCH", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeSingleToDoubleEyelid,
                             doubleEyePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_EYE_SINGLE_TO_DOUBLE", 0.0, 0)});

        // nose
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseSize,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_NoseSize", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseWing,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_Nose", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseBridge,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_NoseBridge", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMovNose,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MovNose", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseTip,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_NoseTip", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeNoseRoot,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_NoseRoot", 0.0, 1)});

        // eyebrow
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowSize,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowSize", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowPosition,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowPosition", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowTilt,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowTilt", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowRidge,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowRidge", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowDistance,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowDistance", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeBrowWidth,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_BrowWidth", 0.0, 1)});

        // mouth
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthZoom,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_ZoomMouth", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthWidth,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_MouseWidth", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthMove,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MovMouth", 0.0, 1)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeMouthSmile,
                             reshapePath,
                             std::vector<TagInfo>{TagInfo("Internal_Deform_MouthCorner", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyReshape, (int64_t)BEType::BETypeBeautyReshapeWhitenTeeth,
                             beauty4ItemsPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_WHITEN_TEETH", 0.0, 0)});
    }

}

void BEDataManager::initBeautyBodyModel() {
    std::string bodyPath = BEResourceProvider::getInstance().composerNodePath() + "body/allslim";
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyThin,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_THIN", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyLegLong,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_LONG_LEG", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodySlimLeg,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_SLIM_LEG", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodySlimWaist,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_SLIM_WAIST", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyEnhanceNeck,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_ENHANCE_NECK", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodySlimArm,
                         bodyPath,
                         std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_SLIM_ARM", 0.0, 0)});
    
    std::string enlargeBreastLitePath = BEResourceProvider::getInstance().composerNodePath() + "body/allslim";
    std::string enhanceHipLitePath = BEResourceProvider::getInstance().composerNodePath() + "body/allslim";
    std::string enlargeBreastStandardPath = BEResourceProvider::getInstance().composerNodePath() + "body/allslim_breast";
    std::string enhanceHipStandardPath = BEResourceProvider::getInstance().composerNodePath() + "body/allslim_breast";
    if(m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyEnlargeBreast,
                             enlargeBreastLitePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_ENLARGR_BREAST", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyEnhanceHip,
                             enhanceHipLitePath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_ENHANCE_HIP", 0.0, 1)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyEnlargeBreast,
                             enlargeBreastStandardPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_ENLARGR_BREAST", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeBeautyBody, (int64_t)BEType::BETypeBeautyBodyEnhanceHip,
                             enhanceHipStandardPath,
                             std::vector<TagInfo>{TagInfo("BEF_BEAUTY_BODY_ENHANCE_HIP", 0.0, 1)});
    }
}

void BEDataManager::initBackgroundBlurModel() {
    std::string blurPath = BEResourceProvider::getInstance().stickerPath() + "stickers/";
    registerComposerNode((int64_t)BEType::BETypeBackgroundBlur, (int64_t)BEType::BETypeBackgroundBlurBase,
                         blurPath + "background_blur",
                         std::vector<TagInfo>{TagInfo("", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBackgroundBlur, (int64_t)BEType::BETypeBackgroundBlurStrong,
                         blurPath + "bg_blur_strong",
                         std::vector<TagInfo>{TagInfo("", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeBackgroundBlur, (int64_t)BEType::BETypeBackgroundBlurAdjust,
                         blurPath + "background_blur_in",
                         std::vector<TagInfo>{TagInfo("Blur_intensity", 0.0, 0)});
}

void BEDataManager::initMakeupModel() {
    initMakeupLipModel();
    initMakeupBlusherModel();
    initMakeupEyelashModel();
    initMakeupPupilModel();
    initMakeupHairModel();
    initMakeupEyeshadowModel();
    initMakeupEyebrowModel();
    initMakeupFacialModel();
    initMakeupEyeLightModel();
    initMakeupEyePlumpModel();
}

void BEDataManager::initMakeupLipModel() {
    std::string lipPath = BEResourceProvider::getInstance().composerNodePath() + "lip/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 1,
                             lipPath + "lite/fuguhong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 2,
                             lipPath + "lite/shaonvfen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 3,
                             lipPath + "lite/yuanqiju",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 4,
                             lipPath + "lite/xiyouse",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 5,
                             lipPath + "lite/xiguahong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 6,
                             lipPath + "lite/sironghong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 7,
                             lipPath + "lite/zangjuse",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 8,
                             lipPath + "lite/meizise",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 9,
                             lipPath + "lite/shanhuse",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 10,
                             lipPath + "lite/doushafen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Lips", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 1,
                             lipPath + "standard/liangze",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Lips", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 2,
                             lipPath + "standard/wumian",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Lips", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 3,
                             lipPath + "standard/yaochun",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Lips", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupLip + 4,
                             lipPath + "standard/yunran",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Lips", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)});
    }
}

void BEDataManager::initMakeupBlusherModel() {
    std::string blusherPath = BEResourceProvider::getInstance().composerNodePath() + "blush/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 1,
                             blusherPath + "lite/weixun",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 2,
                             blusherPath + "lite/richang",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 3,
                             blusherPath + "lite/mitao",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 4,
                             blusherPath + "lite/tiancheng",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 5,
                             blusherPath + "lite/qiaopi",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 6,
                             blusherPath + "lite/xinji",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 7,
                             blusherPath + "lite/shaishang",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Blusher", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 1,
                             blusherPath + "standard/mitao",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 2,
                             blusherPath + "standard/weixun",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 3,
                             blusherPath + "standard/yuanqi",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 4,
                             blusherPath + "standard/qise",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 5,
                             blusherPath + "standard/shaishang",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 6,
                             blusherPath + "standard/rixi",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupBlusher + 7,
                             blusherPath + "standard/suzui",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Blusher", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
    }
}

void BEDataManager::initMakeupEyelashModel() {
    std::string eyelashPath = BEResourceProvider::getInstance().composerNodePath() + "eyelashes/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {

    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyelash + 1,
                             eyelashPath + "ziran",
                             std::vector<TagInfo>{
                                TagInfo("Internal_Makeup_Eyelash", 0.0, 0),
                                TagInfo("R", 0.0, 0),
                                TagInfo("G", 0.0, 0),
                                TagInfo("B", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyelash + 2,
                             eyelashPath + "juanqiao",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Eyelash", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyelash + 3,
                             eyelashPath + "chibang",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Eyelash", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyelash + 4,
                             eyelashPath + "manhua",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Eyelash", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyelash + 5,
                             eyelashPath + "xiachui",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Eyelash", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
    }
}

void BEDataManager::initMakeupPupilModel() {
    std::string pupilPath = BEResourceProvider::getInstance().composerNodePath() + "pupil/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 1,
                             pupilPath + "hunxuezong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 2,
                             pupilPath + "kekezong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 3,
                             pupilPath + "mitaofen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 4,
                             pupilPath + "shuiguanghei",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 5,
                             pupilPath + "xingkonglan",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 6,
                             pupilPath + "chujianhui",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 1,
                             pupilPath + "yuansheng",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 2,
                             pupilPath + "xinxinzi",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 3,
                             pupilPath + "huoxing",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 4,
                             pupilPath + "huilv",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupPupil + 5,
                             pupilPath + "huitong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Pupil", 0.0, 0)});
    }
}

void BEDataManager::initMakeupHairModel() {
    std::string hairPath = BEResourceProvider::getInstance().composerNodePath() + "hair/";
    registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupHair + 1,
                         hairPath + "anlan",
                         std::vector<TagInfo>{TagInfo("", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupHair + 2,
                         hairPath + "molv",
                         std::vector<TagInfo>{TagInfo("", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupHair + 3,
                         hairPath + "shenzong",
                         std::vector<TagInfo>{TagInfo("", 0.0, 0)});
}

void BEDataManager::initMakeupEyeshadowModel() {
    std::string eyeShadowPath = BEResourceProvider::getInstance().composerNodePath() + "eyeshadow/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 1,
                             eyeShadowPath + "wanxiahong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 2,
                             eyeShadowPath + "shaonvfen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 3,
                             eyeShadowPath + "qizhifen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 4,
                             eyeShadowPath + "meizihong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 5,
                             eyeShadowPath + "jiaotangzong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 6,
                             eyeShadowPath + "yuanqiju",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 7,
                             eyeShadowPath + "naichase",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 1,
                             eyeShadowPath + "dadizong",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 2,
                             eyeShadowPath + "hanxi",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 3,
                             eyeShadowPath + "nvshen",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 4,
                             eyeShadowPath + "xingzi",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeshadow + 5,
                             eyeShadowPath + "bingtangshanzha",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Eye", 0.0, 0)});
    }

}

void BEDataManager::initMakeupEyebrowModel() {
    std::string eyeBrowPath = BEResourceProvider::getInstance().composerNodePath() + "eyebrow/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 1,
                             eyeBrowPath + "lite/BR01",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Brow", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 2,
                             eyeBrowPath + "lite/BK01",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Brow", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 3,
                             eyeBrowPath + "lite/BK02",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Brow", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 4,
                             eyeBrowPath + "lite/BK03",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Brow", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 1,
                             eyeBrowPath + "standard/biaozhun",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Brow", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 2,
                             eyeBrowPath + "standard/liuye",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Brow", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 3,
                             eyeBrowPath + "standard/rongrong",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Brow", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyebrow + 4,
                             eyeBrowPath + "standard/yesheng",
                             std::vector<TagInfo>{
                                 TagInfo("Internal_Makeup_Brow", 0.0, 0),
                                 TagInfo("R", 0.0, 0),
                                 TagInfo("G", 0.0, 0),
                                 TagInfo("B", 0.0, 0)
                             });
    }
}

void BEDataManager::initMakeupFacialModel() {
    std::string facialPath = BEResourceProvider::getInstance().composerNodePath() + "facial/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupFacial + 1,
                             facialPath + "jingzhi",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Facial", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupFacial + 1,
                             facialPath + "ziran",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Facial", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupFacial + 2,
                             facialPath + "xiaov",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Facial", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupFacial + 3,
                             facialPath + "fajixian",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Facial", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupFacial + 4,
                             facialPath + "gaoguang",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_Facial", 0.0, 0)});
    }
}

void BEDataManager::initMakeupEyeLightModel() {
    std::string eyelightPath = BEResourceProvider::getInstance().composerNodePath() + "eyelight/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {

    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeLight + 1,
                             eyelightPath + "ziranguang",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_EyeLight", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeLight + 2,
                             eyelightPath + "yueyaguang",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_EyeLight", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyeLight + 3,
                             eyelightPath + "juguangdeng",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_EyeLight", 0.0, 0)});
    }
}

void BEDataManager::initMakeupEyePlumpModel() {
    std::string plumpPath = BEResourceProvider::getInstance().composerNodePath() + "wocan/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 1,
                             plumpPath + "ziran",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
    }
    else {
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 1,
                             plumpPath + "suyan",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 2,
                             plumpPath + "chulian",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 3,
                             plumpPath + "manhuayan",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 4,
                             plumpPath + "xiachui",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
        registerComposerNode((int64_t)BEType::BETypeMakeup, (int64_t)BEType::BETypeMakeupEyePlump + 5,
                             plumpPath + "taohua",
                             std::vector<TagInfo>{TagInfo("Internal_Makeup_WoCan", 0.0, 0)});
    }
}

void BEDataManager::initQualityModel()
{
    std::string qualityPath = BEResourceProvider::getInstance().composerNodePath() + "palette/";
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 1,
                            qualityPath + "color",
                            std::vector<TagInfo>{TagInfo("Intensity_Temperature", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 2,
                            qualityPath + "light",
                            std::vector<TagInfo>{TagInfo("Intensity_Hue", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 3,
                            qualityPath + "color",
                            std::vector<TagInfo>{TagInfo("Intensity_Saturation", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 4,
                            qualityPath + "light",
                            std::vector<TagInfo>{TagInfo("Intensity_Light", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 5,
                            qualityPath + "contrast",
                            std::vector<TagInfo>{TagInfo("Intensity_Contrast", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 6,
                            qualityPath + "light",
                            std::vector<TagInfo>{TagInfo("Intensity_Highlight", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 7,
                            qualityPath + "color",
                            std::vector<TagInfo>{TagInfo("Intensity_Shadow", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 8,
                            qualityPath + "light",
                            std::vector<TagInfo>{TagInfo("Intensity_Light_Sensation", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 9,
                            qualityPath + "particle",
                            std::vector<TagInfo>{TagInfo("Intensity_Particle", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 10,
                            qualityPath + "color",
                            std::vector<TagInfo>{TagInfo("Intensity_Fade", 0.0, 0)});
    registerComposerNode((int64_t)BEType::BETypeQuality, (int64_t)BEType::BETypeQuality + 11,
                            qualityPath + "vignette",
                            std::vector<TagInfo>{TagInfo("Intensity_Vignette", 0.0, 0)});    
}

void BEDataManager::initBlackTechModel()
{
    std::string blackTechPath = BEResourceProvider::getInstance().composerNodePath() + "hack/";
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        registerComposerNode((int64_t)BEType::BETypeBlackTech, (int64_t)BEType::BETypeBlackTechQvBan,
                                blackTechPath + "acne_removal_lite",
                                std::vector<TagInfo>{TagInfo("clean", 0.0, 0)});
    } else {
        registerComposerNode((int64_t)BEType::BETypeBlackTech, (int64_t)BEType::BETypeBlackTechQvBan,
                                blackTechPath + "acne_removal_standard",
                                std::vector<TagInfo>{TagInfo("clean", 0.0, 0)});
    }
}

void BEDataManager::initStyleMakeupModel() {
    std::string styleMakeupPath = BEResourceProvider::getInstance().composerNodePath() + "style_makeup/";
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 1,
                         styleMakeupPath + "baicha",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 2,
                         styleMakeupPath + "meiyouxiaoxiong",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 3,
                         styleMakeupPath + "bingchuan",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 4,
                         styleMakeupPath + "qise",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 5,
                         styleMakeupPath + "aidou",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 6,
                         styleMakeupPath + "youya",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 7,
                         styleMakeupPath + "cwei",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 8,
                         styleMakeupPath + "nuannan",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 9,
                         styleMakeupPath + "baixi",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 10,
                         styleMakeupPath + "wennuan",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 11,
                         styleMakeupPath + "shensui",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 12,
                         styleMakeupPath + "tianmei",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 13,
                         styleMakeupPath + "duanmei",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 14,
                         styleMakeupPath + "oumei",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 15,
                         styleMakeupPath + "zhigan",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 16,
                         styleMakeupPath + "hanxi",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 17,
                         styleMakeupPath + "yuanqi",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 18,
                         styleMakeupPath + "shuiwu",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 19,
                         styleMakeupPath + "naicha",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 20,
                         styleMakeupPath + "mitaofen3",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 21,
                         styleMakeupPath + "fenxia",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 22,
                         styleMakeupPath + "tiansang",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 23,
                         styleMakeupPath + "insfeng2",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 24,
                         styleMakeupPath + "heisirong",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 25,
                         styleMakeupPath + "nyushen",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 26,
                         styleMakeupPath + "qizhi",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 27,
                         styleMakeupPath + "naixiong",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    // 3D
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 28,
                         styleMakeupPath + "ailingzhuang",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 29,
                         styleMakeupPath + "dongnanyachunyu",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 30,
                         styleMakeupPath + "manhuashaonyu",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 31,
                         styleMakeupPath + "zhiyafangzhuang",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 32,
                         styleMakeupPath + "baikaishui",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 33,
                         styleMakeupPath + "hanguoxuemei",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 34,
                         styleMakeupPath + "nenmozhuang",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 35,
                         styleMakeupPath + "touxianggan",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
    registerComposerNode((int64_t)BEType::BETypeStyleMakeup, (int64_t)BEType::BETypeStyleMakeup + 36,
                         styleMakeupPath + "wodenvhai",
                         std::vector<TagInfo>{
                             TagInfo("Filter_ALL", 0.0, 0),
                             TagInfo("Makeup_ALL", 0.0, 0)
                         });
}

void BEDataManager::registerComposerNode(int64_t mainId, int64_t subId, const std::string& nodePath, std::vector<TagInfo> tagArray) {
    auto itr = m_mapComposerNode.find(subId);
    if (itr == m_mapComposerNode.end())
    {
        //m_allComposerNodeMap.insert(std::make_pair(subId, new BEComposerNode(majorId, subId, isMajor, NodeName, key)));
        m_mapComposerNode.insert(std::make_pair(subId, new BEComposerNodeModel(mainId,
                                                                               subId,
                                                                               nodePath,
                                                                               tagArray)));
    }
}

BEComposerNodeModel* BEDataManager::getComposerNodeModel(int64_t type) {
    auto itr = m_mapComposerNode.find(type);
    if (itr != m_mapComposerNode.end())
    {
        return itr->second;
    }
    return nullptr;
}

std::string BEDataManager::getBeautyPath() {
    return m_effectType == BEEffectType::BEEffectTypeLite?"beauty_IOS_lite" : "beauty_IOS_standard";
}

std::string BEDataManager::getReshapePath() {
    return m_effectType == BEEffectType::BEEffectTypeLite?"reshape_lite" : "reshape_standard";
}

void BEDataManager::setEffectEnable(bool v) {
    m_bEffectEnable = v;
}

bool BEDataManager::isEffectEnable() {
    return m_bEffectEnable;
}

void BEDataManager::setLocalLanguage(int localType) {
    m_localLanguage = localType;
}

std::map<int64_t, float>& BEDataManager::getDefaultEffects() {
    if (m_effectType == BEEffectType::BEEffectTypeLite) {
        return m_liteDefaultEffects;
    }
    else {
        return m_standardDefaultEffects;
    }

}
