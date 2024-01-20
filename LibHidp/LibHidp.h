#pragma once

typedef VOID* HIDPHANDLE;
typedef HIDPHANDLE* PHIDPHANDLE;


typedef void (*PSET_FEATURE_ROUTINE)(HIDPHANDLE hidpHandle, UCHAR reportId, ULONG reportBufferLen, UCHAR* data);
typedef void (*PGET_FEATURE_ROUTINE)(HIDPHANDLE hidpHandle, UCHAR reportId, ULONG bufferLength, PULONG pReportBufferLen, UCHAR* data);

#ifdef LIBHIDP_EXPORTS
#    define LIBHIDP_API __declspec(dllexport)
#else
#    define LIBHIDP_API __declspec(dllimport)
#endif

LIBHIDP_API HRESULT HidpCreate(DWORD reportDescriptorSize, LPCVOID reportDescriptor, PSET_FEATURE_ROUTINE setFeatureRoutine, PGET_FEATURE_ROUTINE getFeatureRoutine, PHIDPHANDLE pHidpHandle);

LIBHIDP_API BOOL HidpClose(HIDPHANDLE hidpHandle);

LIBHIDP_API HRESULT HidpSubmitReport(HIDPHANDLE hidpHandle, UCHAR reportId, SIZE_T reportSize, PBYTE reportData);