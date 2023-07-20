#ifndef SPI_CPU_H_
#define SPI_CPU_H_

/* 헤더 파일 포함 */
#include "Ifx_Types.h"
#include "IfxQspi_SpiMaster.h"
#include "IfxQspi_SpiSlave.h"

/* 매크로 정의 */
#define SPI_BUFFER_SIZE     5                       /* 버퍼 크기                  */

/* 데이터 구조체 */
typedef struct
{
    uint8 spiMasterTxBuffer[SPI_BUFFER_SIZE];       /* QSPI 마스터 전송 버퍼   */
    uint8 spiMasterRxBuffer[SPI_BUFFER_SIZE];       /* QSPI 마스터 수신 버퍼   */
    uint8 spiSlaveTxBuffer[SPI_BUFFER_SIZE];        /* QSPI 슬레이브 전송 버퍼 */
    uint8 spiSlaveRxBuffer[SPI_BUFFER_SIZE];        /* QSPI 슬레이브 수신 버퍼 */
} qspiBuffers;

typedef struct
{
    qspiBuffers               spiBuffers;           /* 버퍼 인스턴스              */
    IfxQspi_SpiMaster         spiMaster;            /* QSPI 마스터 핸들           */
    IfxQspi_SpiMaster_Channel spiMasterChannel;     /* QSPI 마스터 채널 핸들      */
    IfxQspi_SpiSlave          spiSlave;             /* QSPI 슬레이브 핸들         */
} qspiComm;

/* 함수 프로토타입 */
void initPeripherals(void);
void transferData(void);

#endif /* SPI_CPU_H_ */
