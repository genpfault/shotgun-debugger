/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * md2.h - header for MD2 model loading routines
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * MD2 model loader by DigiBen, www.gametutorials.com. Adapted from his code.
 * The MD2 model format is copyright id software, inc.
 */

#ifndef _MD2_H_
#define _MD2_H_

// MD2 loading code by DigiBen, www.gametutorials.com, adapted by me for SDL //

// These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES       4096
#define MD2_MAX_VERTICES        2048
#define MD2_MAX_TEXCOORDS       2048
#define MD2_MAX_FRAMES          512
#define MD2_MAX_SKINS           32
#define MD2_MAX_FRAMESIZE       (MD2_MAX_VERTICES * 4 + 128)

#define MAX_TEXTURES 100                                // The maximum amount of textures to load

#define ANIMATION_SPEED         25.0f

// It was a lot easier for me to keep Ben's vector classes even though I have my own...
// "A good programmer is a lazy programmer..."
class CVector3
{
public:
    float x, y, z;
};

class CVector2
{
public:
    float x, y;
};


struct tFace
{
    int vertIndex[3];
    int coordIndex[3];
};

struct tMaterialInfo
{
    char  strName[255];
    char  strFile[255];
    byte color[3];
    int   textureId;
    float uTile;
    float vTile;
    float uOffset;
    float vOffset;
} ;

struct tAnimationInfo
{
    char strName[255];
    int startFrame;
    int endFrame;
};

class ModelObject
{
public:
    ModelObject() {}
    void draw();
    void destroy();

    int numOfVerts;
    int numOfFaces;
    int numTexVertex;
    int materialID;
    bool hasTexture;
    char strName[255];
    CVector3* pVerts;
    CVector3* pNormals;
    CVector2* pTexVerts;
    tFace* pFaces;
};

class Model
{
public:
    Model()
    {
        scale = 1.0;
        memset( rot, 0, 3 * sizeof( float ) );
        zTranslate = false;
    }
    void draw( int currentAnim, int currentFrame, int nextFrame, float time, float speed );
    void setTransforms( float sc, float rotX, float rotY, float rotZ, float trX, float trY, float trZ )
    {
        scale = sc;
        rot[X] = rotX;
        rot[Y] = rotY;
        rot[Z] = rotZ;
        trans[X] = trX;
        trans[Y] = trY;
        trans[Z] = trZ;
    }

    void destroy();
    void animate( int currentAnim, int currentFrame, int nextFrame, float time, float speed );
    int texture;

    int numOfObjects;
    int numOfMaterials;
    int numOfAnimations;
    int* m_glCommandBuffer;
    vector<tAnimationInfo> pAnimations;
    vector<tMaterialInfo> pMaterials;
    vector<ModelObject> pObject;

    float scale;
    float rot[3];
    float trans[3];
    float zTranslate;
};

struct tMd2Header
{
    int magic;          // This is used to identify the file
    int version;          // The version number of the file (Must be 8)
    int skinWidth;        // The skin width in pixels
    int skinHeight;        // The skin height in pixels
    int frameSize;        // The size in bytes the frames are
    int numSkins;        // The number of skins associated with the model
    int numVertices;        // The number of vertices (constant for each frame)
    int numTexCoords;      // The number of texture coordinates
    int numTriangles;      // The number of faces (polygons)
    int numGlCommands;      // The number of gl commands
    int numFrames;        // The number of animation frames
    int offsetSkins;        // The offset in the file for the skin data
    int offsetTexCoords;      // The offset in the file for the texture data
    int offsetTriangles;      // The offset in the file for the face data
    int offsetFrames;      // The offset in the file for the frames data
    int offsetGlCommands;    // The offset in the file for the gl commands data
    int offsetEnd;        // The end of the file offset
};

struct tMd2AliasTriangle
{
    byte vertex[3];
    byte lightNormalIndex;
};

struct tMd2Triangle
{
    float vertex[3];
    float normal[3];
};

struct tMd2Face
{
    short vertexIndices[3];
    short textureIndices[3];
};

struct tMd2TexCoord
{
    short u, v;
};

struct tMd2AliasFrame
{
    float scale[3];
    float translate[3];
    char name[16];
    tMd2AliasTriangle aliasVertices[1];
};

struct tMd2Frame
{
    char strName[16];
    tMd2Triangle* pVertices;
};

typedef char tMd2Skin[64];

class MD2Loader
{
public:
    MD2Loader();
    bool ImportMD2( Model* pModel, const char* strFileName, int textureIndex );
private:
    void ReadMD2Data();
    void ParseAnimations( Model* pModel );
    void ConvertDataStructures( Model* pModel );
    void ComputeNormals( Model* pModel );
    void CleanUp();
    FILE* m_FilePointer;

    tMd2Header        m_Header;
    tMd2Skin*        m_pSkins;
    tMd2TexCoord*      m_pTexCoords;
    tMd2Face*        m_pTriangles;
    tMd2Frame*        m_pFrames;
};

#endif

