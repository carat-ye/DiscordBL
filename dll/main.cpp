﻿// All functions are based off of "DWORD* obj, int rpcParami, const char** rpcParam"

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <stdio.h>
#include "torque.h"
#include "discord-rpc.h"
#include "discord_register.h"
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
using namespace std;
// stolen from http://www.geeksforgeeks.org/write-your-own-atoi/
int myAtoi(const char *str)
{
	int res = 0;
	for (int i = 0; str[i] != '\0'; ++i)
		res = res * 10 + str[i] - '0';
	return res;
}
char const* concenate(const char* string, const char* variable, const char* end)
{
	std::string first = string;
	std::string second = variable;
	std::string third = end;
	std::string constructor = first + second + third;
	const char* concenatedString = constructor.c_str();
	return concenatedString;
}
// Discord handlers
static void handleDiscordReady()
{
	Printf("DiscordBL | Connected to Discord");
}

static void handleDiscordDisconnected(int errcode, const char* message)
{
	Printf("DiscordBL | Disconnected from Rich Presence (%d: %s)", errcode, message);
}

static void handleDiscordError(int errcode, const char* message)
{
	Printf("DiscordBL | Error occured! (%d: %s)", errcode, message);
}

static void handleDiscordJoin(const char* secret)
{
	Printf("DiscordBL | Join event occured (%s)", secret);
	Eval(concenate("DiscordBL::JoinServer(\"", secret, "\");"), false, nullptr);
}

static void handleDiscordSpectate(const char* secret)
{
	Printf("DiscordBL | Spectate event occured (%s)", secret);
}

static void handleDiscordJoinRequest(const DiscordJoinRequest* request)
{
	Printf("DiscordBL | Join request event occured from %s - %s - %s",
		request->username,
		request->avatar,
		request->userId);
	Eval(concenate("DiscordBL::DecideJoinRequest(\"", request->username, "\");"), false, nullptr);
}

static void discordInitialize(DWORD* obj, int rpcParami, const char** rpcParam)
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = handleDiscordReady;
	handlers.disconnected = handleDiscordDisconnected;
	handlers.errored = handleDiscordError;
	handlers.joinGame = handleDiscordJoin;
	handlers.spectateGame = handleDiscordSpectate;
	handlers.joinRequest = handleDiscordJoinRequest;
	Discord_Initialize("379129890487795722", &handlers, 1, NULL);

	Printf("DiscordBL | Discord initialized");
}

static void replyBackWeLoaded(DWORD* obj, int rpcParami, const char** rpcParam)
{
	// Once the DLL loaded do some super h4x0r stuff such as registering
	Eval("if($Pref::DiscordBL::Registered $= \"\" || $Pref::DiscordBL::Register $= 0) { \
		doDiscordBLRegister(); \
		$Pref::DiscordBL:Registered = 1;\
		", false, nullptr);
}

static void doDiscordBLRegister(DWORD* obj, int rpcParami, const char** rpcParam)
{
	Discord_Register("379129890487795722", "ptlaaxobimwroe");
	Discord_RegisterSteamGame("379129890487795722", "250340");
}
static void replyDiscordPresence(DWORD* obj, int rpcParami, const DiscordJoinRequest* request, const char** rpcParam)
{
	if(myAtoi(rpcParam[1]) == 0)
	{
		Discord_Respond(request->userId, DISCORD_REPLY_NO);
	}
	else if(myAtoi(rpcParam[1]) == 1)
	{
		Discord_Respond(request->userId, DISCORD_REPLY_YES);
	}
	else if(myAtoi(rpcParam[1]) == 2)
	{
		Discord_Respond(request->userId, DISCORD_REPLY_IGNORE);
	}
	else
	{
		Printf("DiscordBL | Invalid response code");
	}
}
static void updateDiscordPresence(DWORD* obj, int rpcParami, const char** rpcParam)
{
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = rpcParam[1];
	discordPresence.largeImageKey = "logo";
	discordPresence.largeImageText = rpcParam[2];

	if (myAtoi(rpcParam[3]) != 0)
	{
		discordPresence.joinSecret = rpcParam[7];
		//discordPresence.matchSecret = "DiscordBL";
		discordPresence.partyId = rpcParam[9];
		discordPresence.partySize = myAtoi(rpcParam[5]);
		discordPresence.partyMax = myAtoi(rpcParam[6]);
		discordPresence.details = rpcParam[4];
		// Printf(rpcParam[8]);
		if (!strcmp(rpcParam[8], "singleplayer"))
		{
			discordPresence.smallImageKey = "singleplayer";
			discordPresence.smallImageText = "Singleplayer";
		}
		else if(!strcmp(rpcParam[8], "multiplayer"))
		{
			discordPresence.smallImageKey = "multiplayer";
			discordPresence.smallImageText = "Multiplayer";
		}
		else if(!strcmp(rpcParam[8], "localplay"))
		{
			discordPresence.smallImageKey = "localplay";
			discordPresence.smallImageText = "LAN";
		}
	}

	Discord_UpdatePresence(&discordPresence);
	Printf("DiscordBL | Rich Presence updated");
}

// Begin entry point
bool Init()
{
	if (!InitTorque())
		return false;
	ConsoleFunction(NULL, "discordInitalize", discordInitialize,
		"discordInitalize() - Initializes the Discord RPC connection.", 1, 1);
	ConsoleFunction(NULL, "updateDiscordPresence", updateDiscordPresence,
		"updateDiscordPresence(string details, string playerName, bool additionalDetails, string serverName, int playerCount, int maxPlayers, string partyKey, string status, string partyId) - Updates Discord's Rich Presence.", 4, 10);
	ConsoleFunction(NULL, "replyBackWeLoaded", replyBackWeLoaded, 
		"replyBackWeLoaded() - Tells the DLL that the Add-On loaded.", 1, 1);
	ConsoleFunction(NULL, "doDiscordBLRegister", doDiscordBLRegister,
		"doDiscordBLRegister() - Registers the URL protocol for DiscordBL.", 1, 1);
	Printf("DiscordBL | DLL module attached");

	return true;
}

bool Detach()
{
	Discord_Shutdown();
	return true;
}

//Entry point
int __stdcall DllMain(HINSTANCE hInstance, unsigned long reason, void *reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		return Init();

	case DLL_PROCESS_DETACH:
		return Detach();

	default:
		return true;
	}
}
