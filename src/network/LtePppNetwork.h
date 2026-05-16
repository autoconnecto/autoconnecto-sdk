#pragma once

#include "../core/Config.h"

bool ltePppConnect(SDKConfig& config);

bool ltePppLinkUp();

void ltePppMaintain();

int ltePppSignalRssi();
