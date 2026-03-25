/*
 * portable 모듈을 위한 실제 보드/SKD 바인딩 구현부다.
 * generated peripheral 이름을 숨기고,
 * 하드웨어 전용 callback을 모듈 친화적인 인터페이스로 바꾼다.
 */
#include "runtime_io.h"

#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "pins_driver.h"
#include "sdk_project_config.h"

#define RUNTIME_IO_SLAVE1_BUTTON_PIN          12U
#define RUNTIME_IO_SLAVE1_BUTTON_PORT         ((void *)PTC)
#define RUNTIME_IO_SLAVE1_BUTTON_MASK         (1UL << RUNTIME_IO_SLAVE1_BUTTON_PIN)

#define RUNTIME_IO_LIN_XCVR_ENABLE_PIN        9U
#define RUNTIME_IO_LIN_XCVR_ENABLE_PORT       PTE
#define RUNTIME_IO_LIN_XCVR_ENABLE_MASK       (1UL << RUNTIME_IO_LIN_XCVR_ENABLE_PIN)

#define RUNTIME_IO_LED_PORT                   ((void *)PTD)
#define RUNTIME_IO_LED_RED_PIN                15U
#define RUNTIME_IO_LED_GREEN_PIN              16U
#define RUNTIME_IO_LED_ACTIVE_ON              0U

#define RUNTIME_IO_MASTER_LIN_ADC_PID         0x24U
#define RUNTIME_IO_MASTER_LIN_OK_PID          0x25U
#define RUNTIME_IO_MASTER_LIN_OK_TOKEN        0xA5U
#define RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS   100U
#define RUNTIME_IO_ADC_GROUP                  0U

#define RUNTIME_IO_ADC_RANGE_MAX              4096U
#define RUNTIME_IO_ADC_SAFE_MAX               (RUNTIME_IO_ADC_RANGE_MAX / 3U)
#define RUNTIME_IO_ADC_WARNING_MAX            ((RUNTIME_IO_ADC_RANGE_MAX * 2U) / 3U)
#define RUNTIME_IO_ADC_EMERGENCY_MIN          ((RUNTIME_IO_ADC_RANGE_MAX * 5U) / 6U)

/*
 * runtime IO 계층이 캐시하는 ADC 바인딩 상태다.
 * portable ADC 모듈은 이것을 opaque context로만 보고,
 * generated SDK 설정 세부사항은 runtime_io가 소유한다.
 */
typedef struct
{
    uint8_t             initialized;
#ifdef INST_ADC_CONFIG_1
    adc_chan_config_t   chan_config;
#endif
} RuntimeIoAdcContext;

/*
 * generated driver 계층과 공유하는 LIN 바인딩 상태다.
 * 활성 노드 역할과 timeout 설정을 기억하고,
 * portable LIN 모듈은 추상 callback만 사용하도록 유지한다.
 */
typedef struct
{
    uint8_t  initialized;
    uint8_t  role;
    uint16_t timeout_ticks;
} RuntimeIoLinContext;

/*
 * 현재 LIN callback이 전달될 대상 모듈 포인터다.
 * 각 펌웨어 이미지는 동시에 하나의 LIN 인스턴스만 가지므로,
 * 보드 callback도 모듈 포인터 하나만 저장한다.
 */
static LinModule *g_runtime_io_lin_module;
static RuntimeIoAdcContext g_runtime_io_adc_context;
static RuntimeIoLinContext g_runtime_io_lin_context;

/*
 * SDK 전용 상태 코드를 공통 InfraStatus로 변환한다.
 * 이렇게 하면 상위 계층이 vendor enum에 의존하지 않고,
 * 여러 모듈에서 오류 처리를 단순하게 유지할 수 있다.
 */
static InfraStatus RuntimeIo_StatusFromSdk(status_t status)
{
    return (status == STATUS_SUCCESS) ? INFRA_STATUS_OK : INFRA_STATUS_IO_ERROR;
}

#ifdef INST_LIN2

/*
 * generated LIN driver event를 모듈 event로 번역한다.
 * 이 callback은 runtime_io 안에 남겨두어,
 * portable LIN 로직이 vendor 전용 payload를 직접 해석하지 않게 한다.
 */
static void RuntimeIo_LinSdkCallback(uint32_t instance, void *lin_state_ptr)
{
    lin_state_t *lin_state;
    LinEventId   event_id;
    uint8_t      current_pid;

    (void)instance;

    lin_state = (lin_state_t *)lin_state_ptr;
    if (lin_state == NULL)
    {
        return;
    }

    current_pid = lin_state->currentId;
    if (lin_state->timeoutCounterFlag != false)
    {
        lin_state->timeoutCounterFlag = false;
        RuntimeIo_LinNotifyEvent(LIN_EVENT_ERROR, current_pid);
        return;
    }

    if (lin_state->currentEventId == LIN_RECV_BREAK_FIELD_OK)
    {
        if (g_runtime_io_lin_context.initialized != 0U)
        {
            LIN_DRV_SetTimeoutCounter(INST_LIN2, g_runtime_io_lin_context.timeout_ticks);
        }
        return;
    }

    event_id = LIN_EVENT_NONE;
    switch (lin_state->currentEventId)
    {
        case LIN_PID_OK:
            event_id = LIN_EVENT_PID_OK;
            break;

        case LIN_RX_COMPLETED:
            event_id = LIN_EVENT_RX_DONE;
            break;

        case LIN_TX_COMPLETED:
            event_id = LIN_EVENT_TX_DONE;
            break;

        case LIN_PID_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_RX_OVERRUN:
        case LIN_SYNC_ERROR:
            event_id = LIN_EVENT_ERROR;
            break;

        default:
            break;
    }

    if (event_id != LIN_EVENT_NONE)
    {
        RuntimeIo_LinNotifyEvent(event_id, current_pid);
    }
}

#endif

#ifdef INST_ADC_CONFIG_1

/*
 * generated ADC 인스턴스를 portable sampling에 맞게 준비한다.
 * runtime IO가 보드 전용 converter 설정을 한 번 수행하고,
 * 그 결과 context를 generic ADC 모듈에 넘겨준다.
 */
static InfraStatus RuntimeIo_AdcInitSdk(void *context)
{
    RuntimeIoAdcContext *adc_context;

    adc_context = (RuntimeIoAdcContext *)context;
    if (adc_context == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(adc_context, 0, sizeof(*adc_context));
    adc_context->chan_config = adc_config_1_ChnConfig0;
    adc_context->chan_config.interruptEnable = false;

    ADC_DRV_ConfigConverter(INST_ADC_CONFIG_1, &adc_config_1_ConvConfig0);
    ADC_DRV_AutoCalibration(INST_ADC_CONFIG_1);
    adc_context->initialized = 1U;
    return INFRA_STATUS_OK;
}

static InfraStatus RuntimeIo_AdcSampleSdk(void *context, uint16_t *out_raw_value)
{
    RuntimeIoAdcContext *adc_context;
    uint16_t             raw_value;

    adc_context = (RuntimeIoAdcContext *)context;
    if ((adc_context == NULL) || (out_raw_value == NULL))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (adc_context->initialized == 0U)
    {
        return INFRA_STATUS_NOT_READY;
    }

    raw_value = 0U;
    ADC_DRV_ConfigChan(INST_ADC_CONFIG_1, RUNTIME_IO_ADC_GROUP, &adc_context->chan_config);
    ADC_DRV_WaitConvDone(INST_ADC_CONFIG_1);
    ADC_DRV_GetChanResult(INST_ADC_CONFIG_1, RUNTIME_IO_ADC_GROUP, &raw_value);

    if (raw_value > 4095U)
    {
        raw_value = 4095U;
    }

    *out_raw_value = raw_value;
    return INFRA_STATUS_OK;
}

#else

static InfraStatus RuntimeIo_AdcInitSdk(void *context)
{
    (void)context;
    return INFRA_STATUS_UNSUPPORTED;
}

static InfraStatus RuntimeIo_AdcSampleSdk(void *context, uint16_t *out_raw_value)
{
    (void)context;

    if (out_raw_value != NULL)
    {
        *out_raw_value = 0U;
    }

    return INFRA_STATUS_UNSUPPORTED;
}

#endif

#ifdef INST_LIN2

/*
 * 선택된 역할에 맞게 generated LIN peripheral을 초기화한다.
 * master와 slave 이미지가 하나의 portable 모듈을 공유하므로,
 * 실제 SDK 모드 선택은 보드 바인딩이 여기서 담당한다.
 */
static InfraStatus RuntimeIo_LinInitSdk(void *context)
{
    RuntimeIoLinContext *lin_context;
    status_t             status;

    lin_context = (RuntimeIoLinContext *)context;
    if (lin_context == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    if (lin_context->role == LIN_ROLE_MASTER)
    {
        lin2_InitConfig0.nodeFunction = (bool)MASTER;
        lin2_InitConfig0.autobaudEnable = false;
    }
    else
    {
        lin2_InitConfig0.nodeFunction = (bool)SLAVE;
        lin2_InitConfig0.autobaudEnable = false;
    }

    status = LIN_DRV_Init(INST_LIN2, &lin2_InitConfig0, &lin2_State);
    if (status != STATUS_SUCCESS)
    {
        return RuntimeIo_StatusFromSdk(status);
    }

    (void)LIN_DRV_InstallCallback(INST_LIN2, RuntimeIo_LinSdkCallback);
    lin_context->initialized = 1U;
    return INFRA_STATUS_OK;
}

static InfraStatus RuntimeIo_LinMasterSendHeaderSdk(void *context, uint8_t pid)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return INFRA_STATUS_NOT_READY;
    }

    return RuntimeIo_StatusFromSdk(LIN_DRV_MasterSendHeader(INST_LIN2, pid));
}

static InfraStatus RuntimeIo_LinStartReceiveSdk(void *context, uint8_t *buffer, uint8_t length)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return RuntimeIo_StatusFromSdk(LIN_DRV_ReceiveFrameData(INST_LIN2, buffer, length));
}

static InfraStatus RuntimeIo_LinStartSendSdk(void *context, const uint8_t *buffer, uint8_t length)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U) ||
        (buffer == NULL) || (length == 0U))
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    return RuntimeIo_StatusFromSdk(LIN_DRV_SendFrameData(INST_LIN2, buffer, length));
}

static void RuntimeIo_LinGotoIdleSdk(void *context)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    (void)LIN_DRV_GotoIdleState(INST_LIN2);
}

static void RuntimeIo_LinSetTimeoutSdk(void *context, uint16_t timeout_ticks)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    lin_context->timeout_ticks = timeout_ticks;
    LIN_DRV_SetTimeoutCounter(INST_LIN2, timeout_ticks);
}

static void RuntimeIo_LinServiceTickSdk(void *context)
{
    RuntimeIoLinContext *lin_context;

    lin_context = (RuntimeIoLinContext *)context;
    if ((lin_context == NULL) || (lin_context->initialized == 0U))
    {
        return;
    }

    LIN_DRV_TimeoutService(INST_LIN2);
}

#else

static InfraStatus RuntimeIo_LinInitSdk(void *context)
{
    (void)context;
    return INFRA_STATUS_UNSUPPORTED;
}

static InfraStatus RuntimeIo_LinMasterSendHeaderSdk(void *context, uint8_t pid)
{
    (void)context;
    (void)pid;
    return INFRA_STATUS_UNSUPPORTED;
}

static InfraStatus RuntimeIo_LinStartReceiveSdk(void *context, uint8_t *buffer, uint8_t length)
{
    (void)context;
    (void)buffer;
    (void)length;
    return INFRA_STATUS_UNSUPPORTED;
}

static InfraStatus RuntimeIo_LinStartSendSdk(void *context, const uint8_t *buffer, uint8_t length)
{
    (void)context;
    (void)buffer;
    (void)length;
    return INFRA_STATUS_UNSUPPORTED;
}

static void RuntimeIo_LinGotoIdleSdk(void *context)
{
    (void)context;
}

static void RuntimeIo_LinSetTimeoutSdk(void *context, uint16_t timeout_ticks)
{
    (void)context;
    (void)timeout_ticks;
}

static void RuntimeIo_LinServiceTickSdk(void *context)
{
    (void)context;
}

#endif

/*
 * 상위 계층이 시작되기 전에 보드 전체 자원을 초기화한다.
 * clock, pin muxing, LIN transceiver enable을 여기서 처리하여,
 * 이후 모듈이 보드 사용 가능 상태를 전제로 동작하게 한다.
 */
InfraStatus RuntimeIo_BoardInit(void)
{
    status_t status;

    status = CLOCK_SYS_Init(g_clockManConfigsArr,
                            CLOCK_MANAGER_CONFIG_CNT,
                            g_clockManCallbacksArr,
                            CLOCK_MANAGER_CALLBACK_CNT);
    if (status != STATUS_SUCCESS)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    status = CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
    if (status != STATUS_SUCCESS)
    {
        return INFRA_STATUS_IO_ERROR;
    }

    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

    /*
     * 하드웨어 바인딩 확인:
     * 이후 pin_mux 에 LIN 트랜시버 enable 핀이 추가되면,
     * enable 제어 코드는 이 위치에만 모아 두는 것이 좋다.
     */
#ifdef INST_LIN2
    PINS_DRV_SetPinsDirection(RUNTIME_IO_LIN_XCVR_ENABLE_PORT, RUNTIME_IO_LIN_XCVR_ENABLE_MASK);
    PINS_DRV_SetPins(RUNTIME_IO_LIN_XCVR_ENABLE_PORT, RUNTIME_IO_LIN_XCVR_ENABLE_MASK);
#endif

    return INFRA_STATUS_OK;
}

/*
 * 현재 LIN 모듈을 callback 목적지로 등록한다.
 * 이렇게 한 단계 우회하면 SDK callback은 전역으로 유지하면서도,
 * portable 모듈은 역할별 event를 정상적으로 받을 수 있다.
 */
void RuntimeIo_AttachLinModule(LinModule *module)
{
    g_runtime_io_lin_module = module;
}

void RuntimeIo_LinNotifyEvent(LinEventId event_id, uint8_t current_pid)
{
    if (g_runtime_io_lin_module == NULL)
    {
        return;
    }

    LinModule_OnEvent(g_runtime_io_lin_module, event_id, current_pid);
}

/*
 * 이 이미지가 어떤 역할로 동작해야 하는지 보고한다.
 * AppCore는 startup 시 이 값을 보고,
 * 어떤 policy 모듈과 하드웨어 바인딩을 초기화할지 선택한다.
 */
uint8_t RuntimeIo_GetActiveRole(void)
{
    return APP_ACTIVE_ROLE;
}

uint8_t RuntimeIo_GetLocalNodeId(void)
{
    switch (APP_ACTIVE_ROLE)
    {
        case APP_ROLE_MASTER:
            return APP_NODE_ID_MASTER;

        case APP_ROLE_SLAVE1:
            return APP_NODE_ID_SLAVE1;

        case APP_ROLE_SLAVE2:
            return APP_NODE_ID_SLAVE2;

        default:
            return APP_NODE_ID_MASTER;
    }
}

/*
 * 현장 반응 노드용 LED 배선 정보를 제공한다.
 * LED 모듈은 보드 전용 상수를 몰라도,
 * 의미 기반 pin/polarity 설정만 받아 동작할 수 있다.
 */
InfraStatus RuntimeIo_GetSlave1LedConfig(LedConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    out_config->gpio_port = RUNTIME_IO_LED_PORT;
    out_config->red_pin = RUNTIME_IO_LED_RED_PIN;
    out_config->green_pin = RUNTIME_IO_LED_GREEN_PIN;
    out_config->active_on_level = RUNTIME_IO_LED_ACTIVE_ON;
    return INFRA_STATUS_OK;
}

InfraStatus RuntimeIo_GetSlave2LedConfig(LedConfig *out_config)
{
    return RuntimeIo_GetSlave1LedConfig(out_config);
}

uint8_t RuntimeIo_ReadSlave1ButtonPressed(void)
{
    GPIO_Type *gpio_port;

    gpio_port = (GPIO_Type *)RUNTIME_IO_SLAVE1_BUTTON_PORT;
    return ((PINS_DRV_ReadPins(gpio_port) & RUNTIME_IO_SLAVE1_BUTTON_MASK) == 0U) ? 1U : 0U;
}

/*
 * portable ADC 모듈이 사용할 설정 구조를 만든다.
 * threshold와 sample period, 보드 callback을 여기서 묶어,
 * app 계층은 하나의 opaque config만 보게 만든다.
 */
InfraStatus RuntimeIo_GetSlave2AdcConfig(AdcConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    out_config->init_fn = RuntimeIo_AdcInitSdk;
    out_config->sample_fn = RuntimeIo_AdcSampleSdk;
    out_config->hw_context = &g_runtime_io_adc_context;
    out_config->sample_period_ms = APP_TASK_ADC_MS;
    out_config->range_max = RUNTIME_IO_ADC_RANGE_MAX;
    out_config->safe_max = RUNTIME_IO_ADC_SAFE_MAX;
    out_config->warning_max = RUNTIME_IO_ADC_WARNING_MAX;
    out_config->emergency_min = RUNTIME_IO_ADC_EMERGENCY_MIN;
    out_config->blocking_mode = 1U;
#ifdef INST_ADC_CONFIG_1
    return INFRA_STATUS_OK;
#else
    return INFRA_STATUS_UNSUPPORTED;
#endif
}

/*
 * coordinator 노드용 LIN master 설정을 만든다.
 * protocol PID와 보드 callback을 미리 묶어서,
 * portable LIN 상태기계 초기화 전에 넘겨준다.
 */
InfraStatus RuntimeIo_GetMasterLinConfig(LinConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    (void)memset(&g_runtime_io_lin_context, 0, sizeof(g_runtime_io_lin_context));
    out_config->role = LIN_ROLE_MASTER;
    out_config->pid_status = RUNTIME_IO_MASTER_LIN_ADC_PID;
    out_config->pid_ok = RUNTIME_IO_MASTER_LIN_OK_PID;
    out_config->ok_token = RUNTIME_IO_MASTER_LIN_OK_TOKEN;
    out_config->status_frame_size = 8U;
    out_config->ok_frame_size = 1U;
    out_config->timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->poll_period_ms = APP_TASK_LIN_POLL_MS;
    g_runtime_io_lin_context.role = LIN_ROLE_MASTER;
    g_runtime_io_lin_context.timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->binding.init_fn = RuntimeIo_LinInitSdk;
    out_config->binding.master_send_header_fn = RuntimeIo_LinMasterSendHeaderSdk;
    out_config->binding.start_receive_fn = RuntimeIo_LinStartReceiveSdk;
    out_config->binding.start_send_fn = RuntimeIo_LinStartSendSdk;
    out_config->binding.goto_idle_fn = RuntimeIo_LinGotoIdleSdk;
    out_config->binding.set_timeout_fn = RuntimeIo_LinSetTimeoutSdk;
    out_config->binding.service_tick_fn = RuntimeIo_LinServiceTickSdk;
    out_config->binding.context = &g_runtime_io_lin_context;
#ifdef INST_LIN2
    return INFRA_STATUS_OK;
#else
    return INFRA_STATUS_UNSUPPORTED;
#endif
}

/*
 * 센서 노드용 LIN slave 설정을 만든다.
 * portable LIN 모듈은 같은 protocol 필드를 재사용하지만,
 * callback을 통해 slave 전용 SDK 동작을 전달받는다.
 */
InfraStatus RuntimeIo_GetSlaveLinConfig(LinConfig *out_config)
{
    if (out_config == NULL)
    {
        return INFRA_STATUS_INVALID_ARG;
    }

    (void)memset(out_config, 0, sizeof(*out_config));
    (void)memset(&g_runtime_io_lin_context, 0, sizeof(g_runtime_io_lin_context));
    out_config->role = LIN_ROLE_SLAVE;
    out_config->pid_status = RUNTIME_IO_MASTER_LIN_ADC_PID;
    out_config->pid_ok = RUNTIME_IO_MASTER_LIN_OK_PID;
    out_config->ok_token = RUNTIME_IO_MASTER_LIN_OK_TOKEN;
    out_config->status_frame_size = 8U;
    out_config->ok_frame_size = 1U;
    out_config->timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->poll_period_ms = APP_TASK_LIN_POLL_MS;
    g_runtime_io_lin_context.role = LIN_ROLE_SLAVE;
    g_runtime_io_lin_context.timeout_ticks = RUNTIME_IO_MASTER_LIN_TIMEOUT_TICKS;
    out_config->binding.init_fn = RuntimeIo_LinInitSdk;
    out_config->binding.master_send_header_fn = RuntimeIo_LinMasterSendHeaderSdk;
    out_config->binding.start_receive_fn = RuntimeIo_LinStartReceiveSdk;
    out_config->binding.start_send_fn = RuntimeIo_LinStartSendSdk;
    out_config->binding.goto_idle_fn = RuntimeIo_LinGotoIdleSdk;
    out_config->binding.set_timeout_fn = RuntimeIo_LinSetTimeoutSdk;
    out_config->binding.service_tick_fn = RuntimeIo_LinServiceTickSdk;
    out_config->binding.context = &g_runtime_io_lin_context;
#ifdef INST_LIN2
    return INFRA_STATUS_OK;
#else
    return INFRA_STATUS_UNSUPPORTED;
#endif
}
