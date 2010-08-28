// cpBlobs
// XBMC screensaver displaying metaballs moving around in an environment
// Simon Windmill (siw@coolpowers.com)

#include "../../addons/include/xbmc_addon_cpp_dll.h"
#include "../../addons/include/xbmc_scr_dll.h"
#include "cpBlobsMain.h"

#include "Blobby.h"
#include <string>

Blobby *m_pBlobby;

////////////////////////////////////////////////////////////////////////////////

static float g_fTicks = 0.0f;

////////////////////////////////////////////////////////////////////////////////

// these global parameters can all be user-controlled via the XML file

float g_fTickSpeed = 0.01f;

D3DXVECTOR3 g_WorldRotSpeeds;
std::string g_strPresetsDir;
DllSetting g_settingPreset(DllSetting::SPIN, "preset", "30000");
std::vector<DllSetting> m_vecSettings;
StructSetting ** m_structSettings;
int m_iVisElements;
std::string g_strCubemap;
std::string g_strDiffuseCubemap;
std::string g_strSpecularCubemap;

bool g_bShowCube = true;
  
DWORD g_BlendStyle;

DWORD g_BGTopColor, g_BGBottomColor;

float g_fFOV, g_fAspectRatio;

////////////////////////////////////////////////////////////////////////////////

// stuff for the environment cube
struct CubeVertex
{
  D3DXVECTOR3 position;
  D3DXVECTOR3 normal;
};

#define FVF_CUBEVERTEX D3DFVF_XYZ | D3DFVF_NORMAL

// man, how many times have you typed (or pasted) this data for a cube's
// vertices and normals, eh?
CubeVertex g_cubeVertices[] =
{
  {D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f, 0.0f,1.0f), },
  {D3DXVECTOR3(1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f, 0.0f,1.0f), },
  {D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f, 0.0f,1.0f), },
  {D3DXVECTOR3(1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f, 0.0f,1.0f), },

  {D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), },
  {D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), },
  {D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), },
  {D3DXVECTOR3(1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f, 0.0f, -1.0f), },

  {D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), },
  {D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), },
  {D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), },
  {D3DXVECTOR3(1.0f, 1.0f,-1.0f), D3DXVECTOR3(0.0f, -1.0f, 0.0f), },

  {D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), },
  {D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), },
  {D3DXVECTOR3(1.0f,-1.0f, 1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), },
  {D3DXVECTOR3(1.0f,-1.0f,-1.0f), D3DXVECTOR3(0.0f,1.0f, 0.0f), },

  {D3DXVECTOR3(1.0f, 1.0f,-1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(1.0f,-1.0f,-1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(1.0f,-1.0f, 1.0f), D3DXVECTOR3(-1.0f, 0.0f, 0.0f), },

  {D3DXVECTOR3(-1.0f, 1.0f,-1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(-1.0f,-1.0f,-1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), },
  {D3DXVECTOR3(-1.0f,-1.0f, 1.0f), D3DXVECTOR3(1.0f, 0.0f, 0.0f), }
};

LPDIRECT3DVERTEXBUFFER9 g_pCubeVertexBuffer = NULL;

////////////////////////////////////////////////////////////////////////////////

// stuff for the background plane

struct BG_VERTEX 
{
    D3DXVECTOR4 position;
    DWORD       color;
};

BG_VERTEX g_BGVertices[4];

////////////////////////////////////////////////////////////////////////////////

static HRESULT CreateCubeTextureFromFile(std::string & filename, LPDIRECT3DCUBETEXTURE9 * ppCubeTexture) 
{
  HRESULT res = -1;

	FILE *f = fopen(filename.c_str(), "rb");
  if (f != NULL)
  {
	  fseek(f, 0, SEEK_END);
	  int len = ftell(f);
	  fseek(f, 0, SEEK_SET);
    unsigned char* pBuffer = new unsigned char[len];
	  fread(pBuffer, len, 1, f);
	  fclose(f);

    res = D3DXCreateCubeTextureFromFileInMemory(m_pd3dDevice, pBuffer, len, ppCubeTexture);
    delete [] pBuffer;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////

// fill in background vertex array with values that will
// completely cover screen
void SetupGradientBackground(DWORD dwTopColor, DWORD dwBottomColor)
{
  float x1 = -0.5f;
  float y1 = -0.5f;
  float x2 = (float)m_iWidth - 0.5f;
  float y2 = (float)m_iHeight - 0.5f;
  
  g_BGVertices[0].position = D3DXVECTOR4(x2, y1, 0.0f, 1.0f);
  g_BGVertices[0].color = dwTopColor;

  g_BGVertices[1].position = D3DXVECTOR4(x2, y2, 0.0f, 1.0f);
  g_BGVertices[1].color = dwBottomColor;

  g_BGVertices[2].position = D3DXVECTOR4(x1, y1, 0.0f, 1.0f);
  g_BGVertices[2].color = dwTopColor;

  g_BGVertices[3].position = D3DXVECTOR4(x1, y2, 0.0f, 1.0f);
  g_BGVertices[3].color = dwBottomColor;
  
  return;
}

///////////////////////////////////////////////////////////////////////////////


void RenderGradientBackground()
{
  // clear textures
  m_pd3dDevice->SetTexture(0, NULL);
  m_pd3dDevice->SetTexture(1, NULL);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  // don't write to z-buffer
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE); 
    
  m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
  m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, g_BGVertices, sizeof(BG_VERTEX));

  // restore state
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE); 
  
  return;
}

////////////////////////////////////////////////////////////////////////////////

LPDIRECT3DCUBETEXTURE9  g_pCubeTexture  = NULL;
LPDIRECT3DCUBETEXTURE9  g_pDiffuseCubeTexture  = NULL;
LPDIRECT3DCUBETEXTURE9  g_pSpecularCubeTexture  = NULL;
extern "C" {
// XBMC has loaded us into memory,
// we should set our core values
// here and load any settings we
// may have from our config file
ADDON_STATUS Create(void* hdl, void* props)
{
  if (!props)
    return STATUS_UNKNOWN;
 
  SCR_PROPS* scrprops = (SCR_PROPS*) props;
  g_strPresetsDir.assign(scrprops->presets);

  m_pd3dDevice = (LPDIRECT3DDEVICE9) scrprops->device;
  m_iWidth = scrprops->width;
  m_iHeight = scrprops->height;

  m_pBlobby = new Blobby();
  m_pBlobby->m_iNumPoints = 5;
  
  CreateSettings();

  return STATUS_OK;
}

// XBMC tells us we should get ready
// to start rendering. This function
// is called once when the screensaver
// is activated by XBMC.
void Start()
{  
  // Load the settings
  LoadSettings();

  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
  
  m_pBlobby->Init(m_pd3dDevice);
  
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

  CreateCubeTextureFromFile(g_strCubemap, &g_pCubeTexture);
  CreateCubeTextureFromFile(g_strDiffuseCubemap, &g_pDiffuseCubeTexture);
  CreateCubeTextureFromFile(g_strSpecularCubemap, &g_pSpecularCubeTexture);

  m_pd3dDevice->CreateVertexBuffer(24*sizeof(CubeVertex), 0, 
                                   FVF_CUBEVERTEX, D3DPOOL_DEFAULT, 
                                   &g_pCubeVertexBuffer, NULL);
 
  void *pVertices = NULL;

  g_pCubeVertexBuffer->Lock(0, sizeof(g_cubeVertices), &pVertices, 0);
  memcpy(pVertices, g_cubeVertices, sizeof(g_cubeVertices));
  g_pCubeVertexBuffer->Unlock();

  SetupGradientBackground(g_BGTopColor, g_BGBottomColor);

  return;
}

// XBMC tells us to render a frame of
// our screensaver. This is called on
// each frame render in XBMC, you should
// render a single frame only - the DX
// device will already have been cleared.
void Render()
{
  m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0);

  m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(255, 255, 255, 255));
  m_pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
  m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
  m_pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESS);
  m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

  m_pd3dDevice->SetPixelShader(NULL);

  // I know I'm not scaling by time here to get a constant framerate,
  // but I believe this to be acceptable for this application
  m_pBlobby->AnimatePoints(g_fTicks);  
  m_pBlobby->March();

  // setup rotation
  D3DXMATRIX matWorld;
  D3DXMatrixIdentity(&matWorld);
  D3DXMatrixRotationYawPitchRoll(&matWorld, g_WorldRotSpeeds.x * g_fTicks, g_WorldRotSpeeds.y * g_fTicks, g_WorldRotSpeeds.z * g_fTicks);
  m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

  // setup viewpoint
  D3DXMATRIX matView;
  D3DXVECTOR3 campos(0.0f, 0.0f, -0.8f);
  D3DXVECTOR3 camto(0.0f, 0.0f, 0.0f);
  D3DXVECTOR3 upvec(0.0f, 1.0f, 0.0f);
  D3DXMatrixLookAtLH(&matView, &campos, &camto, &upvec);
  m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

  // setup projection
  D3DXMATRIX matProj;
  D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(g_fFOV), g_fAspectRatio, 0.05f, 100.0f);
  m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

  // setup cubemap
  m_pd3dDevice->SetTexture(0, g_pCubeTexture);
  m_pd3dDevice->SetTexture(1, NULL);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

  // rotate the cubemap to match the world
  m_pd3dDevice->SetTransform(D3DTS_TEXTURE0, &matWorld);
  m_pd3dDevice->SetTransform(D3DTS_TEXTURE1, &matWorld);
    
  // draw the box (inside-out)
  if (g_bShowCube)
  {
    m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    m_pd3dDevice->SetFVF(FVF_CUBEVERTEX);
    m_pd3dDevice->SetStreamSource(0, g_pCubeVertexBuffer, 0, sizeof(CubeVertex));
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,  0, 2);
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,  4, 2);
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,  8, 2);
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 12, 2);
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 16, 2);
    m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 20, 2);
  }
  else
    RenderGradientBackground();

  m_pd3dDevice->SetTexture(0, g_pDiffuseCubeTexture);  
  m_pd3dDevice->SetTexture(1, g_pSpecularCubeTexture);
  m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

  m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
  m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  m_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

  m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);  
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP,   g_BlendStyle);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
  m_pd3dDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
  m_pd3dDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

  m_pd3dDevice->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_DISABLE);
  m_pd3dDevice->SetTextureStageState(2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

  m_pBlobby->Render(m_pd3dDevice);

  // increase tick count
  g_fTicks += g_fTickSpeed;

  return;
}

// XBMC tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
void Stop()
{
  if (g_pCubeTexture != NULL) 
    g_pCubeTexture->Release();

  if (g_pDiffuseCubeTexture != NULL) 
    g_pDiffuseCubeTexture->Release();

  if (g_pSpecularCubeTexture != NULL) 
    g_pSpecularCubeTexture->Release();
  
  delete m_pBlobby;

  if (g_pCubeVertexBuffer != NULL)
  {
    g_pCubeVertexBuffer->Release();
    g_pCubeVertexBuffer = NULL;
  }

  return;
}

///////////////////////////////////////////////////////////////////////////////

void GetInfo(SCR_INFO* pInfo)
{
  // not used, but can be used to pass info
  // back to XBMC if required in the future
  return;
}



void Destroy()
{
}

ADDON_STATUS GetStatus()
{
  return STATUS_OK;
}

bool HasSettings()
{
  return true;
}

unsigned int GetSettings(StructSetting ***sSet)
{
  if(m_vecSettings.empty())
    CreateSettings();

  m_iVisElements = DllUtils::VecToStruct(m_vecSettings, &m_structSettings);
  *sSet = m_structSettings;
  return m_iVisElements;
}

ADDON_STATUS SetSetting(const char *settingName, const void *settingValue)
{
  if (!settingName || !settingValue)
    return STATUS_UNKNOWN;

  if (strcmp(settingName, "preset")==0)
  {
    int iValue = *(int *) settingValue;
    if ((iValue >= 0) && 
        (iValue < (int) g_settingPreset.entry.size()))
    {
      g_settingPreset.current = iValue;
      return STATUS_OK;
    }
  }
  return STATUS_UNKNOWN;
}

void FreeSettings()
{
  DllUtils::FreeStruct(m_iVisElements, &m_structSettings);
}

void Remove()
{
}

} // extern "C"
