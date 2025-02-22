#include "globals.h"

// Define global variables
std::atomic<int> pendingRequests(0);
std::atomic<bool> stopThreads(false);
