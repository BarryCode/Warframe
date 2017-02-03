/*
* Warframe D3D Hack Source V1.1b by Nseven

How to compile:
- download and install "Microsoft Visual Studio Express 2015 for Windows DESKTOP" https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx

- open wfdxhook.vcxproj (not wfdxhook.vcxproj.filters) with Visual Studio 2015 (Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\WDExpress.exe)
- select x86(32bit) or x64(64bit)
- compile dll, press f7 or click the green triangle

x86 compiled dll will be in WFDXHook\Release folder
x64 compiled dll will be in WFDXHook\x64\Release folder

If you share your dll with others, remove dependecy on vs runtime before compiling:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)

How to use:
- start warframe launcher
- click options
- disable fullscreen, disable dx10, disable dx11, disable 64 bit if you compiled in x86, enable 64 bit if you compiled in x64
- click OK, press PLAY
- if x86 -> inject dll in main screen into Warframe.exe
- if x64 -> inject dll in main screen into Warframe.x64.exe

Menu key:
- insert

Logging:
ALT + CTRL + L toggles logger
- press O to decrease values
- press P to increase, hold down P key until a texture changes
- press I to log values of changed textures
*/


#include "main.h" //less important stuff & helper funcs here

typedef HRESULT(APIENTRY *SetVertexShaderConstantF)(IDirect3DDevice9*, UINT, const float*, UINT);
HRESULT APIENTRY SetVertexShaderConstantF_hook(IDirect3DDevice9*, UINT, const float*, UINT);
SetVertexShaderConstantF SetVertexShaderConstantF_orig = 0;

typedef HRESULT(APIENTRY *DrawIndexedPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
DrawIndexedPrimitive DrawIndexedPrimitive_orig = 0;

typedef HRESULT(APIENTRY *DrawPrimitive)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
HRESULT APIENTRY DrawPrimitive_hook(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
DrawPrimitive DrawPrimitive_orig = 0;

typedef HRESULT(APIENTRY* EndScene) (IDirect3DDevice9*);
HRESULT APIENTRY EndScene_hook(IDirect3DDevice9*);
EndScene EndScene_orig = 0;

typedef HRESULT(APIENTRY *Reset)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
HRESULT APIENTRY Reset_hook(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
Reset Reset_orig = 0;

typedef HRESULT(APIENTRY *SetStreamSource)(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
HRESULT APIENTRY SetStreamSource_hook(IDirect3DDevice9*, UINT, IDirect3DVertexBuffer9*, UINT, UINT);
SetStreamSource SetStreamSource_orig = 0;

typedef HRESULT(APIENTRY *SetVertexShader)(IDirect3DDevice9*, IDirect3DVertexShader9*);
HRESULT APIENTRY SetVertexShader_hook(IDirect3DDevice9*, IDirect3DVertexShader9*);
SetVertexShader SetVertexShader_orig = 0;

typedef HRESULT(APIENTRY *SetPixelShader)(IDirect3DDevice9*, IDirect3DPixelShader9*);
HRESULT APIENTRY SetPixelShader_hook(IDirect3DDevice9*, IDirect3DPixelShader9*);
SetPixelShader SetPixelShader_orig = 0;

typedef HRESULT(APIENTRY *SetTexture)(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9*, DWORD, IDirect3DBaseTexture9*);
SetTexture SetTexture_orig = 0;

typedef HRESULT(APIENTRY *SetViewport)(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
HRESULT APIENTRY SetViewport_hook(IDirect3DDevice9*, CONST D3DVIEWPORT9*);
SetViewport SetViewport_orig = 0;

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
int aimkey = 2;
int aimfov = 40;				//aim fov in % (40+ may create problems)
int espfov = 90;				//esp fov in % 90
bool IsPressed = false;			//
DWORD gametick = timeGetTime(); //autoshoot timer
unsigned int asdelay = 1;		//1-4

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
	//float RealDistance;
	float CrosshairDistance;
};
std::vector<AimInfo_t>AimInfo;

void AddAim(LPDIRECT3DDEVICE9 Device, int iTeam)
{
	float aimx, aimy; //aimz;
	D3DXMATRIX matrix, m1;
	D3DXVECTOR4 position, input;
	Device->GetVertexShaderConstantF(0, matrix, 4);

	//adjust aimheight
	if (aimbot == 1)
		input.x = 0.0f; //do not move left if aiming at lvl bar in pve
	else if (aimbot == 2)
		input.x = 1000.0f; //move left to aim at center of hp bar in pvp

	if (aimkey == 2 && GetAsyncKeyState(VK_RBUTTON) & 0x8000)
		input.y += 700.0f + ((float)aimheight * 80.0f);
	else
		input.y += 400.0f + ((float)aimheight * 40.0f);

	input.z = 0.0f;
	input.w = 1.0f;

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

// menu part
char *opt_OnOff[] = { "[OFF]", "[ON]" };
char *opt_Keys[] = { "[OFF]", "[Shift]", "[RMouse]", "[LMouse]", "[Ctrl]", "[Alt]", "[Space]", "[X]", "[C]" };
char *opt_Chams[] = { "[OFF]", "[NPCs 1]", "[NPCs 2]", "[Players 3]" };
char *opt_Items[] = { "[OFF]", "[Text]", "[Esp/Text]" };
char *opt_onetwo[] = { "[OFF]", "[1]", "[2]" };
char *opt_Autoshoot[] = { "[OFF]", "[Auto]", "[OnKeyDown]" };
char *opt_Sensitivity[] = { "[OFF]", "[2]", "[3]", "[4]", "[5]", "[6]", "[7]", "[8]", "[9]" };
char *opt_Aimheight[] = { "[0]", "[-1]", "[-2]", "[-3]", "[-4]", "[-5]", "[-6]", "[-7]", "[-8]", "[-9]", "[-10]", "[-11]", "[-12]" };
char *opt_Esp[] = { "[OFF]", "[Box]", "[Pic]" };
char *opt_Aimbot[] = { "[OFF]", "[PvE]", "[PvP]" };

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
		AddItem(pDevice, " Chams", chams, opt_Chams, 3);
		AddItem(pDevice, " Items", items, opt_Items, 2);
		AddItem(pDevice, " CacheGlow", cacheglow, opt_onetwo, 2);
		AddItem(pDevice, " Aimbot", aimbot, opt_Aimbot, 2);
		AddItem(pDevice, " Aimkey", aimkey, opt_Keys, 8);
		AddItem(pDevice, " Aimsens", aimsens, opt_Sensitivity, 8);
		AddItem(pDevice, " Aimheight", aimheight, opt_Aimheight, 12);
		AddItem(pDevice, " AimCheckEspFov", aimcheckespfov, opt_OnOff, 1);
		AddItem(pDevice, " Esp", esp, opt_Esp, 2);
		AddItem(pDevice, " Autoshoot", autoshoot, opt_Autoshoot, 2);

		if (MenuSelection >= Current)
			MenuSelection = 1;

		if (MenuSelection < 1)
			MenuSelection = Current;
	}
}

//=====================================================================================================================

HRESULT APIENTRY SetVertexShaderConstantF_hook(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float *pConstantData, UINT Vector4fCount)
{
	if (pConstantData != NULL)
	{
		//pConstantDataFloat = (float*)pConstantData;

		mStartRegister = StartRegister;
		mVector4fCount = Vector4fCount;
	}

	return SetVertexShaderConstantF_orig(pDevice, StartRegister, pConstantData, Vector4fCount);
}

//==========================================================================================================================

HRESULT APIENTRY SetStreamSource_hook(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride)
{
	if (StreamNumber == 0) 
	{
		Stride = sStride;

		if (pStreamData && Stride == 32)
		{
			pStreamData->GetDesc(&vdesc);
		}
	}

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9 *veShader)
{
	if (veShader != NULL)
	{
		vShader = veShader;
		vShader->GetFunction(NULL, &vSize);
	}

	return SetVertexShader_orig(pDevice, veShader);
}

//==========================================================================================================================

HRESULT APIENTRY SetPixelShader_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9 *piShader)
{
	if (piShader != NULL)
	{
		pShader = piShader;
		pShader->GetFunction(NULL, &pSize);
	}

	return SetPixelShader_orig(pDevice, piShader);
}

//==========================================================================================================================

HRESULT APIENTRY DrawIndexedPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	//wallhack
	if (MODELS || CACHE_GLOW || MISC)
	{
		//behind walls (enables wallhack for models, glow, misc items)
		if (MODELS && wallhack == 1 || MODELS && chams > 0 || CACHE_GLOW && cacheglow > 0 || MISC)
		{
			pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
			pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_NEVER);
		}

		//chams 1 for NPCs
		if (!PLAYERS && MODELS && chams == 1)
		{
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			pDevice->SetPixelShader(NULL);
			pDevice->SetPixelShader(shadRed);
		}
		//chams 2 for NPCs
		else if (!PLAYERS && MODELS && chams == 2)
		{
			float PvERed[4] = { 1.0f, 0.0f, 0.0f, 3.0f };
			pDevice->SetPixelShaderConstantF(50, PvERed, 1);//50red, 51green, 52blue
		}
		else if (PLAYERS && chams == 3)
		{
			pDevice->SetPixelShader(NULL);
			//pDevice->SetPixelShader(sSubB);
			pDevice->SetPixelShader(shadDepthBlue);
		}

		DWORD dwOldBLENDOP;
		//color itemglow
		if ((!STUFFTHATSHOULDNOTGLOW && CACHE_GLOW) && (wallhack == 1 || cacheglow > 0))
		{
			if (cacheglow == 1)
			{
				pDevice->GetRenderState(D3DRS_BLENDOP, &dwOldBLENDOP);
				pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT); //reduce blending

				float vals[4] = { ((255 >> 24) & 0xFF) / 1.0f, ((255 >> 16) & 0xFF) / 1.0f, ((255 >> 8) & 0xFF) / 1.0f, 1.0f };
				pDevice->SetPixelShaderConstantF(2, vals, 1);
			}
			else if (cacheglow == 2)
			{
				float vals[4] = { (255 >> 24) / 1.0f, (255 >> 16) / 1.0f, 255.0f, 1.0f };
				pDevice->SetPixelShaderConstantF(2, vals, 1);
				//pDevice->SetPixelShader(sSubA); //great but bad with amd
				//pDevice->SetPixelShader(shadBlue); //buggy
			}
		}

		DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

		//in front of walls (disables wallhack for models, glow, misc items)
		if (MODELS && wallhack == 1 || MODELS && chams > 0 || CACHE_GLOW && cacheglow > 0 || MISC)
		{
			pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
		}

		//chams 1 for NPCs
		if (!PLAYERS && MODELS && chams == 1)
		{
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pDevice->SetPixelShader(NULL);
			pDevice->SetPixelShader(shadBlue);
		}
		//chams 2 for NPCs
		else if (!PLAYERS && MODELS && chams == 2)
		{
			float PvEGreen[4] = { 0.0f, 1.0f, 0.0f, 3.0f };
			pDevice->SetPixelShaderConstantF(51, PvEGreen, 1);//50red, 51green, 52blue
		}
		//else if (PLAYERS && chams == 3)
		//{

		//}

		//color itemglow
		if ((!STUFFTHATSHOULDNOTGLOW && CACHE_GLOW) && (wallhack == 1 || cacheglow > 0))
		{
			if (cacheglow == 1)
			{
				pDevice->SetRenderState(D3DRS_BLENDOP, dwOldBLENDOP); //
			}
			else if (cacheglow == 2)
			{
				pDevice->SetPixelShader(shadDepthBlue); //blend fuqups
			}
		}

	}


	//esp
	if (MODELS)
	{
		AddEsp(pDevice, 1, "Model", White, 1.5f);
	}

	//items
	if (items > 0 && YELLOWPART)
		AddEsp(pDevice, 2, "Resource", Cyan, 0.0f);

	if (items > 0 && BLUEPART)
		AddEsp(pDevice, 2, "Resource", Cyan, 0.0f);

	if (items > 0 && MOD1)
		AddEsp(pDevice, 2, "MOD", Green, 0.0f);

	if (items > 0 && CREDITS)
		AddEsp(pDevice, 2, "Credits", Gold, 0.0f);

	if (items > 0 && RARECONTAINER)
		AddEsp(pDevice, 2, "RARECONTAINER", DarkGoldenRod, 0.0f);

	if (items > 0 && GRENADE)
		AddEsp(pDevice, 2, "GRENADE", OrangeRed, 0.0f);

	/*
	//small bruteforce logger
	//ALT + CTRL + L toggles logger
	if (logger)
	{
		//hold down P key until a texture changes, press I to log values of those textures
		if (GetAsyncKeyState('O') & 1) //-
			countnum--;
		if (GetAsyncKeyState('P') & 1) //+
			countnum++;
		if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState('9') & 1)) //reset, set to -1
			countnum = -1;
		if (countnum == NumVertices / 10 || countnum == vSize / 10)
			if (pSize > 44 && GetAsyncKeyState('I') & 1) //press I to log to log.txt
				Log("Stride == %d && NumVertices == %d && primCount == %d && vSize == %d && pSize == %d && mStartRegister == %d && mVector4fCount == %d && vdesc.Size == %d && startIndex == %d", Stride, NumVertices, primCount, vSize, pSize, mStartRegister, mVector4fCount, vdesc.Size, startIndex);
				//Log("Stride == %d && NumVertices == %d && primCount == %d && vSize == %d && pSize == %d && mStartRegister == %d && mVector4fCount == %d && vdesc.Size == %d", Stride, NumVertices, primCount, vSize, pSize, mStartRegister, mVector4fCount, vdesc.Size);
		pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (countnum == NumVertices / 10 || countnum == vSize / 10)
		{
			return D3D_OK; //delete texture
			pDevice->SetTexture(0, NULL);
			//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}
	*/
	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

//==========================================================================================================================

HRESULT APIENTRY DrawPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	//aimbot part 1
	if (aimbot == 1) //best for pve
		if (Stride == 24 && PrimitiveCount == 2 && vSize == 316 && pSize == 1084 && mStartRegister == 11 && mVector4fCount == 1 && dWidth == 64 && dHeight == 64) //level and crosshair
		{
			//worldtoscreen
			AddAim(pDevice, 1);
		}

	if (aimbot == 2) //best in pvp
		if (Stride == 24 && PrimitiveCount == 2 && vSize == 316 && pSize == 1084 && mStartRegister == 11 && mVector4fCount == 1 && dWidth == 8 && dHeight == 8)//wholething
		{
			//worldtoscreen
			AddAim(pDevice, 1);
		}

	return DrawPrimitive_orig(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

//==========================================================================================================================

HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	//sprite
	PreClear(pDevice);

	//pDevice->GetViewport(&Viewport); //get viewport
	//ScreenCenterX = Viewport.Width / 2.0f;
	//ScreenCenterY = Viewport.Height / 2.0f;

	if (FirstInit == FALSE)
	{
		FirstInit = TRUE;
		//pDevice->GetViewport(&Viewport); //get viewport
		//ScreenCenterX = Viewport.Width / 2.0f;
		//ScreenCenterY = Viewport.Height / 2.0f;

		//generate shader
		GenerateShader(pDevice, &shadRed, 1.0f, 0.0f, 0.0f, 0.5f, false);
		GenerateShader(pDevice, &shadBlue, 0.0f, 0.0f, 1.0f, 0.5f, false);
		GenerateShader(pDevice, &shadDepthBlue, 0.0f, 0.0f, 1.0f, 0.5f, true);
		//GenerateShader(pDevice, &shadGreen, 0.0f, 1.0f, 0.0f, false);
		//GenerateShader(pDevice, &shadYellow, 1.0f, 1.0f, 0.0f, false);

		//generate circle shader
		DX9CreateEllipseShader(pDevice);

		//load settings
		crosshair = Load("Crosshair", "Crosshair", crosshair, GetDirectoryFile("settings.ini"));
		wallhack = Load("Wallhack", "Wallhack", wallhack, GetDirectoryFile("settings.ini"));
		chams = Load("Chams", "Chams", chams, GetDirectoryFile("settings.ini"));
		items = Load("Items", "Items", items, GetDirectoryFile("settings.ini"));
		cacheglow = Load("cacheglow", "cacheglow", cacheglow, GetDirectoryFile("settings.ini"));
		aimbot = Load("Aimbot", "Aimbot", aimbot, GetDirectoryFile("settings.ini"));
		aimkey = Load("Aimkey", "Aimkey", aimkey, GetDirectoryFile("settings.ini"));
		aimsens = Load("Aimsens", "Aimsens", aimsens, GetDirectoryFile("settings.ini"));
		aimheight = Load("Aimheight", "Aimheight", aimheight, GetDirectoryFile("settings.ini"));
		aimcheckespfov = Load("Aimcheckespfov", "AimCheckEspFov", aimcheckespfov, GetDirectoryFile("settings.ini"));
		esp = Load("Esp", "Esp", esp, GetDirectoryFile("settings.ini"));
		autoshoot = Load("Autoshoot", "Autoshoot", autoshoot, GetDirectoryFile("settings.ini"));
	}

	if (pFont == NULL)
	{
		HRESULT hr = D3DXCreateFont(pDevice, 13, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &pFont);

		if (FAILED(hr)) {
			//Log("D3DXCreateFont failed");
		}
	}

	if (pFont)
	{
		//pDevice->GetViewport(&Viewport);
		BuildMenu(pDevice);
	}

	//crosshair (no shit)
	if (crosshair == 1)
		DrawCrosshair(pDevice);

	//esp part 2
	if ((esp > 0 || aimbot > 0 || items > 0) && (EspInfo.size() != NULL))
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < EspInfo.size(); i++)
		{
			//esp 1
			if (esp == 1 && EspInfo[i].iTeam == 1 && EspInfo[i].vOutX > 1 && EspInfo[i].vOutY > 1 && EspInfo[i].RealDistance > 3.0f)
			{
				//show box
				DrawBorder(pDevice, (int)EspInfo[i].vOutX - 9, (int)EspInfo[i].vOutY, 20, 30, 1, Green);

				//draw real distance
				DrawString(pFont, (int)EspInfo[i].vOutX - 9, (int)EspInfo[i].vOutY, Green, "%.f", EspInfo[i].RealDistance*2.0f);
			}

			//esp 2
			if (esp == 2 && EspInfo[i].iTeam == 1 && EspInfo[i].vOutX > 1 && EspInfo[i].vOutY > 1 && EspInfo[i].RealDistance > 3.0f)
			{
				//drawpic
				PrePresent(pDevice, (int)EspInfo[i].vOutX - 32, (int)EspInfo[i].vOutY - 20);

				//draw real distance
				DrawString(pFont, (int)EspInfo[i].vOutX - 9, (int)EspInfo[i].vOutY, Green, "%.f", EspInfo[i].RealDistance*2.0f);
			}

			//item esp
			if (items == 1 && EspInfo[i].iTeam == 2 && EspInfo[i].vOutX > 1 && EspInfo[i].vOutY > 1)
				DrawString(pFont, (int)EspInfo[i].vOutX, (int)EspInfo[i].vOutY, EspInfo[i].cColor, EspInfo[i].oName);

			if (items == 2 && EspInfo[i].iTeam == 2 && EspInfo[i].vOutX > 1 && EspInfo[i].vOutY > 1)
			{
				//drawpic
				//PrePresent2(pDevice, (int)EspInfo[i].vOutX - 32, (int)EspInfo[i].vOutY - 20);

				//DWORD col[4] = { 0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00 };//yellow
				DX9DrawEllipse(pDevice, (int)EspInfo[i].vOutX - 12, (int)EspInfo[i].vOutY - 10, 25, 25, 6, &EspInfo[i].cColor);
				DrawString(pFont, (int)EspInfo[i].vOutX, (int)EspInfo[i].vOutY, EspInfo[i].cColor, EspInfo[i].oName);
			}

			//aimfov
			float radiusx = espfov * (ScreenCenterX / 100.0f);
			float radiusy = espfov * (ScreenCenterY / 100.0f);

			//get crosshairdistance
			EspInfo[i].CrosshairDistance = GetDistance(EspInfo[i].vOutX, EspInfo[i].vOutY, ScreenCenterX, ScreenCenterY);

			//if in fov
			if (EspInfo[i].vOutX >= ScreenCenterX - radiusx && EspInfo[i].vOutX <= ScreenCenterX + radiusx && EspInfo[i].vOutY >= ScreenCenterY - radiusy && EspInfo[i].vOutY <= ScreenCenterY + radiusy)

				//get closest/nearest target to crosshair
				if (EspInfo[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = EspInfo[i].CrosshairDistance;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1)
		{
			//reduce aimdown while reloading
			float radius = 25 * (ScreenCenterX / 100);
			if (EspInfo[BestTarget].vOutX >= ScreenCenterX - radius && EspInfo[BestTarget].vOutX <= ScreenCenterX + radius && EspInfo[BestTarget].vOutY >= ScreenCenterY - radius && EspInfo[BestTarget].vOutY <= ScreenCenterY + radius)
				inespfov = true;
			else inespfov = false;
			//if (inespfov)
				//DrawString(pFont, 210, 210, Green, "inespfov");
			//else if (!inespfov) DrawString(pFont, 200, 200, Red, "NOT inespfov");

			//hp bar distance is always 5.3 thats why we need esp distance to bodies instead
			bestRealDistance = EspInfo[BestTarget].RealDistance;

			//change color to white if best target is found
			if (esp == 1 && EspInfo[BestTarget].iTeam == 1 && EspInfo[BestTarget].vOutX > 1 && EspInfo[BestTarget].vOutY > 1 && EspInfo[BestTarget].RealDistance > 3.0f)
			{
				DrawBorder(pDevice, (int)EspInfo[BestTarget].vOutX - 9, (int)EspInfo[BestTarget].vOutY, 20, 30, 1, White);
				DrawString(pFont, (int)EspInfo[BestTarget].vOutX - 9, (int)EspInfo[BestTarget].vOutY, White, "%.f", EspInfo[BestTarget].RealDistance*2.0f);
			}

			//change color to white if best target is found
			if (esp == 2 && EspInfo[BestTarget].iTeam == 1 && EspInfo[BestTarget].vOutX > 1 && EspInfo[BestTarget].vOutY > 1 && EspInfo[BestTarget].RealDistance > 3.0f)
			{
				//drawpic
				//PrePresent3(pDevice, (int)EspInfo[BestTarget].vOutX - 32, (int)EspInfo[BestTarget].vOutY - 20);

				DrawString(pFont, (int)EspInfo[BestTarget].vOutX - 9, (int)EspInfo[BestTarget].vOutY, White, "%.f", EspInfo[BestTarget].RealDistance*2.0f);//EspInfo[BestTarget].RealDistance*2.0f
			}

		}
	}
	EspInfo.clear();


	//Shift|RMouse|LMouse|Ctrl|Alt|Space|X|C
	if (aimkey == 0) Daimkey = 0;
	if (aimkey == 1) Daimkey = VK_SHIFT;
	if (aimkey == 2) Daimkey = VK_RBUTTON;
	if (aimkey == 3) Daimkey = VK_LBUTTON;
	if (aimkey == 4) Daimkey = VK_CONTROL;
	if (aimkey == 5) Daimkey = VK_MENU;
	if (aimkey == 6) Daimkey = VK_SPACE;
	if (aimkey == 7) Daimkey = 0x58; //X
	if (aimkey == 8) Daimkey = 0x43; //C

	//aimbot part 2
	//if (aimbot > 0 && AimInfo.size() != NULL && GetAsyncKeyState(Daimkey))
	if (aimbot > 0 && AimInfo.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < AimInfo.size(); i++)
		{
			//aimfov
			float radiusx = aimfov * (ScreenCenterX / 100.0f);
			float radiusy = aimfov * (ScreenCenterY / 100.0f);

			//get crosshairdistance
			AimInfo[i].CrosshairDistance = GetDistance(AimInfo[i].vOutX, AimInfo[i].vOutY, ScreenCenterX, ScreenCenterY);

			//aim at team 1 or 2
			//if (aimbot == AimInfo[i].iTeam)

			//if in fov
			if (AimInfo[i].vOutX >= ScreenCenterX - radiusx && AimInfo[i].vOutX <= ScreenCenterX + radiusx && AimInfo[i].vOutY >= ScreenCenterY - radiusy && AimInfo[i].vOutY <= ScreenCenterY + radiusy)

				//get closest/nearest target to crosshair
				if (AimInfo[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = AimInfo[i].CrosshairDistance;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if ((aimcheckespfov == 0 && BestTarget != -1)||(aimcheckespfov == 1 && inespfov && BestTarget != -1))
		{
			double DistX = AimInfo[BestTarget].vOutX - ScreenCenterX;
			double DistY = AimInfo[BestTarget].vOutY - ScreenCenterY;

			DistX /= aimsens;
			DistY /= aimsens;

			float dstmultiplier;
			if (aimkey == 2 && GetAsyncKeyState(VK_RBUTTON) & 0x8000)
				dstmultiplier = 0.26f;
			else dstmultiplier = 0.20f;

			//DrawString(pFont, 200, 200, White, "%f", dstmultiplier);

			//aim
			if(GetAsyncKeyState(Daimkey) & 0x8000)
			{
				if (bestRealDistance > 0.0f && bestRealDistance <= 2.0f) //0-4
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY + 29.0f, 0, NULL);
				else if (bestRealDistance > 2.0f && bestRealDistance <= 4.0f) //4-8
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY + 23.0f, 0, NULL);
				else if (bestRealDistance > 4.0f && bestRealDistance <= 6.0f) //8-12
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY + 17.0f, 0, NULL);
				else if (bestRealDistance > 6.0f && bestRealDistance <= 8.0f) //12-16
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY + 11.0f, 0, NULL);
				else if (bestRealDistance > 8.0f && bestRealDistance <= 10.0f) //16-20
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY + 5.0f, 0, NULL);
				else if (bestRealDistance > 10.0f)
					mouse_event(MOUSEEVENTF_MOVE, (float)DistX, (float)DistY - (bestRealDistance*dstmultiplier), 0, NULL); //0.3up, 0.1down
					//mouse_event(MOUSEEVENTF_MOVE, (int)DistX, (int)DistY, 0, NULL); //would go down is target is far 
			}

			//autoshoot on
			if ( (autoshoot == 1)||(autoshoot == 2 && (GetAsyncKeyState(Daimkey) & 0x8000)) )
			{
				if (!IsPressed)
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					IsPressed = true;
				}
			}
		}
	}
	AimInfo.clear();

	if (autoshoot > 0 && IsPressed)
	{
		if (timeGetTime() - gametick > asdelay) //
		{
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			IsPressed = false;
			gametick = timeGetTime();
		}
		//gametick = timeGetTime();
	}
	/*
	//logger
	if ((GetAsyncKeyState(VK_MENU)) && (GetAsyncKeyState(VK_CONTROL)) && (GetAsyncKeyState(0x4C) & 1)) //ALT + CTRL + L toggles logger
		logger = !logger;
	if (logger) //&& countnum >= 0)
	{
		char szString[255];
		sprintf_s(szString, "countnum = %d", countnum);
		DrawString(pFont, 220, 100, White, (char*)&szString[0]);
		DrawString(pFont, 220, 110, Yellow, "hold P to +");
		DrawString(pFont, 220, 120, Yellow, "hold O to -");
		DrawString(pFont, 220, 130, Green, "press I to log");
	}
	*/
	return EndScene_orig(pDevice);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9* pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	//mStage = Sampler;
	
	pCurrentTexture = static_cast<IDirect3DTexture9*>(pTexture);

	if (Stride == 24 && Sampler == 0 && pCurrentTexture)
	{
		if (reinterpret_cast<IDirect3DTexture9 *>(pCurrentTexture)->GetType() == D3DRTYPE_TEXTURE)
		{
			pCurrentTexture->GetLevelDesc(0, &desc);
			if (desc.Pool == D3DPOOL_DEFAULT) 
			//if (desc.Pool == D3DPOOL_MANAGED) 
			{
				dWidth = desc.Width;
				dHeight = desc.Height;

				//pCurrentTexture->LockRect(0, &pLockedRect, NULL, D3DLOCK_NOOVERWRITE | D3DLOCK_READONLY); //no
				//pCurrentTexture->LockRect(0, &pLockedRect, NULL, 0); //low fps
				//pCurrentTexture->LockRect(0, &pLockedRect, NULL, D3DLOCK_NO_DIRTY_UPDATE); //low fps

				//if (pLockedRect.pBits != NULL)
				//get crc
				//texCRC = QuickChecksum((DWORD*)pLockedRect.pBits, 1024);

				//pCurrentTexture->UnlockRect(0);
			}
		}
	}
	

	return SetTexture_orig(pDevice, Sampler, pTexture);
}

//==========================================================================================================================

HRESULT APIENTRY SetViewport_hook(IDirect3DDevice9* pDevice, CONST D3DVIEWPORT9* pViewport)
{
	//get viewport/screensize
	Viewport = *pViewport;
	ScreenCenterX = (float)Viewport.Width / 2.0f;
	ScreenCenterY = (float)Viewport.Height / 2.0f;

	return SetViewport_orig(pDevice, pViewport);
}

//==========================================================================================================================

HRESULT APIENTRY Reset_hook(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	DeleteRenderSurfaces();

	if (pFont)
		pFont->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	HRESULT cooperativeStat = pDevice->TestCooperativeLevel();

	switch (cooperativeStat)
	{
	case D3DERR_DEVICELOST:
		//Log("D3DERR_DEVICELOST");
		break;
	case D3DERR_DEVICENOTRESET:
		//Log("D3DERR_DEVICENOTRESET");
		break;
	case D3DERR_DRIVERINTERNALERROR:
		//Log("D3DERR_DRIVERINTERNALERROR");
		break;
	case D3D_OK:
		break;
	}

	if (SUCCEEDED(ResetReturn))
	{
		if (pFont)
			pFont->OnResetDevice();

		//pDevice->GetViewport(&Viewport);
		//ScreenCenterX = Viewport.Width / 2.0f;
		//ScreenCenterY = Viewport.Height / 2.0f;
	}

	return ResetReturn;
}

//==========================================================================================================================

DWORD WINAPI DirectXInit(__in  LPVOID lpParameter)
{
	while (GetModuleHandle("d3d9.dll") == 0)
	{
		Sleep(100);
	}

	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3ddev = NULL;

	HWND tmpWnd = CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, dllHandle, NULL);
	if(tmpWnd == NULL)
	{
		//Log("[DirectX] Failed to create temp window");
		return 0;
	}

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(d3d == NULL)
	{
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D interface");
		return 0;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp)); 
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = tmpWnd;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
	if(result != D3D_OK)
	{
		d3d->Release();
		DestroyWindow(tmpWnd);
		//Log("[DirectX] Failed to create temp Direct3D device");
		return 0;
	}

	// We have the device, so walk the vtable to get the address of all the dx functions in d3d9.dll
	#if defined _M_X64
	DWORD64* dVtable = (DWORD64*)d3ddev;
	dVtable = (DWORD64*)dVtable[0];
	#elif defined _M_IX86
	DWORD* dVtable = (DWORD*)d3ddev;
	dVtable = (DWORD*)dVtable[0]; // == *d3ddev
	#endif
	//Log("[DirectX] dvtable: %x", dVtable);

	//for(int i = 0; i < 95; i++)
	//{
		//Log("[DirectX] vtable[%i]: %x, pointer at %x", i, dVtable[i], &dVtable[i]);
	//}
	
	// Set EndScene_orig to the original EndScene etc.
	EndScene_orig = (EndScene)dVtable[42];
	SetVertexShaderConstantF_orig = (SetVertexShaderConstantF)dVtable[94];
	DrawIndexedPrimitive_orig = (DrawIndexedPrimitive)dVtable[82];
	DrawPrimitive_orig = (DrawPrimitive)dVtable[81];
	Reset_orig = (Reset)dVtable[16];
	SetStreamSource_orig = (SetStreamSource)dVtable[100];
	SetVertexShader_orig = (SetVertexShader)dVtable[92];
	SetPixelShader_orig = (SetPixelShader)dVtable[107];
	SetTexture_orig = (SetTexture)dVtable[65];
	SetViewport_orig = (SetViewport)dVtable[47];

	// Detour functions x86 & x64
	if (MH_Initialize() != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[42], &EndScene_hook, reinterpret_cast<void**>(&EndScene_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[42]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[94], &SetVertexShaderConstantF_hook, reinterpret_cast<void**>(&SetVertexShaderConstantF_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[94]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[82], &DrawIndexedPrimitive_hook, reinterpret_cast<void**>(&DrawIndexedPrimitive_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[82]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[81], &DrawPrimitive_hook, reinterpret_cast<void**>(&DrawPrimitive_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[81]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[16], &Reset_hook, reinterpret_cast<void**>(&Reset_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[16]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[100], &SetStreamSource_hook, reinterpret_cast<void**>(&SetStreamSource_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[100]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[92], &SetVertexShader_hook, reinterpret_cast<void**>(&SetVertexShader_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[92]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[107], &SetPixelShader_hook, reinterpret_cast<void**>(&SetPixelShader_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[107]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[65], &SetTexture_hook, reinterpret_cast<void**>(&SetTexture_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[65]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[47], &SetViewport_hook, reinterpret_cast<void**>(&SetViewport_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[47]) != MH_OK) { return 1; }

	//Log("[Detours] EndScene detour attached\n");

	d3ddev->Release();
	d3d->Release();
	DestroyWindow(tmpWnd);
		
	return 1;
}

//==========================================================================================================================

BOOL WINAPI DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH: // A process is loading the DLL.
		dllHandle = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL); // disable unwanted thread notifications to reduce overhead
		GetModuleFileNameA(hinstDLL, dlldir, 512);
		for (int i = (int)strlen(dlldir); i > 0; i--)
		{
			if (dlldir[i] == '\\')
			{
				dlldir[i + 1] = 0;
				break;
			}
		}
		CreateThread(0, 0, DirectXInit, 0, 0, 0); //init our hooks
		break;

	case DLL_PROCESS_DETACH: // A process unloads the DLL.
		/*
		if (MH_Uninitialize() != MH_OK) { return 1; }
		if (MH_DisableHook((DWORD_PTR*)dVtable[42]) != MH_OK) { return 1; }
		*/
		break;
	}
	return TRUE;
}
