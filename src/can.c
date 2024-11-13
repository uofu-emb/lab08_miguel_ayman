#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "pico/time.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

static struct can2040 cbus;

QueueHandle_t msgs;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    xQueueSendToBack(msgs, msg, portMAX_DELAY);
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5;
    
    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
    irq_set_enabled(PIO0_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
    
    printf("setup completed\n");
}

// Function to send the first CAN message
void first_send() {
    // Prepare message content
    char *my_msg = "Hello";
    struct can2040_msg msg;
    msg.id = 0x123;          // Example CAN ID
    msg.dlc = 5;             // Data length (5 bytes for "Hello")

    // Copy data to msg.data
    for (int i = 0; i < msg.dlc; i++) {
        msg.data[i] = my_msg[i];
    }
    printf("mesg data = %x", msg.data);
    // Transmit the CAN message
    can2040_transmit(&cbus, &msg);    
    printf("message sent\n");
}


void main_task(__unused void *params)
{
  struct can2040_msg data;
  xQueueReceive(msgs, &data, portMAX_DELAY);
  printf("Got message\n");
}

int main(void)
{
    stdio_init_all();
    printf("Startup delay\n");
    sleep_ms(3000);
    msgs = xQueueCreate(100, sizeof(struct can2040_msg));
    canbus_setup();
    TaskHandle_t task;
    first_send();
    sleep_ms(3000);
    printf("starting reciever thread\n");
    
    xTaskCreate(main_task, "MainThread",
                configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL, &task);
    vTaskStartScheduler();
    return 0;
}
