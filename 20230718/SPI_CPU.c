#include "SPI_CPU.h"
#include "IfxPort.h"

/* 매크로 */
#define QSPI1_MASTER                &MODULE_QSPI1   /* QSPI1 마스터 모듈                                           */
#define QSPI3_SLAVE                 &MODULE_QSPI3   /* QSPI3 슬레이브 모듈                                         */

#define LED1                        &MODULE_P00,5   /* LED1 포트, 핀 정의                                          */

#define MASTER_CHANNEL_BAUDRATE     1000000         /* 마스터 채널의 통신 속도 설정                                  */

/* 마스터 및 슬레이브 SPI 통신을 위한 인터럽트 서비스 루틴 우선 순위 */
#define ISR_PRIORITY_MASTER_TX      50
#define ISR_PRIORITY_MASTER_RX      51
#define ISR_PRIORITY_MASTER_ER      52
#define ISR_PRIORITY_SLAVE_TX       53
#define ISR_PRIORITY_SLAVE_RX       54
#define ISR_PRIORITY_SLAVE_ER       55

/* 전역 변수 */
qspiComm g_qspi;

/* 함수 프로토타입 */
void initQSPI1Master(void);
void initQSPI1MasterChannel(void);
void initQSPI1MasterBuffers(void);
void initQSPI3Slave(void);
void initQSPI3SlaveBuffers(void);
void initQSPI(void);
void initLED(void);
void verifyData(void);

/* 함수 구현 */

/* 마스터 데이터 전송용 ISR */
IFX_INTERRUPT(masterTxISR, 0, ISR_PRIORITY_MASTER_TX);
void masterTxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrTransmit(&g_qspi.spiMaster);
}

/* 마스터 데이터 수신용 ISR */
IFX_INTERRUPT(masterRxISR, 0, ISR_PRIORITY_MASTER_RX);
void masterRxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrReceive(&g_qspi.spiMaster);
}

/* 마스터 에러 처리용 ISR */
IFX_INTERRUPT(masterErISR, 0, ISR_PRIORITY_MASTER_ER);
void masterErISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiMaster_isrError(&g_qspi.spiMaster);
}

/* 슬레이브 데이터 전송용 ISR */
IFX_INTERRUPT(slaveTxISR, 0, ISR_PRIORITY_SLAVE_TX);
void slaveTxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrTransmit(&g_qspi.spiSlave);
}

/* 슬레이브 데이터 수신용 ISR */
IFX_INTERRUPT(slaveRxISR, 0, ISR_PRIORITY_SLAVE_RX);
void slaveRxISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrReceive(&g_qspi.spiSlave);
}

/* 슬레이브 에러 처리용 ISR */
IFX_INTERRUPT(slaveErISR, 0, ISR_PRIORITY_SLAVE_ER);
void slaveErISR()
{
    IfxCpu_enableInterrupts();
    IfxQspi_SpiSlave_isrError(&g_qspi.spiSlave);
}

/* QSPI1 마스터 모듈 초기화 함수 */
void initQSPI1Master(void)
{
    IfxQspi_SpiMaster_Config spiMasterConfig;

    IfxQspi_SpiMaster_initModuleConfig(&spiMasterConfig, QSPI1_MASTER);

    spiMasterConfig.base.mode = SpiIf_Mode_master;                      /* 모드를 마스터로 설정                       */

    /* 통신에 사용할 포트 핀 선택 */
    const IfxQspi_SpiMaster_Pins qspi1MasterPins = {
        &IfxQspi1_SCLK_P10_2_OUT, IfxPort_OutputMode_pushPull,          /* SCLK 핀                       (CLK)     */
        &IfxQspi1_MTSR_P10_3_OUT, IfxPort_OutputMode_pushPull,          /* MasterTransmitSlaveReceive 핀 (MOSI)    */
        &IfxQspi1_MRSTA_P10_1_IN, IfxPort_InputMode_pullDown,           /* MasterReceiveSlaveTransmit 핀 (MISO)    */
        IfxPort_PadDriver_cmosAutomotiveSpeed3                          /* 패드 드라이버 모드                      */
    };
    spiMasterConfig.pins = &qspi1MasterPins;                            /* 마스터의 포트 핀 설정                    */

    /* ISR 우선 순위 및 서비스 제공자 설정 */
    spiMasterConfig.base.txPriority = ISR_PRIORITY_MASTER_TX;
    spiMasterConfig.base.rxPriority = ISR_PRIORITY_MASTER_RX;
    spiMasterConfig.base.erPriority = ISR_PRIORITY_MASTER_ER;
    spiMasterConfig.base.isrProvider = IfxSrc_Tos_cpu0;

    /* QSPI 마스터 모듈 초기화 */
    IfxQspi_SpiMaster_initModule(&g_qspi.spiMaster, &spiMasterConfig);
}

/* QSPI1 마스터 채널 초기화 함수 */
void initQSPI1MasterChannel(void)
{
    IfxQspi_SpiMaster_ChannelConfig spiMasterChannelConfig;

    /* 기본 설정으로 초기화 */
    IfxQspi_SpiMaster_initChannelConfig(&spiMasterChannelConfig, &g_qspi.spiMaster);

    spiMasterChannelConfig.base.baudrate = MASTER_CHANNEL_BAUDRATE;     /* SCLK 주파수를 1 MHz로 설정                */

    /* Chip Select 신호를 위한 포트 핀 선택 */
    const IfxQspi_SpiMaster_Output qspi1SlaveSelect = {
        &IfxQspi1_SLSO9_P10_5_OUT, IfxPort_OutputMode_pushPull,         /* Slave Select 포트 핀 (CS)               */
        IfxPort_PadDriver_cmosAutomotiveSpeed1                          /* 패드 드라이버 모드                      */
    };
    spiMasterChannelConfig.sls.output = qspi1SlaveSelect;

    /* QSPI 마스터 채널 초기화 */
    IfxQspi_SpiMaster_initChannel(&g_qspi.spiMasterChannel, &spiMasterChannelConfig);
}

/* QSPI 마스터 SW 버퍼 초기화 함수 */
void initQSPI1MasterBuffers(void)
{
    for (uint8 i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        g_qspi.spiBuffers.spiMasterTxBuffer[i] = (uint8)(i + 1);        /* TX 마스터 버퍼에 패턴으로 데이터 채움     */
        g_qspi.spiBuffers.spiMasterRxBuffer[i] = 0;                     /* RX 버퍼를 0으로 초기화                   */
    }
}

/* QSPI3 슬레이브 모듈 초기화 함수 */
void initQSPI3Slave(void)
{
    IfxQspi_SpiSlave_Config spiSlaveConfig;

    IfxQspi_SpiSlave_initModuleConfig(&spiSlaveConfig, QSPI3_SLAVE);

    /* 통신에 사용할 포트 핀 선택 */
    const IfxQspi_SpiSlave_Pins qspi3SlavePins = {
        &IfxQspi3_SCLKA_P02_7_IN, IfxPort_InputMode_pullDown,           /* SCLK 핀                       (CLK)     */
        &IfxQspi3_MTSRA_P02_6_IN, IfxPort_InputMode_pullDown,           /* MasterTransmitSlaveReceive 핀 (MOSI)    */
        &IfxQspi3_MRST_P02_5_OUT, IfxPort_OutputMode_pushPull,          /* MasterReceiveSlaveTransmit 핀 (MISO)    */
        &IfxQspi3_SLSIA_P02_4_IN, IfxPort_InputMode_pullDown,           /* SlaveSelect 핀                (CS)      */
        IfxPort_PadDriver_cmosAutomotiveSpeed3                          /* 패드 드라이버 모드                      */
    };
    spiSlaveConfig.pins = &qspi3SlavePins;                              /* 슬레이브 포트 핀 설정                   */

    /* ISR 우선 순위 및 서비스 제공자 설정 */
    spiSlaveConfig.base.txPriority = ISR_PRIORITY_SLAVE_TX;
    spiSlaveConfig.base.rxPriority = ISR_PRIORITY_SLAVE_RX;
    spiSlaveConfig.base.erPriority = ISR_PRIORITY_SLAVE_ER;
    spiSlaveConfig.base.isrProvider = IfxSrc_Tos_cpu0;

    /* QSPI 슬레이브 모듈 초기화 */
    IfxQspi_SpiSlave_initModule(&g_qspi.spiSlave, &spiSlaveConfig);
}

/* QSPI 슬레이브 SW 버퍼 초기화 함수 */
void initQSPI3SlaveBuffers(void)
{
    for (uint8 i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        g_qspi.spiBuffers.spiSlaveTxBuffer[i] = 0;                      /* TX 슬레이브 버퍼를 0으로 초기화          */
        g_qspi.spiBuffers.spiSlaveRxBuffer[i] = 0;                      /* RX 슬레이브 버퍼를 0으로 초기화          */
    }
}

/* LED 초기화 함수 */
void initLED(void)
{
    /* LED1이 연결된 포트 핀인 00.5를 출력 push-pull 모드로 설정 */
    IfxPort_setPinModeOutput(LED1, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* LED를 끕니다 (LED는 low-level active입니다) */
    IfxPort_setPinHigh(LED1);
}

/* QSPI 모듈 및 LED 초기화 함수 */
void initQSPI(void)
{
    /* 슬레이브를 먼저 초기화하고 */
    initQSPI3Slave();
    initQSPI3SlaveBuffers();

    /* 마스터를 두번째로 초기화합니다 */
    initQSPI1Master();
    initQSPI1MasterChannel();
    initQSPI1MasterBuffers();
}

/* QSPI 모듈 및 LED 초기화 함수 호출 */
void initPeripherals(void)
{
    initLED();
    initQSPI();
}

/* 데이터 전송 함수 */
void transferData(void)
{
    while(IfxQspi_SpiSlave_getStatus(&g_qspi.spiSlave) == SpiIf_Status_busy ||
            IfxQspi_SpiMaster_getStatus(&g_qspi.spiMasterChannel) == SpiIf_Status_busy)
    {   /* 이전 통신이 완료될 때까지 기다립니다 */
    }

    /* 슬레이브에 정의된 길이의 데이터 스트림을 수신하도록 지시 */
    IfxQspi_SpiSlave_exchange(&g_qspi.spiSlave, NULL_PTR, &g_qspi.spiBuffers.spiSlaveRxBuffer[0], SPI_BUFFER_SIZE);

    /* 마스터를 통해 데이터 스트림을 전송합니다 */
    IfxQspi_SpiMaster_exchange(&g_qspi.spiMasterChannel, &g_qspi.spiBuffers.spiMasterTxBuffer[0], NULL_PTR, SPI_BUFFER_SIZE);

    verifyData();
}

/* 수신된 데이터가 올바른지 확인하는 함수 */
void verifyData(void)
{
    uint32 i;
    uint32 error = 0;

    /* 슬레이브가 모든 데이터를 수신할 때까지 기다립니다 */
    while(IfxQspi_SpiSlave_getStatus(&g_qspi.spiSlave) == SpiIf_Status_busy)
    {
    }

    /* 수신된 데이터와 전송한 데이터를 비교하여 오류를 확인합니다 */
    for(i = 0; i < SPI_BUFFER_SIZE; i++)
    {
        if(g_qspi.spiBuffers.spiSlaveRxBuffer[i] != g_qspi.spiBuffers.spiMasterTxBuffer[i])
        {
            error++;
        }
    }

    /* 오류가 없으면 LED1을 켭니다 (LED는 low-level active입니다) */
    if(error == 0)
    {
        IfxPort_setPinLow(LED1);
    }
}
