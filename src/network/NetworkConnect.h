#pragma once

#include "../core/Callbacks.h"
#include "../core/Config.h"

bool networkConnect(SDKConfig& config);

void networkOnWifiJoin(WifiJoinCallback cb);

bool networkLastWifiJoin(WifiJoinInfo& out);

void networkMaintain(SDKConfig& config);

bool networkLinkUp(const SDKConfig& config);

int networkSignalRssi(const SDKConfig& config);
