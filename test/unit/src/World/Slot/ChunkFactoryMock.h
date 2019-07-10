#pragma once

class ChunkFactoryMock : public AV::ChunkFactory{
public:
    ChunkFactoryMock() : ChunkFactory(0) {}

    MOCK_METHOD0(initialise, void());
    MOCK_METHOD2(startRecipeJob, void(AV::RecipeData* data, int targetIndex));
};
