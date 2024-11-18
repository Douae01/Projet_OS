#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "fsl_pint.h"
#include "fsl_inputmux.h"
#include "fsl_usart.h"
#include "vfs.h"
#include "target.h"

/***************************************************************************
 * external symbols
 ***************************************************************************/
extern FileObject * opened_fds[];	/* device/vfs.c  */

/***************************************************************************
 * test device driver
 ***************************************************************************/
static int dev_init_test(Device *dev);
static int dev_open_test(FileObject *f);
static int dev_close_test(FileObject *f);
static int dev_read_test(FileObject *f, void *buf, size_t len);

Device dev_test={
    .name="test",
    .refcnt=0,
    .sem_read=NULL,
    .sem_write=NULL,
    .init=dev_init_test,
    .open=dev_open_test,
    .close=dev_close_test,
    .read=dev_read_test,
    .write=NULL,
    .ioctl=NULL
};

static int dev_init_test(Device *dev)
{
    dev->mutex=sem_new(1);
    if (dev->mutex) return 1;
    return 0;
}

static int dev_open_test(FileObject *f)
{
	sem_p(f->dev->mutex);
    if (f->flags & (O_READ)) {
        f->dev->refcnt++;
        sem_v(f->dev->mutex);
        return 1;
    }
    sem_v(f->dev->mutex);
    return 0;
}

static int dev_close_test(FileObject *f)
{
	sem_p(f->dev->mutex);
    f->dev->refcnt--;
    sem_v(f->dev->mutex);
    return 1;
}

static int dev_read_test(FileObject *f, void *buf, size_t len)
{
	int n;
	char *file="ceci est un test\r\n";
	sem_p(f->dev->mutex);
	// calculate max available readable data length
	n=f->offset<strlen(file) ? strlen(file)-f->offset : 0;
	// actually, we have to read the min from available length and expected length
	n=n<(int)len ? n : (int)len;
	memcpy(buf, file+f->offset, n);
	f->offset+=n;
	sem_v(f->dev->mutex);
	return n;
}

/***************************************************************************
 * leds device driver
 ***************************************************************************/
static void leds(uint32_t val)
{
	// bit 0 of val controls LED RED
	GPIO_PinWrite(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN, (~(val>>0))&1);
	//  bit 1 of val controls LED GREEN
	GPIO_PinWrite(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN, (~(val>>1))&1);
	//  bit 2 of val controls LED BLUE
	GPIO_PinWrite(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN, (~(val>>2))&1);
}

static int dev_init_leds(Device *dev);
static int dev_open_leds(FileObject *f);
static int dev_close_leds(FileObject *f);
static int dev_write_leds(FileObject *f, void *buf, size_t len);

Device dev_leds={
    .name="leds",
    .refcnt=0,
    .init=dev_init_leds,
    
	/* A COMPLETER */

	.sem_read=NULL,
	.sem_write=NULL,
	.open=dev_open_leds,
	.close=dev_close_leds,
	.read=NULL,
	.write=dev_write_leds,
	.ioctl=NULL
};

static int dev_init_leds(Device *dev)
{
	// leds init
	gpio_pin_config_t ledcfg = { kGPIO_DigitalOutput, 1};
	GPIO_PortInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT);
	GPIO_PinInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN,&ledcfg);

	leds(0);

	/* A COMPLETER */
	dev->mutex = sem_new(1);//create r/w mutex
	return 1;
}

static int dev_open_leds(FileObject *f)
{
	sem_p(f->dev->mutex);
    if (f->dev->refcnt || (f->flags & (O_READ|O_NONBLOCK|O_APPEND|O_SHLOCK|O_EXLOCK|O_ASYNC|O_SYNC|O_CREAT|O_TRUNC|O_EXCL)))
        goto err;
    if (f->flags & O_WRITE) {
        f->dev->refcnt++;
        sem_v(f->dev->mutex);
        return 1;
    }
err:
	sem_v(f->dev->mutex);
    return 0;
}

static int dev_close_leds(FileObject *f)
{
	sem_p(f->dev->mutex);
    f->dev->refcnt--;
    sem_v(f->dev->mutex);
    return 1;
}

static int dev_write_leds(FileObject *f, void *buf, size_t len)
{
	/* A COMPLETER */
	sem_p(f->dev->mutex);
	leds(*(uint32_t*)buf);
	sem_v(f->dev->mutex);
    return 1;
}

/***************************************************************************
 * SWCenter External Interrupt Button device driver
 ***************************************************************************/
static int dev_init_btn(Device *dev);
static int dev_open_btn(FileObject *f);
static int dev_close_btn(FileObject *f);
static int dev_read_btn(FileObject *f, void *buf, size_t len);

Device dev_swuser={
    .name="swuser",
    .refcnt=0,
    .init=dev_init_btn,
    
	/* A COMPLETER */
	.open = dev_open_btn,
	.close = dev_close_btn,
	.read = dev_read_btn,
	.write = NULL,
	.sem_read = NULL,
	.sem_write = NULL,
	.ioctl = NULL
};

/*
 *  ISR callback (IRQ mode !!)
 */
static void on_swuser_cb(pint_pin_int_t pintr, uint32_t pmatch_status)
{
	/* A COMPLETER */
	sem_v(dev_swuser.sem_read);
}

static int dev_init_btn(Device *dev)
{
    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, kINPUTMUX_GpioPort1Pin9ToPintsel);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);
    /* Initialize PINT */
    PINT_Init(PINT);
    /* Setup Pin Interrupt 0 for rising edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, on_swuser_cb);
    NVIC_SetPriority(PIN_INT0_IRQn,10);
	/* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);

	/* A COMPLETER */
    dev->sem_read = sem_new(1);// init pour lu sem
    dev->mutex = sem_new(1); //init pour deblock

    return 1;
}

static int dev_open_btn(FileObject *f)
{
	sem_p(f->dev->mutex);
    if (f->dev->refcnt || (f->flags & (O_WRITE|O_NONBLOCK|O_APPEND|O_SHLOCK|O_EXLOCK|O_ASYNC|O_SYNC|O_CREAT|O_TRUNC|O_EXCL)))
        goto err;
    if (f->flags & O_READ) {
        f->dev->refcnt++;
		sem_v(f->dev->mutex);
        return 1;
    }
err:
	sem_v(f->dev->mutex);
    return 0;
}

static int dev_close_btn(FileObject *f)
{
	sem_p(f->dev->mutex);
    f->dev->refcnt--;
	sem_v(f->dev->mutex);
    return 1;
}

static int dev_read_btn(FileObject *f, void *buf, size_t len)
{
	/* A COMPLETER */
	sem_p(f->dev->sem_read);
    return 4;
}

/***************************************************************************
 * Serial Port Device Driver
 ***************************************************************************/

static int dev_init_ser(Device *dev);
static int dev_open_ser(FileObject *f);
static int dev_close_ser(FileObject *f);
static int dev_read_ser(FileObject *fileObj, void *buffer, size_t length);
static int dev_write_ser(FileObject *fileObj, const void *buffer, size_t length);

Device dev_serial = {
    .name = "serial",
    .refcnt = 0,
    .init = dev_init_ser,

	/* A COMPLETER */
    .open = dev_open_ser,
    .close = dev_close_ser,
    .read = dev_read_ser,
    .write = dev_write_ser,
    .sem_read = NULL,
    .sem_write = NULL,
    .ioctl = NULL
};

#define RING_BUF_SIZE	32

typedef volatile struct RingBuffer {
	char data[RING_BUF_SIZE];
	int	i_w;
	int i_r;
} RingBuffer;

RingBuffer rxbuf = {
	.i_w=0,
	.i_r=0
};

RingBuffer txbuf = {
	.i_w=0,
	.i_r=0
};

volatile int user_break=0;

// USART Interrupt Service Routine
void FLEXCOMM0_IRQHandler()
{
    uint32_t status = USART_GetStatusFlags(USART0);

    // Handle RX errors
    if ((USART0->FIFOSTAT & USART_FIFOSTAT_RXERR_MASK) != 0U) {
        USART0->FIFOSTAT |= USART_FIFOSTAT_RXERR_MASK;
        USART0->FIFOCFG |= USART_FIFOCFG_EMPTYRX_MASK;  // Clear RX FIFO
    } else if (((rxbuf.i_w + 1) % RING_BUF_SIZE) != rxbuf.i_r) {
        // Receive data into RX buffer
        while ((status & kUSART_RxFifoNotEmptyFlag) && (((rxbuf.i_w + 1) % RING_BUF_SIZE) != rxbuf.i_r)) {
            rxbuf.data[rxbuf.i_w] = USART0->FIFORD;
            rxbuf.i_w = (rxbuf.i_w + 1) % RING_BUF_SIZE;
            status = USART_GetStatusFlags(USART0);
            user_break = 1;
        }
    } else {
        USART_ReadByte(USART0);  // Discard the byte if buffer is full
    }

    // Transmit data from TX buffer
    while ((status & kUSART_TxFifoNotFullFlag) && (txbuf.i_r != txbuf.i_w)) {
        USART0->FIFOWR = txbuf.data[txbuf.i_r];
        txbuf.i_r = (txbuf.i_r + 1) % RING_BUF_SIZE;
        status = USART_GetStatusFlags(USART0);

        if (txbuf.i_r == txbuf.i_w) {
            USART_DisableInterrupts(USART0, kUSART_TxLevelInterruptEnable);
        }
    }
}

// USART Initialization Function
void uart_init(USART_Type *base, uint32_t baudrate)
{
    usart_config_t config;
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = baudrate;
    config.enableTx = true;
    config.enableRx = true;

    USART_Init(base, &config, CLOCK_GetFlexCommClkFreq(0U));

    // Enable RX interrupt
    USART_EnableInterrupts(base, kUSART_RxLevelInterruptEnable);
    NVIC_SetPriority(FLEXCOMM0_IRQn, 3);
    NVIC_EnableIRQ(FLEXCOMM0_IRQn);
}

// Initialize serial device
static int dev_init_ser(Device *dev)
{
	uart_init(USART0,115200U);
    dev->mutex=sem_new(1);
    if (dev->mutex)
       	return 1;
    return 0;
}

// Open the serial device
static int dev_open_ser(FileObject *f)
{
	sem_p(f->dev->mutex);
	    if (f->dev->refcnt || (f->flags & (O_NONBLOCK|O_APPEND|O_SHLOCK|O_EXLOCK|O_ASYNC|O_SYNC|O_CREAT|O_TRUNC|O_EXCL)))
	        goto err;
	    if (f->flags & (O_READ | O_WRITE)) {
	        f->dev->refcnt++;
			sem_v(f->dev->mutex);
	        return 1;
	    }
	err:
		sem_v(f->dev->mutex);
	    return 0;
}

// Close the serial device
static int dev_close_ser(FileObject *f)
{
    sem_p(f->dev->mutex);
    f->dev->refcnt--;
    sem_v(f->dev->mutex);
    return 1;
}

// Read from serial device using ring buffer
static int dev_read_ser(FileObject *fileObj, void *buffer, size_t length)
{
    sem_p(fileObj->dev->mutex);
    size_t bytesRead = 0;
    char *dataBuffer = (char *)buffer;

    while (bytesRead < length && rxbuf.i_r != rxbuf.i_w) {
        dataBuffer[bytesRead] = rxbuf.data[rxbuf.i_r];
        rxbuf.i_r = (rxbuf.i_r + 1) % RING_BUF_SIZE;
        bytesRead++;
    }
    fileObj->offset += bytesRead;
    sem_v(fileObj->dev->mutex);
    return bytesRead;
}

// Write to serial device using ring buffer
static int dev_write_ser(FileObject *fileObj, const void *buffer, size_t length)
{
    sem_p(fileObj->dev->mutex);
    const char *dataBuffer = (const char *)buffer;

    for (size_t i = 0; i < length; i++) {
        while ((txbuf.i_w + 1) % RING_BUF_SIZE == txbuf.i_r);  // Wait if buffer is full
        txbuf.data[txbuf.i_w] = dataBuffer[i];
        txbuf.i_w = (txbuf.i_w + 1) % RING_BUF_SIZE;
    }
    USART_EnableInterrupts(USART0, kUSART_TxLevelInterruptEnable);  // Enable TX interrupt
    sem_v(fileObj->dev->mutex);
    return length;
}


/***************************************************************************
 * Device table
 ***************************************************************************/
Device * device_table[]={
	&dev_test,
	&dev_leds,
    &dev_swuser,
	&dev_serial,
	NULL
};

/*****************************************************************************
 * Target Init
 *****************************************************************************/
extern Semaphore* vfs_mutex;

void dev_init()
{
    int i=0;
    Device *dev=device_table[0];
    while (dev) {
        if (dev->init) dev->init(dev);
        dev=device_table[++i];
    }

    for (int i=0;i<MAX_OPENED_FDS;i++)
        opened_fds[i]=NULL;

    vfs_mutex=sem_new(1);
}
