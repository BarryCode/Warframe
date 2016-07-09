#include <windows.h>
#include <vector>
#include <fstream>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include "DXSDK\d3dx9.h"
#if defined _M_X64
#pragma comment(lib, "DXSDK/x64/d3dx9.lib") 
#elif defined _M_IX86
#pragma comment(lib, "DXSDK/x86/d3dx9.lib")
#endif

//#include <d3dx9.h>
//#pragma comment(lib, "d3dx9.lib") 
#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE dllHandle;

//Stride
UINT Stride;

//vertexshader
IDirect3DVertexShader9* vShader;
UINT vSize;

//pixelshader
IDirect3DPixelShader9* pShader;
UINT pSize;

//texture
D3DLOCKED_RECT pLockedRect;
D3DSURFACE_DESC desc;
IDirect3DTexture9* pCurrentTexture;

//crc
DWORD texCRC;

//DPvertexshader
//IDirect3DVertexShader9* DPvShader;
//UINT DPvSize;

//DPpixelshader
//IDirect3DPixelShader9* DPpShader;
//UINT DPpSize;

// model rec
UINT mStartRegister;
UINT mVector4fCount;

//gettexture
LPDIRECT3DBASETEXTURE9 tex = NULL;
int dWidth;
int dHeight;
//int dPitch;

//esp model distance
float bestRealDistance;

//used for logging/cycling through values
bool logger = false;
int countnum = -1;

bool FirstInit = false; //init once

//vdesc.Size
D3DVERTEXBUFFER_DESC vdesc;
D3DVERTEXBUFFER_DESC DPvdesc;

//sprite
bool MenuSpriteCreated, MenuSpriteCreated2 = NULL;
LPDIRECT3DTEXTURE9 MenuIMAGE, MenuIMAGE2;
LPD3DXSPRITE MenuSPRITE, MenuSPRITE2;
D3DXVECTOR3 MenuImagePos, MenuImagePos2;

DWORD dwStartTime = 0; //time as the timer started
DWORD dwTime = 0; //windowsuptime
//bool bResetStartTime = true; //refresh starttime

//models
#define MODELS (mStartRegister == 52 && mVector4fCount >= 82)

//shiny glow around caches and everything
#define CACHE_GLOW (Stride == 24 && vSize == 352 && pSize == 540 && mStartRegister == 12 && mVector4fCount == 1)
#define STUFFTHATSHOULDNOTGLOW (NumVertices == 258 || NumVertices == 98 || NumVertices == 428 || NumVertices == 462 || NumVertices == 507 || NumVertices == 1432 || NumVertices == 1376 || NumVertices == 355 || NumVertices == 1619)
//NumVertices == 746 small container
//NumVertices == 994 && 999 big container
//NumVertices == 258 & 462 & 507 & 98 & 428 ammo
//NumVertices == 1432 credits
//NumVertices == 355 resource
//NumVertices == 1619 mod

//yellow round thing, blue round thing, mod thing1&2&3, credits
#define YELLOWPART ((Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1252 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1))

#define BLUEPART ((Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1224 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1492 && */mStartRegister == 18 && mVector4fCount == 1))

#define MOD1 ((Stride == 24 && NumVertices == 1619 && vSize == 768 && /*pSize == 1460 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 11 && mVector4fCount == 1))

#define CREDITS ((Stride == 24 && NumVertices == 1377 && vSize == 804 && /*pSize == 2612 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1432 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 193 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 193 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 776 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 776 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 582 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)||\
(Stride == 24 && NumVertices == 1376 && primCount == 582 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1))

#define GRENADE ((Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 128 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 249 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 840 && /*pSize == 2076 && */mStartRegister == 18 && mVector4fCount == 1))
//(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 608 && /*pSize == 732 && */mStartRegister == 20 && mVector4fCount == 1)|| \
//(Stride == 24 && NumVertices == 223 && primCount == 222 && vSize == 680 && /*pSize == 1280 && */mStartRegister == 15 && mVector4fCount == 1)|| \
//(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 188 && /*pSize == 60 && */mStartRegister == 13 && mVector4fCount == 1))

//rare and reinforced grineer, may have changed with new patch
#define Rare_Grineer_Container (Stride == 24 && NumVertices == 1063 && primCount == 1184 && vSize == 840 && /*pSize == 1740 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_MetalPlatesA (Stride == 24 && NumVertices == 104 && primCount == 520 && vSize == 768 && /*pSize == 1264 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_MetalPlatesB (Stride == 24 && NumVertices == 440 && primCount == 520 && vSize == 768 && /*pSize == 1264 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_Glow (Stride == 24 && NumVertices == 247 && primCount == 384 && vSize == 492 && /*pSize == 904 && */mStartRegister == 16 && mVector4fCount == 1)
#define Reinforced_Orokin_Container_Frame (Stride == 24 && NumVertices == 6045 && primCount == 3216 && vSize == 804 && /*pSize == 3016 && */mStartRegister == 12 && mVector4fCount == 1)
#define RareandNormal_Orokin_Container (Stride == 24 && NumVertices == 1505 && primCount == 1880 && vSize == 900 && /*pSize == 3884 && */mStartRegister == 12 && mVector4fCount == 1) //no, dfr

#define RARECONTAINER (Rare_Grineer_Container||Reinforced_Grineer_Container_MetalPlatesA||Reinforced_Grineer_Container_MetalPlatesB||Reinforced_Grineer_Container_Glow||Reinforced_Orokin_Container_Frame)

#define MISC (YELLOWPART||BLUEPART||MOD1||CREDITS||RARECONTAINER||RareandNormal_Orokin_Container)

//corpus helmet
//Stride == 24 && NumVertices == 572 && primCount == 228 && vSize == 864 && pSize == 2444 && mStartRegister == 24 && mVector4fCount == 1 && vdesc.Size == 50432

//Rare Grineer Container _/
//Rare Corpus Container
//Rare Orokin Container _/
//Reinforced Grineer Container _/
//Reinforced Corpus Container
//Reinforced Orokin Container
//Syndicate Medallions(18)

#define PLAYERS (vdesc.Size == 113216 || \
	vdesc.Size == 93312 || \
	vdesc.Size == 203456 || \
	vdesc.Size == 147328 || \
	vdesc.Size == 160064 || \
	vdesc.Size == 245504 || \
	vdesc.Size == 166272 || \
	vdesc.Size == 264640 || \
	vdesc.Size == 174080 || \
	vdesc.Size == 263040 || \
	vdesc.Size == 358912 || \
	vdesc.Size == 349920 || \
	vdesc.Size == 204512 || \
	vdesc.Size == 103904 || \
	vdesc.Size == 126752 || \
	vdesc.Size == 405984 || \
	vdesc.Size == 223872 || \
	vdesc.Size == 281088 || \
	vdesc.Size == 284160 || \
	vdesc.Size == 151168 || \
	vdesc.Size == 115200 || \
	vdesc.Size == 229696 || \
	vdesc.Size == 113696 || \
	vdesc.Size == 178464 || \
	vdesc.Size == 115968 || \
	vdesc.Size == 244736 || \
	vdesc.Size == 119808 || \
	vdesc.Size == 130208 || \
	vdesc.Size == 197952 || \
	vdesc.Size == 269632)

//==========================================================================================================================

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

// getdir & log
char dlldir[320];
char* GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}

DWORD QuickChecksum(DWORD *pData, int size)
{
	if (!pData) { return 0x0; }

	DWORD sum;
	DWORD tmp;
	sum = *pData;

	for (int i = 1; i < (size / 4); i++)
	{
		tmp = pData[i];
		tmp = (DWORD)(sum >> 29) + tmp;
		tmp = (DWORD)(sum >> 17) + tmp;
		sum = (DWORD)(sum << 3) ^ tmp;
	}

	return sum;
}
//==========================================================================================================================

// colors
#define Green				D3DCOLOR_ARGB(255, 000, 255, 000)
#define Red					D3DCOLOR_ARGB(255, 255, 000, 000)
#define Blue				D3DCOLOR_ARGB(255, 000, 000, 255)
#define Orange				D3DCOLOR_ARGB(255, 255, 165, 000)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 000)
#define Pink				D3DCOLOR_ARGB(255, 255, 192, 203)
#define Cyan				D3DCOLOR_ARGB(255, 000, 255, 255)
#define Purple				D3DCOLOR_ARGB(255, 160, 032, 240)
#define Black				D3DCOLOR_ARGB(255, 000, 000, 000) 
#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Grey				D3DCOLOR_ARGB(255, 112, 112, 112)
#define SteelBlue			D3DCOLOR_ARGB(255, 033, 104, 140)
#define LightSteelBlue		D3DCOLOR_ARGB(255, 201, 255, 255)
#define LightBlue			D3DCOLOR_ARGB(255, 026, 140, 306)
#define Salmon				D3DCOLOR_ARGB(255, 196, 112, 112)
#define Brown				D3DCOLOR_ARGB(255, 168, 099, 020)
#define Teal				D3DCOLOR_ARGB(255, 038, 140, 140)
#define Lime				D3DCOLOR_ARGB(255, 050, 205, 050)
#define ElectricLime		D3DCOLOR_ARGB(255, 204, 255, 000)
#define Gold				D3DCOLOR_ARGB(255, 255, 215, 000)
#define OrangeRed			D3DCOLOR_ARGB(255, 255, 69, 0)
#define GreenYellow			D3DCOLOR_ARGB(255, 173, 255, 047)
#define AquaMarine			D3DCOLOR_ARGB(255, 127, 255, 212)
#define SkyBlue				D3DCOLOR_ARGB(255, 000, 191, 255)
#define SlateBlue			D3DCOLOR_ARGB(255, 132, 112, 255)
#define Crimson				D3DCOLOR_ARGB(255, 220, 020, 060)
#define DarkOliveGreen		D3DCOLOR_ARGB(255, 188, 238, 104)
#define PaleGreen			D3DCOLOR_ARGB(255, 154, 255, 154)
#define DarkGoldenRod		D3DCOLOR_ARGB(255, 255, 185, 015)
#define FireBrick			D3DCOLOR_ARGB(255, 255, 048, 048)
#define DarkBlue			D3DCOLOR_ARGB(255, 000, 000, 204)
#define DarkerBlue			D3DCOLOR_ARGB(255, 000, 000, 153)
#define DarkYellow			D3DCOLOR_ARGB(255, 255, 204, 000)
#define LightYellow			D3DCOLOR_ARGB(255, 255, 255, 153)
#define DarkOutline			D3DCOLOR_ARGB(255, 37,   48,  52)
#define TBlack				D3DCOLOR_ARGB(180, 000, 000, 000) 

//==========================================================================================================================

// menu

int MenuSelection = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int Show = false; //off by default

POINT cPos;

#define ItemColorOn Green
#define ItemColorOff Red
#define ItemCurrent White
#define GroupColor Yellow
#define KategorieFarbe Yellow
#define ItemText White

LPD3DXFONT pFont; //font
bool m_bCreated = false;

int CheckTabs(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&cPos);
		ScreenToClient(GetForegroundWindow(), &cPos);
		if (cPos.x > x && cPos.x < x + w && cPos.y > y && cPos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				return 1;
			}
			return 2;
		}
	}
	return 0;
}

void FillRGB(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangle(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}
	V[4] =
	{
		{ x, (y + h), 0.0f, 0.0f, Color },
		{ x, y, 0.0f, 0.0f, Color },
		{ (x + w), (y + h), 0.0f, 0.0f, Color },
		{ (x + w), y, 0.0f, 0.0f, Color }
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, INT px, DWORD BorderColor)
{
	DrawRectangle(Device, x, (y + h - px), w, px, BorderColor);
	DrawRectangle(Device, x, y, px, h, BorderColor);
	DrawRectangle(Device, x, y, w, px, BorderColor);
	DrawRectangle(Device, (x + w - px), y, px, h, BorderColor);
}

VOID DrawBoxWithBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor, DWORD BorderColor)
{
	DrawRectangle(Device, x, y, w, h, BoxColor);
	DrawBorder(Device, x, y, w, h, 1, BorderColor);
}

VOID DrawBox(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor)
{
	DrawBorder(Device, x, y, w, h, 1, BoxColor);
}

void DrawBox3(IDirect3DDevice9* m_pD3Ddev, int x, int y, int w, int h, D3DCOLOR Color)
{
	D3DRECT rec;
	rec.x1 = x;
	rec.x2 = x + w;
	rec.y1 = y;
	rec.y2 = y + h;
	m_pD3Ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3Ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, D3DPT_TRIANGLESTRIP);
	m_pD3Ddev->Clear(1, &rec, D3DCLEAR_TARGET, Color, 1, 1);
}

void WriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawString(LPD3DXFONT pFont, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		pFont->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = pFont->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTabs(PosX, PosY + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = KategorieFarbe;

		if (Check == 2)
			ColorText = ItemCurrent;

		if (MenuSelection == Current)
			ColorText = ItemCurrent;

		WriteText(PosX - 5, PosY + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 175, PosY + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

//-----------------------------------------------------------------------------
// Name: Save()
// Desc: Saves Menu Item states for later Restoration
//-----------------------------------------------------------------------------

void Save(char* szSection, char* szKey, int iValue, LPCSTR file)
{
	char szValue[255];
	sprintf_s(szValue, "%d", iValue);
	WritePrivateProfileString(szSection, szKey, szValue, file);
}

//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads Menu Item States From Previously Saved File
//-----------------------------------------------------------------------------

int Load(char* szSection, char* szKey, int iDefaultValue, LPCSTR file)
{
	int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, file);
	return iResult;
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTabs(PosX, PosY + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			DrawBox(pDevice, PosX, PosY + (Current * 15), 10, 10, Green);
			ColorText = ItemColorOn;
		}
		if (var == 0)
		{
			DrawBox(pDevice, PosX, PosY + (Current * 15), 10, 10, Red);
			ColorText = ItemColorOff;
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = ItemCurrent;

		if (MenuSelection == Current)
		{
			if (GetAsyncKeyState(VK_RIGHT) & 1)
			{
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) & 1)
			{
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (MenuSelection == Current)
			ColorText = ItemCurrent;

		WriteText(PosX + 13, PosY + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 148, PosY + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//==========================================================================================================================

// crosshair
HRESULT DoubleLine(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT x2, FLOAT y2, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}

	V[2] =
	{
		{ (float)x, (float)y, 0.0f, 1.0f, Color },
		{ (float)x2, (float)y2, 0.0f, 1.0f, Color },
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_LINELIST, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawCrosshair(LPDIRECT3DDEVICE9 Device)
{
	INT center_x = (Viewport.Width / 2);
	INT center_y = (Viewport.Height / 2);
	DrawBox(Device, center_x - 5, center_y - 1, 11, 3, Black);
	DrawBox(Device, center_x - 1, center_y - 5, 3, 11, Black);
	DoubleLine(Device, center_x - 4, center_y, center_x + 5, center_y, White);
	DoubleLine(Device, center_x, center_y - 4, center_x, center_y + 5, White);
}

//==========================================================================================================================

IDirect3DPixelShader9 *shadRed;
IDirect3DPixelShader9 *shadGreen;
IDirect3DPixelShader9 *shadBlue;
IDirect3DPixelShader9 *shadDepthBlue;
IDirect3DPixelShader9 *shadDepthGreen;
IDirect3DPixelShader9 *shadYellow;
IDirect3DPixelShader9 *shadViolet;
IDirect3DPixelShader9 *shadSiren;
IDirect3DPixelShader9 *shadOrange;
IDirect3DPixelShader9 *shadLimon;
IDirect3DPixelShader9 *shadPink;
IDirect3DPixelShader9 *shadWhite;

//generate shader
HRESULT GenerateShader(IDirect3DDevice9 *pDevice, IDirect3DPixelShader9 **pShader, float r, float g, float b, float a, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer *pShaderBuf = NULL;
	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);
	int PXSHVER1 = (D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion));
	int PXSHVER2 = (D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
	if (setzBuf)
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", PXSHVER1, PXSHVER2, r, g, b, a);
	else
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0", PXSHVER1, PXSHVER2, r, g, b, a);
	D3DXAssembleShader(szShader, sizeof(szShader), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pDevice->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

//=====================================================================================================================

//draw sprites, pic esp v3.0
LPD3DXSPRITE lpSprite, lpSprite2, lpSprite3 = NULL;
LPDIRECT3DTEXTURE9 lpSpriteImage, lpSpriteImage2, lpSpriteImage3 = NULL;
bool bSpriteCreated, bSpriteCreated2, bSpriteCreated3 = false;

bool CreateOverlaySprite(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("team1.png"), &lpSpriteImage); //png in hack dir
	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		bSpriteCreated = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpSprite);
	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite failed");
		bSpriteCreated = false;
		return false;
	}

	bSpriteCreated = true;

	return true;
}

bool CreateOverlaySprite2(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("team2.png"), &lpSpriteImage2); //png in hack dir
	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		bSpriteCreated2 = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpSprite2);
	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite failed");
		bSpriteCreated2 = false;
		return false;
	}

	bSpriteCreated2 = true;

	return true;
}

bool CreateOverlaySprite3(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, GetDirectoryFile("team3.png"), &lpSpriteImage3); //png in hack dir
	if (FAILED(hr))
	{
		//Log("D3DXCreateTextureFromFile failed");
		bSpriteCreated3 = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpSprite3);
	if (FAILED(hr))
	{
		//Log("D3DXCreateSprite failed");
		bSpriteCreated3 = false;
		return false;
	}

	bSpriteCreated3 = true;

	return true;
}

// COM utils
template<class COMObject>
void SafeRelease(COMObject*& pRes)
{
	IUnknown *unknown = pRes;
	if (unknown)
	{
		unknown->Release();
	}
	pRes = NULL;
}

// This will get called before Device::Clear(). If the device has been reset
// then all the work surfaces will be created again.
void PreClear(IDirect3DDevice9* device)
{
	if (!bSpriteCreated)
		CreateOverlaySprite(device);

	if (!bSpriteCreated2)
		CreateOverlaySprite2(device);

	if (!bSpriteCreated3)
		CreateOverlaySprite3(device);
}

// Delete work surfaces when device gets reset
void DeleteRenderSurfaces()
{
	if (lpSprite != NULL)
	{
		//Log("SafeRelease(lpSprite)");
		SafeRelease(lpSprite);
	}

	if (lpSprite2 != NULL)
	{
		//Log("SafeRelease(lpSprite2)");
		SafeRelease(lpSprite2);
	}

	if (lpSprite3 != NULL)
	{
		//Log("SafeRelease(lpSprite3)");
		SafeRelease(lpSprite3);
	}

	bSpriteCreated = false;
	bSpriteCreated2 = false;
	bSpriteCreated3 = false;
}

// This gets called right before the frame is presented on-screen - Device::Present().
// First, create the display text, FPS and info message, on-screen. Then then call
// CopySurfaceToTextureBuffer() to downsample the image and copy to shared memory
void PrePresent(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (bSpriteCreated)
	{
		if (lpSprite != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			lpSprite->Begin(D3DXSPRITE_ALPHABLEND);
			lpSprite->Draw(lpSpriteImage, NULL, NULL, &position, 0xFFFFFFFF);
			lpSprite->End();
		}
	}

	// draw text
}

void PrePresent2(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (bSpriteCreated2)
	{
		if (lpSprite2 != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			lpSprite2->Begin(D3DXSPRITE_ALPHABLEND);
			lpSprite2->Draw(lpSpriteImage2, NULL, NULL, &position, 0xFFFFFFFF);
			lpSprite2->End();
		}
	}

	// draw text
}

void PrePresent3(IDirect3DDevice9* Device, int cx, int cy)
{
	int textOffsetLeft;

	//draw sprite
	if (bSpriteCreated3)
	{
		if (lpSprite3 != NULL)
		{
			D3DXVECTOR3 position;
			position.x = (float)cx;
			position.y = (float)cy;
			position.z = 0.0f;

			textOffsetLeft = (int)position.x; //for later to offset text from image

			lpSprite3->Begin(D3DXSPRITE_ALPHABLEND);
			lpSprite3->Draw(lpSpriteImage3, NULL, NULL, &position, 0xFFFFFFFF);
			lpSprite3->End();
		}
	}

	// draw text
}

//==========================================================================================================================
