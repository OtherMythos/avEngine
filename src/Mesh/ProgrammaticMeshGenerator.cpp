#include "ProgrammaticMeshGenerator.h"

#include "Ogre.h"
#include "Vao/OgreVaoManager.h"
#include <OgreMeshManager2.h>
#include <OgreMesh2.h>
#include <OgreItem.h>
#include "OgreSubMesh2.h"

namespace AV{
    Ogre::VertexArrayObject* ProgrammaticMeshGenerator::mRectVertexArray = 0;

    void ProgrammaticMeshGenerator::createMesh(){
        generateSphereMesh();
        generateCapsuleMesh();
        generatePlaneMesh();
        generateCubeMesh();
        generateRect2dVao();
        generateLineBox();
        generateLinePoint();
        generateLine();
        generateLineCircleMesh();
        generateLineSphere();

        //TODO there needs to be a procedure to delete these objects.
    }

    void ProgrammaticMeshGenerator::shutdown(){
        Ogre::VaoManager* vaoManager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getVaoManager();

        vaoManager->destroyVertexArrayObject(mRectVertexArray);
    }

    void ProgrammaticMeshGenerator::generateRect2dVao(){
        Ogre::VaoManager* vaoManager = Ogre::Root::getSingletonPtr()->getRenderSystem()->getVaoManager();

        /* Define our face index data */
        const Ogre::uint16 indexData[6] = { 0, 1, 2, 2, 3, 0 };

        /* Copy it to a SIMD array (the memory will be managed by the vao */
        Ogre::uint16* faceIndices = reinterpret_cast<Ogre::uint16*>(
              OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * 6,
                               Ogre::MEMCATEGORY_GEOMETRY) );
        memcpy(faceIndices, indexData, sizeof(indexData));

        /* Let's create the index buffer */
        Ogre::IndexBufferPacked* indexBuffer = 0;
        try
        {
           indexBuffer = vaoManager->createIndexBuffer(
                 Ogre::IndexBufferPacked::IT_16BIT, 6, Ogre::BT_IMMUTABLE,
                 faceIndices, true);
        }
        catch(Ogre::Exception& e)
        {
           /* With exceptions, we should need to free it */
           OGRE_FREE_SIMD(indexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
           indexBuffer = NULL;
           throw e;
        }

        /* Define our vertices elements: Positions and UVs */
        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3,
                 Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2,
                 Ogre::VES_TEXTURE_COORDINATES));

        /* Define our coordinates */
        float x1 = 0.0f, y1 = 0.0f, x2 = 1.0f, y2 = -1.0f;
        //calculateCoordinates(x1, y1, x2, y2);

        /* Define our vertices (with UVs)  */
        const float faceVertices[4 * 5] =
        {
           x2, y2, 0.0f, 1.0f, 1.0f,
           x2, y1, 0.0f, 1.0f, 0.0f,
           x1, y1, 0.0f, 0.0f, 0.0f,
           x1, y2, 0.0f, 0.0f, 1.0f
       };

        /* Let's copy it to a SIMD array */
        float *vertices = reinterpret_cast<float*>(OGRE_MALLOC_SIMD(
                 sizeof(float) * 4 * 5, Ogre::MEMCATEGORY_GEOMETRY));
        memcpy(vertices, faceVertices, sizeof(float) * 4 * 5);

        /* And create our packed vertex buffer */
        Ogre::VertexBufferPacked* vertexBuffer;
        try
        {
            vertexBuffer = vaoManager->createVertexBuffer(vertexElements, 4,
                  Ogre::BT_DYNAMIC_PERSISTENT, vertices, false);
        }
        catch(Ogre::Exception &e)
        {
           OGRE_FREE_SIMD(vertexBuffer, Ogre::MEMCATEGORY_GEOMETRY);
           vertexBuffer = NULL;
           throw e;
        }

        /* Finally, the Vao. */
        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back(vertexBuffer);

        mRectVertexArray = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer,
              Ogre::OT_TRIANGLE_LIST);
    }

    Ogre::MeshPtr ProgrammaticMeshGenerator::generateSphereMesh(){
        static const int stackCount = 10;
        static const int sectorCount = 20;
        static const int radius = 1;

        static const float PI_Val = 3.141592653;


        std::vector<int> indices;
        // generate CCW index list of sphere triangles
        for(int i = 0; i < stackCount; ++i){
            int k1 = i * (sectorCount + 1);     // beginning of current stack
            int k2 = k1 + sectorCount + 1;      // beginning of next stack

            for(int j = 0; j < sectorCount; ++j, ++k1, ++k2){
                if(i != 0){
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if(i != (stackCount-1)){
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }

        //Windows (msvc) doesn't like compiling variable length arrays on the stack, so I have to do it as pointers.
        Ogre::uint16* c_indexData = new Ogre::uint16[indices.size()];

        for(int i = 0; i < indices.size(); i++){
            c_indexData[i] = indices[i];
        }

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(indices.size(), c_indexData);

        std::vector<float> verts;

        float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
        float s, t;                                     // vertex texCoord

        const float sectorStep = 2 * PI_Val / sectorCount;
        const float stackStep = PI_Val / stackCount;

        for(int i = 0; i <= stackCount; ++i){
            float stackAngle = PI_Val / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);             // r * cos(u)
            float z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectorCount+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for(int j = 0; j <= sectorCount; ++j){
                float sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position (x, y, z)
                float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);

                // normalized vertex normal (nx, ny, nz)
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                verts.push_back(nx);
                verts.push_back(ny);
                verts.push_back(nz);

                /*// vertex tex coord (s, t) range between [0, 1]
                s = (float)j / sectorCount;
                t = (float)i / stackCount;
                texCoords.push_back(s);
                texCoords.push_back(t);*/
            }
        }

        const int cubeVerticesCount = verts.size();
        float* c_originalVertices = new float[cubeVerticesCount];

        for(int i = 0; i < verts.size(); i++){
            c_originalVertices[i] = verts[i];
        }

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));

        Ogre::MeshPtr retMesh = createStaticMesh("sphere", indexBuffer, vertexElements, cubeVerticesCount, c_originalVertices, Ogre::OT_TRIANGLE_LIST, 6);

        delete[] c_indexData;
        delete[] c_originalVertices;

        return retMesh;

    }

    void ProgrammaticMeshGenerator::generateLineCircleMesh(){
        static const float radius = 1.0f;
        static const float PI_VAL = 3.141592653f;
        static const int NUM_CIRCLE_VIRTS = 30;

        const int indexSize = NUM_CIRCLE_VIRTS * 2;
        Ogre::uint16 indexData[indexSize];

        for(int i = 0; i < NUM_CIRCLE_VIRTS; i++){
            indexData[i * 2] = i;
            if(i == NUM_CIRCLE_VIRTS-1){
                indexData[i * 2 + 1] = 0;
            }else{
                indexData[i * 2 + 1] = i + 1;
            }
        }

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(indexSize, indexData);

        const int vertexListSize = NUM_CIRCLE_VIRTS * 3;
        float originalVertices[vertexListSize];
        for(int i = 0; i < NUM_CIRCLE_VIRTS; i++){
            const float piVal = (2 * PI_VAL) * (float(i) / NUM_CIRCLE_VIRTS);
            const float x = radius * cosf(piVal);
            const float z = radius * sinf(piVal);
            originalVertices[i * 3] = x;
            originalVertices[i * 3 + 1] = 0.0f;
            originalVertices[i * 3 + 2] = z;
        }

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

        Ogre::MeshPtr retMesh = createStaticMesh("lineCircle", indexBuffer, vertexElements, vertexListSize, originalVertices, Ogre::OT_LINE_LIST);
    }

    void ProgrammaticMeshGenerator::generateLineSphere(){
        static const float radius = 1.0f;
        static const float PI_VAL = 3.141592653f;
        static const int NUM_CIRCLE_VIRTS = 30;
        static const int NUM_CIRCLES = 3;

        const int indexSize = NUM_CIRCLE_VIRTS * 2 * NUM_CIRCLES;
        Ogre::uint16 indexData[indexSize];

        for(int c = 0; c < NUM_CIRCLES; c++){
            for(int i = 0; i < NUM_CIRCLE_VIRTS; i++){
                indexData[c * (NUM_CIRCLE_VIRTS * 2) + i * 2] = (c * NUM_CIRCLE_VIRTS) + i;
                if(i == NUM_CIRCLE_VIRTS-1){
                    indexData[c * (NUM_CIRCLE_VIRTS * 2) + i * 2 + 1] = (c * NUM_CIRCLE_VIRTS) + 0;
                }else{
                    indexData[c * (NUM_CIRCLE_VIRTS * 2) + i * 2 + 1] = (c * NUM_CIRCLE_VIRTS) + i + 1;
                }
            }
        }

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(indexSize, indexData);

        const int vertexListSize = NUM_CIRCLE_VIRTS * 3 * NUM_CIRCLES;
        float originalVertices[vertexListSize];
        for(int c = 0; c < NUM_CIRCLES; c++){
            for(int i = 0; i < NUM_CIRCLE_VIRTS; i++){
                const float piVal = (2 * PI_VAL) * (float(i) / NUM_CIRCLE_VIRTS);
                const float x = radius * cosf(piVal);
                const float z = radius * sinf(piVal);
                if(c == 0){
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3] = x;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 1] = 0.0f;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 2] = z;
                }else if(c == 1){
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3] = 0.0f;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 1] = x;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 2] = z;
                }else{
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3] = x;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 1] = z;
                    originalVertices[c * (NUM_CIRCLE_VIRTS * 3) + i * 3 + 2] = 0.0f;
                }
            }
        }

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

        createStaticMesh("lineSphere", indexBuffer, vertexElements, vertexListSize, originalVertices, Ogre::OT_LINE_LIST);
    }

    Ogre::MeshPtr ProgrammaticMeshGenerator::generateCapsuleMesh(){
        static const int sectorCount = 20;          // radial segments around the capsule
        static const int hemiStacks = 8;            // stacks per hemispherical cap
        static const float radius = 1.0f;
        static const float cylinderHeight = 2.0f;   // straight section between the two caps

        static const float PI_Val = 3.141592653f;
        static const float halfPi = PI_Val / 2.0f;

        //A capsule is two hemispheres joined by a cylinder. It's built as an ordered list of
        //horizontal rings from the top pole down to the bottom pole, then every consecutive
        //pair of rings is stitched together. Crucially, the gap between the top cap's equator
        //ring and the bottom cap's equator ring is bridged by that same stitching, which
        //produces the cylindrical band - so the band shares its edge vertices with the caps
        //and is always flush against them. (The old version displaced sphere vertices along
        //an axis perpendicular to the tessellation poles, so the "band" was just stretched
        //sphere triangles that never lined up with the caps.)
        //
        //The pole axis is y, and the origin sits at the bottom pole (y = 0) so the capsule
        //rests on the floor rather than clipping through it:
        //  bottom pole y=0, bottom cap centre y=radius, top cap centre y=radius+cylinderHeight,
        //  top pole y=2*radius+cylinderHeight.
        const float bottomCentre = radius;
        const float topCentre = radius + cylinderHeight;
        const float sectorStep = 2.0f * PI_Val / sectorCount;
        const float stackStep = halfPi / hemiStacks;

        //Latitude and cap-centre (y offset) of each ring, ordered top pole first.
        std::vector<float> ringLat;
        std::vector<float> ringCentre;
        //Top cap: latitude +pi/2 (pole) down to 0 (equator).
        for(int i = 0; i <= hemiStacks; i++){
            ringLat.push_back(halfPi - i * stackStep);
            ringCentre.push_back(topCentre);
        }
        //Bottom cap: latitude 0 (equator) down to -pi/2 (pole).
        for(int i = 0; i <= hemiStacks; i++){
            ringLat.push_back(-i * stackStep);
            ringCentre.push_back(bottomCentre);
        }
        const int ringCount = (int)ringLat.size();

        std::vector<float> verts;
        for(int r = 0; r < ringCount; r++){
            const float lat = ringLat[r];
            const float cosLat = cosf(lat);
            const float sinLat = sinf(lat);
            const float ringRadius = radius * cosLat;
            const float y = ringCentre[r] + radius * sinLat;
            for(int j = 0; j <= sectorCount; j++){
                const float sectorAngle = j * sectorStep;
                const float cosS = cosf(sectorAngle);
                const float sinS = sinf(sectorAngle);

                //position
                verts.push_back(ringRadius * cosS);
                verts.push_back(y);
                verts.push_back(ringRadius * sinS);

                //outward normal from the cap centre. At the equator rings (lat 0) this is
                //horizontal, which is exactly the cylinder's normal, so the band is smooth.
                verts.push_back(cosLat * cosS);
                verts.push_back(sinLat);
                verts.push_back(cosLat * sinS);
            }
        }

        //Stitch consecutive rings, skipping the degenerate triangle at each pole.
        std::vector<int> indices;
        for(int r = 0; r < ringCount - 1; r++){
            for(int j = 0; j < sectorCount; j++){
                const int a = r * (sectorCount + 1) + j;        // upper ring, this sector
                const int b = a + 1;                            // upper ring, next sector
                const int c = (r + 1) * (sectorCount + 1) + j;  // lower ring, this sector
                const int d = c + 1;                            // lower ring, next sector

                if(r != 0){                     // top pole: the upper ring collapses to a point
                    indices.push_back(a);
                    indices.push_back(b);
                    indices.push_back(c);
                }
                if(r != ringCount - 2){         // bottom pole: the lower ring collapses to a point
                    indices.push_back(b);
                    indices.push_back(d);
                    indices.push_back(c);
                }
            }
        }

        //Windows (msvc) doesn't like compiling variable length arrays on the stack, so I have to do it as pointers.
        Ogre::uint16* c_indexData = new Ogre::uint16[indices.size()];
        for(int i = 0; i < indices.size(); i++){
            c_indexData[i] = indices[i];
        }
        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(indices.size(), c_indexData);

        const int capsuleVerticesCount = verts.size();
        float* c_originalVertices = new float[capsuleVerticesCount];
        for(int i = 0; i < verts.size(); i++){
            c_originalVertices[i] = verts[i];
        }

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));

        Ogre::MeshPtr retMesh = createStaticMesh("capsule", indexBuffer, vertexElements, capsuleVerticesCount, c_originalVertices, Ogre::OT_TRIANGLE_LIST, 6);

        //The capsule spans y ∈ [0, 2*radius + cylinderHeight] and x/z ∈ [-radius, radius], so
        //replace the default unit bounds set by createStaticMesh with one that encloses it.
        const float halfHeight = radius + cylinderHeight * 0.5f;
        retMesh->_setBounds(Ogre::Aabb(Ogre::Vector3(0.0f, halfHeight, 0.0f), Ogre::Vector3(radius, halfHeight, radius)), false);
        retMesh->_setBoundingSphereRadius(sqrtf(radius * radius + halfHeight * halfHeight + radius * radius));

        delete[] c_indexData;
        delete[] c_originalVertices;

        return retMesh;

    }

    void ProgrammaticMeshGenerator::generateLinePoint(){
        static const int cubeArraySize = 3 * 2;
        static const Ogre::uint16 c_indexData[cubeArraySize]{
            0, 1,
            2, 3,
            4, 5
        };

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(cubeArraySize, &c_indexData[0]);


        static const int cubeVerticesCount = 3 * 6;
        static const float c_originalVertices[cubeVerticesCount] = {
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, 1.0f
        };

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

        createStaticMesh("linePoint", indexBuffer, vertexElements, cubeVerticesCount, &c_originalVertices[0], Ogre::OT_LINE_LIST);
    }

    void ProgrammaticMeshGenerator::generateLine(){
        static const int cubeArraySize = 1 * 2;
        static const Ogre::uint16 c_indexData[cubeArraySize]{
            0, 1
        };
        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(cubeArraySize, &c_indexData[0]);

        static const int cubeVerticesCount = 3 * 2;
        static const float c_originalVertices[cubeVerticesCount] = {
            0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

        createStaticMesh("line", indexBuffer, vertexElements, cubeVerticesCount, &c_originalVertices[0], Ogre::OT_LINE_LIST);
    }

    void ProgrammaticMeshGenerator::generateLineBox(){
        static const int cubeArraySize = 2 * 3 * 4;
        static const Ogre::uint16 c_indexData[cubeArraySize]{
            0, 0 + 1,
            0, 0 + 2,
            0, 0 + 3,

            4, 4 + 1,
            4, 4 + 2,
            4, 4 + 3,

            8, 8 + 1,
            8, 8 + 2,
            8, 8 + 3,

            12, 12 + 1,
            12, 12 + 2,
            12, 12 + 3,
        };

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(cubeArraySize, &c_indexData[0]);


        static const int cubeVerticesCount = 3 * 4 * 4;
        static const float offset = 0.2f;
        static const float c_originalVertices[cubeVerticesCount] = {
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,

            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
        };

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));

        createStaticMesh("lineBox", indexBuffer, vertexElements, cubeVerticesCount, &c_originalVertices[0], Ogre::OT_LINE_LIST);
    }

    Ogre::MeshPtr ProgrammaticMeshGenerator::generatePlaneMesh(){
        const Ogre::uint16 c_indexData[6] = { 0, 1, 2, 2, 3, 0 };

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(6, &c_indexData[0]);

        const float x1 = -1.0f, y1 = 1.0f, x2 = 1.0f, y2 = -1.0f;

        const float faceVertices[4 * 5] =
        {
           x2, y2, 0.0f, 1.0f, 1.0f,
           x2, y1, 0.0f, 1.0f, 0.0f,
           x1, y1, 0.0f, 0.0f, 0.0f,
           x1, y2, 0.0f, 0.0f, 1.0f
       };

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES));

        return createStaticMesh("plane", indexBuffer, vertexElements, 4 * 5, &faceVertices[0], Ogre::OT_TRIANGLE_LIST, 5);
    }

    Ogre::MeshPtr ProgrammaticMeshGenerator::generateCubeMesh(){
        static const int cubeArraySize = 12 * 3;
        static const Ogre::uint16 c_indexData[cubeArraySize]{
            0, 1, 2,
            0, 2, 3,
            4, 5, 6,
            4, 6, 7,
            8, 9, 10,
            8, 10, 11,
            12, 13, 14,
            12, 14, 15,
            16, 17, 18,
            16, 18, 19,
            20, 21, 22,
            20, 22, 23
        };

        Ogre::IndexBufferPacked *indexBuffer = createIndexBuffer(cubeArraySize, &c_indexData[0]);


        static const int cubeVerticesCount = 6 * 4 * 6;
        static const float c_originalVertices[cubeVerticesCount] = {
            1.000000,-1.000000,-1.000000, 0.000000,-1.000000,-0.000000,
            1.000000,-1.000000,1.000000, 0.000000,-1.000000,-0.000000,
            -1.000000,-1.000000,1.000000, 0.000000,-1.000000,-0.000000,
            -1.000000,-1.000000,-1.000000, 0.000000,-1.000000,-0.000000,

            1.000000,1.000000,-1.000000, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,-1.000000, 0.000000,1.000000,0.000000,
            -1.000000,1.000000,1.000000, 0.000000,1.000000,0.000000,
            1.000000,1.000000,1.000001, 0.000000,1.000000,0.000000,

            1.000000,-1.000000,-1.000000, 1.000000,0.000000,0.000000,
            1.000000,1.000000,-1.000000, 1.000000,0.000000,0.000000,
            1.000000,1.000000,1.000001, 1.000000,0.000000,0.000000,
            1.000000,-1.000000,1.000000, 1.000000,0.000000,0.000000,

            1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,
            1.000000,1.000000,1.000001, -0.000000,-0.000000,1.000000,
            -1.000000,1.000000,1.000000, -0.000000,-0.000000,1.000000,
            -1.000000,-1.000000,1.000000, -0.000000,-0.000000,1.000000,

            -1.000000,-1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,1.000000,-1.000000, -1.000000,-0.000000,-0.000000,
            -1.000000,-1.000000,-1.000000, -1.000000,-0.000000,-0.000000,

            1.000000,1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,-1.000000,-1.000000, 0.000000,0.000000,-1.000000,
            -1.000000,1.000000,-1.000000, 0.000000,0.000000,-1.000000
        };

        Ogre::VertexElement2Vec vertexElements;
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_POSITION));
        vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT3, Ogre::VES_NORMAL));

        //TODO error I divide by 3 but sometimes the stride is 6.
        return createStaticMesh("cube", indexBuffer, vertexElements, cubeVerticesCount, &c_originalVertices[0], Ogre::OT_TRIANGLE_LIST, 6);
    }

    Ogre::IndexBufferPacked* ProgrammaticMeshGenerator::createIndexBuffer(int cubeArraySize, const Ogre::uint16* indexData){
        Ogre::IndexBufferPacked *indexBuffer = 0;

        Ogre::uint16 *cubeIndices = reinterpret_cast<Ogre::uint16*>( OGRE_MALLOC_SIMD(sizeof(Ogre::uint16) * cubeArraySize, Ogre::MEMCATEGORY_GEOMETRY));
        //memcpy( cubeIndices, indexData, sizeof( *indexData ) );
        memcpy( cubeIndices, indexData, sizeof(Ogre::uint16) * cubeArraySize );

        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        try{
            //Actually create an index buffer and assign it to the pointer created earlier.
            //Also populate the index buffer with these values.
            //This goes, type, number of indices, Buffer type, the actual data, keep as shadow
            indexBuffer = vaoManager->createIndexBuffer(Ogre::IndexBufferPacked::IT_16BIT, cubeArraySize, Ogre::BT_IMMUTABLE, cubeIndices, true);
        }
        catch(Ogre::Exception &e){
            OGRE_FREE_SIMD( indexBuffer, Ogre::MEMCATEGORY_GEOMETRY );
            indexBuffer = 0;
            throw e;
        }

        return indexBuffer;
    }


    Ogre::MeshPtr ProgrammaticMeshGenerator::createStaticMesh(const Ogre::String& name, Ogre::IndexBufferPacked *indexBuffer, const Ogre::VertexElement2Vec& vertexElements, int arraySize, const float* vertexData, Ogre::OperationType t, int strideSize){
        Ogre::RenderSystem *renderSystem = Ogre::Root::getSingletonPtr()->getRenderSystem();
        Ogre::VaoManager *vaoManager = renderSystem->getVaoManager();

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createManual(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        Ogre::SubMesh *subMesh = mesh->createSubMesh();

        //vertexElements.push_back(Ogre::VertexElement2(Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES)


        float *cubeVertices = reinterpret_cast<float*>( OGRE_MALLOC_SIMD(sizeof(float) * arraySize, Ogre::MEMCATEGORY_GEOMETRY ) );

        memcpy(cubeVertices, vertexData, sizeof(float) * arraySize);

        Ogre::VertexBufferPacked *vertexBuffer = 0;
        try{
            vertexBuffer = vaoManager->createVertexBuffer(vertexElements, arraySize/strideSize, Ogre::BT_DEFAULT, cubeVertices, true);
        }catch(Ogre::Exception &e){
            vertexBuffer = 0;
            throw e;
        }

        Ogre::VertexBufferPackedVec vertexBuffers;
        vertexBuffers.push_back(vertexBuffer);

        Ogre::VertexArrayObject* vao = vaoManager->createVertexArrayObject(vertexBuffers, indexBuffer, t);

        subMesh->mVao[Ogre::VpNormal].push_back(vao);
        subMesh->mVao[Ogre::VpShadow].push_back(vao);

        mesh->_setBounds( Ogre::Aabb( Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_SCALE ), false );
        mesh->_setBoundingSphereRadius( 1.732f );

        return mesh;
    }


}
