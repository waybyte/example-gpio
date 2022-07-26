/**
 * @file main.c
 * @brief Exmaple application GPIO
 * @version 0.1
 * @date 2022-07-25
 * 
 * @copyright Copyright (c) 2022 Waybyte Solutions
 * 
 */

#include <stdio.h>
#include <unistd.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>
#include <hw/gpio.h>

#ifdef SOC_RDA8910
#define STDIO_PORT "/dev/ttyUSB0"
#define GPIO_LED GPIO_13
#define GPIO_IN GPIO_1
#else
#define STDIO_PORT "/dev/ttyS0"
#define GPIO_LED GPIO_0
#define GPIO_IN GPIO_1
#endif

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((struct ril_callinfo_t *)param2)->number);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

static void gpio_callback(int handle, int pin, int level)
{
	printf("GPIO %d state changed: %d\n", pin, level);
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	int iohandle, iohandle_input;

	/*
	 * Initialize library and Setup STDIO
	 */
	logicrom_init(STDIO_PORT, urc_callback);

	printf("System Ready\n");

	/* GPIO Output */
	iohandle = gpio_request(GPIO_LED, GPIO_FLAG_OUTPUT | GPIO_FLAG_DEFHIGH);
	if (iohandle == 0)
		printf("GPIO %d request fail\n", GPIO_LED);
	else
		printf("GPIO %d handle: %d\n", GPIO_LED, iohandle);

	/* GPIO Input */
	iohandle_input = gpio_request(GPIO_IN, GPIO_FLAG_INPUT | GPIO_FLAG_PULLUP);
	if (iohandle_input) {
		printf("GPIO %d input handle: %d\n", GPIO_IN, iohandle_input);
		printf("GPIO current state: %d\n", gpio_read(iohandle_input));
		/* setup input trigger on both edges and 100ms debounce */
		gpio_trigger_enable(iohandle_input, gpio_callback, 100, GPIO_TRIG_BOTH);
	} else {
		printf("GPIO %d input request fail\n", GPIO_IN);
	}

	printf("System Initialization finished\n");

	while (1) {
		/* Toggle GPIO */
		gpio_write(iohandle, GPIO_LEVEL_LOW);
		os_task_sleep(500);
		gpio_write(iohandle, GPIO_LEVEL_HIGH);
		os_task_sleep(500);
	}
}

