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

#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE dllHandle;

UINT Stride;

//elementcount
D3DVERTEXELEMENT9 decl[MAXD3DDECLLENGTH];
UINT numElements;

//vertexshader
IDirect3DVertexShader9* vShader;
UINT vSize;

//pixelshader
IDirect3DPixelShader9* pShader;
UINT pSize;

//startregister vector4fcount
UINT mStartRegister;
UINT mVector4fCount;

//settexture
IDirect3DTexture9* sCurrentTex = NULL;
int sWidth;
int sHeight;
int sFormat;
DWORD qCRC;
DWORD qCRC2;
DWORD dwSize;

//drawprimitive gettexture
LPDIRECT3DBASETEXTURE9 dTexture = nullptr;
IDirect3DTexture9* dCurrentTex = NULL;
int dWidth;
int dHeight;
int dFormat;

//esp model distance
float bestRealDistance;

//used for logging/cycling through values
bool logger = false;
int countnum = -1;

bool FirstInit = false; //init once

//vdesc.Size
//D3DVERTEXBUFFER_DESC vdesc;

//viewport
D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

//aim
float lvlsymX, lvlsymY, hpbarX, hpbarY;

IDirect3DIndexBuffer9* ppIndexData;
D3DINDEXBUFFER_DESC iDesc;

//DWORD dwStartTime = 0; //time as the timer started
//DWORD dwTime = 0; //windowsuptime

//=====================================================================================================================

//model recogntion: (outdated)

//models
#define MODELS (mStartRegister == 52 && mVector4fCount >= 82)

//shiny glow around caches and everything
#define CACHE_GLOW (Stride == 24 && vSize == 352 && pSize == 540 && mStartRegister == 12 && mVector4fCount == 1)

//mods
#define MOD ((Stride == 24 && NumVertices == 1619 && vSize == 768 && /*pSize == 1460 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 11 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 1311 && vSize == 768 && /*pSize == 1724 &&*/ mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 1311 && vSize == 352 && /*pSize == 540 &&*/ mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 953 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 11 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 953 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1))

//=====================================================================================================================

// settings
int crosshair = 0;				//crosshair
int wallhack = 1;				//wallhack
int chams = 2;					//chams
int items = 2;					//items
int cacheglow = 0;				//glow around caches
int aimbot = 1;					//aimbot
int aimsens = 3;				//aim sens
int aimheight = 4;				//aim height
int aimcheckespfov = 0;			//reduce aimdown while reloading
int esp = 0;					//esp
int autoshoot = 2;				//autoshoot

DWORD Daimkey = VK_RBUTTON;		//aimkey
int aimkey = 2;					//aimkey menu value
int aimfov = 40;				//aim fov in % (40+ may create problems)
int espfov = 90;				//esp fov in % 90
bool IsPressed = false;			//
DWORD gametick = timeGetTime(); //autoshoot timer
unsigned int asdelay = 1;		//1-4

//==========================================================================================================================

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

void doDisassembleShader(LPDIRECT3DDEVICE9 pDevice, char* FileName)
{
	std::ofstream oLogFile(FileName, std::ios::trunc);

	if (!oLogFile.is_open())
		return;

	IDirect3DVertexShader9* pShader;

	pDevice->GetVertexShader(&pShader);

	UINT pSizeOfData;

	pShader->GetFunction(NULL, &pSizeOfData);

	BYTE* pData = new BYTE[pSizeOfData];

	pShader->GetFunction(pData, &pSizeOfData);

	LPD3DXBUFFER bOut;

	D3DXDisassembleShader(reinterpret_cast<DWORD*>(pData), NULL, NULL, &bOut);

	oLogFile << static_cast<char*>(bOut->GetBufferPointer()) << std::endl;

	oLogFile.close();

	delete[] pData;

	pShader->Release();

}

//=====================================================================================================================

//get distance
float GetDistance(float Xx, float Yy, float xX, float yY)
{
	return sqrt((yY - Yy) * (yY - Yy) + (xX - Xx) * (xX - Xx));
}

//aim worldtoscreen
struct AimInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<AimInfo_t>AimInfo;

void AddAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	float aimx, aimy; //aimz;
	D3DXMATRIX matrix, m1;
	D3DXVECTOR4 position, input;
	Device->GetVertexShaderConstantF(0, matrix, 4);
	
	input.x = 0.0f;
	input.y = 1200.0f;
	input.z = 0.0f;
	input.w = 1.0f;

	//adjust aimheight
	//if (aimbot == 1)
		//input.x = 0.0f; //do not move left if aiming at lvl bar in pve
	//else if (aimbot == 2)
		//input.x = 1000.0f; //move left to aim at center of hp bar in pvp

	if (aimkey == 2 && GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		input.y += 1.0f + ((float)aimheight * 80.0f);
	else
		input.y += 1.0f + ((float)aimheight * 40.0f);


	D3DXMatrixTranspose(&m1, &matrix);
	//D3DXVec4Transform(&position, &input, &m1);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	//aimz = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	if (matrix._44 > 1.0f)
	{
		aimx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
		aimy = Viewport.Y + (Viewport.Height / 2.0f) - ((position.y / position.w) * (Viewport.Height / 2.0f));

		AimInfo_t pAimInfo = { static_cast<float>(aimx), static_cast<float>(aimy), iTeam };

		AimInfo.push_back(pAimInfo);
	}
}


//aim2 
struct AimInfo2_t
{
	float vOutX, vOutY;
	INT       iTeam;
	float CrosshairDistance;
};
std::vector<AimInfo2_t>AimInfo2;

void AddAim2(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	float aimx, aimy; //aimz;
	D3DXMATRIX matrix, m1;
	D3DXVECTOR4 position, input;
	Device->GetVertexShaderConstantF(0, matrix, 4);

	input.x = 0.0f;
	input.y = 522.0f;
	input.z = 0.0f;
	input.w = 1.0f;


	if (aimkey == 2 && GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		input.y += 1.0f + ((float)aimheight * 80.0f);
	else
		input.y += 1.0f + ((float)aimheight * 40.0f);


	D3DXMatrixTranspose(&m1, &matrix);
	//D3DXVec4Transform(&position, &input, &m1);

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	//aimz = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	if (matrix._44 > 1.0f)
	{
		aimx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
		aimy = Viewport.Y + (Viewport.Height / 2.0f) - ((position.y / position.w) * (Viewport.Height / 2.0f));

		AimInfo2_t pAimInfo2 = { static_cast<float>(aimx), static_cast<float>(aimy), iTeam };

		AimInfo2.push_back(pAimInfo2);
	}
}


// esp worldtoscreen
struct EspInfo_t
{
	float vOutX, vOutY;
	INT       iTeam;
	char* oName;
	D3DCOLOR cColor;
	float RealDistance;
	float CrosshairDistance;
};
std::vector<EspInfo_t>EspInfo;
bool inespfov = false;

void AddEsp(LPDIRECT3DDEVICE9 Device, int iTeam, char* oName, D3DCOLOR cColor, float YHeight)
{
	float espx, espy, espz;
	D3DXMATRIX matrix, m1;
	D3DXVECTOR4 position, input;
	Device->GetVertexShaderConstantF(0, matrix, 4);

	input.x = 0.0f;
	input.y = YHeight; //1.5fdefault, 0.0f for items==1
	input.z = 0.0f;
	input.w = 1.0f;

	D3DXMatrixTranspose(&m1, &matrix);
	//D3DXVec4Transform(&position, &input, &m1); 

	position.x = input.x * matrix._11 + input.y * matrix._21 + input.z * matrix._31 + matrix._41;
	position.y = input.x * matrix._12 + input.y * matrix._22 + input.z * matrix._32 + matrix._42;
	position.z = input.x * matrix._13 + input.y * matrix._23 + input.z * matrix._33 + matrix._43;
	position.w = input.x * matrix._14 + input.y * matrix._24 + input.z * matrix._34 + matrix._44;

	espz = Viewport.MinZ + position.z * (Viewport.MaxZ - Viewport.MinZ); //real distance

	if (espz > 0.0f && matrix._44 > 1.0f)
	{
		espx = ((position.x / position.w) * (Viewport.Width / 2.0f)) + Viewport.X + (Viewport.Width / 2.0f);
		espy = Viewport.Y + (Viewport.Height / 2.0f) - ((position.y / position.w) * (Viewport.Height / 2.0f));

		EspInfo_t pModelInfo = { static_cast<float>(espx), static_cast<float>(espy), iTeam, oName, cColor, espz };

		EspInfo.push_back(pModelInfo);
	}
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

// menu part
char *opt_OnOff[] = { "[OFF]", "[ON]" };
char *opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
char *opt_Chams[] = { "[OFF]", "[NPCs 1]", "[NPCs 2]", "[Players 3]" };
char *opt_Items[] = { "[OFF]", "[Text]", "[Esp/Text]" };
char *opt_onetwo[] = { "[OFF]", "[1]", "[2]" };
char *opt_Autoshoot[] = { "[OFF]", "[Auto]", "[OnKeyDown]" };
char *opt_Sensitivity[] = { "[OFF]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]" };
char *opt_Aimheight[] = { "[0]", "[-1]", "[-2]", "[-3]", "[-4]", "[-5]", "[-6]" };
char *opt_Esp[] = { "[OFF]", "[Box]", "[Pic]" };
char *opt_Aimbot[] = { "[OFF]", "[PvE]", "[PvP]" };

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
				//return 1; //mouse selection OFF
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
	if (x > 1 && x < Viewport.Width && y > 1 && y < Viewport.Height)
	{
		DrawRectangle(Device, x, (y + h - px), w, px, BorderColor);
		DrawRectangle(Device, x, y, px, h, BorderColor);
		DrawRectangle(Device, x, y, w, px, BorderColor);
		DrawRectangle(Device, (x + w - px), y, px, h, BorderColor);
	}
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

	if(pFont && X < Viewport.Width && Y < Viewport.Height)
	{
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

void BuildMenu(LPDIRECT3DDEVICE9 pDevice)
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		Show = !Show;

		//save settings
		Save("Crosshair", "Crosshair", crosshair, GetDirectoryFile("settings.ini"));
		Save("Wallhack", "Wallhack", wallhack, GetDirectoryFile("settings.ini"));
		Save("Chams", "Chams", chams, GetDirectoryFile("settings.ini"));
		Save("Items", "Items", items, GetDirectoryFile("settings.ini"));
		Save("CacheGlow", "CacheGlow", cacheglow, GetDirectoryFile("settings.ini"));
		Save("Aimbot", "Aimbot", aimbot, GetDirectoryFile("settings.ini"));
		Save("Aimkey", "Aimkey", aimkey, GetDirectoryFile("settings.ini"));
		Save("Aimsens", "Aimsens", aimsens, GetDirectoryFile("settings.ini"));
		Save("Aimheight", "Aimheight", aimheight, GetDirectoryFile("settings.ini"));
		Save("AimCheckEspFov", "AimCheckEspFov", aimcheckespfov, GetDirectoryFile("settings.ini"));
		Save("Esp", "Esp", esp, GetDirectoryFile("settings.ini"));
		Save("Autoshoot", "Autoshoot", autoshoot, GetDirectoryFile("settings.ini"));
	}

	if (Show)
	{
		if (GetAsyncKeyState(VK_UP) & 1)
			MenuSelection--;

		if (GetAsyncKeyState(VK_DOWN) & 1)
			MenuSelection++;

		//Background
		FillRGB(pDevice, 25, 38, 157, 186, TBlack);

		DrawBox(pDevice, 20, 15, 168, 20, DarkOutline);
		cWriteText(105, 18, White, "Warframe D3D");
		DrawBox(pDevice, 20, 34, 168, Current * 15, DarkOutline);

		Current = 1;
		//Category(pDevice, " [D3D]");
		AddItem(pDevice, " Crosshair", crosshair, opt_OnOff, 1);
		AddItem(pDevice, " Wallhack", wallhack, opt_OnOff, 1);
		//AddItem(pDevice, " Chams", chams, opt_Chams, 3);
		//AddItem(pDevice, " Items", items, opt_Items, 2);
		//AddItem(pDevice, " CacheGlow", cacheglow, opt_onetwo, 2);
		AddItem(pDevice, " Aimbot", aimbot, opt_Aimbot, 2);
		AddItem(pDevice, " Aimkey", aimkey, opt_Keys, 8);
		AddItem(pDevice, " Aimsens", aimsens, opt_Sensitivity, 8);
		AddItem(pDevice, " Aimheight", aimheight, opt_Aimheight, 6);
		//AddItem(pDevice, " AimCheckEspFov", aimcheckespfov, opt_OnOff, 1);
		AddItem(pDevice, " Esp", esp, opt_Esp, 2);
		AddItem(pDevice, " Autoshoot", autoshoot, opt_Autoshoot, 2);

		if (MenuSelection >= Current)
			MenuSelection = 1;

		if (MenuSelection < 1)
			MenuSelection = Current;
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

HRESULT GenerateShader(IDirect3DDevice9 *pD3Ddev, IDirect3DPixelShader9 **pShader, float r, float g, float b, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer *pShaderBuf = NULL;
	if (setzBuf)
		sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", r, g, b, 1.0f);
	else
		sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0", r, g, b, 1.0f);
	D3DXAssembleShader(szShader, (strlen(szShader) + 1), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pD3Ddev->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

//=====================================================================================================================
