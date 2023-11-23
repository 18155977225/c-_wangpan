#include "protocol.h"


PDU *mkPDU(uint uiMsgLen)
{
    // 总pdu大小
    uint uiPDULen = sizeof(PDU) + uiMsgLen;
    // 申请空间给pdu
    PDU *pdu = (PDU*)malloc(uiPDULen);
    if (pdu == NULL) {
        exit(EXIT_FAILURE);
    }
    memset(pdu, 0, uiPDULen);
    pdu->uiPDULen = uiPDULen;
    pdu->uiMsgLen = uiMsgLen;
    return pdu;
}
