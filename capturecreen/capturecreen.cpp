// capturecreen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DXGICaptor.h"
#include <iostream>  
#include <windows.h>  
using namespace std;
int main()
{
	INT imglen = 8000000;
	VideoDXGICaptor vdc;
	vdc.Init();
	double alltime=0;
	for (int i = 0; i < 120; i++) {
		double start = GetTickCount();
		vdc.CaptureImage(NULL, imglen);
		double end = GetTickCount();
		alltime += end - start;
		vdc.ResetDevice();
	//	if(i==99)
	//	cout << "GetTickCount:" << i<< endl;
	}
	cout << "GetTickCount:" << alltime/100 << endl;
	vdc.Deinit();
	return 0;
}
