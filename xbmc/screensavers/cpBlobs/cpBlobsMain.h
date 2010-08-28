// include file for screensaver template
#include "xbsBase.h"

//variables
LPDIRECT3DDEVICE9 m_pd3dDevice;
int  m_iWidth;
int m_iHeight;
SCR_INFO vInfo;

//XML Settings function prototypes
void CreateSettings();
void LoadSettings();
void SetDefaults();
