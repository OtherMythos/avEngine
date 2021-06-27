
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

    void AnimatedLabel::_fillBuffersAndCommands( Colibri::UiVertex * colibrigui_nonnull * colibrigui_nonnull
                                        RESTRICT_ALIAS vertexBuffer,
                                        Colibri::GlyphVertex * colibrigui_nonnull * colibrigui_nonnull
                                        RESTRICT_ALIAS textVertBuffer,
                                        const Ogre::Vector2 &parentPos,
                                        const Ogre::Vector2 &parentCurrentScrollPos,
                                        const Colibri::Matrix2x3 &parentRot )
    {
        currentId = 0;
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

        uint32_t finalColour = rgbaColour + animData.rgbaColour;
        topLeft += Ogre::Vector2(animData.xOffset, animData.yOffset);
        bottomRight += Ogre::Vector2(animData.xOffset, animData.yOffset);

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

        currentId++;
    }
    //-------------------------------------------------------------------------
    void AnimatedLabel::updateGlyphs( Colibri::States::States state, bool bPlaceGlyphs )
    {
        Label::updateGlyphs( state, bPlaceGlyphs );

        uint8_t i = static_cast<uint8_t>(Colibri::States::Idle);
        m_animData.resize(m_shapes[i].size());
    }

}
