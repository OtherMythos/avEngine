#pragma once

#include "OgreDataStream.h"
#include "OgreVector3.h"

#include <string>
#include <vector>

namespace AV{

    /**
    CPU side representation of a parsed obj file.
    Vertices are interleaved as position (3 floats), normal (3 floats), uv (2 floats).
    Indices are always 32 bit here. Whether the gpu buffer should be 16 or 32 bit
    can be decided by the consumer based on numVertices().
    */
    struct ObjMeshData{
        static const size_t ELEMENTS_PER_VERTEX = 8;

        struct SubMesh{
            //Populated by a usemtl definition. Empty if none was provided.
            std::string materialName;
            std::vector<Ogre::uint32> indices;
        };

        std::vector<float> vertices;
        std::vector<SubMesh> subMeshes;

        Ogre::Vector3 boundsMin;
        Ogre::Vector3 boundsMax;

        size_t numVertices() const { return vertices.size() / ELEMENTS_PER_VERTEX; }
    };

    /**
    A parser for wavefront obj files.
    Supports v, vt, vn, f (including quads and n-gons, which are fan triangulated),
    negative (relative) indices and submesh separation by usemtl.
    Material (.mtl) files are not supported. o, g, s and mtllib definitions are ignored.
    If the file contains no normals they are generated as smooth vertex normals.
    */
    class ObjMeshParser{
    public:
        ObjMeshParser() = delete;

        /**
        Parse an obj file from a string.
        @returns True on success. On failure outError contains a description of the problem.
        */
        static bool parse(const std::string& content, ObjMeshData& outData, std::string& outError);

        /**
        Parse an obj file from an Ogre data stream.
        */
        static bool parse(Ogre::DataStreamPtr& stream, ObjMeshData& outData, std::string& outError);
    };
}
