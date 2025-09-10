#define HW_RVL

//#define DEBUG_MANUALMOVESPHERES
//#define DEBUG_MANUALSPAWNSPHERES
//#define DEBUG_MANUALDELETESPERE
//#define DEBUG_TESTLOOP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/lwp_watchdog.h>
#include <asndlib.h>
#include <mp3player.h>

#include "input.hpp"
#include "spherePool.hpp"
#include "projectilePool.hpp"
#include "camera.hpp"
#include "vectorUtils.hpp"
#include "cube.hpp"
#include "sd.hpp"

#include "nlohmann/json.hpp"

#include "Letter_Gothic_Std_14_Bold_png.h"

using std::string;
using std::to_string;
using std::vector;
using nlohmann::json;

typedef std::unordered_map<int, vector<Cube>> TowerMap;

enum class GameState {
	MAINMENU = 0,
	GAME,
	SHOP,
	TEST
};

//these represent the power states used by Sys_ResetSystem(), except i think 0 idk what that normally is
enum class PowerState {
	ON = -1,
	TOLOADER = 0,
	OFF = 4
};

static GameState gameState = GameState::MAINMENU;
static PowerState powerState = PowerState::ON;

static GRRLIB_texImg* texFont;
static GRRLIB_texImg* texPrevFrame;
static GRRLIB_texImg* texPlatform;

static json saveGame;

//these are to turn off the console :3
static void ResetCallback(u32 irq, void* ctx) {
	powerState = PowerState::TOLOADER;
}
static void PowerCallback() {
	powerState = PowerState::OFF;
}
static void WiimotePowerCallback(s32 chan) {
	powerState = PowerState::OFF;
}

static int logOffset = 1;
static void Printf(std::string inputString) {
	const int allowedLetters = 50;
	while (inputString.length() > 0) {
		string outString = inputString;
		if (inputString.length() > allowedLetters) {
			outString = inputString.substr(0, allowedLetters);
			inputString = inputString.substr(allowedLetters);
		}
		else {
			inputString = "";
		}
		GRRLIB_Printf((640 - (11 * outString.length())) / 2, 25 * logOffset, texFont, RGBA(0xe2, 0xe2, 0xe2, 0xff), 1, outString.c_str());
		logOffset++;
	}
}

//things that need to be ran first time before anything else really happens
static void Init() {
	//init video system through grrlib
	GRRLIB_Init();
	GRRLIB_SetAntiAliasing(false);
	GRRLIB_SetBackgroundColour(0x11, 0x1d, 0x4a, 255);

	texFont = GRRLIB_LoadTexture(Letter_Gothic_Std_14_Bold_png);
	GRRLIB_InitTileSet(texFont, 11, 24, 32);

	//loading screen once grrlib is init
	//GX_AbortFrame();
	GRRLIB_2dMode();
	GRRLIB_Render();
	GRRLIB_Render();
	Printf("Loading...");
	GRRLIB_Render();
	

	texPrevFrame = GRRLIB_CreateEmptyTexture(rmode->fbWidth, rmode->efbHeight);
	texPlatform = GRRLIB_LoadTextureFromFile("sd:/apps/3dwii/maps/textures/testmap1.png");
	
	//init gamecube controllers
	PAD_Init();
	//init wiimote connections
	WPAD_Init();
	WPAD_SetVRes(0, rmode->fbWidth, rmode->efbHeight);
	//set data format to recieve accelerometer data and IR data (i think?)
	WPAD_SetDataFormat(0, WPAD_FMT_BTNS_ACC_IR);
	//init audio stuff
	ASND_Init();
	MP3Player_Init();
	//so that the power/reset buttons work on the physical wii (power turns off, reset goes back to hb menu)
	SYS_SetResetCallback(ResetCallback);
	SYS_SetPowerCallback(PowerCallback);
	WPAD_SetPowerButtonCallback(WiimotePowerCallback);
	
	//This is kept here as a reminder of the pain and suffering that calling this function caused me
	//settime(0);

	//test if write lock is on
	if (WriteLockTest()) {
		while (powerState == PowerState::ON) {
			RefreshPads();
			logOffset = 2;
			Printf("Warning!");
			Printf("SD card write-lock is currently on!");
			Printf("(or you have virtually no sd space remaining...)");
			Printf("Due to buggy and inconsistent behaviour in testing");
			Printf("when the write-lock is on, this game cannot be run");
			Printf("with the write-lock on.");
			Printf("(my own sd card adapter is probably the issue)");
			Printf("");
			Printf("Game will return to loader on controller input...");

			if (GetButtonsDown(0)) break;

			GRRLIB_Render();
		}
		//quit to loader since i cant be bothered to deal with write lock nonsense
		if (powerState == PowerState::ON) powerState = PowerState::TOLOADER;
		//return early to not even load save file
		return;
	}

	//load save and display warning/close game if necessary
	LoadSaveFile(saveGame);

	if (!saveGame["errornewsave"].is_null()) {
		while (powerState == PowerState::ON) {
			RefreshPads();
			logOffset = 2;
			Printf("FATAL ERROR");
			Printf(to_string(saveGame["errornewsave"]));
			Printf("Game will return to loader on button input");

			if (GetButtonsDown(0)) break;

			GRRLIB_Render();
		}
		//quit to loader since game can't function without save
		if (powerState == PowerState::ON) powerState = PowerState::TOLOADER;
	}
	if (!saveGame["erroroldsave"].is_null()) {
		while (powerState == PowerState::ON) {
			RefreshPads();
			logOffset = 2;
			Printf("Warning!");
			Printf(to_string(saveGame["erroroldsave"]));
			Printf("This message is caused most likely");
			Printf("from a corrupt save file.");
			Printf("If so, corrupt save file has been backed up");
			Printf("incase you want to try recover it.");
			Printf("Press any button to continue...");

			if (GetButtonsDown(0)) break;

			GRRLIB_Render();
		}
	}
}

//void Exit(vector<Sphere*> sphereList, SpherePool spherePool)


//draw a (currently multicoloured) cube with normal
static void DrawColourfulCubeNormal() {

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4 * 6);

		//bottom
		GX_Position3f32(1.0f, -1.0f, 1.0f);
		GX_Normal3f32(0.0f, -1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(1.0f, -1.0f, -1.0f);
		GX_Normal3f32(0.0f, -1.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(-1.0f, -1.0f, -1.0f);
		GX_Normal3f32(0.0f, -1.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(-1.0f, -1.0f, 1.0f);
		GX_Normal3f32(0.0f, -1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		//top
		GX_Position3f32(1.0f, 1.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(1.0f, 1.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		//back
		GX_Position3f32(1.0f, -1.0f, -1.0f);
		GX_Normal3f32(0.0f, 0.0f, -1.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(-1.0f, -1.0f, -1.0f);
		GX_Normal3f32(0.0f, 0.0f, -1.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, -1.0f);
		GX_Normal3f32(0.0f, 0.0f, -1.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(1.0f, 1.0f, -1.0f);
		GX_Normal3f32(0.0f, 0.0f, -1.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		//front
		GX_Position3f32(1.0f, -1.0f, 1.0f);
		GX_Normal3f32(0.0f, 0.0f, 1.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(1.0f, 1.0f, 1.0f);
		GX_Normal3f32(0.0f, 0.0f, 1.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, 1.0f);
		GX_Normal3f32(0.0f, 0.0f, 1.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(-1.0f, -1.0f, 1.0f);
		GX_Normal3f32(0.0f, 0.0f, 1.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		//left
		GX_Position3f32(-1.0f, -1.0f, -1.0f);
		GX_Normal3f32(-1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, -1.0f);
		GX_Normal3f32(-1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(-1.0f, 1.0f, 1.0f);
		GX_Normal3f32(-1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(-1.0f, -1.0f, 1.0f);
		GX_Normal3f32(-1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		//right
		GX_Position3f32(1.0f, -1.0f, -1.0f);
		GX_Normal3f32(1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0x00, 0x00, 0xff));
		GX_Position3f32(1.0f, -1.0f, 1.0f);
		GX_Normal3f32(1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0xff, 0x00, 0xff));
		GX_Position3f32(1.0f, 1.0f, 1.0f);
		GX_Normal3f32(1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0x00, 0x00, 0xff, 0xff));
		GX_Position3f32(1.0f, 1.0f, -1.0f);
		GX_Normal3f32(1.0f, 0.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));

	GX_End();
}

static void RunGame() {


	////////////////////////////////
	//init variables needed for game
	////////////////////////////////
	
	

	//i.e. camera, sphere+projectile pool/list, the map being used
	Camera camera({ 0,0,0 }, 0.0, 0.0, 90.0);

	std::shared_ptr<SpherePool> spherePool = std::make_shared<SpherePool>();
	std::shared_ptr<ProjectilePool> projectilePool = std::make_shared<ProjectilePool>();
	
	//todo: make quadtree to optimise projectile collisions and cube detection
	vector<std::shared_ptr<Sphere>> sphereList;
	vector<std::shared_ptr<Projectile>> projectileList;


	//test map init
	std::ifstream mapFilePath("sd:/apps/3dwii/maps/data/testmap1.json");
	json mapData = json::parse(mapFilePath);
	if (mapFilePath.is_open()) {
		mapFilePath.close();
	}
	vector<guVector> mapPath;

	for (auto it = mapData["map"]["path"].begin(); it != mapData["map"]["path"].end(); ++it) {
		mapPath.push_back({ (*it)["x"], (*it)["y"], (*it)["z"] });
	}

	//coordinate for where each tower begins
	vector<guVector> mapTowerPoints;
	//selected tower
	int selectedTower = -1;
	//selected cube in tower (or 1 above to place new cube!)
	//valid range [0, towerMap[selectedTower].size()]
	int selectedCubeInTower = 0;

	for (auto it = mapData["map"]["towerpoint"].begin(); it != mapData["map"]["towerpoint"].end(); ++it) {
		mapTowerPoints.push_back({ (*it)["x"], (*it)["y"], (*it)["z"] });
	}

	//towerMap[i] represents tower at mapTowerPoints[i]'s co-ordinates
	TowerMap towerMap;
	//init towerMap so that all indexes filled with empty vector<Cube>
	for (size_t i = 0; i < mapTowerPoints.size(); i++) {
		towerMap[i];
	}

	//init list of spheres in each round
	std::ifstream waveFilePath("sd:/apps/3dwii/data/waves.json");
	json waveData = json::parse(waveFilePath);
	if (waveFilePath.is_open()) {
		waveFilePath.close();
	}

	struct SphereWaveData {
		u32 type;
		float timeIntoRound;
		bool boss;
	};

	vector<vector<SphereWaveData>> waveInfo;

	//iterates over waves
	for (auto it = waveData["waves"].begin(); it != waveData["waves"].end(); it++) {
		vector<SphereWaveData> tempWaveInfo;

		for (auto jt = (*it).begin(); jt != (*it).end(); jt++) {
			tempWaveInfo.push_back({ (*jt)["type"], (*jt)["time"], (*jt)["boss"] });
		}

		waveInfo.push_back(tempWaveInfo);
	}


	//wave variables
	u32 wave = 0;
	bool autoAdvanceWave = false;
	u32 sphereToBeSpawned = 0;
	bool noMoreSpheres = true;
	bool gameWon = false;
	bool waveMoneyGiven = true;

	//fps and general time variables
	u8 FPS = 60;
	double deltaTime = 1;
	u64 timeAtFrameStart = gettime();
	u64 timeAtWaveStart = timeAtFrameStart;
	//for when the game is paused and times need to be updated on unpause
	u64 timeDifference = 0;

	int frames = 0;

	//actual gameplay variables and not just setup
	int health = 1000;
	int money = 100000;
	bool paused = false;



	//////////////////
	//actual game loop
	//////////////////



	while (powerState == PowerState::ON) {
		RefreshPads();

		logOffset = 1;

		GRRLIB_2dMode();

		Printf(to_string(towerMap.size()));
		Printf(to_string(FPS) + " " + to_string(deltaTime));
		Printf(to_string(wave));

		//get controller inputs
		u32 buttonDown = GetButtonsDown(0);
		u32 buttonHeld = GetButtonsHeld(0);
		Stick buttonStickL = GetLeftStick(0);
		Stick buttonStickR = GetRightStick(0);
		float buttonTriggerL = GetLeftTrigger(0);
		float buttonTriggerR = GetRightTrigger(0);

		Printf(to_string(buttonStickL.x) + " " + to_string(buttonStickL.y));
		Printf(to_string(buttonStickR.x) + " " + to_string(buttonStickR.y));

		frames++;

		Printf(to_string(WPAD_ButtonsHeld(0)));
		Printf(to_string(buttonHeld));

#ifdef DEBUG_MANUALDELETESPERE
		//debug delete sphere
		if (buttonDown & BUTTON_DOWN) {
			if (sphereList.size() > 0) {
				spherePool->ReturnSphere(sphereList[0]);
				sphereList.erase(sphereList.begin());
			}
		}
#endif


		//////////////////////////////
		//handle spheres + projectiles
		//////////////////////////////


#ifdef DEBUG_MANUALMOVESPHERES
		if (buttonHeld & BUTTON_UP) {
			//bool endOfPath = sphereList[0]->UpdatePosition(mapPath, deltaTime);
			//Printf(texFont, to_string(endOfPath));

			for (size_t i = 0; i < sphereList.size(); i++) {
				sphereList[i]->UpdatePosition(mapPath, deltaTime);
				Printf(to_string(sphereList[i]->GetDistanceThroughPath()));
			}
		}
#else
		for (size_t i = 0; i < sphereList.size(); i++) {
			if (!sphereList[i]->UpdatePosition(mapPath, deltaTime)) {
				//sphere has reached end of track, remove health
				health -= sphereList[i]->GetHealth();
			}
		}
#endif

		//spawn spheres
#ifdef DEBUG_MANUALSPAWNSPHERES
		//spawn spheres (real would be from json data probably)
		if (buttonDown & BUTTON_RIGHT) {
			sphereList.push_back(spherePool->NewSphere(mapPath, 1));
		}
#else
		//if it's time for sphere to spawn

		//secs_to_ticks replaced
		if (!noMoreSpheres && ((u64)(waveInfo[wave][sphereToBeSpawned].timeIntoRound * (u64)60'750'000) <= timeAtFrameStart - timeAtWaveStart)) {
			sphereList.push_back(spherePool->NewSphere(mapPath, waveInfo[wave][sphereToBeSpawned].type));
			sphereToBeSpawned++;
			if (sphereToBeSpawned >= waveInfo[wave].size()) {
				sphereToBeSpawned = 0;
				noMoreSpheres = true;
			}
		}
		//if (secs_to_ticks(waveInfo[wave][i].timeIntoRound) <= timeAtWaveStart);
#endif

		//move projectiles
		for (auto it = projectileList.begin(); it != projectileList.end(); ) {
			if (!(*it)->UpdatePosition(deltaTime)) {
				projectilePool->ReturnProjectile((*it));
				it = projectileList.erase(it);
			}
			else {
				it++;
			}
		}

		//TODO: iterator bs (??)
		//collision detection
		for (auto projIt = projectileList.begin(); projIt != projectileList.end(); projIt++) {
			if (!(*projIt)->GetAlive()) continue;

			//note/todo: to optimise, make a quadtree instead of iterating over every single sphere that exists
			for (auto sphereIt = sphereList.begin(); sphereIt != sphereList.end(); sphereIt++) {

				if ((*sphereIt)->GetAlive() && (*projIt)->IsCollide((*sphereIt))) {

					(*sphereIt)->Hit((*projIt)->GetDamage());

					if (!(*projIt)->SphereHit((*sphereIt))) break;
				}
			}
		}

		//return killed spheres/projectiles
		auto projIt = projectileList.begin();
		while (projIt != projectileList.end()) {
			if (!(*projIt)->GetAlive()) {
				projectilePool->ReturnProjectile((*projIt));
				projIt = projectileList.erase(projIt);
			}
			else {
				projIt++;
			}
		}
		//spheres also give money when dying so that's handled here
		auto sphereIt = sphereList.begin();
		while (sphereIt != sphereList.end()) {
			if (!(*sphereIt)->GetAlive()) {
				money += (*sphereIt)->GetKilledMoney();
				spherePool->ReturnSphere((*sphereIt));
				sphereIt = sphereList.erase(sphereIt);
			}
			else {
				sphereIt++;
			}
		}


		/////////////////////////
		//cubes scan
		/////////////////////////


		//update cube times if necessary
		if (timeDifference > 0) {
			for (auto it = towerMap.begin(); it != towerMap.end(); it++) {
				for (auto jt = (*it).second.begin(); jt != (*it).second.end(); jt++) {
					jt->AddTimeDifference(timeDifference);
				}
			}
			//last instance timeDifference is needed in main loop, reset (technically first aswell)
			timeDifference = 0;
		}

		//cubes scan spheres to spawn projectiles
		for (auto it = towerMap.begin(); it != towerMap.end(); it++) {
			for (auto jt = (*it).second.begin(); jt != (*it).second.end(); jt++) {

				//if sphere found and ready to fire
				if ((*jt).ScanSpheres(sphereList, timeAtFrameStart)) {
					projectileInfo projInfo = (*jt).GetProjectileInfo();
					projectileBeginInfo projBeginInfo = (*jt).GetProjectileBeginInfo();

					projectileList.push_back(projectilePool->NewProjectile(projBeginInfo.position, projBeginInfo.direction, projInfo.speed, projInfo.pierce, projInfo.distanceLifetime, projInfo.damage));
				}
			}
		}

		//update wave variables as needed
		//if wave defeated
		if (sphereList.size() == 0 && noMoreSpheres) {

			//give wave money once wave defeated
			if (!waveMoneyGiven) {
				money += 100 + (wave * 10);
				waveMoneyGiven = true;
			}

			//if no more waves left
			if (wave >= waveInfo.size() - 1) {
				//if still alive (edge case where very final sphere could be leaked and even if health <= 0, could win without check)
				if (health > 0) {
					gameWon = true;
				}
			}
			//advance wave
			else if (autoAdvanceWave || (buttonDown & BUTTON_SELECT && selectedTower < 0)) {
				if (wave < waveInfo.size() - 1) {
					wave++;
					noMoreSpheres = false;
					waveMoneyGiven = false;
					timeAtWaveStart = timeAtFrameStart;
				}
				else {

				}
			}
		}
		


		/////////////////////
		//player input handle
		/////////////////////



		//pause game
		if (buttonDown & BUTTON_PAUSE) {
			paused = true;
		}
		
		//camera movement
		camera.MoveUp(buttonTriggerL * deltaTime);
		camera.MoveDown(buttonTriggerR * deltaTime);
		camera.MoveForward(buttonStickL.y * deltaTime);
		camera.MoveRight(buttonStickL.x * deltaTime);
		camera.RotateCamera(buttonStickR.x * deltaTime, buttonStickR.y * deltaTime);

		//selected tower controls
		if (selectedTower >= 0) {
			if (buttonDown & BUTTON_SELECT) {
				//either do something with cube or place new cube ontop
				if (selectedCubeInTower == (int)towerMap[selectedTower].size()) {
					//check for enough money to place (note/todo: change money based on cube value)
					if (money >= 200) {
						towerMap[selectedTower].push_back(Cube(
							{ mapTowerPoints[selectedTower].x, mapTowerPoints[selectedTower].y + 1 + (2 * towerMap[selectedTower].size()), mapTowerPoints[selectedTower].z }, 0));
						money -= 200;
					}
				}
				else {
					//todo: upgrades or something? idk
				}
			}

			if (buttonDown & BUTTON_UP) {
				selectedCubeInTower++;
				//bring selectedCubeInTower back into valid bounds if it leaves it
				selectedCubeInTower %= (towerMap[selectedTower].size() + 1);
			}
			if (buttonDown & BUTTON_DOWN) {
				selectedCubeInTower--;
				//why the fuck does modulo division result in negative numbers for negative numbers rahhhhh
				if (selectedCubeInTower < 0) selectedCubeInTower += (towerMap[selectedTower].size() + 1);
			}


			if (buttonDown & BUTTON_DESELECT) {
				selectedTower = -1;
			}
		}

		//update internal camera position, move render camera position
		camera.UpdateCameraValues();
		CameraData cameraData = camera.GetCameraData();

		//do stuff dependent on camera (selecting cube tower, etc)

		//work out which (if any) tower point is being looked at
		int cameraPointingAtTower = -1;

		for (size_t i = 0; i < mapTowerPoints.size(); i++) {
			guVector cameraToPoint = DirectionUnitVector(cameraData.pos, mapTowerPoints[i]);
			
			float mostCenteredPoint;
			float currentCenteredPoint = guVecDotProduct(&cameraData.direction, &cameraToPoint);
			if (currentCenteredPoint < 0.9f) continue;

			if (cameraPointingAtTower == -1) {
				cameraPointingAtTower = i;
				mostCenteredPoint = currentCenteredPoint;
			}
			else {
				//if new point more centered than old point
				if (currentCenteredPoint > mostCenteredPoint) {
					cameraPointingAtTower = i;
					mostCenteredPoint = currentCenteredPoint;
				}
			}
		}

		Printf(to_string(cameraPointingAtTower));

		//select tower
		if ((selectedTower < 0) && (buttonDown & BUTTON_SELECT) && (cameraPointingAtTower >= 0)) {
			selectedTower = cameraPointingAtTower;
			selectedCubeInTower = towerMap[selectedTower].size();
		}

		//debug prints so i can see camera values
		/*
		Printf("hoz:" + to_string(cameraData.hoz));
		Printf("ver:" + to_string(cameraData.ver));
		Printf("dir x:" + to_string(cameraData.direction.x));
		Printf("dir y:" + to_string(cameraData.direction.y));
		Printf("dir z:" + to_string(cameraData.direction.z));

		//*/



		/////////////////////
		//render game
		/////////////////////




		GRRLIB_Camera3dSettings(cameraData.pos.x, cameraData.pos.y, cameraData.pos.z, 0, 1, 0, cameraData.pos.x + cameraData.direction.x, cameraData.pos.y + cameraData.direction.y, cameraData.pos.z + cameraData.direction.z);

		GRRLIB_SetLightAmbient(0x808080FF);

		GRRLIB_SetLightOff();

		//draw outline of base of tower
		if (selectedTower < 0) {
			if (cameraPointingAtTower >= 0) {
				GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 0, 0);

				guVector base = mapTowerPoints[cameraPointingAtTower];

				GRRLIB_ObjectView(base.x, base.y, base.z, 0, 0, 0, 1.5f, 1.5f, 1.5f);
				GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 5);

					GX_Position3f32(1.0f, 0.0f, 1.0f);
					GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
					GX_Position3f32(-1.0f, 0.0f, 1.0f);
					GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
					GX_Position3f32(-1.0f, 0.0f, -1.0f);
					GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
					GX_Position3f32(1.0f, 0.0f, -1.0f);
					GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
					GX_Position3f32(1.0f, 0.0f, 1.0f);
					GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));

				GX_End();
			}
		}
		//draw selected cube highlight
		else {
			GRRLIB_2dMode();
			guVector base = mapTowerPoints[selectedTower];

			float highlightRotation = 0;

			if (selectedCubeInTower >= (int)towerMap[selectedTower].size()) {
				//TODO: draw feint highlight of cube 
			}
			else {
				//selected cube in tower has been placed, outline needs to match rotation of cube
				highlightRotation = towerMap[selectedTower][selectedCubeInTower].GetRotation();
			}
			Printf(to_string(highlightRotation));

			GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 0, 0);

			GRRLIB_ObjectView(base.x, base.y + 1 + (2 * selectedCubeInTower), base.z, 0, RadToDeg(highlightRotation), 0, 1, 1, 1);
			GX_Begin(GX_LINESTRIP, GX_VTXFMT0, 10);

				//bottom
				GX_Position3f32(1.0f, -1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(1.0f, -1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(-1.0f, -1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(-1.0f, -1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(1.0f, -1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0x33, 0x33, 0xff));
				//top
				//note that transition from bottom to top includes front right vertical line
				GX_Position3f32(1.0f, 1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0x33, 0x33, 0xff));
				GX_Position3f32(-1.0f, 1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(-1.0f, 1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(1.0f, 1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(1.0f, 1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));

			GX_End();

			GX_Begin(GX_LINES, GX_VTXFMT0, 6);

				//front left vertical line
				GX_Position3f32(1.0f, -1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0x33, 0x33, 0xff));
				GX_Position3f32(1.0f, 1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0x33, 0x33, 0xff));
				//back left vertical line
				GX_Position3f32(-1.0f, -1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(-1.0f, 1.0f, -1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				//back right vertical line
				GX_Position3f32(-1.0f, -1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
				GX_Position3f32(-1.0f, 1.0f, 1.0f);
				GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));

			GX_End();

		}

		GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 0, 1);

		//GRRLIB_ObjectView(0,3,0, 0,0,0, 1,1,1);
		//GRRLIB_DrawSphere(0.2f, 20, 20, true, ((lightColour << 8) | 0xFF));

		guVector lightPos = { 0.0f, 100.0f, 0.0f };
		GRRLIB_SetLightDiff(0, lightPos, 20.0f, 1.0f, 0xffffffff);


		GRRLIB_ObjectView(0, -2, 0, 0,0,0, 1, 1, 1);
		DrawColourfulCubeNormal();

		//old platform place

		u64 timeEverythingElse = gettime() - timeAtFrameStart;

		//render spheres
		for (size_t i = 0; i < sphereList.size(); i++) {

			//frustum culling
			guVector cameraToPoint = DirectionUnitVector(cameraData.pos, sphereList[i]->GetPosition());
			if (guVecDotProduct(&cameraData.direction, &cameraToPoint) < cosf(DegToRad(cameraData.FOV / 1.5f))) continue;

			GRRLIB_ObjectView(sphereList[i]->GetPosition().x, sphereList[i]->GetPosition().y, sphereList[i]->GetPosition().z, 0, 0, 0, 1, 1, 1);

			float distanceFromCam = VectorDistance(cameraData.pos, sphereList[i]->GetPosition());
			int spherePrecision = 0;
			if (distanceFromCam < 15.0f) {
				spherePrecision = 10;
			}
			else if (distanceFromCam < 25.0f) {
				spherePrecision = 9;
			}
			else if (distanceFromCam < 40.0f) {
				spherePrecision = 8;
			}
			else {
				spherePrecision = 6;
			}
			GRRLIB_DrawSphere(1.0f, spherePrecision, spherePrecision, true, Sphere::Colour(sphereList[i]->GetType()));
		}

		//render projectiles (todo: change this to support "unique looking" projectiles)
		for (size_t i = 0; i < projectileList.size(); i++) {

			//frustum culling
			guVector cameraToPoint = DirectionUnitVector(cameraData.pos, projectileList[i]->GetPosition());
			if (guVecDotProduct(&cameraData.direction, &cameraToPoint) < cosf(DegToRad(cameraData.FOV / 1.5f))) continue;

			GRRLIB_ObjectView(projectileList[i]->GetPosition().x, projectileList[i]->GetPosition().y, projectileList[i]->GetPosition().z, 0, 0, 0, 1, 1, 1);
			GRRLIB_DrawSphere(0.125f, 6, 6, true, RGBA(0xff, 0xff, 0xff, 0xff));
		}

		//temp to render points where cubes can be placed
		for (size_t i = 0; i < mapTowerPoints.size(); i++) {
			if (towerMap[i].size() != 0) continue;

			//frustum culling
			guVector cameraToPoint = DirectionUnitVector(cameraData.pos, mapTowerPoints[i]);
			if (guVecDotProduct(&cameraData.direction, &cameraToPoint) < cosf(DegToRad(cameraData.FOV/1.5f))) continue;

			GRRLIB_ObjectView(mapTowerPoints[i].x, mapTowerPoints[i].y, mapTowerPoints[i].z, 0, 0, 0, 1, 1, 1);
			GRRLIB_DrawSphere(0.25f, 8, 8, true, RGBA(0xff, 0x33, 0x33, 0xff));
		}

		//render cubes in towers
		for (auto it = towerMap.begin(); it != towerMap.end(); it++) {
			for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {

				//frustum culling
				guVector cameraToPoint = DirectionUnitVector(cameraData.pos, jt->GetPosition());
				if (guVecDotProduct(&cameraData.direction, &cameraToPoint) < cosf(DegToRad(cameraData.FOV / 1.5f))) continue;

				guVector cubePos = jt->GetPosition();
				GRRLIB_ObjectView(cubePos.x, cubePos.y, cubePos.z, 0, RadToDeg(jt->GetRotation()), 0, 1, 1, 1);
				GRRLIB_DrawCube(2.0f, true, Cube::Colour(0));
			}
		}
		//3 platforms to test transparency
		//platform
		GRRLIB_ObjectView(0, -5, 0, 0, 0, 0, 20, 1, 20);
		GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 1, 1);

		GRRLIB_SetTexture(texPlatform, 0);
		
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

			GX_Position3f32(1.0f, 0.0f, 1.0f);
			GX_Normal3f32(0.0f, 1.0f, 0.0f);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(1.0f, 1.0f);
			GX_Position3f32(-1.0f, 0.0f, 1.0f);
			GX_Normal3f32(0.0f, 1.0f, 0.0f);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(0.0f, 1.0f);
			GX_Position3f32(-1.0f, 0.0f, -1.0f);
			GX_Normal3f32(0.0f, 1.0f, 0.0f);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(0.0f, 0.0f);
			GX_Position3f32(1.0f, 0.0f, -1.0f);
			GX_Normal3f32(0.0f, 1.0f, 0.0f);
			GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
			GX_TexCoord2f32(1.0f, 0.0f);

		GX_End();


		//platform
		GRRLIB_ObjectView(0, -8, 0, 0, 0, 0, 20, 1, 20);
		GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 1, 1);

		GRRLIB_SetTexture(texPlatform, 0);

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

		GX_Position3f32(1.0f, 0.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(1.0f, 1.0f);
		GX_Position3f32(-1.0f, 0.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(0.0f, 1.0f);
		GX_Position3f32(-1.0f, 0.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(0.0f, 0.0f);
		GX_Position3f32(1.0f, 0.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(1.0f, 0.0f);

		GX_End();

		//platform
		GRRLIB_ObjectView(0, -11, 0, 0, 0, 0, 20, 1, 20);
		GRRLIB_3dMode(0.1, 1000, cameraData.FOV, 1, 1);

		GRRLIB_SetTexture(texPlatform, 0);

		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

		GX_Position3f32(1.0f, 0.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(1.0f, 1.0f);
		GX_Position3f32(-1.0f, 0.0f, 1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(0.0f, 1.0f);
		GX_Position3f32(-1.0f, 0.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(0.0f, 0.0f);
		GX_Position3f32(1.0f, 0.0f, -1.0f);
		GX_Normal3f32(0.0f, 1.0f, 0.0f);
		GX_Color1u32(RGBA(0xff, 0xff, 0xff, 0xff));
		GX_TexCoord2f32(1.0f, 0.0f);

		GX_End();

		u64 timeSpheres = gettime() - timeAtFrameStart - timeEverythingElse;



		/////////////////////
		//user interface
		/////////////////////



		//ui
		GRRLIB_2dMode();

		Printf(to_string(timeEverythingElse));
		Printf(to_string(timeSpheres));

		GRRLIB_Printf(20, 20, texFont, 0xffffffff, 1, ("Health: " + to_string(health)).c_str());
		GRRLIB_Printf(20, 45, texFont, 0xffffffff, 1, ("Money: " + to_string(money)).c_str());
		
		if (selectedTower < 0) GRRLIB_Circle(rmode->fbWidth * 0.5f, rmode->efbHeight * 0.5f, 3, 0xffffffff, true);

		//save frame in texture to be displayed when gameplay pauses
		GRRLIB_Screen2Texture(0, 0, texPrevFrame, GX_FALSE);



		GRRLIB_Render();



		////////////////////
		//pause screen loops
		////////////////////



		//ticks_to_microsecs expanded and replaced here
		deltaTime = ((double)((gettime() - timeAtFrameStart))) / 60.75;
		timeAtFrameStart = gettime();
		deltaTime /= 50049.0 / 3.0;	//so that deltaTime is close to 1 when fps is 60
		FPS = (u8)(60 / deltaTime);


		bool haveStoppedMainLoop = true;
		//if the game needs to pause (pause button, game won/lost), blocking loop entered here
		//if game lost
		if (health <= 0) {

			//pause begin animation
			u8 opacity = 255;
			for (; opacity > 32; opacity -= 23) {
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Render();
			}

			//interactive part when game fully paused
			while (powerState == PowerState::ON) {
				RefreshPads();
				if (GetButtonsDown(0) & BUTTON_PAUSE) break;
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Printf((640 - (11 * 19)) / 2, 100, texFont, RGBA(0xe2, 0xe2, 0xe2, 0xff), 1, "Game lost womp womp");
				GRRLIB_Render();
			}

			//breaks main game loop
			break;
		}
		//if game won
		else if (gameWon) {
			gameWon = false;

			//pause begin animation
			u8 opacity = 255;
			for (; opacity > 32; opacity -= 23) {
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Render();
			}

			//interactive part when game fully paused
			while (powerState == PowerState::ON) {
				RefreshPads();
				if (GetButtonsDown(0) & BUTTON_PAUSE) break;
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Printf((640 - (11 * 9)) / 2, 100, texFont, RGBA(0xe2, 0xe2, 0xe2, 0xff), 1, "Game Won!");
				GRRLIB_Render();
			}

			//pause end animation
			for (; opacity < 255; opacity += 23) {
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Render();
			}

			//breaks main game loop, note: will probably remove if i decide to do some sort of free play mode
			break;
		}
		//pause button pressed
		else if (paused) {
			paused = false;
			bool quitGame = false;

			//pause begin animation
			u8 opacity = 255;
			for (; opacity > 32; opacity -= 23) {
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Render();
			}

			//interactive part when game fully paused
			while (powerState == PowerState::ON) {
				RefreshPads();
				if (GetButtonsDown(0) & BUTTON_SELECT) {
					quitGame = true;
					break;
				}
				if (GetButtonsDown(0) & BUTTON_PAUSE) break;
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Printf((640 - (11 * 7)) / 2, 100, texFont, RGBA(0xe2, 0xe2, 0xe2, 0xff), 1, "Paused!");
				GRRLIB_Render();
			}

			//pause end animation
			for (; opacity < 255; opacity += 23) {
				GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
				GRRLIB_Render();
			}

			if (quitGame) break;
		}
		else {
			haveStoppedMainLoop = false;
		}

		if (haveStoppedMainLoop) {
			//to offset the times recorded when paused (not perfect but being off by a few thousand ticks really isnt that impactful)
			//only places that time is recorded (pls future me keep this updated) is:
			// these time variables
			// time stored in cubes when last fired

			timeDifference = gettime() - timeAtFrameStart;

			timeAtWaveStart += timeDifference;
			timeAtFrameStart += timeDifference;
		}
		

	}
	
	//free the memory claimed by the program
	//note: i did this to guarantee object destruction when i used raw pointers, but im p sure smart pointers make all of this cleanup code redundant
	//oh well
	while (sphereList.size() > 0) {
		spherePool->ReturnSphere(sphereList.back());
		sphereList.pop_back();
	}
	while (projectileList.size() > 0) {
		projectilePool->ReturnProjectile(projectileList.back());
		projectileList.pop_back();
	}

	//for whatever reason, manually calling the destructor, and then exiting the function scope causes an exception dsi, so fuck knows
	//spherePool.~SpherePool();

	gameState = GameState::MAINMENU;
}

static void MainMenu() {
	GRRLIB_2dMode();

	const int numOptions = 4;
	int selectedOption = 0;

	bool breakLoop = false;

	//vars to stop inputs on main menu and animate map opening
	bool mapMenuOpen = false;
	bool animateMapSelect = false;
	bool animateMapDeselect = false;
	u8 opacity = 255;
	float mapMenuOffset = 565;
	//0 = map select, 1 = difficulty select
	int mapMenuState = 0;

	//vars so the stick can be used on menu and not activate every frame
	int stickInputTime = 14;

	u32 colourSelected = RGBA(0xE2, 0xE2, 0xE2, 0xFF);
	u32 colourDeselected = RGBA(0x71, 0x71, 0x71, 0x99);


	while (powerState == PowerState::ON && gameState == GameState::MAINMENU) {
		RefreshPads();

		logOffset = 4;
		//Printf(to_string(saveGame));

		u32 buttonDown = GetButtonsDown(0);
		Stick buttonStickL = GetLeftStick(0);

		bool stickUp = false;
		bool stickDown = false;
		bool stickLeft = false;
		bool stickRight = false;

		//stick handling code
		if (++stickInputTime >= 12) {
			if (buttonStickL.y > 0.5) {
				stickUp = true;
				stickInputTime = 0;
			}
			if (buttonStickL.y < -0.5) {
				stickDown = true;
				stickInputTime = 0;
			}
			if (buttonStickL.x > 0.5) {
				stickRight = true;
				stickInputTime = 0;
			}
			if (buttonStickL.x < -0.5) {
				stickLeft = true;
				stickInputTime = 0;
			}
		}

		//main menu inputs, not map menu inputs
		if (!mapMenuOpen) {
			//option selected on menu, do action based on option
			if (buttonDown & BUTTON_SELECT) {
				switch (selectedOption) {
				//play (opens map select)
				case 0:
					mapMenuOpen = true;
					animateMapSelect = true;
					break;
				//shop
				case 1:
					//todo: make shop (somehow)
					gameState = GameState::SHOP;
					break;
				//quit to loader
				case 2:
					powerState = PowerState::TOLOADER;
					break;
				//turn off wii
				case 3:
					powerState = PowerState::OFF;
					break;
				}
			}

			if (stickDown) {
				selectedOption++;
				selectedOption %= numOptions;
			}
			if (stickUp) {
				selectedOption--;
				//once again, fuck negative modulo resulting in negative numbers
				selectedOption = (selectedOption + numOptions) % numOptions;
			}
		}


		//for whatever reason, the -0.01f on the x coord fixes issues where pixels leak from other letters. I have no idea why or even how
		GRRLIB_Printf((640 - (11 * 2 * 7)) / 2 - 0.01f, 30, texFont, RGBA(0x9A, 0x03, 0x1E, 0xFF), 2, "Spheres");
		GRRLIB_Printf((640 - (11 * 2 * 2)) / 2 - 0.01f, 70, texFont, RGBA(0xE2, 0xE2, 0xE2, 0xFF), 2, "VS");
		GRRLIB_Printf((640 - (11 * 2 * 5)) / 2 - 0.01f, 110, texFont, RGBA(0x31, 0xC7, 0xF6, 0xFF), 2, "Cubes");

		//render options
		GRRLIB_Printf(50, 200, texFont, (selectedOption == 0) ? colourSelected : colourDeselected, 1, "Play");
		GRRLIB_Printf(50, 230, texFont, (selectedOption == 1) ? colourSelected : colourDeselected, 1, "Shop (Under Construction, currently saves game)");
		GRRLIB_Printf(50, 260, texFont, (selectedOption == 2) ? colourSelected : colourDeselected, 1, "Quit to loader");
		GRRLIB_Printf(50, 290, texFont, (selectedOption == 3) ? colourSelected : colourDeselected, 1, "Turn off wii");

		GRRLIB_SetBackgroundColour(0x00, 0x00, 0x00, 0xff);

		//map menu
		if (mapMenuOpen) {
			GRRLIB_Screen2Texture(0, 0, texPrevFrame, GX_TRUE);
			const int margin = 75;

			if (animateMapSelect) {
				opacity -= 23;
				mapMenuOffset -= 56.5;
				if (mapMenuOffset <= 1) {
					animateMapSelect = false;
				}
			}
			else if (animateMapDeselect) {
				opacity += 23;
				mapMenuOffset += 56.5;
				if (mapMenuOffset >= 564) {
					animateMapDeselect = false;
					mapMenuOpen = false;
				}
			}
			//input handled in else case so that inputs only happen with no animations
			else {
				if (buttonDown & BUTTON_DESELECT) {
					animateMapDeselect = true;
				}
				else if (buttonDown & BUTTON_SELECT) {
					gameState = GameState::GAME;
				}
			}


			GRRLIB_DrawImg(0, 0, texPrevFrame, 0, 1, 1, 0xffffff00 | opacity);
			GRRLIB_Rectangle(margin + mapMenuOffset, margin, rmode->fbWidth - (margin * 2), rmode->efbHeight - (margin * 2), RGBA(0x11, 0x1d, 0x4a, 0x0ff), true);
		}

		GRRLIB_SetBackgroundColour(0x11, 0x1d, 0x4a, 255);

		/* Debug info
		Printf(to_string(buttonDown));
		Printf(to_string(buttonStickL.x) + " " + to_string(buttonStickL.y));
		Printf(to_string(texPrevFrame->w) + " x " + to_string(texPrevFrame->h));
		Printf(to_string(texPrevFrame->offsetx) + " offset " + to_string(texPrevFrame->offsety));
		Printf(to_string(texPrevFrame->handlex) + " handle " + to_string(texPrevFrame->handley));
		*/
		GRRLIB_Render();
	}
#ifdef DEBUG_TESTLOOP
	gameState = GameState::TEST;
#endif
}


static void ShopMenu() {
	saveGame["save"]["cubux"] = (long)saveGame["save"]["cubux"] + 1;
	string message = "";
	if (WriteSaveFile(saveGame, message)) {
		message = message + " game saved successfully!";
	}
	else {
		message = message + " error occurred!";
	}
	while (powerState == PowerState::ON) {
		RefreshPads();
		logOffset = 2;
		Printf(message);
		if (GetButtonsDown(0)) break;
		GRRLIB_Render();
	}
	
	//temporary reload of save to check if actually written
	LoadSaveFile(saveGame);
	gameState = GameState::MAINMENU;
}

//current test: what happens when not running 3d settings every frame
static void RunTest() {
	Camera camera({ 0,0,10 }, 0.0, 0.0, 90.0);

	CameraData camData = camera.GetCameraData();
	GRRLIB_Camera3dSettings(camData.pos.x, camData.pos.y, camData.pos.z, 0, 1, 0, camData.direction.x + camData.pos.x, camData.direction.y + camData.pos.y, camData.direction.z + camData.pos.z);
	GRRLIB_3dMode(0.1f, 100.0f, camData.FOV, 0, 1);

	while (powerState == PowerState::ON) {
		logOffset = 1;
		RefreshPads();
		
		u32 buttonHeld = WPAD_ButtonsHeld(0);
		u32 buttonDown = WPAD_ButtonsDown(0);
		Stick buttonStickL = GetLeftStick(0);
		Stick buttonStickR = GetRightStick(0);
		float buttonTriggerL = GetLeftTrigger(0);
		float buttonTriggerR = GetRightTrigger(0);
		double deltaTime = 1;

		//camera movement
		camera.MoveUp(buttonTriggerL * deltaTime);
		camera.MoveDown(buttonTriggerR * deltaTime);
		camera.MoveForward(buttonStickL.y * deltaTime);
		camera.MoveRight(buttonStickL.x * deltaTime);
		camera.RotateCamera(buttonStickR.x * deltaTime, buttonStickR.y * deltaTime);

		if (buttonDown & BUTTON_PAUSE) {
			break;
		}

		GRRLIB_ObjectView(0,0,0, 0,0,0, 1,1,1);
		DrawColourfulCubeNormal();

		GRRLIB_Render();
	}

	/*
	while (1) {
		logOffset = 2;
		Printf("exited test loop!");
		GRRLIB_Render();
		WPAD_ScanPads();
		if (WPAD_ButtonsDown(0)) break;
	}
	//*/

	gameState = GameState::MAINMENU;
}

int main(int argc, char* argv[]) {
	Init();

	while (powerState == PowerState::ON) {
		switch (gameState) {
		case GameState::MAINMENU:
			MainMenu();
			break;
		case GameState::GAME:
			RunGame();
			break;
		case GameState::SHOP:
			ShopMenu();
			break;
		case GameState::TEST:
			RunTest();
			break;
		default:
			//illegal game state (how)! go back to main menu
			gameState = GameState::MAINMENU;
		}
	}

	/*
	GRRLIB_2dMode();
	while (1) {
		logOffset = 2;
		Printf("exited game!");
		Printf(to_string(argc));
		for (int i = 0; i < argc; i++) {
			Printf((std::string)argv[i]);
		}
		GRRLIB_Render();
		RefreshPads();
		if (GetButtonsDown(0)) break;
	}
	//*/

	GRRLIB_FreeTexture(texFont);
	GRRLIB_FreeTexture(texPrevFrame);
	GRRLIB_FreeTexture(texPlatform);
	GRRLIB_Exit();
	
	//return to hbc/turn off wii
	switch (powerState) {
	case PowerState::TOLOADER:
		exit(0);
		break;
	default:
		SYS_ResetSystem((int)powerState, 0, 0);
	}

	return 0;
}



/*
//old camera movement, likely will never include this again
if (buttonHeld & WPAD_NUNCHUK_BUTTON_C) {
	f32 camVerPrev = camera.rotationVer;
	if (buttonHeld & WPAD_BUTTON_B) {
		camera.FOV -= 1.0f * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_A) {
		camera.FOV += 1.0f * deltaTime;
	}
	//rotate camera
	if (buttonHeld & WPAD_BUTTON_UP) {
		camera.rotationVer += (M_PI / 64.0f) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_DOWN) {
		camera.rotationVer -= (M_PI / 64.0f) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_RIGHT) {
		camera.rotationHoz += (M_PI / 64.0f) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_LEFT) {
		camera.rotationHoz -= (M_PI / 64.0f) * deltaTime;
	}

	if (camera.rotationHoz < 0) camera.rotationHoz += 2 * M_PI;
	if (camera.rotationHoz > 2 * M_PI) camera.rotationHoz -= 2 * M_PI;
	if (camera.rotationVer < -(M_PI / 2)) camera.rotationVer = camVerPrev;
	if (camera.rotationVer > (M_PI / 2)) camera.rotationVer = camVerPrev;
}
else {
	if (buttonHeld & WPAD_BUTTON_B) {
		camera.pos.z += (cosf(camera.rotationHoz) / 16) * deltaTime;
		camera.pos.x += (-sinf(camera.rotationHoz) / 16) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_A) {
		camera.pos.z -= (cosf(camera.rotationHoz) / 16) * deltaTime;
		camera.pos.x -= (-sinf(camera.rotationHoz) / 16) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_UP) {
		camera.pos.y += (1.0f / 16) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_DOWN) {
		camera.pos.y -= (1.0f / 16) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_RIGHT) {
		camera.pos.x += (cosf(camera.rotationHoz) / 16) * deltaTime;
		camera.pos.z += (sinf(camera.rotationHoz) / 16) * deltaTime;
	}
	if (buttonHeld & WPAD_BUTTON_LEFT) {
		camera.pos.x -= (cosf(camera.rotationHoz) / 16) * deltaTime;
		camera.pos.z -= (sinf(camera.rotationHoz) / 16) * deltaTime;
	}
}
*/