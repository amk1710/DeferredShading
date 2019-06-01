#include <iostream>
#include "GLWindowManager.h"

using namespace std;


int main()
{
	GLWindowManager wm = GLWindowManager();
	
	
	//wm.LoadModel("golfball/golfball.obj", false);
	//wm.LoadBumpmap("golfball/golfball.png");
	//wm.LoadTexture("golfball/golfball.png");

	wm.LoadModel2("stones/stones.obj", false);
	//wm.LoadModel2("golfball/golfball.obj", false);
	//wm.LoadBumpmap("stones/stones_norm.jpg");
	//wm.LoadTexture("stones/stones.jpg");
		
	wm.InitializeSceneInfo();
	wm.StartRenderLoop();
}