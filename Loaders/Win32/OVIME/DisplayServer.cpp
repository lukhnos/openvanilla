#define OV_DEBUG 
#include "DisplayServer.h"
#include "OVIME.h"

AVDisplayServer *DisplayServer::lockIMC(HIMC h)
{
	hIMC = h;
	lpIMC = ImmLockIMC(hIMC);
	lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
	lpMyPrivate = (LPMYPRIVATE)ImmLockIMCC(lpIMC->hPrivate);
	return this;
}
AVDisplayServer *DisplayServer::releaseIMC()
{
	ImmUnlockIMCC(lpIMC->hPrivate);
	ImmUnlockIMCC(lpIMC->hCompStr);
	ImmUnlockIMC(hIMC);
	return this;
}
AVDisplayServer *DisplayServer::setBufString(const char *str)
{
	wchar_t wstr[1024];
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wstr, 1024);
	wcscpy(lpMyPrivate->PreEditStr, wstr);
	MakeCompStr(lpMyPrivate, lpCompStr);
	murmur("\tAVDisplayServer *DisplayServer::setBufString(%s)",str);
	UISetCompStr(lpMyPrivate->PreEditStr); 	//�n���n���ˬd���S��PreEditStr���S���F��H

	return this;
}
AVDisplayServer *DisplayServer::sendBuf(const char *str)
{
	wchar_t wstr[1024];
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wstr, 1024);
	wcscpy(GETLPRESULTSTR(lpCompStr), wstr);
	lpCompStr->dwResultStrLen = (int)wcslen(wstr);
	wcscpy(lpMyPrivate->PreEditStr, L"");
	
	MakeCompStr(lpMyPrivate, lpCompStr);
	UIClearCompStr();//�Y��update C# comp string �P�B���

	MyGenerateMessage(hIMC,
		WM_IME_COMPOSITION, 0, GCS_RESULTSTR);
	MyGenerateMessage(hIMC,
		WM_IME_ENDCOMPOSITION, 0, 0);	
	return this;
}
AVDisplayServer *DisplayServer::setCandiString(const char *str)
{
	wchar_t wstr[1024];
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wstr, 1024);
	wcscpy(lpMyPrivate->CandStr, wstr);
	UpdateCandidate(lpIMC, wstr);
//	MyGenerateMessage(hIMC,
//			WM_IME_COMPOSITION, 0, GCS_COMPSTR);
	UISetCandStr(lpMyPrivate->CandStr);
	return this;
}
AVDisplayServer *DisplayServer::showNotify(const char *str)
{
	murmur("\tAVDisplayServer *DisplayServer::showNotify");
	wchar_t wstr[1024];
	MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wstr, 1024);
	UIShowNotifyWindow(wstr);
	return this;
}
AVDisplayServer *DisplayServer::hideNotify()
{
	UIHideNotifyWindow();	
	return this;
}


DisplayServer *DisplayServer::moveBuf(int x, int y)
{
	murmur("\tAVDisplayServer *DisplayServer::moveBuf(%d,%d)",x,y);
	UIMoveCompWindow(x,y);
	return this;
}

DisplayServer *DisplayServer::moveCandi(int x, int y)
{
	murmur("\tAVDisplayServer *DisplayServer::moveCandi(%d,%d)",x,y);
	UIMoveCandWindow(x,y);
	return this;
}

DisplayServer *DisplayServer::SetCompStarted(bool t)
{
	if(t)
		murmur("\t*DisplayServer::SetCompStarted=true");
	else
		murmur("\t*DisplayServer::SetCompStarted=false");
	
	compStarted=t;
	return this;
}
DisplayServer *DisplayServer::showStatus(bool t)
{	
	if(t)
	{
		murmur("\tAVDisplayServer *DisplayServer::showStatus");
		UIShowStatusWindow();
	}
	else
	{
		murmur("\tAVDisplayServer *DisplayServer::hideStatus");
		UIHideStatusWindow();
	}
	return this;
}


AVDisplayServer *DisplayServer::showBuf(bool t)
{		
	if(compStarted)
	{
		if(t && lpMyPrivate->PreEditStr && wcslen(lpMyPrivate->PreEditStr))	
		{
			murmur("\tAVDisplayServer *DisplayServer::showBuf");		
			UIShowCompWindow();
		}
		else
		{
			murmur("\tAVDisplayServer *DisplayServer::hideBuf");
			UIHideCompWindow();
		}
	}
	return this;
}

AVDisplayServer *DisplayServer::showCandi(bool t)
{	
	//james test
	if(t &&  lpMyPrivate->CandStr && wcslen(lpMyPrivate->CandStr))	
	{
		murmur("\tAVDisplayServer *DisplayServer::showCandi");
		UIShowCandWindow();
	}
	else
	{
		murmur("\tAVDisplayServer *DisplayServer::hideCandi");
		UIHideCandWindow();
	}
	return this;
}
AVDisplayServer *DisplayServer::setCursorPos(int i)
{
	lpCompStr->dwCursorPos = i;
	murmur("\tDisplayServer::setCursorPos-> %d",i);
	UISetCursorPos(lpCompStr->dwCursorPos);
	return this;
}
AVDisplayServer *DisplayServer::setMarkFrom(int i)
{
	UISetMarkFrom(i);
	return this;
}
AVDisplayServer *DisplayServer::setMarkTo(int i)
{
	UISetMarkTo(i);
	return this;
}