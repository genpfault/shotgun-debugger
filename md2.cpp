/*
 * Shotgun Debugger
 * Copyright 2005 Game Creation Society
 *
 * Programmed by Matt Sarnoff
 * http://www.msarnoff.org
 * http://www.gamecreation.org
 *
 * md2.cpp - MD2 model loading routines
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

#include "sdb.h"
#include "byteswap.h"

void ModelObject::draw()
{
  glBegin(GL_TRIANGLES);
    for(int j = 0; j < numOfFaces; j++)
    {
      for(int whichVertex = 0; whichVertex < 3; whichVertex++)
      {
        int index = pFaces[j].vertIndex[whichVertex];
        int index2 = pFaces[j].coordIndex[whichVertex];
        glNormal3f(-pNormals[index].x, -pNormals[index].y, -pNormals[index].z);
        
        if(pTexVerts) 
        {
          glTexCoord2f(pTexVerts[index2].x, -pTexVerts[index2].y);
        }
        
        glVertex3f(pVerts[index].x, pVerts[index].y, pVerts[index].z);
      }
    }      
  glEnd();
}
    
void ModelObject::destroy()
{
  if(pFaces)    delete [] pFaces;
  if(pNormals)  delete [] pNormals;
  if(pVerts)    delete [] pVerts;
  if(pTexVerts) delete [] pTexVerts;
}
    
void Model::draw(int currentAnim, int currentFrame, int nextFrame, float time, float speed)
{
  // Apply the transformations
  glPushMatrix();
  glScalef(scale, scale, scale);
  glTranslatef(trans[X], trans[Y], trans[Z]);
  glRotatef(rot[Z], 0.0, 0.0, 1.0);
  glRotatef(rot[X], 1.0, 0.0, 0.0);
  glRotatef(rot[Y], 0.0, 1.0, 0.0);
  

  if (numOfObjects == 1)
    pObject[0].draw();
  else
   animate(currentAnim, currentFrame, nextFrame, time, speed);
  glPopMatrix();
}
    
void Model::destroy()
{
  for (int i = 0; i < numOfObjects; i++)
    pObject[i].destroy();
}


void Model::animate(int currentAnim, int currentFrame, int nextFrame, float time, float speed)
{
  if (pObject.size() <= 0)
    return;
  
  //printf("Curr:%d Next:%d\n", currentFrame+1, nextFrame+1);
    
  ModelObject *pFrame = &pObject[currentFrame];

  ModelObject *pNextFrame =  &pObject[nextFrame];

  ModelObject *pFirstFrame = &pObject[0];
  
  float t = time / (1.0/speed);

  glBegin(GL_TRIANGLES);

    for (int j = 0; j < pFrame->numOfFaces; j++)
    {
      for (int whichVertex = 0; whichVertex < 3; whichVertex++)
      {
        int vertIndex = pFirstFrame->pFaces[j].vertIndex[whichVertex];

        int texIndex  = pFirstFrame->pFaces[j].coordIndex[whichVertex];
            
        if(pFirstFrame->pTexVerts) 
        {
          glTexCoord2f(pFirstFrame->pTexVerts[texIndex].x, 
                -pFirstFrame->pTexVerts[texIndex].y);
        }

        CVector3 vPoint1 = pFrame->pVerts[vertIndex];
        CVector3 vPoint2 = pNextFrame->pVerts[vertIndex];
        
        glVertex3f(vPoint1.x + t * (vPoint2.x - vPoint1.x), 
              vPoint1.y + t * (vPoint2.y - vPoint1.y),
              vPoint1.z + t * (vPoint2.z - vPoint1.z));
      }
    }

  glEnd();
}

MD2Loader::MD2Loader()
{
  memset(&m_Header, 0, sizeof(tMd2Header));

  m_pSkins=NULL;
  m_pTexCoords=NULL;
  m_pTriangles=NULL;
  m_pFrames=NULL;
}


bool MD2Loader::ImportMD2(Model *pModel, const char *strFileName, int textureIndex)
{
  m_FilePointer = fopen(strFileName, "rb");
  
  if(!m_FilePointer) 
    errorMessage(8, "FLAGRANT SYSTEM ERROR", "Unable to load model %s", strFileName);

  fread(&m_Header, 1, sizeof(tMd2Header), m_FilePointer);
  m_Header.magic            = letoh32(m_Header.magic);
  m_Header.version          = letoh32(m_Header.version);
  m_Header.skinWidth        = letoh32(m_Header.skinWidth);
  m_Header.skinHeight       = letoh32(m_Header.skinHeight);
  m_Header.frameSize        = letoh32(m_Header.frameSize);
  m_Header.numSkins         = letoh32(m_Header.numSkins);
  m_Header.numVertices      = letoh32(m_Header.numVertices);
  m_Header.numTexCoords     = letoh32(m_Header.numTexCoords);
  m_Header.numTriangles     = letoh32(m_Header.numTriangles);
  m_Header.numGlCommands    = letoh32(m_Header.numGlCommands);
  m_Header.numFrames        = letoh32(m_Header.numFrames);
  m_Header.offsetSkins      = letoh32(m_Header.offsetSkins);
  m_Header.offsetTexCoords  = letoh32(m_Header.offsetTexCoords);
  m_Header.offsetTriangles  = letoh32(m_Header.offsetTriangles);
  m_Header.offsetFrames     = letoh32(m_Header.offsetFrames);
  m_Header.offsetGlCommands = letoh32(m_Header.offsetGlCommands);
  m_Header.offsetEnd        = letoh32(m_Header.offsetEnd);

  if(m_Header.version != 8)
    errorMessage(8, "FLAGRANT SYSTEM ERROR", "Invalid file format (version not 8) %s", strFileName);
  ReadMD2Data();
  
  pModel->texture = textureIndex;
    
  ConvertDataStructures(pModel);

  ComputeNormals(pModel);

  if(textureIndex >= 0)
  {
    tMaterialInfo texture;

    strcpy(texture.strFile, "");

    texture.textureId = 0;

    texture.uTile = texture.uTile = 1;

    pModel->numOfMaterials = 1;

    pModel->pMaterials.push_back(texture);
  }

  CleanUp();

  return true;
}

void MD2Loader::ReadMD2Data()
{
  unsigned char buffer[MD2_MAX_FRAMESIZE];
  int j = 0;

  m_pSkins     = new tMd2Skin [m_Header.numSkins];
  m_pTexCoords = new tMd2TexCoord [m_Header.numTexCoords];
  m_pTriangles = new tMd2Face [m_Header.numTriangles];
  m_pFrames    = new tMd2Frame [m_Header.numFrames];

  // Next, we start reading in the data by seeking to our skin names offset
  fseek(m_FilePointer, m_Header.offsetSkins, SEEK_SET);
  
  // Depending on the skin count, we read in each skin for this model
  fread(m_pSkins, sizeof(tMd2Skin), m_Header.numSkins, m_FilePointer);
  
  // Move the file pointer to the position in the file for texture coordinates
  fseek(m_FilePointer, m_Header.offsetTexCoords, SEEK_SET);
  
  // Read in all the texture coordinates in one fell swoop
  fread(m_pTexCoords, sizeof(tMd2TexCoord), m_Header.numTexCoords, m_FilePointer);
  for (int i=0; i < m_Header.numTexCoords; ++i) {
    m_pTexCoords[i].u = letoh16(m_pTexCoords[i].u);
    m_pTexCoords[i].v = letoh16(m_pTexCoords[i].v);
  }

  // Move the file pointer to the triangles/face data offset
  fseek(m_FilePointer, m_Header.offsetTriangles, SEEK_SET);
  
  // Read in the face data for each triangle (vertex and texCoord indices)
  fread(m_pTriangles, sizeof(tMd2Face), m_Header.numTriangles, m_FilePointer);
  for (int i=0; i < m_Header.numTriangles; ++i) {
    for (int j=0; j < 3; ++j) {
      m_pTriangles[i].vertexIndices[j]  = letoh16(m_pTriangles[i].vertexIndices[j]);
      m_pTriangles[i].textureIndices[j] = letoh16(m_pTriangles[i].textureIndices[j]);
    }
  }
      
  // Move the file pointer to the vertices (frames)
  fseek(m_FilePointer, m_Header.offsetFrames, SEEK_SET);

  for (int i=0; i < m_Header.numFrames; i++)
  { 
    tMd2AliasFrame *pFrame = (tMd2AliasFrame *) buffer;

    // Read in the first frame of animation
    fread(pFrame, 1, m_Header.frameSize, m_FilePointer);
    for (int j=0; j < 3; ++j) {
      pFrame->scale[j]     = letohf(pFrame->scale[j]);
      pFrame->translate[j] = letohf(pFrame->translate[j]);
    }

    m_pFrames[i].pVertices = new tMd2Triangle [m_Header.numVertices];
      
    strcpy(m_pFrames[i].strName, pFrame->name);
      
    //printf("Frame %d: %s\n", i+1, m_pFrames[i].strName);
        

    tMd2Triangle *pVertices = m_pFrames[i].pVertices;
  
    for (int j=0; j < m_Header.numVertices; j++)
    {
      pVertices[j].vertex[0] = pFrame->aliasVertices[j].vertex[0] * pFrame->scale[0] + pFrame->translate[0];
      pVertices[j].vertex[2] = -1 * (pFrame->aliasVertices[j].vertex[1] * pFrame->scale[1] + pFrame->translate[1]);
      pVertices[j].vertex[1] = pFrame->aliasVertices[j].vertex[2] * pFrame->scale[2] + pFrame->translate[2];
    }
  }
}

void MD2Loader::ParseAnimations(Model *pModel)
{

  tAnimationInfo animation, animation2;
  string strLastName = "";

  for(int i = 0; i < pModel->numOfObjects; i++)
  {
    string strName  = m_pFrames[i].strName;
    //cout << strName << " " << (i+1) << endl;
    int frameNum = 0;

    for(unsigned int j = 0; j < strName.length(); j++)
    {
      if( isdigit(strName[j]) && j >= strName.length() - 2)
      {
        frameNum = atoi(&strName[j]);
        strName.erase(j, strName.length() - j);
        break;
      }
    }

    if(strName != strLastName || i == pModel->numOfObjects-1)
    {
      if(strLastName != "")
      {
        strcpy(animation.strName, strLastName.c_str());

        animation.endFrame = i;

        pModel->pAnimations.push_back(animation); 
        
        //printf("Animation %d (%s) from %d to %d\n", pModel->numOfAnimations, animation.strName, animation.startFrame+1, animation.endFrame+1);
        
        memset(&animation, 0, sizeof(tAnimationInfo));

        pModel->numOfAnimations++;
        //printf("Animations: %d\n", pModel->numOfAnimations);
        
        //cout << strName << " " << strLastName << endl;
        // Exception case if the object list ends with a one-frame animation.
        if (i == pModel->numOfObjects-1 && strName != strLastName)
        {
          strcpy(animation2.strName, strName.c_str());
          animation2.startFrame = pModel->numOfObjects-1;
          animation2.endFrame = pModel->numOfObjects;
          pModel->pAnimations.push_back(animation2); 
          //printf("Animation %d (%s) from %d to %d\n", pModel->numOfAnimations, animation2.strName, animation2.startFrame+1, animation2.endFrame+1);
          pModel->numOfAnimations++;
          //printf("Animations: %d\n", pModel->numOfAnimations);
        }
      }
      if (frameNum == 0)
        animation.startFrame = frameNum - 1 + i;
      else
        animation.startFrame = frameNum + i;
    }

    strLastName = strName;
  }
}


void MD2Loader::ConvertDataStructures(Model *pModel)
{
  int j = 0, i = 0;

  pModel->numOfObjects = m_Header.numFrames;

  ParseAnimations(pModel);

  for (i=0; i < pModel->numOfObjects; i++)
  {
    ModelObject currentFrame;

    currentFrame.numOfVerts   = m_Header.numVertices;
    currentFrame.numTexVertex = m_Header.numTexCoords;
    currentFrame.numOfFaces   = m_Header.numTriangles;

    currentFrame.pVerts    = new CVector3 [currentFrame.numOfVerts];

    for (j=0; j < currentFrame.numOfVerts; j++)
    {
      currentFrame.pVerts[j].x = m_pFrames[i].pVertices[j].vertex[0];
      currentFrame.pVerts[j].y = m_pFrames[i].pVertices[j].vertex[1];
      currentFrame.pVerts[j].z = m_pFrames[i].pVertices[j].vertex[2];
    }

    delete m_pFrames[i].pVertices;

    if(i > 0)
    {
      pModel->pObject.push_back(currentFrame);
      continue;
    }

    currentFrame.pTexVerts = new CVector2 [currentFrame.numTexVertex];
    currentFrame.pFaces    = new tFace [currentFrame.numOfFaces];

    for(j=0; j < currentFrame.numTexVertex; j++)
    {
      currentFrame.pTexVerts[j].x = m_pTexCoords[j].u / float(m_Header.skinWidth);
      currentFrame.pTexVerts[j].y = 1 - m_pTexCoords[j].v / float(m_Header.skinHeight);
    }

    for(j=0; j < currentFrame.numOfFaces; j++)
    {
      currentFrame.pFaces[j].vertIndex[0] = m_pTriangles[j].vertexIndices[0];
      currentFrame.pFaces[j].vertIndex[1] = m_pTriangles[j].vertexIndices[1];
      currentFrame.pFaces[j].vertIndex[2] = m_pTriangles[j].vertexIndices[2];

      currentFrame.pFaces[j].coordIndex[0] = m_pTriangles[j].textureIndices[0];
      currentFrame.pFaces[j].coordIndex[1] = m_pTriangles[j].textureIndices[1];
      currentFrame.pFaces[j].coordIndex[2] = m_pTriangles[j].textureIndices[2];
    }

    pModel->pObject.push_back(currentFrame);
  }
}
  
void MD2Loader::CleanUp()
{
  fclose(m_FilePointer);

  if(m_pSkins)	 delete [] m_pSkins;
  if(m_pTexCoords) delete m_pTexCoords;
  if(m_pTriangles) delete m_pTriangles;
  if(m_pFrames)	 delete m_pFrames;
}

/* It was a lot easier to keep Ben's 3D vector class here than wank around with
my 2D-hybrid shit... */

// This computes the magnitude of a normal.   (magnitude = sqrt(x^2 + y^2 + z^2)
#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))

CVector3 Vector(CVector3 vPoint1, CVector3 vPoint2)
{
  CVector3 vVector;

  vVector.x = vPoint1.x - vPoint2.x;
  vVector.y = vPoint1.y - vPoint2.y;
  vVector.z = vPoint1.z - vPoint2.z;

  return vVector;
}

CVector3 AddVector(CVector3 vVector1, CVector3 vVector2)
{
  CVector3 vResult;
  
  vResult.x = vVector2.x + vVector1.x;
  vResult.y = vVector2.y + vVector1.y;
  vResult.z = vVector2.z + vVector1.z;

  return vResult;
}

CVector3 DivideVectorByScalar(CVector3 vVector1, float scalar)
{
  CVector3 vResult;  
  vResult.x = vVector1.x / scalar;
  vResult.y = vVector1.y / scalar;
  vResult.z = vVector1.z / scalar;

  return vResult;
}

CVector3 Cross(CVector3 vVector1, CVector3 vVector2)
{
  CVector3 vCross;
  vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
  vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
  vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

  return vCross;
}

CVector3 Normalize(CVector3 vNormal)
{
  double Magnitude;

  Magnitude = Mag(vNormal);

  vNormal.x /= (float)Magnitude;
  vNormal.y /= (float)Magnitude;
  vNormal.z /= (float)Magnitude;

  return vNormal;
}

void MD2Loader::ComputeNormals(Model *pModel)
{
  CVector3 vVector1, vVector2, vNormal, vPoly[3];

  if(pModel->numOfObjects <= 0)
    return;

  for(int index = 0; index < pModel->numOfObjects; index++)
  {
    ModelObject *pObject = &(pModel->pObject[index]);

    CVector3 *pNormals		= new CVector3 [pObject->numOfFaces];
    CVector3 *pTempNormals	= new CVector3 [pObject->numOfFaces];
    pObject->pNormals		= new CVector3 [pObject->numOfVerts];

    for(int i=0; i < pObject->numOfFaces; i++)
    {												
      vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
      vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
      vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

      vVector1 = Vector(vPoly[0], vPoly[2]);
      vVector2 = Vector(vPoly[2], vPoly[1]);

      vNormal  = Cross(vVector1, vVector2);
      pTempNormals[i] = vNormal;
      vNormal  = Normalize(vNormal);

      pNormals[i] = vNormal;
    }

    CVector3 vSum = {0.0, 0.0, 0.0};
    CVector3 vZero = vSum;
    int shared=0;

    for (int i = 0; i < pObject->numOfVerts; i++)
    {
      for (int j = 0; j < pObject->numOfFaces; j++)
      {
        if (pObject->pFaces[j].vertIndex[0] == i || 
          pObject->pFaces[j].vertIndex[1] == i || 
          pObject->pFaces[j].vertIndex[2] == i)
        {
          vSum = AddVector(vSum, pTempNormals[j]);
          shared++;
        }
      }      
      pObject->pNormals[i] = DivideVectorByScalar(vSum, float(-shared));

      pObject->pNormals[i] = Normalize(pObject->pNormals[i]);	

      vSum = vZero;
      shared = 0;
    }
  
    delete [] pTempNormals;
    delete [] pNormals;
  }
}

