#include <stdio.h>
#include "board.h"

#include "oslib.h"

#define MAIN_EX10

/*********************************************************************/
#ifdef MAIN_TEST
int res;

int main()
{
	res = test_add(8,7);

	return 0;
}
#endif
/*********************************************************************/
#ifdef MAIN_EX1
void code1()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
}

void code2()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);

        for (int k=0;k<50000;k++);
    }
}

void code3()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);

        for (int k=0;k<50000;k++);
    }
}


int main()
{
    task_new(code1, 512);   // tache 1
    task_new(code2, 512);   // tache 2
    task_new(code3, 512);   // tache 3

    os_start();

    return 0;
}
#endif
/*********************************************************************/
#ifdef MAIN_EX2

void code()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
}

int main()
{
    task_new(code, 512);   // tache 1
    task_new(code, 512);   // tache 2
    task_new(code, 512);   // tache 3

    os_start();

    return 0;
}
#endif
/*********************************************************************/
#ifdef MAIN_EX3

Semaphore* sem=NULL;
int cpt=0;

int sw_user()
{
    static uint16_t btn1_state=0xFFFF;
    btn1_state = (btn1_state<<1) | (GPIO_PinRead(BOARD_SW3_GPIO,BOARD_SW3_GPIO_PORT,BOARD_SW3_GPIO_PIN) & 1) | 0xE000;
    if (btn1_state==0xF000) return 1;
    return 0;
}

void tache1()
{
    while (1) {
        sem_p(sem);
        printf("tache 1 : cpt = %d\r\n",++cpt);
    }
}

void tache2()
{
    while (1) {
    	if (sw_user()) {
    		sem_v(sem);
    	}
    	volatile int k;
    	for (k=0;k<1000;k++) ;
    }
}

int main()
{
	gpio_pin_config_t sw3cfg = { kGPIO_DigitalInput, 0};
	GPIO_PortInit(BOARD_SW3_GPIO,BOARD_SW3_GPIO_PORT);
	GPIO_PinInit(BOARD_SW3_GPIO,BOARD_SW3_GPIO_PORT,BOARD_SW3_GPIO_PIN,&sw3cfg);

    sem = sem_new(0);

    task_new(tache1, 512);
    task_new(tache2, 256);

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX4

Semaphore * mutex=NULL;

void code()
{
    uint32_t id=task_id();

    while(1) {

        sem_p(mutex);
        printf("tache %d\r\n", (int)id);
        sem_v(mutex);

        for (int k=0;k<50000;k++);
    }
}

void dummy()
{
    while (1);
}

int main()
{
    mutex = sem_new(1);

    task_new(code, 512);   // task 1
    task_new(code, 512);   // task 2
    task_new(code, 512);   // task 3
    task_new(dummy, 128);   // task 4

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX5
static void leds_init()
{
	gpio_pin_config_t ledcfg = { kGPIO_DigitalOutput, 1};
	GPIO_PortInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT);
	GPIO_PinInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN,&ledcfg);
}

static void leds(uint32_t val)
{
	// bit 0 of val controls LED RED
	GPIO_PinWrite(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN, (~(val>>0))&1);
	//  bit 1 of val controls LED GREEN
	GPIO_PinWrite(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN, (~(val>>1))&1);
	//  bit 2 of val controls LED BLUE
	GPIO_PinWrite(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN, (~(val>>2))&1);
}

void tache1()
{
	uint32_t state=0;

    while (1) {
    	state=!state;
        leds(state<<2);			// toggle led
        task_wait(200);
    }
}

void tache2()
{
    while (1) {
    }
}


int main()
{
	leds_init();

	task_new(tache1, 256);
    task_new(tache2, 128);

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX6
Semaphore * mutex=NULL;

static void leds_init()
{
	gpio_pin_config_t ledcfg = { kGPIO_DigitalOutput, 1};
	GPIO_PortInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT);
	GPIO_PinInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN,&ledcfg);
}

static void leds(uint32_t val)
{
	// bit 0 of val controls LED RED
	GPIO_PinWrite(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN, (~(val>>0))&1);
	//  bit 1 of val controls LED GREEN
	GPIO_PinWrite(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN, (~(val>>1))&1);
	//  bit 2 of val controls LED BLUE
	GPIO_PinWrite(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN, (~(val>>2))&1);
}

void tache1()
{
    uint32_t id=task_id();

    while (1) {
        sem_p(mutex);
        printf("- tache %d\r\n", (int)id);
        sem_v(mutex);
        leds(0);			// toggle blue led
        task_wait(1500);
        leds(1<<2);			// toggle blue led
        task_wait(1500);
    }
}

void tache2()
{
    uint32_t id=task_id();

    while (1) {
        sem_p(mutex);
        printf("tache %d\r\n", (int)id);
        sem_v(mutex);
        task_wait(500);
    }
}

void tache3()
{
    while(1) { }
}


int main()
{
	leds_init();

    mutex=sem_new(1);

    task_new(tache1, 512);
    task_new(tache2, 512);
    task_new(tache3, 128);

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX7
void code1()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
}

void code2()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
}

void code3()
{
    uint32_t id=task_id();

    while(1) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
}

void code4()
{
    int   i;
    uint32_t id=task_id();

    for (i=0;i<10;i++) {
		printf("tache %d\r\n",(int)id);
        for (int k=0;k<50000;k++);
    }
    printf("end of task %d\r\n",id);
}

int main()
{
    task_new(code1, 512);   // task 1
    task_new(code2, 512);   // task 2
    task_new(code3, 512);   // task 3
    task_new(code4, 512);   // task 4

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX8
char buffer[32];
int fd;

void tache1()
{
	int n, i;

	fd=open("/dev/test", O_READ);
	if (fd!=-1) {
		// lseek(fd,5);
		n=read(fd, buffer, 32);

		for(i=0;i<n;i++) putchar((int)buffer[i]);

		close(fd);
	}

	while (1) { }
}

void idle()
{
    while (1) { }
}

int main()
{
    task_new(tache1,512);
    task_new(idle,0);

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX9

Semaphore * sem;

void tache1()
{
    uint8_t data=1;

    int fd=open("/dev/leds",O_WRONLY);

    while (1) {
        write(fd,&data,1);
        data=data<<1;
        if (data==0x8) data=1;
        task_wait(500);
    }
}

void idle()
{
    while(1) {}
}


int main()
{
    task_new(tache1,256);
    task_new(idle,0);

    os_start();

    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX10

void tache1()
{
    uint8_t data=1;

    int lfd=open("/dev/leds",O_WRONLY);

    int bfd=open("/dev/swuser", O_RDONLY);

    while (1) {
        int pushed;
        read(bfd,(void*)&pushed,4);    // bloque la tache jusqu'Ã  l'appui sur swcenter
        write(lfd,&data,1);
        data=data<<1;
        if (data==0x8) data=1;
    }
}


void idle()
{
    while(1) {}
}


int main()
{
    task_new(tache1,256);
    task_new(idle,0);

    os_start();

    return 0;
}
#endif
/*********************************************************************/
#ifdef MAIN_EX11

Semaphore * sem;

void idle()
{
    while (1) {}
}

volatile int taskA_running = 1; // 1 means running, 0 means stop
// Fonctions auxiliaires pour les LEDs et tâches
void tache1()
{
    uint8_t data=1;

    int fd=open("/dev/leds",O_WRONLY);

    while (taskA_running) {
        write(fd,&data,1);
        data=data<<1;
        if (data==0x8) data=1;
        task_wait(500);
    }
    write(fd, "\0", 1);
}

static void leds_init()
{
	gpio_pin_config_t ledcfg = { kGPIO_DigitalOutput, 1};
	GPIO_PortInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT);
	GPIO_PinInit(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN,&ledcfg);
	GPIO_PinInit(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN,&ledcfg);
}

static void leds(uint32_t val)
{
	// bit 0 of val controls LED RED
	GPIO_PinWrite(BOARD_LED_RED_GPIO,BOARD_LED_RED_GPIO_PORT,BOARD_LED_RED_GPIO_PIN, (~(val>>0))&1);
	//  bit 1 of val controls LED GREEN
	GPIO_PinWrite(BOARD_LED_GREEN_GPIO,BOARD_LED_GREEN_GPIO_PORT,BOARD_LED_GREEN_GPIO_PIN, (~(val>>1))&1);
	//  bit 2 of val controls LED BLUE
	GPIO_PinWrite(BOARD_LED_BLUE_GPIO,BOARD_LED_BLUE_GPIO_PORT,BOARD_LED_BLUE_GPIO_PIN, (~(val>>2))&1);
}

volatile int ledRedState = 0;  // 0 means off, 1 means on
volatile int ledGreenState = 0; // 0 means off, 1 means on
volatile int ledBlueState = 0;  // 0 means off, 1 means on

void serialTask()
{
    int serial = open("/dev/serial", O_RDWR);
    if (serial < 0) {
        perror("Erreur d'ouverture du port série");
        return;
    }

    char buff[255];

    while (1)
    {
        // Lire le choix de l'utilisateur
        if (read(serial, buff, 1) > 0)
        {
            char choice = buff[0];
            snprintf(buff, sizeof(buff), "\r\n%c\r\n", choice);
            write(serial, buff, strlen(buff));
            switch (choice)
            {
                case 'R':
                    ledRedState = !ledRedState;
                    leds(ledRedState ? 1 : 0);  // Allume si state est 1, éteint si state est 0
                    break;
                case 'G':
                    ledGreenState = !ledGreenState;
                    leds(ledGreenState ? 2 : 0); // Allume si state est 1, éteint si state est 0
                    break;
                case 'B':
                    ledBlueState = !ledBlueState;
                    leds(ledBlueState ? 4 : 0); // Allume si state est 1, éteint si state est 0
                    break;
                case 'A':
                    task_new(tache1, 256);
                    taskA_running = 1;
                    break;
                case 'Q':
                    write(serial, "\r\nFin programme...\r\n", 19);
                    taskA_running = 0;
                    leds(0); // éteindre les LEDs
                    close(serial);
                    task_kill();
                    break;
                default:
                    write(serial, "\r\nChoix non valide!\r\n", 18);
                    break;
            }
        }
    }
}

int main() {
    // Lancer les tâches de série et de veille
    task_new(serialTask, 1024);
    task_new(idle, 0);

    // Démarrer le système d'exploitation (hypothèse d'un RTOS)
    os_start();
    return 0;
}

#endif
/*********************************************************************/
#ifdef MAIN_EX12

void accelTask()
{
    int serial = open("/dev/serial",O_WRONLY);

	int lfd=open("/dev/accel",O_RDONLY);

    char buff[255];
    char strDebug[] = "Accelerometer Data:\n";
    write(serial, strDebug, strlen(strDebug));

    int32_t accData[3];

    while (1)
    {
        status_t res = read(lfd, &accData, sizeof(accData));
        if (res == sizeof(accData)) {
        	int x,y,z;
        	x=accData[0];
        	y=accData[1];
        	z=accData[2];

            snprintf(buff, sizeof(buff), "\r\nEn mg => X: %d, Y: %d, Z: %d\r\n", x,y,z);
            write(serial, buff, strlen(buff));
        }
        task_wait(500);
    }
}

void idle()
{
    while (1){}
}

int main()
{
    task_new(accelTask, 1024);
    task_new(idle, 0);
    os_start();
    return 0;
}

#endif

