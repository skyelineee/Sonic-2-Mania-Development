#include "RPCHelpers.hpp"
#include "discord.h"
#include <ctime>

// not an RSDK object like the other helpers, this just contains the functions themselves for updating the discord rich presence
discord::Core *core{};

void InitDiscord()
{
    //Discord.CreateFlags.Default will require Discord to be running for the game to work
    auto result = discord::Core::Create(APPLICATION_ID, DiscordCreateFlags_NoRequireDiscord, &core); // just figured out clientid and applicationid are the same thing ama
}

// Used whenever a stage setup calls for it, otherwise wont do anything
void SetPresence(const char *details, const char *state, const char *largeImage, const char *largeText, const char *smallImage, const char *smallText)
{
    discord::Activity activity{};
    activity.SetDetails(details);
    activity.SetState(state);
    discord::ActivityAssets activityAssets{};
    activityAssets.SetLargeImage(largeImage);
    activityAssets.SetLargeText(largeText);
    activityAssets.SetSmallImage(smallImage);
    activityAssets.SetSmallText(smallText);
    discord::ActivityTimestamps activityTimestamps{};
    activityTimestamps.SetStart(time(nullptr));
    activity.GetTimestamps() = activityTimestamps;
    activity.GetAssets() = activityAssets;
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
    });
}

// Called every frame in the RPUpdate object, will crash the game if SetPresence was never called first, so ill just add the object in stage setups that already call SetPresence
void RPCallback()
{
    ::core->RunCallbacks();
}
