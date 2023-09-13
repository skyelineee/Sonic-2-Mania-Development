#pragma once
#define APPLICATION_ID (1149220865985228810)

struct RPCStatus 
{

};

// ==============================
// FUNCTIONS
// ==============================

//void SetPresence(RPCStatus *status);
void InitDiscord();
void SetPresence(const char *details, const char *state, const char *largeImage, const char *largeText, const char *smallImage, const char *smallText);
void RPCallback();