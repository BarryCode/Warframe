/*
* Warframe D3D Hack Source V1.2 by Nseven

How to compile:
- open wfdxhook.vcxproj (not wfdxhook.vcxproj.filters) with Visual Studio Community 2017 (..\Microsoft Visual Studio\2017\Community\Common7\IDE\devenv.exe)
- select x86(32bit) or x64(64bit)
- to compile dll, press f5 or click the green triangle

Optional: remove dependecy on vs runtime:
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

typedef HRESULT(APIENTRY *SetIndices)(IDirect3DDevice9*,IDirect3DIndexBuffer9 *pIndexData);
HRESULT APIENTRY SetIndices_hook(IDirect3DDevice9*, IDirect3DIndexBuffer9 *pIndexData);
SetIndices SetIndices_orig = 0;

typedef HRESULT(APIENTRY *SetVertexDeclaration)(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
HRESULT APIENTRY SetVertexDeclaration_hook(IDirect3DDevice9*, IDirect3DVertexDeclaration9*);
SetVertexDeclaration SetVertexDeclaration_orig = 0;

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

		//if (pStreamData)// && Stride == 32)
		//{
			//pStreamData->GetDesc(&vdesc);
		//}
	}

	return SetStreamSource_orig(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

//==========================================================================================================================

HRESULT APIENTRY SetIndices_hook(IDirect3DDevice9* pDevice, IDirect3DIndexBuffer9 *pIndexData)
{
	//if (pIndexData != NULL)
	//{
		//pIndexData->GetDesc(&iDesc);
	//}

	return SetIndices_orig(pDevice, pIndexData);
}

//==========================================================================================================================

HRESULT APIENTRY SetVertexDeclaration_hook(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pDecl)
{
	if (pDecl != NULL)
	{
		pDecl->GetDeclaration(decl, &numElements);
	}

	return SetVertexDeclaration_orig(pDevice, pDecl);
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
	//unfinished

	if(wallhack == 1 && mStartRegister == 52 && mVector4fCount >= 82 && pSize < 5000 && vSize < 2392)
	{
		pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
		pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	}

	//esp
	if (mStartRegister == 52 && mVector4fCount >= 82 && pSize < 5000 && vSize < 2392)
	{
		AddEsp(pDevice, 1, "Model", White, 1.5f);
	}
	
	
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
		if (countnum == NumVertices|| countnum == pSize/100)
			if (pSize > 44 && GetAsyncKeyState('I') & 1) //press I to log to log.txt
				Log("Stride == %d && NumVertices == %d && primCount == %d && vSize == %d && pSize == %d && decl->Type == %d && numElements == %d && mStartRegister == %d && mVector4fCount == %d && iDesc.Size == %d", Stride, NumVertices, primCount, vSize, pSize, decl->Type, numElements, mStartRegister, mVector4fCount, iDesc.Size);
		//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (countnum == NumVertices || countnum == pSize / 100)
		{
			pDevice->SetTexture(0, NULL);
			return D3D_OK; //delete texture
			//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}
	*/
	return DrawIndexedPrimitive_orig(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

//==========================================================================================================================

HRESULT APIENTRY DrawPrimitive_hook(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 256 && sHeight == 32) //lvl symbol
		//pDevice->SetPixelShader(shadRed);

	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 32 && sHeight == 32) //hp bar
		//pDevice->SetPixelShader(shadGreen);

	//best in pve
	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 256 && sHeight == 32) //lvl symbol
	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1)// && sWidth == 256 && sHeight == 32) //lvl symbol
	if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 32 && sHeight == 32)
	{
		//pDevice->SetPixelShader(shadRed);
		if (GetAsyncKeyState(VK_F10) & 1) //press I to log to log.txt
		Log("Stride == %d && PrimitiveCount == %d && vSize == %d && pSize == %d && decl->Type == %d && numElements == %d && mStartRegister == %d && mVector4fCount == %d && sWidth == %d && sHeight == %d && sFormat == %d && StartVertex == %d && iDesc.Size == %d", Stride, PrimitiveCount, vSize, pSize, decl->Type, numElements, mStartRegister, mVector4fCount, sWidth, sHeight, sFormat, StartVertex, iDesc.Size);

		//worldtoscreen
		AddAim(pDevice, 1);
	}

	//best for pvp (two entities)
	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 628 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 32 && sHeight == 32) //===== long hp bar
	//if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 32 && sHeight == 32)
	if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 256 && sHeight == 32) //lvl sym
	{
		//pDevice->SetPixelShader(shadGreen);
		//if (GetAsyncKeyState(VK_F10) & 1) //press I to log to log.txt
		//Log("Stride == %d && PrimitiveCount == %d && vSize == %d && pSize == %d && decl->Type == %d && numElements == %d && mStartRegister == %d && mVector4fCount == %d && sWidth == %d && sHeight == %d && sFormat == %d && StartVertex == %d && iDesc.Size == %d", Stride, PrimitiveCount, vSize, pSize, decl->Type, numElements, mStartRegister, mVector4fCount, sWidth, sHeight, sFormat, StartVertex, iDesc.Size);

		//worldtoscreen
		AddAim2(pDevice, 1);
	}

	
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
		if (countnum == sWidth)
			if (PrimitiveCount == 2 && GetAsyncKeyState('I') & 1) //press I to log to log.txt
				Log("Stride == %d && PrimitiveCount == %d && vSize == %d && pSize == %d && decl->Type == %d && numElements == %d && mStartRegister == %d && mVector4fCount == %d && sWidth == %d && sHeight == %d && iDesc.Size == %d", Stride, PrimitiveCount, vSize, pSize, decl->Type, numElements, mStartRegister, mVector4fCount, sWidth, sHeight, iDesc.Size);
		//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		if (Stride == 24 && PrimitiveCount == 2 && vSize == 352 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && countnum == sWidth)
		{
			//pDevice->SetPixelShader(shadGreen);
			//pDevice->SetTexture(0, NULL);
			//return D3D_OK; //delete texture
			//pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
	}
	
	return DrawPrimitive_orig(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

//==========================================================================================================================

HRESULT APIENTRY EndScene_hook(IDirect3DDevice9* pDevice)
{
	if (FirstInit == FALSE)
	{
		FirstInit = TRUE;

		//generate shader
		GenerateShader(pDevice, &shadRed, 1.0f, 0.0f, 0.0f, false);
		GenerateShader(pDevice, &shadBlue, 0.0f, 0.0f, 1.0f, false);
		GenerateShader(pDevice, &shadDepthBlue, 0.0f, 0.0f, 1.0f, true);
		GenerateShader(pDevice, &shadGreen, 0.0f, 1.0f, 0.0f, false);
		GenerateShader(pDevice, &shadYellow, 1.0f, 1.0f, 0.0f, false);

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
			//if (x < Viewport.Width && y < Viewport.Height)
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
				//PrePresent(pDevice, (int)EspInfo[i].vOutX - 32, (int)EspInfo[i].vOutY - 20);

				//draw real distance
				DrawString(pFont, (int)EspInfo[i].vOutX - 9, (int)EspInfo[i].vOutY, Green, "%.f", EspInfo[i].RealDistance*2.0f);
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


	//aimbot part 1
	//check hp bar
	if (aimbot > 0 && AimInfo.size() != NULL) 
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;
		hpbarX = 0.0f;

		for (unsigned int i = 0; i < AimInfo.size(); i++)
		{
			//DrawString(pFont, (int)AimInfo[i].vOutX, (int)AimInfo[i].vOutY, Green, "1");

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
		if ((aimcheckespfov == 0 && BestTarget != -1) || (aimcheckespfov == 1 && inespfov && BestTarget != -1))
		{
			hpbarX = AimInfo[BestTarget].vOutX;
			hpbarY = AimInfo[BestTarget].vOutY;

			DrawString(pFont, 200, 200, White, "%.2f", hpbarX);
			DrawString(pFont, 200, 220, White, "%.2f", hpbarY);

			//if mouse is left from target
			if (AimInfo[BestTarget].vOutX > (ScreenCenterX)) {
				hpbarX = (AimInfo[BestTarget].vOutX - 56.0f);
			}

			//if mouse is right from target
			else if (AimInfo[BestTarget].vOutX < (ScreenCenterX)) {
				hpbarX = (AimInfo[BestTarget].vOutX + 56.0f);
			}

			DrawString(pFont, hpbarX, AimInfo[BestTarget].vOutY, Green, "lvl");


			double DistX = hpbarX - ScreenCenterX;
			double DistY = AimInfo[BestTarget].vOutY - ScreenCenterY;

			DistX /= aimsens;
			DistY /= aimsens;

			//if lvl symb in range of centered hp bar
			//if (hpbarX >= lvlsymX - countnum && hpbarX <= lvlsymX + countnum && hpbarY >= lvlsymY - countnum && hpbarY <= lvlsymY + countnum)
			//if (GetAsyncKeyState(Daimkey) & 0x8000)
			//mouse_event(MOUSEEVENTF_MOVE, (int)DistX, (int)DistY, 0, NULL);  

			//autoshoot on
			if ((autoshoot == 1) || (autoshoot == 2 && (GetAsyncKeyState(Daimkey) & 0x8000)))
			{
				if (!IsPressed)
				{
					mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
					IsPressed = true;
				}
			}
		}
		else
			hpbarX = 0.0f;
		//hpbaronscreen = false;
	}
	AimInfo.clear();



	//aimbot2
	//check lvl symb
	if (aimbot > 0 && AimInfo2.size() != NULL)
	{
		UINT BestTarget = -1;
		DOUBLE fClosestPos = 99999;

		for (unsigned int i = 0; i < AimInfo2.size(); i++)
		{
			//DrawString(pFont, (int)AimInfo2[i].vOutX, (int)AimInfo2[i].vOutY, Green, "1");

			//aimfov
			float radiusx = aimfov * (ScreenCenterX / 100.0f);
			float radiusy = aimfov * (ScreenCenterY / 100.0f);

			//get crosshairdistance
			AimInfo2[i].CrosshairDistance = GetDistance(AimInfo2[i].vOutX, AimInfo2[i].vOutY, ScreenCenterX, ScreenCenterY);

			//aim at team 1 or 2
			//if (aimbot == AimInfo[i].iTeam)

			//if in fov
			if (AimInfo2[i].vOutX >= ScreenCenterX - radiusx && AimInfo2[i].vOutX <= ScreenCenterX + radiusx && AimInfo2[i].vOutY >= ScreenCenterY - radiusy && AimInfo2[i].vOutY <= ScreenCenterY + radiusy)

				//get closest/nearest target to crosshair
				if (AimInfo2[i].CrosshairDistance < fClosestPos)
				{
					fClosestPos = AimInfo2[i].CrosshairDistance;
					BestTarget = i;
				}
		}


		//if nearest target to crosshair
		if (BestTarget != -1)
		{
			DrawString(pFont, AimInfo2[BestTarget].vOutX, AimInfo2[BestTarget].vOutY, Green, "lvl");

			lvlsymX = AimInfo2[BestTarget].vOutX;
			lvlsymY = AimInfo2[BestTarget].vOutY;

			//DrawString(pFont, 200, 200, White, "%.2f", lvlsymX);
			//DrawString(pFont, 200, 220, White, "%.2f", lvlsymY);

			double DistX = AimInfo2[BestTarget].vOutX - ScreenCenterX;
			double DistY = AimInfo2[BestTarget].vOutY - ScreenCenterY;

			DistX /= aimsens;
			DistY /= aimsens;

			//if lvl symb in range of centered hp bar
			//if (lvlsymX >= hpbarX - countnum && lvlsymX <= hpbarX + countnum && lvlsymY >= hpbarY - countnum && lvlsymY <= hpbarY + countnum)//63
			//if (hpbarX >= lvlsymX - countnum && hpbarX <= lvlsymX + countnum && hpbarY >= lvlsymY - countnum && hpbarY <= lvlsymY + countnum)
			//if hpbar out of range, do nothing
			DrawString(pFont, 200, 240, Red, "%.2f", hpbarX);
			DrawString(pFont, 200, 260, Red, "%.2f", hpbarY);
			if(hpbarX > 0 && hpbarY > 0)
			if (GetAsyncKeyState(Daimkey) & 0x8000)
				mouse_event(MOUSEEVENTF_MOVE, (int)DistX, (int)DistY, 0, NULL); //would go down is target is far 

		}
	}
	AimInfo2.clear();


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
	
	return EndScene_orig(pDevice);
}

//==========================================================================================================================

HRESULT APIENTRY SetTexture_hook(IDirect3DDevice9* pDevice, DWORD Sampler, IDirect3DBaseTexture9 *pTexture)
{
	//Stride == 24 && vSize == 352 && pSize == 1084 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && sWidth == 32 && sHeight == 32
	//if (Stride == 24 && Sampler == 0 && pTexture)
	if(Stride == 24 && vSize == 352 && pSize <= 628 && decl->Type == 2 && numElements == 4 && mStartRegister == 11 && mVector4fCount == 1 && pTexture) //why is pS req
	{//1
		dCurrentTex = static_cast<IDirect3DTexture9*>(pTexture);

		//if (pCurrentTex)//
		//{
		D3DSURFACE_DESC surfaceDesc;

		if (FAILED(dCurrentTex->GetLevelDesc(0, &surfaceDesc)))
		{
			//Log("surfaceDesc failed");
			goto out;
		}

		if (SUCCEEDED(dCurrentTex->GetLevelDesc(0, &surfaceDesc)))
			if (surfaceDesc.Pool == D3DPOOL_DEFAULT && dCurrentTex->GetType() == D3DRTYPE_TEXTURE) //reduce fps loss
			{
				//Textures created with D3DPOOL_DEFAULT are not lockable
				//In this case, use GetRenderTargetData to copy texture data from device memory to system memory

				sWidth = surfaceDesc.Width;
				sHeight = surfaceDesc.Height;
				sFormat = surfaceDesc.Format;

			}
	}
out:

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
	if (pFont)
		pFont->OnLostDevice();

	HRESULT ResetReturn = Reset_orig(pDevice, pPresentationParameters);

	if (SUCCEEDED(ResetReturn))
	{
		if (pFont)
			pFont->OnResetDevice();
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
	//SetIndices_orig = (SetIndices)dVtable[104];
	SetVertexDeclaration_orig = (SetVertexDeclaration)dVtable[87];
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
	//if (MH_CreateHook((DWORD_PTR*)dVtable[104], &SetIndices_hook, reinterpret_cast<void**>(&SetIndices_orig)) != MH_OK) { return 1; }
	//if (MH_EnableHook((DWORD_PTR*)dVtable[104]) != MH_OK) { return 1; }
	if (MH_CreateHook((DWORD_PTR*)dVtable[87], &SetVertexDeclaration_hook, reinterpret_cast<void**>(&SetVertexDeclaration_orig)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)dVtable[87]) != MH_OK) { return 1; }
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

/*
vTable Numbers

QueryInterface // 0
AddRef // 1
Release // 2
TestCooperativeLevel // 3
GetAvailableTextureMem // 4
EvictManagedResources // 5
GetDirect3D // 6
GetDeviceCaps // 7
GetDisplayMode // 8
GetCreationParameters // 9
SetCursorProperties // 10
SetCursorPosition // 11
ShowCursor // 12
CreateAdditionalSwapChain // 13
GetSwapChain // 14
GetNumberOfSwapChains // 15
Reset // 16
Present // 17
GetBackBuffer // 18
GetRasterStatus // 19
SetDialogBoxMode // 20
SetGammaRamp // 21
GetGammaRamp // 22
CreateTexture // 23
CreateVolumeTexture // 24
CreateCubeTexture // 25
CreateVertexBuffer // 26
CreateIndexBuffer // 27
CreateRenderTarget // 28
CreateDepthStencilSurface // 29
UpdateSurface // 30
UpdateTexture // 31
GetRenderTargetData // 32
GetFrontBufferData // 33
StretchRect // 34
ColorFill // 35
CreateOffscreenPlainSurface // 36
SetRenderTarget // 37
GetRenderTarget // 38
SetDepthStencilSurface // 39
GetDepthStencilSurface // 40
BeginScene // 41
EndScene // 42
Clear // 43
SetTransform // 44
GetTransform // 45
MultiplyTransform // 46
SetViewport // 47
GetViewport // 48
SetMaterial // 49
GetMaterial // 50
SetLight // 51
GetLight // 52
LightEnable // 53
GetLightEnable // 54
SetClipPlane // 55
GetClipPlane // 56
SetRenderState // 57
GetRenderState // 58
CreateStateBlock // 59
BeginStateBlock // 60
EndStateBlock // 61
SetClipStatus // 62
GetClipStatus // 63
GetTexture // 64
SetTexture // 65
GetTextureStageState // 66
SetTextureStageState // 67
GetSamplerState // 68
SetSamplerState // 69
ValidateDevice // 70
SetPaletteEntries // 71
GetPaletteEntries // 72
SetCurrentTexturePalette // 73
GetCurrentTexturePalette // 74
SetScissorRect // 75
GetScissorRect // 76
SetSoftwareVertexProcessing // 77
GetSoftwareVertexProcessing // 78
SetNPatchMode // 79
GetNPatchMode // 80
DrawPrimitive // 81
DrawIndexedPrimitive // 82
DrawPrimitiveUP // 83
DrawIndexedPrimitiveUP // 84
ProcessVertices // 85
CreateVertexDeclaration // 86
SetVertexDeclaration // 87
GetVertexDeclaration // 88
SetFVF // 89
GetFVF // 90
CreateVertexShader // 91
SetVertexShader // 92
GetVertexShader // 93
SetVertexShaderConstantF // 94
GetVertexShaderConstantF // 95
SetVertexShaderConstantI // 96
GetVertexShaderConstantI // 97
SetVertexShaderConstantB // 98
GetVertexShaderConstantB // 99
SetStreamSource // 100
GetStreamSource // 101
SetStreamSourceFreq // 102
GetStreamSourceFreq // 103
SetIndices // 104
GetIndices // 105
CreatePixelShader // 106
SetPixelShader // 107
GetPixelShader // 108
SetPixelShaderConstantF // 109
GetPixelShaderConstantF // 110
SetPixelShaderConstantI // 111
GetPixelShaderConstantI // 112
SetPixelShaderConstantB // 113
GetPixelShaderConstantB // 114
DrawRectPatch // 115
DrawTriPatch // 116
DeletePatch // 117
CreateQuery // 118
*/