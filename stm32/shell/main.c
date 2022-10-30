#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(512)

static int rate = 100;

static THD_WORKING_AREA(waLED, 128);
static THD_FUNCTION(thdLED, arg) {

    (void)arg;

    chRegSetThreadName("blinker");
    while (true) {
        palTogglePad(GPIOC,13);
        palTogglePad(GPIOB,10);
        chThdSleepMilliseconds(rate);
    }
}

static thread_t *shelltp = NULL;

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if(argc > 0){chprintf(chp,"Usage: test\r\n");return;}

    chprintf(chp,"Serial OK\r\n");
    chprintf(chp,"Rate = %d\r\n",rate);
}

static void up(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if(argc > 0){chprintf(chp,"Usage: up\r\n");return;}

    chprintf(chp,"Increase Rate of LED blink\r\n");
    rate += 50;
    if (rate>500) {
      chprintf(chp,"Maximum rate is 500\r\n");
      rate -= 50;
    }
    chprintf(chp,"Rate = %d\r\n",rate);
}

static void down(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)argv;
    if(argc > 0){chprintf(chp,"Usage: down\r\n");return;}

    chprintf(chp,"Decrease Rate of LED blink\r\n");
    rate -= 50;
    if (rate<50) {
      chprintf(chp,"Rate can not become zero\r\n");
      rate += 50;
    }
    chprintf(chp,"Rate = %d\r\n",rate);
}

static const ShellCommand commands[] = {
    {"check", cmd_test},
    {"up", up},
    {"down", down},
    {NULL, NULL}
};

static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SD1,
    commands
};

int main(void) {

  halInit();
  chSysInit();

  palSetPadMode(GPIOC,13,PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOC,13);
  chThdCreateStatic(waLED, sizeof(waLED), NORMALPRIO, thdLED, NULL);

  palSetPadMode(GPIOA,9,PAL_MODE_STM32_ALTERNATE_PUSHPULL); //TX
  palSetPadMode(GPIOA,10,PAL_MODE_INPUT); //RX
  sdStart(&SD1,NULL);

  palSetPadMode(GPIOB,10,PAL_MODE_OUTPUT_PUSHPULL);

  shellInit();

  while(true){
    if (!shelltp)
      shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminatedX(shelltp)) {
      chThdRelease(shelltp);
      shelltp = NULL;
    }
    chThdSleepMilliseconds(100);
  }
}
