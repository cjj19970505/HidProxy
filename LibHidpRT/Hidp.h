#pragma once
#include "Hidp.g.h"

// WARNING: This file is automatically generated by a tool. Do not directly
// add this file to your project, as any changes you make will be lost.
// This file is a stub you can use as a starting point for your implementation.
//
// To add a copy of this file to your project:
//   1. Copy this file from its original location to the location where you store 
//      your other source files (e.g. the project root). 
//   2. Add the copied file to your project. In Visual Studio, you can use 
//      Project -> Add Existing Item.
//   3. Delete this comment and the 'static_assert' (below) from the copied file.
//      Do not modify the original file.
//
// To update an existing file in your project:
//   1. Copy the relevant changes from this file and merge them into the copy 
//      you made previously.
//    
// This assertion helps prevent accidental modification of generated files.
namespace winrt::LibHidpRT::implementation
{
    struct Hidp : HidpT<Hidp>
    {
        Hidp(HANDLE hidpHandle, HANDLE notificationCompletionPort);

        static winrt::Windows::Foundation::IAsyncOperation<winrt::LibHidpRT::Hidp> CreateAsync(winrt::Windows::Storage::Streams::Buffer reportDescriptor);
        winrt::Windows::Foundation::IAsyncAction SubmitReportAsync(uint8_t reportId, winrt::Windows::Storage::Streams::IBuffer reportData);
        winrt::event_token OnGetFeatureRequest(winrt::Windows::Foundation::EventHandler<winrt::LibHidpRT::GetFeatureRequest> const& handler);
        void OnGetFeatureRequest(winrt::event_token const& token) noexcept;
        winrt::event_token OnSetFeatureRequest(winrt::Windows::Foundation::EventHandler<winrt::LibHidpRT::SetFeatureRequest> const& handler);
        void OnSetFeatureRequest(winrt::event_token const& token) noexcept;
        winrt::Windows::Foundation::IAsyncAction CompleteGetFeatureRequestAsync(winrt::LibHidpRT::GetFeatureRequest request, winrt::Windows::Storage::Streams::IBuffer reportBuffer, bool supported);
        winrt::Windows::Foundation::IAsyncAction CompleteSetFeatureRequestAsync(winrt::LibHidpRT::SetFeatureRequest request, bool supported);
        void Close();

    private:
        Windows::Foundation::IAsyncAction InitAsync(winrt::Windows::Storage::Streams::Buffer reportDescriptor);
        winrt::Windows::Foundation::IAsyncAction _RegisterForNotificationAsync(HANDLE notificationRegisteredHandle);
        HANDLE _HidpHandle = NULL;
        HANDLE _NotificationCompletionPort;
        Windows::Foundation::IAsyncAction _NotificationTask = nullptr;

        winrt::event<Windows::Foundation::EventHandler<winrt::LibHidpRT::GetFeatureRequest>> _OnGetFeatureRequestEvent;
        winrt::event<Windows::Foundation::EventHandler<winrt::LibHidpRT::SetFeatureRequest>> _OnSetFeatureRequestEvent;
    };
}
namespace winrt::LibHidpRT::factory_implementation
{
    struct Hidp : HidpT<Hidp, implementation::Hidp>
    {
    };
}

