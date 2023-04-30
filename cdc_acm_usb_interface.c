#include "usbd_core.h"
#include "usbd_cdc.h"

/*!< endpoint address */
/* Transmissions Device->Host (otherwise known as "IN" in these constants */
/* Need to be >= 0x80 to be considered a transmission. See                */
/* https://github.com/sakumisu/CherryUSB/blob/d7c0add7ef58cfa711cf152c088a7e1c65fa5886/core/usbd_core.c#L1230 */

/*
 * Endpoint Address
 * Bits 0..3b Endpoint Number.
 * Bits 4..6b Reserved. Set to Zero
 * Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
 *
 * So, each endpoint needs to have a unique number in 0..3, so a total of 16
 * endpoints. Bit 7 is directional. So 0x00-0x0F is outbound, and 0x80-0x8f is
 * inbound, and everything must be unique in the last nibble. At least that is
 * my understanding at this point.
 */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x85

#define CDC_IN_DBG_EP  0x83
#define CDC_OUT_DBG_EP 0x04
#define CDC_INT_DBG_EP 0x86

#define USBD_VID           0x10B0 /* Vendor Id  */
#define USBD_PID           0xDEAD /* Product Id */
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033   /* US English */

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN * 2)


/* USB Device descriptors. If you don't know what you're doing, read this
 * first: https://beyondlogic.org/usbnutshell/usb5.shtml
 *
 * We have one and only one Device Descriptor that describes the physical device
 * But what linux is calling a "device" has nothing to do with that. I believe
 * this varies by the type of USB device. For USB CDC ACM, we are looking at
 * each "Interface association" as its own Linux device (as determined by
 * /dev/ttyACMx).
 *
 * From Device Descriptor, we'll have any number of "configurations", which
 * typically describes different power profiles, etc. Typically there is only
 * one configuration, but it doesn't need to be that way.
 *
 * Within the configuraiton, we have various interfaces. Using the macro
 * CDC_ACM_DESCRIPTOR_INIT provided by CherryUSB will provide us with:
 *
 * * Interface Association to describe two interfaces ("in" and "out")
 * * Interface for outbound traffic
 * * Interface for inbound traffic
 *
 * I believe that the CDC_INT_* constants above refer to the interface association
 *
 * To actually communicate, we need to setup endpoints, which are associated
 * with the interfaces above. So the full heirarchy is:
 *
 * Device -1:n- Configuration -1:n- Interface -1:n- Endpoint
 *
 * Where for typical, CDC ACM usage is:
 *
 * Device -1:1- Configuration -1:n- Interface (2 per what Linux calls device) -1:1- Endpoint
 *
 */


/*!< global descriptor */
static const uint8_t cdc_descriptor[] = {
  USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
  USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x04, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
  /*                                           ^ - Number of interfaces. We need a seperate in and out channel for each virtual */
  /*                                               So for each /dev/ttyACMx, add 2 to this number                               */
  /*                                                                                                                            */
  /*                                               The last paramater is the string index for this interface. Linux does not    */
  /*                                               seem to report that anywhere, but maybe Windows does?                        */
  CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x02),
  CDC_ACM_DESCRIPTOR_INIT(0x02, CDC_INT_DBG_EP, CDC_OUT_DBG_EP, CDC_IN_DBG_EP, 0x02),
  ///////////////////////////////////////
  /// string0 descriptor
  ///////////////////////////////////////
  USB_LANGID_INIT(USBD_LANGID_STRING),
  ///////////////////////////////////////
  /// string1 descriptor
  ///////////////////////////////////////
  0x16,                       /* bLength */
  USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
  'E', 0x00,                  /* wcChar0 */
  'm', 0x00,                  /* wcChar1 */
  'i', 0x00,                  /* wcChar2 */
  'l', 0x00,                  /* wcChar3 */
  ' ', 0x00,                  /* wcChar4 */
  'L', 0x00,                  /* wcChar5 */
  'e', 0x00,                  /* wcChar6 */
  'r', 0x00,                  /* wcChar7 */
  'c', 0x00,                  /* wcChar8 */
  'h', 0x00,                  /* wcChar9 */
  ///////////////////////////////////////
  /// string2 descriptor
  ///////////////////////////////////////
  0x22,                       /* bLength */
  USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
  'B', 0x00,                  /* wcChar0 */
  'L', 0x00,                  /* wcChar1 */
  '6', 0x00,                  /* wcChar2 */
  '1', 0x00,                  /* wcChar3 */
  '6', 0x00,                  /* wcChar4 */
  ' ', 0x00,                  /* wcChar5 */
  'B', 0x00,                  /* wcChar6 */
  'a', 0x00,                  /* wcChar7 */
  'r', 0x00,                  /* wcChar8 */
  'e', 0x00,                  /* wcChar9 */
  ' ', 0x00,                  /* wcChar10 */
  'M', 0x00,                  /* wcChar11 */
  'e', 0x00,                  /* wcChar12 */
  't', 0x00,                  /* wcChar13 */
  'a', 0x00,                  /* wcChar14 */
  'l', 0x00,                  /* wcChar15 */
  ///////////////////////////////////////
  /// string3 descriptor
  ///////////////////////////////////////
  0x16,                       /* bLength */
  USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
  '2', 0x00,                  /* wcChar0 */
  '0', 0x00,                  /* wcChar1 */
  '2', 0x00,                  /* wcChar2 */
  '3', 0x00,                  /* wcChar3 */
  '-', 0x00,                  /* wcChar4 */
  '0', 0x00,                  /* wcChar5 */
  '4', 0x00,                  /* wcChar6 */
  '-', 0x00,                  /* wcChar7 */
  '1', 0x00,                  /* wcChar8 */
  '9', 0x00,                  /* wcChar9 */
  ///////////////////////////////////////
  /// string4 descriptor
  ///////////////////////////////////////
  0x14,                       /* bLength */
  USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
  'D', 0x00,                  /* wcChar0 */
  'E', 0x00,                  /* wcChar1 */
  'B', 0x00,                  /* wcChar2 */
  'U', 0x00,                  /* wcChar3 */
  'G', 0x00,                  /* wcChar4 */
  ' ', 0x00,                  /* wcChar5 */
  'L', 0x00,                  /* wcChar6 */
  'O', 0x00,                  /* wcChar7 */
  'G', 0x00,                  /* wcChar8 */
#ifdef CONFIG_USB_HS
  ///////////////////////////////////////
  /// device qualifier descriptor
  ///////////////////////////////////////
  0x0a,
  USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x02,
  0x02,
  0x01,
  0x40,
  0x01,
  0x00,
#endif
  0x00
};

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t debug_buffer[2048];

volatile bool ep_tx_busy_flag = false;
volatile bool ep_dbg_tx_busy_flag = false;

// TODO: Remove these. Debugging only
volatile uint8_t debug_val_1 = 0;
volatile uint8_t debug_val_2 = 0;
volatile uint32_t debug_val32_1 = 0;
volatile uint32_t debug_val32_2 = 0;

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

void usbd_configure_done_callback(void)
{
  /* setup first out ep read transfer */
  usbd_ep_start_read(CDC_OUT_EP, read_buffer, 2048);
  usbd_ep_start_read(CDC_OUT_DBG_EP, read_buffer, 2048);
}

void usbd_cdc_acm_bulk_out(uint8_t ep, uint32_t nbytes)
{
  debug_val_1 = ep; debug_val32_1 = nbytes;
  USB_LOG_RAW("actual out len:%d\r\n", nbytes);

  /* setup next out ep read transfer */
  usbd_ep_start_read(ep, read_buffer, 2048);
}

void usbd_cdc_acm_bulk_in(uint8_t ep, uint32_t nbytes)
{
  debug_val_2 = ep; debug_val32_2 = nbytes;
  USB_LOG_RAW("actual in len:%d\r\n", nbytes);

  if ((nbytes % CDC_MAX_MPS) == 0 && nbytes) {
    /* send zlp */
    usbd_ep_start_write(ep, NULL, 0);
  } else {
    if (ep == CDC_IN_EP) {
      ep_tx_busy_flag = false;
    }else{
      ep_dbg_tx_busy_flag = false;
    }
  }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
  .ep_addr = CDC_OUT_EP,
  .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
  .ep_addr = CDC_IN_EP,
  .ep_cb = usbd_cdc_acm_bulk_in
};

struct usbd_interface intf0;
struct usbd_interface intf1;

struct usbd_endpoint cdc_out_dbg_ep = {
  .ep_addr = CDC_OUT_DBG_EP,
  .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_dbg_ep = {
  .ep_addr = CDC_IN_DBG_EP,
  .ep_cb = usbd_cdc_acm_bulk_in
};

struct usbd_interface intf2;
struct usbd_interface intf3;

/* function ------------------------------------------------------------------*/
void cdc_acm_init(void)
{
  usbd_desc_register(cdc_descriptor);


  /* Add primary comms channel */
  usbd_add_interface(usbd_cdc_acm_init_intf(&intf0));
  usbd_add_interface(usbd_cdc_acm_init_intf(&intf1));
  usbd_add_endpoint(&cdc_out_ep);
  usbd_add_endpoint(&cdc_in_ep);

  /* Add debug log comms channel */
  usbd_add_interface(usbd_cdc_acm_init_intf(&intf2));
  usbd_add_interface(usbd_cdc_acm_init_intf(&intf3));
  usbd_add_endpoint(&cdc_out_dbg_ep);
  usbd_add_endpoint(&cdc_in_dbg_ep);

  usbd_initialize();
}

volatile uint8_t dtr_enable = 0;
volatile uint8_t dtr_debug_enable = 0;

/************************************************
 * Callback function from the host based on
 * control flow commands
 */
void usbd_cdc_acm_set_dtr(uint8_t intf, bool dtr)
{
  /* Based on above init, intf = 0 is normal, intf = 2 is debug */
  if (dtr) {
    if (intf == 0) {
      dtr_enable = 1;
    } else {
      dtr_debug_enable = 1;
    }
  } else {
    if (intf == 0) {
      dtr_enable = 0;
    } else {
      dtr_debug_enable = 0;
    }
  }
}

void cdc_acm_data_send_with_dtr(const uint8_t *data, uint32_t data_len )
{
  if (dtr_enable) {
    ep_tx_busy_flag = true;
    usbd_ep_start_write(CDC_IN_EP, data, data_len);
    while (ep_tx_busy_flag) {
    }
  }
}

void cdc_acm_log_with_dtr(const uint8_t *data, uint32_t data_len )
{
  if (dtr_debug_enable) {
    ep_dbg_tx_busy_flag = true;
    usbd_ep_start_write(CDC_IN_DBG_EP, data, data_len);
    while (ep_dbg_tx_busy_flag) {
    }
  }
}

uint32_t out_inx = 0;
void log(const char *data){
  int len = snprintf(
    (char *)&write_buffer[0],
    2048,
    "%d\r\ndebug u8 val 1: %d, debug val u8 2: %d\r\ndebug 32 val 1: %d, debug 32 val 2: %d\r\nsending to debug...\r\n",
    out_inx++,
    debug_val_1,
    debug_val_2,
    debug_val32_1,
    debug_val32_2
    );
  cdc_acm_data_send_with_dtr(&write_buffer[0], len);


  int dbg_len = snprintf(
    (char *)&debug_buffer[0],
    2048,
    "%d\r\ndebug u8 val 1: %d, debug val u8 2: %d\r\ndebug 32 val 1: %d, debug 32 val 2: %d\r\n(debug log)\r\n",
    out_inx,
    debug_val_1,
    debug_val_2,
    debug_val32_1,
    debug_val32_2
    );
  cdc_acm_log_with_dtr(&debug_buffer[0], dbg_len);
}
