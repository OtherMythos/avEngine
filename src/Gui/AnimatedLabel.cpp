
#include "AnimatedLabel.h"
#include "ColibriGui/ColibriManager.h"

namespace AV
{

    size_t currentId;
    AnimatedLabel::AnimatedLabel( Colibri::ColibriManager *manager ) :
        Label( manager )
    {
        manager->_notifyLabelCreated(this);
    }

    void AnimatedLabel::_fillBuffersAndCommands( Colibri::UiVertex * colibri_nonnull * colibri_nonnull
                                        RESTRICT_ALIAS vertexBuffer,
                                        Colibri::GlyphVertex * colibri_nonnull * colibri_nonnull
                                        RESTRICT_ALIAS textVertBuffer,
                                        const Ogre::Vector2 &parentPos,
                                        const Ogre::Vector2 &parentCurrentScrollPos,
                                        const Colibri::Matrix2x3 &parentRot )
    {
        currentId = 0;
        m_isShadowCall_ = m_shadowOutline;
        Label::_fillBuffersAndCommands(vertexBuffer, textVertBuffer, parentPos, parentCurrentScrollPos, parentRot);
    }

    void AnimatedLabel::addQuad( Colibri::GlyphVertex * RESTRICT_ALIAS vertexBuffer,
                                Ogre::Vector2 topLeft,
                                Ogre::Vector2 bottomRight,
                                uint16_t glyphWidth,
                                uint16_t glyphHeight,
                                uint32_t rgbaColour,
                                Ogre::Vector2 parentDerivedTL,
                                Ogre::Vector2 parentDerivedBR,
                                Ogre::Vector2 invSize,
                                uint32_t offset,
                                float canvasAspectRatio,
                                float invCanvasAspectRatio,
                                Colibri::Matrix2x3 derivedRot )
    {
        Ogre::Vector2 tmp2d;

        assert(currentId < m_animData.size());
        const GlyphAnimationValues& animData = m_animData[currentId];

        //apply position offset so shadow tracks the animated glyph
        topLeft += Ogre::Vector2(animData.xOffset, animData.yOffset);
        bottomRight += Ogre::Vector2(animData.xOffset, animData.yOffset);

        uint32_t finalColour;
        if(m_isShadowCall_) {
            //shadow pass: modulate only the alpha channel by the animation data
            uint8_t a = uint8_t(float((rgbaColour >> 24) & 0xFF) * (float((animData.rgbaColour >> 24) & 0xFF) / 255.0));
            finalColour = (rgbaColour & 0x00FFFFFFu) | (uint32_t(a) << 24);
        } else {
            uint8_t r = uint8_t(float((rgbaColour >> 0) & 0xFF) * (float((animData.rgbaColour >> 0) & 0xFF) / 255.0));
            uint8_t g = uint8_t(float((rgbaColour >> 8) & 0xFF) * (float((animData.rgbaColour >> 8) & 0xFF) / 255.0));
            uint8_t b = uint8_t(float((rgbaColour >> 16) & 0xFF) * (float((animData.rgbaColour >> 16) & 0xFF) / 255.0));
            uint8_t a = uint8_t(float((rgbaColour >> 24) & 0xFF) * (float((animData.rgbaColour >> 24) & 0xFF) / 255.0));
            finalColour = (a << 24) | (b << 16) | (g << 8) | r;
        }

        #define COLIBRI_ADD_VERTEX( _x, _y, _u, _v, clipDistanceTop, clipDistanceLeft, \
                                    clipDistanceRight, clipDistanceBottom ) \
            tmp2d = Widget::mul( derivedRot, _x, _y * invCanvasAspectRatio ); \
            tmp2d.y *= canvasAspectRatio; \
            vertexBuffer->x = tmp2d.x; \
            vertexBuffer->y = -tmp2d.y; \
            vertexBuffer->width = glyphWidth; \
            vertexBuffer->height = glyphHeight; \
            vertexBuffer->offset = offset;\
            vertexBuffer->rgbaColour = finalColour; \
            vertexBuffer->clipDistance[Colibri::Borders::Top]    = clipDistanceTop; \
            vertexBuffer->clipDistance[Colibri::Borders::Left]    = clipDistanceLeft; \
            vertexBuffer->clipDistance[Colibri::Borders::Right]    = clipDistanceRight; \
            vertexBuffer->clipDistance[Colibri::Borders::Bottom]    = clipDistanceBottom; \
            ++vertexBuffer

        COLIBRI_ADD_VERTEX( topLeft.x, topLeft.y,
                            0u, 0u,
                            (topLeft.y - parentDerivedTL.y) * invSize.y,
                            (topLeft.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - topLeft.x) * invSize.x,
                            (parentDerivedBR.y - topLeft.y) * invSize.y );

        COLIBRI_ADD_VERTEX( topLeft.x, bottomRight.y,
                            0u, glyphHeight,
                            (bottomRight.y - parentDerivedTL.y) * invSize.y,
                            (topLeft.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - topLeft.x) * invSize.x,
                            (parentDerivedBR.y - bottomRight.y) * invSize.y );

        COLIBRI_ADD_VERTEX( bottomRight.x, bottomRight.y,
                            glyphWidth, glyphHeight,
                            (bottomRight.y - parentDerivedTL.y) * invSize.y,
                            (bottomRight.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - bottomRight.x) * invSize.x,
                            (parentDerivedBR.y - bottomRight.y) * invSize.y );

        COLIBRI_ADD_VERTEX( bottomRight.x, bottomRight.y,
                            glyphWidth, glyphHeight,
                            (bottomRight.y - parentDerivedTL.y) * invSize.y,
                            (bottomRight.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - bottomRight.x) * invSize.x,
                            (parentDerivedBR.y - bottomRight.y) * invSize.y );

        COLIBRI_ADD_VERTEX( bottomRight.x, topLeft.y,
                            glyphWidth, 0u,
                            (topLeft.y - parentDerivedTL.y) * invSize.y,
                            (bottomRight.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - bottomRight.x) * invSize.x,
                            (parentDerivedBR.y - topLeft.y) * invSize.y );

        COLIBRI_ADD_VERTEX( topLeft.x, topLeft.y,
                            0u, 0u,
                            (topLeft.y - parentDerivedTL.y) * invSize.y,
                            (topLeft.x - parentDerivedTL.x) * invSize.x,
                            (parentDerivedBR.x - topLeft.x) * invSize.x,
                            (parentDerivedBR.y - topLeft.y) * invSize.y );

        #undef COLIBRI_ADD_VERTEX

        if(m_isShadowCall_) {
            //shadow call consumed - actual glyph call is next, which will increment currentId
            m_isShadowCall_ = false;
        } else {
            currentId++;
            //if shadow is enabled, next call for the following glyph will be its shadow
            m_isShadowCall_ = m_shadowOutline;
        }
    }
    //-------------------------------------------------------------------------
    void AnimatedLabel::updateGlyphs( Colibri::States::States state, bool bPlaceGlyphs )
    {
        Label::updateGlyphs( state, bPlaceGlyphs );

        uint8_t i = static_cast<uint8_t>(Colibri::States::Idle);
        m_animData.resize(m_shapes[i].size());
    }

    size_t AnimatedLabel::getNumGlyphs() const
    {
        return m_animData.size();
    }

    bool AnimatedLabel::setGlyphAnimation(uint32 target, uint32_t rgbaColour, float xOffset, float yOffset){
        if(target >= m_animData.size()) return false;

        GlyphAnimationValues& t = m_animData[target];
        t.rgbaColour = rgbaColour;
        t.xOffset = xOffset;
        t.yOffset = yOffset;

        return true;
    }

}
