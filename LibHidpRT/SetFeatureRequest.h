#pragma once
#include "SetFeatureRequest.g.h"

namespace winrt::LibHidpRT::implementation
{
    struct SetFeatureRequest : SetFeatureRequestT<SetFeatureRequest>
    {
        SetFeatureRequest(
            uint8_t reportId, 
            VOID* hidTransferPacket, 
            VOID* vhfOperationHandle, 
            Windows::Storage::Streams::IBuffer reportBuffer): 
            _ReportId(reportId), 
            _HidTransferPacket(hidTransferPacket), 
            _VhfOperationHandle(vhfOperationHandle),
            _ReportBuffer(reportBuffer)
        {
        }

        uint8_t ReportId()
        {
            return _ReportId;
        }
        winrt::Windows::Storage::Streams::IBuffer ReportBuffer()
        {
            return _ReportBuffer;
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
        Windows::Storage::Streams::IBuffer _ReportBuffer;
        VOID* _HidTransferPacket;
        VOID* _VhfOperationHandle;
    };
}
