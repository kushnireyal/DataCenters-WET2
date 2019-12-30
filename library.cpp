#include <new>
#include "library2.h"
#include "DataCentersManager.h"

void* Init(int n) {
    if (n <= 0) return nullptr;
    try {
        auto *manager = new DataCentersManager(n);
        return (void*)manager;
    } catch (std::bad_alloc& ba) {
        return nullptr;
    }
}

StatusType MergeDataCenters(void *DS, int dataCenter1, int dataCenter2) {
    if (!DS || dataCenter1 <= 0 || dataCenter2 <= 0) return INVALID_INPUT;
    auto manager = (DataCentersManager*)DS;
    try {
        return (StatusType)(manager->MergeDataCenters(dataCenter1 - 1, dataCenter2 - 1));
    } catch (std::bad_alloc& ba) {
        return ALLOCATION_ERROR;
    }
}

StatusType AddServer(void *DS, int dataCenterID, int serverID) {
    if (!DS || dataCenterID <= 0 || serverID <= 0) return INVALID_INPUT;
    auto manager = (DataCentersManager*)DS;
    try {
        return (StatusType)(manager->AddServer(dataCenterID - 1, serverID));
    } catch (std::bad_alloc& ba) {
        return ALLOCATION_ERROR;
    }
}

StatusType RemoveServer(void *DS, int serverID) {
    if (!DS || serverID <= 0) return INVALID_INPUT;
    auto manager = (DataCentersManager*)DS;
    return (StatusType)(manager->RemoveServer(serverID));
}

StatusType SetTraffic(void *DS, int serverID,  int traffic) {
    if (!DS || serverID <= 0 || traffic < 0) return INVALID_INPUT;
    auto manager = (DataCentersManager*)DS;
    try {
        return (StatusType)(manager->SetTraffic(serverID, traffic));
    } catch (std::bad_alloc& ba) {
        return ALLOCATION_ERROR;
    }
}

StatusType SumHighestTrafficServers(void *DS, int dataCenterID,  int k, int *traffic) {
    if (!DS || dataCenterID < 0 || k < 0 || !traffic) return INVALID_INPUT;
    auto manager = (DataCentersManager*)DS;
    return (StatusType)(manager->SumHighestTrafficServers(dataCenterID - 1, k, traffic));
}

void Quit(void** DS) {
    auto manager = (DataCentersManager*)(*DS);
    delete manager;
    *DS = nullptr;
}
