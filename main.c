#include "usbh_core.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "usbd_core.h"
#include "usbd_cdc.h"

extern void cdc_acm_init(void);
extern void output(const char *, ...);
extern void debuglog(const char *, ...);
extern void debugwarn(const char *, ...);
extern void debugerror(const char *, ...);

uint32_t buffer_init(char *);

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer_main[2048];

int main(void)
{
  board_init();

  uint32_t inx = 0;
  cdc_acm_init();
  debuglog("Initialized"); 
  while (1) {
    if (inx++ >= 2000){
      output("Hello world\r\n");
      debuglog("Hello to debuggers!\r\n");
      debugwarn("Warning to debuggers!\r\n");
      debugerror("Oh no - an error!\r\n");
      inx = 0;
    }
    bflb_mtimer_delay_ms(1);
  }
}

