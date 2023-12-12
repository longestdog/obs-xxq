#ifndef BE_RESOURCE_MODEL_H
#define BE_RESOURCE_MODEL_H

#include <stdint.h>
#include <string>
#include <vector>

static const int64_t OFFSET = 24;
static const int64_t MASK = 0xFFFFFF;
static const int64_t SUB_OFFSET = 16;
static const int64_t SUB_MASK = 0xFFFF;

enum class BEType {

    //    {zh} 一级菜单        {en} Level 1 menu
    BETypeClose                    = -1,
    BETypeBeautyFace               = 1 << OFFSET,
    BETypeBeautyReshape            = 2 << OFFSET,
    BETypeBeautyBody               = 3 << OFFSET,
    BETypeMakeup                   = 4 << OFFSET,
    BETypeStyleMakeup              = 5 << OFFSET,
    BETypeFilter                   = 6 << OFFSET,
    BETypeSticker                  = 7 << OFFSET,
    BETypeAmazingSticker           = 8 << OFFSET,
    BETypeAnimoji                  = 9 << OFFSET,
    BETypeArscan                   = 10 << OFFSET,
    BETypeArTryShoe                = 11 << OFFSET,
    BETypeArTryHat                 = 12 << OFFSET,
    BETypeArTryWatch               = 13 << OFFSET,
    BETypeArTryBracelet            = 14 << OFFSET,
    BETypeAvatarBoy                = 15 << OFFSET,
    BETypeAvatarGirl               = 16 << OFFSET,
    BETypeArSLAM                   = 17 << OFFSET,
    BETypeArObject                 = 18 << OFFSET,
    BETypeArLandmark               = 19 << OFFSET,
    BETypeArSkyLand                = 20 << OFFSET,
    BETypeArTryRing                = 21 << OFFSET,
    BETypeArTryGlasses             = 22 << OFFSET,
    BETypeArNail                   = 23 << OFFSET,
    BETypeQuality                  = 24 << OFFSET,
    BETypeBlackTech                = 25 << OFFSET,

    BETypeBackgroundBlur           = 26 << OFFSET,

    //    {zh} 二级菜单        {en} Secondary menu
       //    {zh} 美颜        {en} Beauty
       BETypeBeautyFaceSmooth         = BETypeBeautyFace      + (1 << SUB_OFFSET), //  {zh} 磨皮  {en} Dermabrasion
       BETypeBeautyFaceWhiten         = BETypeBeautyFace      + (2 << SUB_OFFSET), //  {zh} 美白  {en} Whitening
       BETypeBeautyFaceSharp          = BETypeBeautyFace      + (3 << SUB_OFFSET), //  {zh} 锐化  {en} Sharpening
       BETypeBeautyFaceClarity        = BETypeBeautyFace      + (4 << SUB_OFFSET), //  {zh} 清晰  {en} Clear
       BETypeBeautyFaceWhitenLengbai  = BETypeBeautyFace      + (5 << SUB_OFFSET), //  {zh} 冷白  {en} WhitenCold
       BETypeBeautyFaceWhitenFenbai   = BETypeBeautyFace      + (6 << SUB_OFFSET), //  {zh} 粉白  {en} WhitenPink
       //    {zh} 美形        {en} Beauty
       //  {zh} 面部  {en} Facial
       BETypeBeautyReshapeFace             = BETypeBeautyReshape    + (1 << SUB_OFFSET), //  {zh} 面部  {en} Facial
       BETypeBeautyReshapeFaceOverall      = BETypeBeautyReshape    + (2 << SUB_OFFSET), //  {zh} 瘦脸  {en} Skinny face
       BETypeBeautyReshapeFaceSmall        = BETypeBeautyReshape    + (3 << SUB_OFFSET), //  {zh} 小脸  {en} Little face
       BETypeBeautyReshapeFaceCut          = BETypeBeautyReshape    + (4 << SUB_OFFSET), //  {zh} 窄脸  {en} Narrow face
       BETypeBeautyReshapeFaceV            = BETypeBeautyReshape    + (5 << SUB_OFFSET), //  {zh} V脸  {en} V face
       BETypeBeautyReshapeForehead         = BETypeBeautyReshape    + (6 << SUB_OFFSET), //  {zh} 额头/发际线  {en} Forehead/hairline
       BETypeBeautyReshapeCheek            = BETypeBeautyReshape    + (7 << SUB_OFFSET), //  {zh} 颧骨  {en} Cheekbones
       BETypeBeautyReshapeJaw              = BETypeBeautyReshape    + (8 << SUB_OFFSET), //  {zh} 下颌骨  {en} Mandible
       BETypeBeautyReshapeChin             = BETypeBeautyReshape    + (9 << SUB_OFFSET), //  {zh} 下巴  {en} Chin
       BETypeBeautyReshapeShrinkHead       = BETypeBeautyReshape    + (10 << SUB_OFFSET), // {zh} 小头  {en} Small head
       BETypeBeautyBoyReshapeFaceOverall   = BETypeBeautyReshape    + (11 << SUB_OFFSET), //  {zh} 男神脸  {en} Boy face
       BETypeBeautyGirlReshapeFaceOverall  = BETypeBeautyReshape    + (12 << SUB_OFFSET), //  {zh} 女神脸  {en} Girl face
       BETypeBeautyNatureReshapeFaceOverall= BETypeBeautyReshape    + (13 << SUB_OFFSET), //  {zh} 自然脸  {en} Natural face
       //  {zh} 眼睛  {en} Eye
       BETypeBeautyReshapeEye              = BETypeBeautyReshape    + (20 << SUB_OFFSET), //  {zh} 眼睛  {en} Eyes
       BETypeBeautyReshapeEyeSize          = BETypeBeautyReshape    + (21 << SUB_OFFSET), //  {zh} 大眼  {en} Big eyes
       BETypeBeautyReshapeEyeHeight        = BETypeBeautyReshape    + (22 << SUB_OFFSET), //  {zh} 眼高度  {en} Eye height
       BETypeBeautyReshapeEyeWidth         = BETypeBeautyReshape    + (23 << SUB_OFFSET), //  {zh} 眼宽度  {en} Eye width
       BETypeBeautyReshapeEyeMove          = BETypeBeautyReshape    + (24 << SUB_OFFSET), //  {zh} 眼移动/眼位置  {en} Eye movement/eye position
       BETypeBeautyReshapeEyeSpacing       = BETypeBeautyReshape    + (25 << SUB_OFFSET), //  {zh} 眼间距  {en} Eye spacing
       BETypeBeautyReshapeEyeLowerEyelid   = BETypeBeautyReshape    + (26 << SUB_OFFSET), //  {zh} 眼睑下至  {en} Eyelid down to
       BETypeBeautyReshapeEyePupil         = BETypeBeautyReshape    + (27 << SUB_OFFSET), //  {zh} 瞳孔大小  {en} Pupil size
       BETypeBeautyReshapeEyeInnerCorner   = BETypeBeautyReshape    + (16 << SUB_OFFSET), //  {zh} 内眼角  {en} Inner canthus
       BETypeBeautyReshapeEyeOuterCorner   = BETypeBeautyReshape    + (28 << SUB_OFFSET), //  {zh} 外眼角  {en} Outer corner of eye
       BETypeBeautyReshapeEyeRotate        = BETypeBeautyReshape    + (29 << SUB_OFFSET), //  {zh} 眼角度/眼角上扬  {en} Eye angle/canthus rise
       //  {zh} 鼻子  {en} Nose
       BETypeBeautyReshapeNose             = BETypeBeautyReshape    + (40 << SUB_OFFSET), //  {zh} 鼻子  {en} Nose
       BETypeBeautyReshapeNoseSize         = BETypeBeautyReshape    + (41 << SUB_OFFSET), //  {zh} 鼻子大小  {en} Nose size
       BETypeBeautyReshapeNoseWing         = BETypeBeautyReshape    + (42 << SUB_OFFSET), //  {zh} 鼻翼?瘦鼻  {en} Nose? Skinny nose
       BETypeBeautyReshapeNoseBridge       = BETypeBeautyReshape    + (43 << SUB_OFFSET), //  {zh} 鼻梁  {en} Bridge of nose
       BETypeBeautyReshapeMovNose          = BETypeBeautyReshape    + (44 << SUB_OFFSET), //  {zh} 鼻子提升?长鼻  {en} Nose lift? Long nose
       BETypeBeautyReshapeNoseTip          = BETypeBeautyReshape    + (45 << SUB_OFFSET), //  {zh} 鼻尖  {en} Nose tip
       BETypeBeautyReshapeNoseRoot         = BETypeBeautyReshape    + (46 << SUB_OFFSET), //  {zh} 山根  {en} Yamagata
       //  {zh} 眉毛  {en} Eyebrows
       BETypeBeautyReshapeBrow             = BETypeBeautyReshape    + (60 << SUB_OFFSET), //  {zh} 眉毛  {en} Eyebrows
       BETypeBeautyReshapeBrowSize         = BETypeBeautyReshape    + (61 << SUB_OFFSET), //  {zh} 眉毛粗细  {en} Eyebrow thickness
       BETypeBeautyReshapeBrowPosition     = BETypeBeautyReshape    + (62 << SUB_OFFSET), //  {zh} 眉毛位置  {en} Eyebrow position
       BETypeBeautyReshapeBrowTilt         = BETypeBeautyReshape    + (63 << SUB_OFFSET), //  {zh} 眉毛倾斜  {en} Tilted eyebrows
       BETypeBeautyReshapeBrowRidge        = BETypeBeautyReshape    + (64 << SUB_OFFSET), //  {zh} 眉峰  {en} Meifeng
       BETypeBeautyReshapeBrowDistance     = BETypeBeautyReshape    + (65 << SUB_OFFSET), //  {zh} 眉毛间距  {en} Eyebrow spacing
       BETypeBeautyReshapeBrowWidth        = BETypeBeautyReshape    + (66 << SUB_OFFSET), //  {zh} 眉毛宽度  {en} Eyebrow width
       //  {zh} 嘴巴  {en} Mouth
       BETypeBeautyReshapeMouth            = BETypeBeautyReshape    + (80 << SUB_OFFSET), //  {zh} 嘴巴  {en} Mouth
       BETypeBeautyReshapeMouthZoom        = BETypeBeautyReshape    + (81 << SUB_OFFSET), //  {zh} 嘴巴大小/嘴形  {en} Mouth size/shape
       BETypeBeautyReshapeMouthWidth       = BETypeBeautyReshape    + (82 << SUB_OFFSET), //  {zh} 嘴巴宽度  {en} Mouth width
       BETypeBeautyReshapeMouthMove        = BETypeBeautyReshape    + (83 << SUB_OFFSET), //  {zh} 嘴巴位置/人中  {en} Mouth position/person
       BETypeBeautyReshapeMouthSmile       = BETypeBeautyReshape    + (84 << SUB_OFFSET), //  {zh} 微笑  {en} Smile
       //  {zh} 妆  {en} Makeup
       BETypeBeautyReshapeBrightenEye      = BETypeBeautyReshape    + (35 << SUB_OFFSET), //  {zh} 亮眼  {en} Bright eyes
       BETypeBeautyReshapeRemovePouch      = BETypeBeautyReshape    + (36 << SUB_OFFSET), //  {zh} 黑眼圈  {en} Dark circles
       BETypeBeautyReshapeRemoveSmileFolds = BETypeBeautyReshape    + (37 << SUB_OFFSET), //  {zh} 法令纹  {en} French lines
       BETypeBeautyReshapeWhitenTeeth      = BETypeBeautyReshape    + (38 << SUB_OFFSET), //  {zh} 白牙  {en} White teeth
       BETypeBeautyReshapeSingleToDoubleEyelid = BETypeBeautyReshape+ (39 << SUB_OFFSET), //  {zh} 双眼皮  {en} Double Eyelids
       BETypeBeautyReshapeEyePlump         = BETypeBeautyReshape    + (40 << SUB_OFFSET), //  {zh} 卧蚕  {en} Sleeping silkworm

       //    {zh} 美体        {en} Body
       BETypeBeautyBodyThin           = BETypeBeautyBody      + (1 << SUB_OFFSET), //  {zh} 瘦身  {en} Slimming
       BETypeBeautyBodyLegLong        = BETypeBeautyBody      + (2 << SUB_OFFSET), //  {zh} 长腿  {en} Long legs
       BETypeBeautyBodySlimLeg        = BETypeBeautyBody      + (3 << SUB_OFFSET), //  {zh} 瘦腿  {en} Slim Legs
       BETypeBeautyBodySlimWaist      = BETypeBeautyBody      + (4 << SUB_OFFSET), //  {zh} 瘦手腕  {en} Slim wrists
       BETypeBeautyBodyEnlargeBreast  = BETypeBeautyBody      + (5 << SUB_OFFSET), //  {zh} 丰胸  {en} Breast enhancement
       BETypeBeautyBodyEnhanceHip     = BETypeBeautyBody      + (6 << SUB_OFFSET), //  {zh} 美胯  {en} Beautiful crotch
       BETypeBeautyBodyEnhanceNeck    = BETypeBeautyBody      + (7 << SUB_OFFSET), //  {zh} 天鹅颈 {en} Beautiful neck
       BETypeBeautyBodySlimArm        = BETypeBeautyBody      + (8 << SUB_OFFSET), //  {zh} 瘦手臂  {en} Thin arms

       //    {zh} 美妆        {en} Beauty makeup
       BETypeMakeupLip                = BETypeMakeup          + (1 << SUB_OFFSET), //  {zh} 口红  {en} Lipstick
       BETypeMakeupBlusher            = BETypeMakeup          + (2 << SUB_OFFSET), //  {zh} 腮红  {en} Blush
       BETypeMakeupEyelash            = BETypeMakeup          + (3 << SUB_OFFSET), //  {zh} 睫毛  {en} Eyelashes
       BETypeMakeupPupil              = BETypeMakeup          + (4 << SUB_OFFSET), //  {zh} 美瞳  {en} Pupils
       BETypeMakeupHair               = BETypeMakeup          + (5 << SUB_OFFSET), //  {zh} 染发  {en} Hair coloring
       BETypeMakeupEyeshadow          = BETypeMakeup          + (6 << SUB_OFFSET), //  {zh} 眼影  {en} Eye shadow
       BETypeMakeupEyebrow            = BETypeMakeup          + (7 << SUB_OFFSET), //  {zh} 眉毛  {en} Eyebrows
       BETypeMakeupFacial             = BETypeMakeup          + (8 << SUB_OFFSET), //  {zh} 塑形  {en} Shape
       BETypeMakeupEyeLight           = BETypeMakeup          + (9 << SUB_OFFSET), //  {zh} 眼神光  {en} Eye light
       BETypeMakeupEyePlump           = BETypeMakeup          + (10 << SUB_OFFSET), //  {zh} 卧蚕  {en} Sleeping silkworm

       BETypeBackgroundBlurBase       = BETypeBackgroundBlur + (1 << SUB_OFFSET),
       BETypeBackgroundBlurStrong     = BETypeBackgroundBlur + (2 << SUB_OFFSET),
       BETypeBackgroundBlurAdjust     = BETypeBackgroundBlur + (3 << SUB_OFFSET),

       BETypeBlackTechQvBan           = BETypeBlackTech      +  1,
       BETypeBlackTechYunFu           = BETypeBlackTech      +  2,
       BETypeBlackTechFengYing        = BETypeBlackTech      +  3,
       BETypeBlackTechLiuChangLian    = BETypeBlackTech      +  4,
};

enum class BEEffectType {
    BEEffectTypeLite = 1,
    BEEffectTypePro = 2,
};

class BEResourceModel {
public:
    BEResourceModel();
    ~BEResourceModel();
};

typedef struct TagInfo {
    TagInfo(const std::string& key, float value, int type):
      m_key(key),
      m_value(value),
      m_type(type) {

    }
    std::string m_key;
    float m_value;
    int m_type;  // 0: 初始值默认0.0f  1: 初始值默认0.5f
} TagInfo;

class BEComposerNodeModel {
public:
    BEComposerNodeModel(int mainId, int subId,
                        const std::string& nodePath,
                        std::vector<TagInfo>& tagArray) {
        m_mainId = mainId;
        m_subId = subId;
        m_nodePath = nodePath;
        m_vecTag = std::move(tagArray);
    }
public:
    int m_mainId;
    int m_subId;
    std::string m_nodePath;
    std::vector<TagInfo> m_vecTag;
};

#endif
