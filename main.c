#include "usbh_core.h"
#include "bflb_mtimer.h"
#include "board.h"

#include "usbd_core.h"
#include "usbd_cdc.h"

extern void cdc_acm_init(void);
extern void cdc_acm_data_send_with_dtr(const uint8_t *, uint32_t);
extern void log(const char *);

uint32_t buffer_init(char *);

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer_main[2048];

int main(void)
{
  board_init();
  uint32_t data_len = buffer_init("Hello world!\r\n");

  uint32_t inx = 0;
  cdc_acm_init();
  log("Initialized"); 
  while (1) {
    if (inx++ >= 2000){
      cdc_acm_data_send_with_dtr(write_buffer_main, data_len);
      log("dtr_enabled_true_callbacks:  . Write\r\n");
      /* cdc_acm_log_with_dtr(write_buffer_main, data_len); */
      inx = 0;
    }
    bflb_mtimer_delay_ms(1);
  }
}

uint32_t buffer_init(char *data) {

  uint32_t data_len = 0;
  for (ssize_t inx = 0; data[inx]; inx++) {
    write_buffer_main[inx] = data[inx];
    if (data[inx]) data_len++;
  }
  return data_len;
}
