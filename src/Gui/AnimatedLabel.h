
#pragma once

#define COLIBRI_FLEXIBILITY_LEVEL 2

#include "System/EnginePrerequisites.h"
#include "ColibriGui/ColibriLabel.h"

namespace AV
{

    class AnimatedLabel : public Colibri::Label
    {
    protected:

        struct GlyphAnimationValues
        {
            uint32_t rgbaColour;
            float xOffset;
            float yOffset;
        };
        typedef std::vector<GlyphAnimationValues> GlyphAnimationValueVec;

        GlyphAnimationValueVec m_animData;

        virtual void addQuad( Colibri::GlyphVertex * RESTRICT_ALIAS vertexBuffer,
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
                                Colibri::Matrix2x3 derivedRot );

        virtual void updateGlyphs( Colibri::States::States state, bool bPlaceGlyphs=true );

        virtual void _fillBuffersAndCommands( Colibri::UiVertex * colibri_nonnull * colibri_nonnull
                                    RESTRICT_ALIAS vertexBuffer,
                                    Colibri::GlyphVertex * colibri_nonnull * colibri_nonnull
                                    RESTRICT_ALIAS textVertBuffer,
                                    const Ogre::Vector2 &parentPos,
                                    const Ogre::Vector2 &parentCurrentScrollPos,
                                    const Colibri::Matrix2x3 &parentRot ) final;

    public:
        AnimatedLabel( Colibri::ColibriManager *manager );

        /**
        Set the animation data for a single glyph.
        @param target The id of the glyph which is targeted.
        @returns True or false depending on whether the glyph id is valid.
        */
        bool setGlyphAnimation(uint32 target, uint32_t rgbaColour, float xOffset, float yOffset);
    };
}
