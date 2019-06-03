#include <iostream>
#include "GLWindowManager.h"

using namespace std;


int main()
{
	GLWindowManager wm = GLWindowManager();
	
	
	//wm.LoadModel("golfball/golfball.obj", false);
	wm.LoadModel("stones/stones.obj", false);
	//wm.LoadBumpmap("golfball/golfball.png");
	//wm.LoadTexture("golfball/golfball.png");
	
	wm.InitializeSceneInfo();
	wm.StartRenderLoop();
}