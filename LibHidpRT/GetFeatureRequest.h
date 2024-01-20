#pragma once
#include "GetFeatureRequest.g.h"
#include "..\HidProxy\hidp.h"

namespace winrt::LibHidpRT::implementation
{
    struct GetFeatureRequest : GetFeatureRequestT<GetFeatureRequest>
    {
        GetFeatureRequest(uint8_t reportId, VOID* hidTransferPacket, VOID* vhfOperationHandle):_ReportId(reportId), _HidTransferPacket(hidTransferPacket), _VhfOperationHandle(vhfOperationHandle)
        {

        }
        uint8_t ReportId()
        {
            return _ReportId;
        }
        VOID* HidTransferPacket()
        {
            return _HidTransferPacket;
        }
        VOID* VhfOperationHandle()
        {
            return _VhfOperationHandle;
        }

    private:
        uint8_t _ReportId;
        VOID* _HidTransferPacket;
        VOID* _VhfOperationHandle;
    };
}
