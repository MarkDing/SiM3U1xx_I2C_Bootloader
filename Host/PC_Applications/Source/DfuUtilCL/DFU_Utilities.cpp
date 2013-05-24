/////////////////////////////////////////////////////////////////////////////
// DFU_Utilities.cpp
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DFU_Utilities.h"
#include "StringUtils.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// Global Function Prototypes
/////////////////////////////////////////////////////////////////////////////

string DecodeDfuCode(DFU_CODE dfuCode)
{
    string code;

    switch (dfuCode)
    {
    case DFU_CODE_SUCCESS:                      code = _T("DFU_CODE_SUCCESS"); break;
    case DFU_CODE_API_INVALID_PARAMETER:        code = _T("DFU_CODE_API_INVALID_PARAMETER"); break;
    case DFU_CODE_API_INVALID_BUFFER_SIZE:      code = _T("DFU_CODE_API_INVALID_BUFFER_SIZE"); break;
    case DFU_CODE_API_INVALID_HEX_RECORD:       code = _T("DFU_CODE_API_INVALID_HEX_RECORD"); break;
    case DFU_CODE_API_NOT_IN_DFU_IDLE_STATE:    code = _T("DFU_CODE_API_NOT_IN_DFU_IDLE_STATE"); break;
    case DFU_CODE_API_DNLOAD_PHASE_FAILED:      code = _T("DFU_CODE_API_DNLOAD_PHASE_FAILED"); break;
    case DFU_CODE_API_MANIFEST_PHASE_FAILED:    code = _T("DFU_CODE_API_MANIFEST_PHASE_FAILED"); break;
    case DFU_CODE_API_INVALID_IMAGE_FILE:       code = _T("DFU_CODE_API_INVALID_IMAGE_FILE"); break;
    case DFU_CODE_API_IMAGE_GENERATION_FAILED:  code = _T("DFU_CODE_API_IMAGE_GENERATION_FAILED"); break;
    case DFU_CODE_API_INVALID_DEVICE_OBJECT:    code = _T("DFU_CODE_API_INVALID_DEVICE_OBJECT"); break;
    case DFU_CODE_API_FATAL_ERROR:              code = _T("DFU_CODE_API_FATAL_ERROR"); break;
    case DFU_CODE_API_ABORTED:                  code = _T("DFU_CODE_API_ABORTED"); break;
    case DFU_CODE_API_NOT_SUPPORTED:            code = _T("DFU_CODE_API_NOT_SUPPORTED"); break;
    case DFU_CODE_HWIF_DEVICE_NOT_FOUND:        code = _T("DFU_CODE_HWIF_DEVICE_NOT_FOUND"); break;
    case DFU_CODE_HWIF_DEVICE_NOT_OPENED:       code = _T("DFU_CODE_HWIF_DEVICE_NOT_OPENED"); break;
    case DFU_CODE_HWIF_DEVICE_ERROR:            code = _T("DFU_CODE_HWIF_DEVICE_ERROR"); break;
    case DFU_CODE_HWIF_TRANSFER_ERROR:          code = _T("DFU_CODE_HWIF_TRANSFER_ERROR"); break;
    case DFU_CODE_HWIF_TRANSFER_TIMEOUT:        code = _T("DFU_CODE_HWIF_TRANSFER_TIMEOUT"); break;
    case DFU_CODE_HWIF_TRANSFER_INCOMPLETE:     code = _T("DFU_CODE_HWIF_TRANSFER_INCOMPLETE"); break;
    default:                                    code = FormatString(_T("Error Code (%d)"), dfuCode);
    }

    return code;
}

string DecodeDfuStatus(DFU_STATUS dfuStatus)
{
    string status;

    switch (dfuStatus)
    {
    case DFU_STATUS_OK:                 status = _T("DFU_STATUS_OK"); break;
    case DFU_STATUS_ERR_TARGET:         status = _T("DFU_STATUS_ERR_TARGET"); break;
    case DFU_STATUS_ERR_FILE:           status = _T("DFU_STATUS_ERR_FILE"); break;
    case DFU_STATUS_ERR_WRITE:          status = _T("DFU_STATUS_ERR_WRITE"); break;
    case DFU_STATUS_ERR_ERASE:          status = _T("DFU_STATUS_ERR_ERASE"); break;
    case DFU_STATUS_ERR_CHECK_ERASED:   status = _T("DFU_STATUS_ERR_CHECK_ERASED"); break;
    case DFU_STATUS_ERR_PROG:           status = _T("DFU_STATUS_ERR_PROG"); break;
    case DFU_STATUS_ERR_VERIFY:         status = _T("DFU_STATUS_ERR_VERIFY"); break;
    case DFU_STATUS_ERR_ADDRESS:        status = _T("DFU_STATUS_ERR_ADDRESS"); break;
    case DFU_STATUS_ERR_NOT_DONE:       status = _T("DFU_STATUS_ERR_NOT_DONE"); break;
    case DFU_STATUS_ERR_FIRMWARE:       status = _T("DFU_STATUS_ERR_FIRMWARE"); break;
    case DFU_STATUS_ERR_VENDOR:         status = _T("DFU_STATUS_ERR_VENDOR"); break;
    case DFU_STATUS_ERR_USBR:           status = _T("DFU_STATUS_ERR_USBR"); break;
    case DFU_STATUS_ERR_POR:            status = _T("DFU_STATUS_ERR_POR"); break;
    case DFU_STATUS_ERR_UNKNOWN:        status = _T("DFU_STATUS_ERR_UNKNOWN"); break;
    case DFU_STATUS_ERR_STALLED_PKT:    status = _T("DFU_STATUS_ERR_STALLED_PKT"); break;
    default:                            status = _T("Unknown Status"); break;
    }

    return status;
}

string DecodeDfuState(DFU_STATE dfuState)
{
    string state;

    switch (dfuState)
    {
    case DFU_STATE_APP_IDLE:                    state = _T("DFU_STATE_APP_IDLE"); break;
    case DFU_STATE_APP_DETACH:                  state = _T("DFU_STATE_APP_DETACH"); break;
    case DFU_STATE_DFU_IDLE:                    state = _T("DFU_STATE_DFU_IDLE"); break;
    case DFU_STATE_DFU_DNLOAD_SYNC:             state = _T("DFU_STATE_DFU_DNLOAD_SYNC"); break;
    case DFU_STATE_DFU_DNBUSY:                  state = _T("DFU_STATE_DFU_DNBUSY"); break;
    case DFU_STATE_DFU_DNLOAD_IDLE:             state = _T("DFU_STATE_DFU_DNLOAD_IDLE"); break;
    case DFU_STATE_DFU_MANIFEST_SYNC:           state = _T("DFU_STATE_DFU_MANIFEST_SYNC"); break;
    case DFU_STATE_DFU_MANIFEST:                state = _T("DFU_STATE_DFU_MANIFEST"); break;
    case DFU_STATE_DFU_MANIFEST_WAIT_RESET:     state = _T("DFU_STATE_DFU_MANIFEST_WAIT_RESET"); break;
    case DFU_STATE_DFU_UPLOAD_IDLE:             state = _T("DFU_STATE_DFU_UPLOAD_IDLE"); break;
    case DFU_STATE_DFU_ERROR:                   state = _T("DFU_STATE_DFU_ERROR"); break;
    default:                                    state = _T("Unknown State"); break;
    }

    return state;
}
