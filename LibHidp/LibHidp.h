#pragma once

typedef HANDLE HIDPHANDLE;
typedef HIDPHANDLE* PHIDPHANDLE;

#ifdef LIBHIDP_EXPORTS
#    define LIBHIDP_API __declspec(dllexport)
#else
#    define LIBHIDP_API __declspec(dllimport)
#endif

LIBHIDP_API HRESULT HidpCreate(DWORD reportDescriptorSize, LPCVOID reportDescriptor, PHIDPHANDLE pHidpHandle);

LIBHIDP_API BOOL HidpClose(HIDPHANDLE hidpHandle);

LIBHIDP_API HRESULT HidpSubmitReport(HIDPHANDLE hidpHandle, UCHAR reportId, SIZE_T reportSize, PBYTE reportData);