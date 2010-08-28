#include "../../addons/include/xbmc_addon_cpp_dll.h"
#include "XmlDocument.h"
#include <stdio.h>
#include "Blobby.h"
#include <string>
#include <io.h>

extern void LOG(char * msg, ...);

extern float g_fFOV;
extern float g_fAspectRatio;

extern float g_fTickSpeed;
extern D3DXVECTOR3 g_WorldRotSpeeds;
extern std::string g_strPresetsDir;
extern DllSetting  g_settingPreset;
extern std::vector<DllSetting> m_vecSettings;
extern StructSetting ** m_structSettings;
extern int m_iVisElements;

extern std::string g_strCubemap;
extern std::string g_strDiffuseCubemap;
extern std::string g_strSpecularCubemap;

extern bool g_bShowCube;
  
extern DWORD g_BlendStyle;

extern Blobby *m_pBlobby;

extern DWORD g_BGTopColor;
extern DWORD g_BGBottomColor;

#define DEFAULT_ADDON_ID "screensaver.cpblobs"
#define DEFAULT_XBMC_ADDONS_PATH "special://xbmc/addons/"
#define DEFAULT_HOME_ADDONS_PATH "special://home/addons/"
#define PRESETS_DIR "/resources/presets/"
#define TEXTURES_DIR "/resources/textures/"
#define PRESETS_MASK PRESETS_DIR "/*.xml"

///////////////////////////////////////////////////////////////////////////////

void CreateSettings()
{
  m_vecSettings.clear();
  m_iVisElements = 0;
  g_settingPreset.entry.clear();
  g_settingPreset.current = 0;

  std::string strMask(g_strPresetsDir);
  strMask.append(PRESETS_MASK);

  struct _finddata_t c_file;
  long hFile = _findfirst(strMask.c_str(), &c_file);
  if (hFile == -1L) 
  {
    g_strPresetsDir.assign(DEFAULT_HOME_ADDONS_PATH DEFAULT_ADDON_ID);
    strMask.assign(g_strPresetsDir)
      .append(PRESETS_MASK);
    hFile = _findfirst(strMask.c_str(), &c_file);
  }
  if (hFile == -1L) 
  {
    g_strPresetsDir.assign(DEFAULT_XBMC_ADDONS_PATH DEFAULT_ADDON_ID);
    strMask.assign(g_strPresetsDir)
      .append(PRESETS_MASK);
    hFile = _findfirst(strMask.c_str(), &c_file);
  }
  if (hFile != -1L)
	{
    std::string strPreset;
    strPreset.assign(c_file.name, 0, strlen(c_file.name) - 4);
    g_settingPreset.AddEntry(strPreset.c_str());

		while (_findnext(hFile, &c_file) == 0)
		{
      strPreset.assign(c_file.name, 0, strlen(c_file.name) - 4);
      g_settingPreset.AddEntry(strPreset.c_str());
		}
		_findclose(hFile);
	}

  m_vecSettings.push_back(g_settingPreset);
}
///////////////////////////////////////////////////////////////////////////////

void SetDefaults()
{
  // set any default values for your screensaver's parameters
  g_fFOV = 45.0f;
  g_fAspectRatio = 1.33f;

  g_WorldRotSpeeds.x = 1.0f;
  g_WorldRotSpeeds.y = 0.5f;
  g_WorldRotSpeeds.z = 0.25f;

  std::string textureDir(g_strPresetsDir);
  g_strCubemap.assign(g_strPresetsDir)
    .append(TEXTURES_DIR)
    .append("cube.dds");
  g_strDiffuseCubemap.assign(g_strPresetsDir)
    .append(TEXTURES_DIR)
    .append("cube_diffuse.dds");
  g_strSpecularCubemap.assign(g_strPresetsDir)
    .append(TEXTURES_DIR)
    .append("cube_specular.dds");

  m_pBlobby->m_fMoveScale = 0.3f;

  g_bShowCube = true;
  g_BlendStyle = 1;

  m_pBlobby->m_BlobPoints[0].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_fInfluence = 0.25f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.x = 2.0f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.y = 4.0f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.z = 0.0f;
  m_pBlobby->m_BlobPoints[1].m_Position.x = 0.6f;
  m_pBlobby->m_BlobPoints[1].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[1].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[1].m_fInfluence = 0.51f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.x = -4.0f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.y = 2.0f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.z = 0.0f;
  m_pBlobby->m_BlobPoints[2].m_Position.x = 0.3f;
  m_pBlobby->m_BlobPoints[2].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[2].m_Position.z = 0.3f;
  m_pBlobby->m_BlobPoints[2].m_fInfluence = 0.1f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.x = -2.0f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.y = 0.0f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.z = 3.0f;
  m_pBlobby->m_BlobPoints[3].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_fInfluence = 0.25f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.x = 0.0f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.y = 2.0f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.z = 1.0f;
  m_pBlobby->m_BlobPoints[4].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_fInfluence = 0.15f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.x = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.y = 0.0f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.z = 1.0f;

  m_pBlobby->SetDensity(32);
  m_pBlobby->m_TargetValue = 24.0f;

  return;
}

///////////////////////////////////////////////////////////////////////////////

// helper functions for parsing vectors etc. from a given line of text
void LoadVector(D3DXVECTOR3 *pVec, char *pStr)
{
  float x, y, z;
  sscanf(pStr, "%f %f %f", &x, &y, &z);
  pVec->x = x;
  pVec->y = y;
  pVec->z = z;
}

void LoadColor(DWORD *pCol, char *pStr)
{
  int r, g, b;
  sscanf(pStr, "%d %d %d", &r, &g, &b);
  *pCol = D3DCOLOR_RGBA(r, g, b, 255);
}

void LoadBlob(BlobPoint *pPoint, char *pStr)
{
  float x, y, z, inf, vx, vy, vz;
  sscanf(pStr, "%f %f %f %f %f %f %f", &x, &y, &z, &inf, &vx, &vy, &vz);

  pPoint->m_Position.x = x;
  pPoint->m_Position.y = y;
  pPoint->m_Position.z = z;

  pPoint->m_fInfluence = inf;

  pPoint->m_Speeds.x = vx;
  pPoint->m_Speeds.y = vy;
  pPoint->m_Speeds.z = vz;
}

///////////////////////////////////////////////////////////////////////////////

// Load settings from the [screensavername].xml configuration file
// the name of the screensaver (filename) is used as the name of
// the xml file - this is sent to us by XBMC when the Init func
// is called.
void LoadSettings()
{
  XmlNode node, childNode;
  CXmlDocument doc;
  
  // Set up the defaults
  SetDefaults();

  std::string strXMLFile(g_strPresetsDir);
  strXMLFile.append(PRESETS_DIR)
    .append(g_settingPreset.entry.at(g_settingPreset.current))
    .append(".xml");

#ifdef _DEBUG
  OutputDebugString("cpBlobs : Loading XML: ");
  OutputDebugString(strXMLFile.c_str());  
  OutputDebugString("\n");
#endif

  // Load the config file
  if (doc.Load(strXMLFile.c_str()) >= 0)
  {
    node = doc.GetNextNode(XML_ROOT_NODE);
    while(node > 0)
    {
      if (strcmpi(doc.GetNodeTag(node), "screensaver"))
      {
        node = doc.GetNextNode(node);
        continue;
      }
      
      if (childNode = doc.GetChildNode(node, "fov"))
        g_fFOV = (float)atof(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "aspectratio"))
        g_fAspectRatio = (float)atof(doc.GetNodeText(childNode));
      
      if (childNode = doc.GetChildNode(node,"showcube"))
        g_bShowCube = !strcmpi(doc.GetNodeText(childNode),"true");

      if (childNode = doc.GetChildNode(node, "bgtopcolor"))
        LoadColor(&g_BGTopColor, doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "bgbottomcolor"))
        LoadColor(&g_BGBottomColor, doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "globalspeed"))
        g_fTickSpeed = (float)atof(doc.GetNodeText(childNode));
      
      if (childNode = doc.GetChildNode(node, "worldrot"))
        LoadVector(&g_WorldRotSpeeds, doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "numblobs"))
        m_pBlobby->m_iNumPoints = atoi(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "cubemap"))
        g_strCubemap.assign(g_strPresetsDir)
          .append(TEXTURES_DIR)
          .append(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "diffusecubemap"))
        g_strDiffuseCubemap.assign(g_strPresetsDir)
          .append(TEXTURES_DIR)
          .append(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "specularcubemap"))
        g_strSpecularCubemap.assign(g_strPresetsDir)
          .append(TEXTURES_DIR)
          .append(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "blendstyle"))
        g_BlendStyle = atoi(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "movescale"))
        m_pBlobby->m_fMoveScale = (float)atof(doc.GetNodeText(childNode));

      if (childNode = doc.GetChildNode(node, "smoothness"))
        m_pBlobby->SetDensity(atoi(doc.GetNodeText(childNode)));

      if (childNode = doc.GetChildNode(node, "blobbiness"))
        m_pBlobby->m_TargetValue = (float)atof(doc.GetNodeText(childNode));


      if (childNode = doc.GetChildNode(node, "blob1"))
        LoadBlob(&m_pBlobby->m_BlobPoints[0], doc.GetNodeText(childNode));
      if (childNode = doc.GetChildNode(node, "blob2"))
        LoadBlob(&m_pBlobby->m_BlobPoints[1], doc.GetNodeText(childNode));
      if (childNode = doc.GetChildNode(node, "blob3"))
        LoadBlob(&m_pBlobby->m_BlobPoints[2], doc.GetNodeText(childNode));
      if (childNode = doc.GetChildNode(node, "blob4"))
        LoadBlob(&m_pBlobby->m_BlobPoints[3], doc.GetNodeText(childNode));
      if (childNode = doc.GetChildNode(node, "blob5"))
        LoadBlob(&m_pBlobby->m_BlobPoints[4], doc.GetNodeText(childNode));

      node = doc.GetNextNode(node);
    }
    doc.Close();
  }
}

