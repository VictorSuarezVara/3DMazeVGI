//******** PRACTICA VISUALITZACI� GR�FICA INTERACTIVA (Escola Enginyeria - UAB)
//******** Entorn b�sic VS2019 MULTIFINESTRA amb OpenGL, interf�cie MFC i Status Bar
//******** Ferran Poveda, Marc Vivet, Carme Juli�, D�bora Gil, Enric Mart� (Setembre 2018)
// Obj_3DS.cpp: implementation of the Obj_3DS class. Visualitzador objectes 3DS amb primitives OpenGL
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Obj3DS.h"
#include <stack>
#include <il\ilut.h>		// Header File For OpenIL

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//
// Constructor de la classe
//
Obj_3DS::Obj_3DS()
{
		
	g_3DModel.numOfObjects=0;
	g_3DModel.numOfMaterials=0;

	for (int i=0;i<MAX_TEXTURES;i++)
		g_Texture[i]=-1;

	fet3DS=false;

}

//
// Funci� d'inicialitzaci� de la classe Obj_3DS
//
void Obj_3DS::Inicialitzar()
{
	g_3DModel.numOfObjects=0;
	g_3DModel.numOfMaterials=0;

	for (int i=0;i<MAX_TEXTURES;i++)
		g_Texture[i]=-1;
	
	fet3DS=false;
}

//
// Funci� d'alliberament de mem�ria de la classe Obj_3DS
//

void Obj_3DS::EliminarMemoria()
{
	g_3DModel.pMaterials.clear();
	fitxerstextures.clear();

	for(int i = 0; i < g_3DModel.numOfObjects; i++)
	{
// Alliberem les cares, normals, vertex i coordenades de textures
		delete [] g_3DModel.pObject[i].pFaces;
		delete [] g_3DModel.pObject[i].pNormals;
		delete [] g_3DModel.pObject[i].pVerts;
		delete [] g_3DModel.pObject[i].pTexVerts;
	}
	
	g_3DModel.numOfObjects=0;
	g_3DModel.numOfMaterials=0;

	for (int i=0;i<MAX_TEXTURES;i++){
		if (g_Texture[i]!=-1){
			const unsigned int *a=&g_Texture[i];
			glDeleteTextures(1,a);
		}
	}
	for (int i=0;i<MAX_TEXTURES;i++)
		g_Texture[i]=-1;
	fet3DS=false;

}

//
// Destructor
//

Obj_3DS::~Obj_3DS()
{
	EliminarMemoria();
}

//
// Carrega un fitxer en format 3DS
//
//bool Obj_3DS::Carregar3DS(CString nomfitxer)
bool Obj_3DS::Carregar3DS(char *nomfitxer)
{
//	strcpy(titol,nom);
	CLoad3DS g_Load3ds;
	char szBasePath[255];	  // Path were all paths in the OBJ start
	char szLibraryFile[255];

// Primer necessitem carregar el fitxer 3DS. Li passem l'adre�a de la
// nostre estructura t3DModel i el nom del fitxer. 
	if (!g_Load3ds.Import3DS(&g_3DModel, nomfitxer))			// Load our .3DS file into our model structure
		return false;

	// Conversi� variable w_char --> char *
	//char *nomf = reinterpret_cast<char *> (nomfitxer.GetBuffer(3));

// Get base path
    strcpy_s(szBasePath, nomfitxer); // Copiar path sencer amb nom de fitxer.
//	strcpy_s(szBasePath, nomf); // Copiar path sencer amb nom de fitxer.
	MakePath(szBasePath);			 // Extreure path (szBasePath)

// Per a tots els materials
	for(int i = 0; i < g_3DModel.numOfMaterials; i++)
	{
		// Comprovem si hi ha un fitxer per carregar per aquest material
		if(strlen(g_3DModel.pMaterials[i].strFile) > 0)
		{

// Afegir base path al fitxer de textura
			strcpy_s(szLibraryFile, szBasePath);						// Copiar el path a szLibraryFile
			strcat_s(szLibraryFile, g_3DModel.pMaterials[i].strFile);	// Afegir al path el nom del fitxer textura
			strcpy_s(g_3DModel.pMaterials[i].strFile, szLibraryFile);	// Copiar a l'estructura el path + nom del fitxer textura

// Utilitzem el nom de la textura per carregar el bitmap
// i ens retorna el seu ID de textura que l'afegim al array de ID's de  textures
			//g_Texture[i]=CreateTexture(g_3DModel.pMaterials[i].strFile);
			wchar_t *nomf = reinterpret_cast <wchar_t *> (g_3DModel.pMaterials[i].strFile);
//			g_Texture[i] = ilutGLLoadImage(g_3DModel.pMaterials[i].strFile);	// Load The Images
			g_Texture[i] = ilutGLLoadImage(nomf);	// Load The Images
			struct nomfitxer a;
			a.num=i;
            strcpy_s(a.nom,g_3DModel.pMaterials[i].strFile);
			fitxerstextures.push_back(a);
			if (g_Texture[i]==NULL)return false;

		}
		// Establim el ID de textura per aquest material
		g_3DModel.pMaterials[i].texureId = i;
	}
	
	fet3DS=true;
	return true;
}

//
// Funci� que dibuixa l'pbjecte 3DS. Si el parametre es false crea una llista nova.
//
void Obj_3DS::Dibuixa3DS(bool actualitza,int obj,bool text)
{
	
	if (actualitza){glCallList(obj);}
	else { glNewList(obj, GL_COMPILE);				
		// Procesem cada objecte del nostre model
		for(int i = 0; i < g_3DModel.numOfObjects; i++)
		{
			// Ens assegurem que son objectes v�lids
			if(g_3DModel.pObject.size() <= 0) break;

			// Agafem l'objecte actual
			t3DObject *pObject = &g_3DModel.pObject[i];
				
			// Check to see if this object has a texture map, if so bind the texture to it.
			// Si l'objecte te textura, n'hi fiquem una
			if ((pObject->bHasTexture) && (text)){

				// Activem el texture mapping i desactivarem el color
				glEnable(GL_TEXTURE_2D);

				// Resetejem el color (blanc)
//				glColor3ub(255, 255, 255);

				// Vinculem la textura al objecte per el seu ID de material
				glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);

			} else {

				// Desactivem el texture mapping i activarem el color
				glDisable(GL_TEXTURE_2D);

				// Resetejem el color (blanc)
//				glColor3ub(255, 255, 255);
			}

			// Dibuixarem triangles
			glBegin(GL_TRIANGLES);					

				// Per a totes les cares del objecte
				for(int j = 0; j < pObject->numOfFaces; j++)
				{
					// Per a cada vertex del triangle
					for(int whichVertex = 0; whichVertex < 3; whichVertex++)
					{
						// Agafem l'index per a cada vertex de la cara
						unsigned int index = pObject->pFaces[j].vertIndex[whichVertex];
				
						// Li passem a OpenGL  la normal per aquest vertex
						glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);
					
						// Si te textura, li passem a OpenGL la textura per aquest vertex
						if(pObject->bHasTexture) {

							// Ens asegurem que hi ha un UVW map aplicat a l'objecte o sin� aquest no te les coordinades de textura
							if(pObject->pTexVerts) {
								glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
							}
						} else {

							// Ens assegurem que hi ha un color/material v�lid assignat al objecte
							// Sempre hem d'assignar com a m�nim un material/color a un objecte
							// pero en el cas de nom�s comprovar el tamany de la llista de materials
							// si el tamany �s al menys 1, i el ID de material �s diferent de -1
							// llavors tenim un material v�lid
							if(g_3DModel.pMaterials.size() && pObject->materialID >= 0) 
							{
								//  Agafem el color que te l'objecte
								BYTE *pColor = g_3DModel.pMaterials[pObject->materialID].color;

								// Assignem el color que hem agafat a l'objecte
								glColor3ub(pColor[0], pColor[1], pColor[2]);
							}
						}

						// Passem el vertex actual del objecte
						glVertex3f(pObject->pVerts[ index ].x, pObject->pVerts[ index ].y, pObject->pVerts[ index ].z);
					}
				}

			// Final del dibuix
			glEnd();
		}
		glEndList();
	}
}


void Obj_3DS::MakePath(char szFileAndPath[])
{
	////////////////////////////////////////////////////////////////////////
	// Rips the filenames out of a path and adds a slash (if needed)
	////////////////////////////////////////////////////////////////////////
	
	// Get string length
	int iNumChars = strlen(szFileAndPath);

	// Loop from the last to the first char
	for (int iCurChar=iNumChars-1; iCurChar>=0; iCurChar--)
	{
		// If the current char is a slash / backslash
		if (szFileAndPath[iCurChar] == char('\\') ||
			szFileAndPath[iCurChar] == char('/'))
		{
			// Terminate the the string behind the slash / backslash
			szFileAndPath[iCurChar + 1] = char('\0');
			return;
		}
	}

	// No slash there, set string length to zero
	szFileAndPath[0] = char('\0');
}