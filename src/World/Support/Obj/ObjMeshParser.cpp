#include "ObjMeshParser.h"

#include "OgreVector2.h"

#include <cstdlib>
#include <cstring>
#include <unordered_map>

namespace AV{

    namespace{

        //A single face corner, i.e v/vt/vn. Values are 0 based, -1 meaning not provided.
        struct IndexTriplet{
            int v;
            int t;
            int n;

            bool operator==(const IndexTriplet& o) const{
                return v == o.v && t == o.t && n == o.n;
            }
        };

        struct IndexTripletHash{
            size_t operator()(const IndexTriplet& t) const{
                size_t seed = std::hash<int>()(t.v);
                seed ^= std::hash<int>()(t.t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                seed ^= std::hash<int>()(t.n) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                return seed;
            }
        };

        struct ParserContext{
            std::vector<Ogre::Vector3> positions;
            std::vector<Ogre::Vector2> uvs;
            std::vector<Ogre::Vector3> normals;

            std::unordered_map<IndexTriplet, Ogre::uint32, IndexTripletHash> vertexMap;
            //Output vertices which had no normal provided, so one should be generated.
            std::vector<Ogre::uint32> pendingNormals;

            ObjMeshData* out;
            size_t lineNum;
            std::string* error;

            bool fail(const std::string& message){
                *error = "Line " + std::to_string(lineNum) + ": " + message;
                return false;
            }
        };

        void splitTokens(char* line, std::vector<char*>& outTokens){
            outTokens.clear();
            char* c = line;
            while(*c != '\0'){
                while(*c == ' ' || *c == '\t' || *c == '\r') *(c++) = '\0';
                if(*c == '\0') break;
                outTokens.push_back(c);
                while(*c != '\0' && *c != ' ' && *c != '\t' && *c != '\r') c++;
            }
        }

        bool parseFloats(const std::vector<char*>& tokens, float* outVals, size_t count){
            if(tokens.size() < count + 1) return false;
            for(size_t i = 0; i < count; i++){
                char* end = 0;
                outVals[i] = strtof(tokens[i + 1], &end);
                if(end == tokens[i + 1]) return false;
            }
            return true;
        }

        //Resolve an obj 1 based or negative relative index to a 0 based index.
        bool resolveIndex(long idx, size_t count, int* outIdx){
            if(idx > 0 && static_cast<size_t>(idx) <= count){
                *outIdx = static_cast<int>(idx - 1);
                return true;
            }
            if(idx < 0 && static_cast<size_t>(-idx) <= count){
                *outIdx = static_cast<int>(count + idx);
                return true;
            }
            return false;
        }

        bool parseFaceCorner(const char* token, ParserContext& ctx, IndexTriplet* outTriplet){
            outTriplet->v = outTriplet->t = outTriplet->n = -1;

            char* next = 0;
            long idx = strtol(token, &next, 10);
            if(next == token) return false;
            if(!resolveIndex(idx, ctx.positions.size(), &(outTriplet->v))) return false;
            if(*next == '\0') return true;
            if(*next != '/') return false;

            next++;
            if(*next != '/'){
                //A vt index is present.
                const char* start = next;
                idx = strtol(start, &next, 10);
                if(next == start) return false;
                if(!resolveIndex(idx, ctx.uvs.size(), &(outTriplet->t))) return false;
                if(*next == '\0') return true;
            }
            if(*next != '/') return false;

            next++;
            const char* start = next;
            idx = strtol(start, &next, 10);
            if(next == start) return false;
            if(!resolveIndex(idx, ctx.normals.size(), &(outTriplet->n))) return false;
            return *next == '\0';
        }

        Ogre::uint32 emitVertex(const IndexTriplet& triplet, ParserContext& ctx){
            auto it = ctx.vertexMap.find(triplet);
            if(it != ctx.vertexMap.end()) return it->second;

            ObjMeshData& data = *(ctx.out);
            Ogre::uint32 newIdx = static_cast<Ogre::uint32>(data.numVertices());

            const Ogre::Vector3& pos = ctx.positions[triplet.v];
            const Ogre::Vector3 normal = triplet.n >= 0 ? ctx.normals[triplet.n] : Ogre::Vector3::ZERO;
            const Ogre::Vector2 uv = triplet.t >= 0 ? ctx.uvs[triplet.t] : Ogre::Vector2::ZERO;

            data.vertices.push_back(pos.x);
            data.vertices.push_back(pos.y);
            data.vertices.push_back(pos.z);
            data.vertices.push_back(normal.x);
            data.vertices.push_back(normal.y);
            data.vertices.push_back(normal.z);
            data.vertices.push_back(uv.x);
            data.vertices.push_back(uv.y);

            if(newIdx == 0){
                data.boundsMin = data.boundsMax = pos;
            }else{
                data.boundsMin.makeFloor(pos);
                data.boundsMax.makeCeil(pos);
            }

            if(triplet.n < 0) ctx.pendingNormals.push_back(newIdx);

            ctx.vertexMap[triplet] = newIdx;
            return newIdx;
        }

        Ogre::Vector3 readPosition(const ObjMeshData& data, Ogre::uint32 idx){
            const float* v = &(data.vertices[idx * ObjMeshData::ELEMENTS_PER_VERTEX]);
            return Ogre::Vector3(v[0], v[1], v[2]);
        }

        void accumulateNormal(ObjMeshData& data, Ogre::uint32 idx, const Ogre::Vector3& normal){
            float* v = &(data.vertices[idx * ObjMeshData::ELEMENTS_PER_VERTEX]);
            v[3] += normal.x;
            v[4] += normal.y;
            v[5] += normal.z;
        }
    }

    bool ObjMeshParser::parse(Ogre::DataStreamPtr& stream, ObjMeshData& outData, std::string& outError){
        const std::string content = stream->getAsString();
        return parse(content, outData, outError);
    }

    bool ObjMeshParser::parse(const std::string& content, ObjMeshData& outData, std::string& outError){
        outData.vertices.clear();
        outData.subMeshes.clear();
        outData.boundsMin = outData.boundsMax = Ogre::Vector3::ZERO;

        ParserContext ctx;
        ctx.out = &outData;
        ctx.lineNum = 0;
        ctx.error = &outError;

        //The current line, copied so the tokeniser can insert terminators.
        std::vector<char> lineBuf;
        std::vector<char*> tokens;
        std::vector<IndexTriplet> corners;

        outData.subMeshes.push_back(ObjMeshData::SubMesh());
        ObjMeshData::SubMesh* currentSubMesh = &(outData.subMeshes.back());

        size_t lineStart = 0;
        bool hasFaces = false;
        while(lineStart < content.size()){
            ctx.lineNum++;
            size_t lineEnd = content.find('\n', lineStart);
            if(lineEnd == std::string::npos) lineEnd = content.size();

            lineBuf.assign(content.begin() + lineStart, content.begin() + lineEnd);
            lineBuf.push_back('\0');
            lineStart = lineEnd + 1;

            splitTokens(lineBuf.data(), tokens);
            if(tokens.empty() || tokens[0][0] == '#') continue;

            const char* keyword = tokens[0];
            if(strcmp(keyword, "v") == 0){
                float vals[3];
                if(!parseFloats(tokens, vals, 3)) return ctx.fail("Invalid vertex definition.");
                ctx.positions.push_back(Ogre::Vector3(vals[0], vals[1], vals[2]));
            }
            else if(strcmp(keyword, "vt") == 0){
                float vals[2];
                if(!parseFloats(tokens, vals, 2)) return ctx.fail("Invalid texture coordinate definition.");
                //Obj uv coordinates have their origin bottom left, Ogre expects top left.
                ctx.uvs.push_back(Ogre::Vector2(vals[0], 1.0f - vals[1]));
            }
            else if(strcmp(keyword, "vn") == 0){
                float vals[3];
                if(!parseFloats(tokens, vals, 3)) return ctx.fail("Invalid normal definition.");
                ctx.normals.push_back(Ogre::Vector3(vals[0], vals[1], vals[2]));
            }
            else if(strcmp(keyword, "f") == 0){
                if(tokens.size() < 4) return ctx.fail("A face requires at least three vertices.");
                corners.clear();
                for(size_t i = 1; i < tokens.size(); i++){
                    IndexTriplet triplet;
                    if(!parseFaceCorner(tokens[i], ctx, &triplet)){
                        return ctx.fail("Invalid face index '" + std::string(tokens[i]) + "'.");
                    }
                    corners.push_back(triplet);
                }

                const Ogre::uint32 first = emitVertex(corners[0], ctx);
                for(size_t i = 1; i + 1 < corners.size(); i++){
                    const Ogre::uint32 second = emitVertex(corners[i], ctx);
                    const Ogre::uint32 third = emitVertex(corners[i + 1], ctx);
                    currentSubMesh->indices.push_back(first);
                    currentSubMesh->indices.push_back(second);
                    currentSubMesh->indices.push_back(third);

                    //Accumulate a face normal into any of the corners which did not provide one.
                    if(corners[0].n < 0 || corners[i].n < 0 || corners[i + 1].n < 0){
                        const Ogre::Vector3 a = readPosition(outData, first);
                        const Ogre::Vector3 faceNormal =
                            (readPosition(outData, second) - a).crossProduct(readPosition(outData, third) - a);
                        if(corners[0].n < 0) accumulateNormal(outData, first, faceNormal);
                        if(corners[i].n < 0) accumulateNormal(outData, second, faceNormal);
                        if(corners[i + 1].n < 0) accumulateNormal(outData, third, faceNormal);
                    }
                }
                hasFaces = true;
            }
            else if(strcmp(keyword, "usemtl") == 0){
                const std::string materialName = tokens.size() >= 2 ? tokens[1] : "";
                if(currentSubMesh->indices.empty()){
                    currentSubMesh->materialName = materialName;
                }else{
                    outData.subMeshes.push_back(ObjMeshData::SubMesh());
                    currentSubMesh = &(outData.subMeshes.back());
                    currentSubMesh->materialName = materialName;
                }
            }
            //o, g, s, mtllib and anything unrecognised are ignored.
        }

        if(!hasFaces){
            outError = "The file contains no faces.";
            return false;
        }

        //A usemtl definition after the final face leaves an empty submesh behind.
        if(outData.subMeshes.back().indices.empty()) outData.subMeshes.pop_back();

        //Normalise the generated normals.
        for(Ogre::uint32 idx : ctx.pendingNormals){
            float* v = &(outData.vertices[idx * ObjMeshData::ELEMENTS_PER_VERTEX]);
            Ogre::Vector3 normal(v[3], v[4], v[5]);
            if(normal.squaredLength() > 0.0f) normal.normalise();
            else normal = Ogre::Vector3::UNIT_Y;
            v[3] = normal.x;
            v[4] = normal.y;
            v[5] = normal.z;
        }

        return true;
    }
}
