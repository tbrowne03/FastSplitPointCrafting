#pragma once

#include "script.h"

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <sstream>

using namespace std;

void ShowSubtitle(string message)
{
	const char* messagePtr = message.c_str();

	//_UILOG_SET_CACHED_OBJECTIVE: Hash._0xFA233F8FE190514C
	UILOG::_0xFA233F8FE190514C((Any*)messagePtr);

	//_UILOG_PRINT_CACHED_OBJECTIVE: Hash._0xE9990552DEC71600
	UILOG::_0xE9990552DEC71600();

	//_UILOG_CLEAR_HAS_DISPLAYED_CACHED_OBJECTIVE: Hash._0xA3108D6981A5CADB
	UILOG::_0xA3108D6981A5CADB();

	//_UILOG_CLEAR_CACHED_OBJECTIVE: Hash._0xDFF0D417277B41F8
	UILOG::_0xDFF0D417277B41F8();
}

void ScriptMain()
{
	//Initialize Variables
	Ped playerPed = PLAYER::PLAYER_PED_ID();
	DWORD keyBinding = 75;
	int specifiedAmmoIncrement;
	int actualAmmoIncrement;
	bool keyConfigSuccess = false;
	bool ammoIncrementSuccess = false;

	//Initialize Valid Ammo Types
	set<uint> validAmmoTypes;
	validAmmoTypes.insert(1681219929u); //Revolver
	validAmmoTypes.insert(1950175060u); //Pistol
	validAmmoTypes.insert(2964851610u); //Repeater
	validAmmoTypes.insert(218444191u);  //Rifle

	//Populate a map to relate ammo type/ split point ammo type
	map<int, int> splitAmmoLookup;
	splitAmmoLookup.insert({ 1681219929u, 1243983880u }); //Revolver, RevolverSplitPoint
	splitAmmoLookup.insert({ 1950175060u, 236338048u  }); //Pistol,   PistolSplitPoint
	splitAmmoLookup.insert({ 2964851610u, 1148521608u }); //Repeater, RepeaterSplitPoint
	splitAmmoLookup.insert({ 218444191u,  218444191u  }); //Rifle,    RifleSplitPoint

	//Load settings file
	ifstream file("FastSplitPointCrafting.ini");

	if (file.is_open()) {
		string line;
		string delimiter = "=";

		while (getline(file, line)) {

			size_t delimiterPos = line.find(delimiter);
			if (delimiterPos == string::npos) {
				continue;
			}

			string settingName = line.substr(0, delimiterPos);
			string settingValue = line.substr(delimiterPos + 1, line.length() - delimiterPos + 1);

			if (settingName.compare("fastAmmoCraftingKey") == 0) {
				istringstream stream(settingValue);
				stream >> hex >> keyBinding;

				if (!stream.fail()) {
					keyConfigSuccess = true;
				}
				
				continue;
			}

			if (settingName.compare("fastAmmoCraftingAmount") == 0) {
				istringstream stream(settingValue);
				stream >> specifiedAmmoIncrement;

				if (!(stream.fail() || specifiedAmmoIncrement < 0 || specifiedAmmoIncrement > 400)) {
					ammoIncrementSuccess = true;
				}

				continue;
			}
		}

		file.close();
	}

	//Use default values
	if (!keyConfigSuccess) {
		keyBinding = 0x4B;
	}

	if (!ammoIncrementSuccess) {
		specifiedAmmoIncrement = 25;
	}

	//Primary loop
	while (true)
	{		
		//If key is pressed
		if (IsKeyJustUp(keyBinding, true))
		{
			//If player is crouching
			//GET_PED_CROUCH_MOVEMENT: Hash._0xD5FE956C70FF370B
			if (PED::_0xD5FE956C70FF370B(playerPed))
			{
				//Get the current weapon
				//_GET_PED_CURRENT_HELD_WEAPON: Hash._0x8425C5F057012DAB
				Weapon currentPlayerWeapon = WEAPON::_0x8425C5F057012DAB(playerPed);

				//Get the ammo type for the weapon
				//_GET_AMMO_TYPE_FOR_WEAPON: Hash._0x5C2EA6C44F515F34
				int defaultAmmoType = WEAPON::_0x5C2EA6C44F515F34(currentPlayerWeapon);

				//If the weapon uses a valid ammo type
				if (validAmmoTypes.count(defaultAmmoType) != 0)
				{
					int ammoRemoveType = defaultAmmoType;
					int ammoAddType;

					map<int, int>::const_iterator index = splitAmmoLookup.find(defaultAmmoType);
					if (index == splitAmmoLookup.end())
					{
						continue;
					}
					else
					{
						ammoAddType = (*index).second;
					}

					//Get the max ammo for the weapon
					int maxAmmoInWeapon = 100;
					int* maxAmmoInWeaponPtr = &maxAmmoInWeapon;
					WEAPON::GET_MAX_AMMO(playerPed, maxAmmoInWeaponPtr, currentPlayerWeapon);

					//Get the current ammount of ammo the player has 
					int currentAmmountOfAddTypeAmmo = (int)WEAPON::GET_PED_AMMO_BY_TYPE(playerPed, ammoAddType);
					int currentAmmountOfRemoveTypeAmmo = (int)WEAPON::GET_PED_AMMO_BY_TYPE(playerPed, ammoRemoveType);

					// Check how much ammo should be crafted. Normally, this is set by mSpecifiedAmmoIncrement. However, sometimes this amount would craft too much ammo, and we should craft less.
					// Note: The max split point ammo a gun can hold is the regular ammo max / 2
					if ((currentAmmountOfAddTypeAmmo + specifiedAmmoIncrement) > (*maxAmmoInWeaponPtr / 2))
					{
						actualAmmoIncrement = (*maxAmmoInWeaponPtr / 2) - currentAmmountOfAddTypeAmmo;
					}
					else
					{
						actualAmmoIncrement = specifiedAmmoIncrement;
					}

					//If the player has enough ammo
					if (currentAmmountOfRemoveTypeAmmo >= actualAmmoIncrement)
					{
						// _GIVE_AMMO_TO_PED: Hash._0x106A811C6D3035F3
						WEAPON::_0x106A811C6D3035F3(playerPed, ammoAddType, actualAmmoIncrement, 0x2CD419DC);

						//_REMOVE_AMMO_FROM_PED_BY_TYPE: Hash._0xB6CFEC32E3742779
						WEAPON::_0xB6CFEC32E3742779(playerPed, ammoRemoveType, actualAmmoIncrement, 0x2188E0A3);

						string fullString = string("Added ") + to_string(actualAmmoIncrement) + string(" split point ammo to inventory.");
						ShowSubtitle(fullString);
					}
					else
					{
						string fullString = "Could not craft split point ammo. Insufficient materials.";
						ShowSubtitle(fullString);
					}
				}
			}
		}

		WAIT(0);
	}
}
