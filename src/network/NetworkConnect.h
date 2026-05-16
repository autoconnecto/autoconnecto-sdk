#pragma once

#include "../core/Config.h"

bool networkConnect(SDKConfig& config);

void networkMaintain(SDKConfig& config);

bool networkLinkUp(const SDKConfig& config);

int networkSignalRssi(const SDKConfig& config);
