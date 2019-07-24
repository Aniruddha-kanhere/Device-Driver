 /*
 * @file    MCSPI_mod.c
 * @author  Aniruddha Kanhere
 * @date    13 July 2019
 * @version 1
 * @brief   SPI character driver to for MCSPI module
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/ioport.h>         // Required for request_mem_region
#include <asm/io.h> 		          // Required for ioremap/ unmap etc.
#include <linux/platform_device.h>// Required for platform_device functions
#include <linux/of_device.h>
#include <linux/device.h>

#include "MCSPI_reg.h"
#include "MCSPI_misc.h"
#include "mcspi_ioctl.h"

#define  DEVICE_NAME "MCSPI"              ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "SPI_Driver_Class"   ///< The device class -- this is a character device driver
#define  MAJOR_NUMBER 0

MODULE_LICENSE      ("GPL v2");                           ///< The license type -- this affects available functionality
MODULE_AUTHOR       ("Aniruddha Kanhere");              ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION  ("A MCSPI LKM for the BBB");  ///< The description -- see modinfo
MODULE_VERSION      ("1.0");                           ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static struct class*  MCSPI_Class  = NULL; ///< The device-driver class struct pointer
static struct device* MCSPI_Device = NULL; ///< The device-driver device struct pointer
static DEFINE_MUTEX(MCSPI_mutex);
struct resource *res;

// The prototype functions for the character driver -- must come before the struct definition
static int     MCSPI_open(struct inode *, struct file *);
static int     MCSPI_release(struct inode *, struct file *);
static ssize_t MCSPI_read(struct file *, char *, size_t, loff_t *);
static ssize_t MCSPI_write(struct file *, const char *, size_t, loff_t *);
static long    MCSPI_ioctl(struct file *, unsigned int, unsigned long);


/*  All Devices are represented as file structure in the kernel.
    The file_operations structure from /linux/fs.h lists the callback functions
    that you wish to associated with your file operations using a C99 syntax
    structure. char devices usually implement open, read, write, ioctl and
    release calls
 */
static struct file_operations fops =
{
   .owner          = THIS_MODULE,
   .open           = MCSPI_open,
   .read           = MCSPI_read,
   .write          = MCSPI_write,
   .release        = MCSPI_release,
   .unlocked_ioctl = MCSPI_ioctl,        //Instead of the normal ioctl with BKL
};


/*.............................................................................
These are default values. If the user decides then these values can be changed
using the ioctl commands. These were in place so that one may directly use the
driver.
NOTE: IOCTL function is yet to defined...
.............................................................................*/
struct MCSPI mcspi = {
  .base_addr      = NULL,
  .tx_rx          = MCSPI_CHCONF_TRM_TX,
  .channel_number = 0,
  .role           = MCSPI_MODULCTRL_MASTER,
  .word_length    = MCSPI_CHCONF_WL_8BIT,
  .phase          = MCSPI_CHCONF_PHA_ODD,
  .polarity       = MCSPI_CHCONF_POL_ACTIVE_HIGH,
  .clock_div      = CLK_2,
  .pin_direction  = MCSPI_D0_IN_D1_OUT,
  .CS_polarity    = MCSPI_CS_ACTIVE_LOW,
  .CS_sensitive   = MCSPI_CS_SENSITIVE_ENABLED,
};

struct MCSPI_data data_var;
struct MCSPI_data *data = &data_var;


/*..............................................................................
*   @breif: Ths function was necessary for setting the muxmode to control the
*           peripheral connected to the pin. This function should be called in
*           either open or init functions i.e. before transmitting data or else
*           one might not get the desired output at all.
*   @params: void
*   @return: 0/error
..............................................................................*/
int MCSPI_mux_mode_set(void)
{
  void __iomem *control_module_base;
  u32 val;
  control_module_base = (void __iomem *)ioremap(CONTROL_MODULE_START, CONTROL_MODULE_SIZE);
  if(IS_ERR(control_module_base))
  {
    DEBUG_ALERT("%s: MUX_MODE: Cannot get access to SPI mux region. Aborting.. \n", DEVICE_NAME);
    return -(PTR_ERR(control_module_base));
  }

  val = MCSPI_read_reg(control_module_base, CONF_SPI0_SCLK_OFFSET);
  val &= ~CONF_MODULE_PIN_MMODE(0x07);
  MCSPI_write_reg(control_module_base, CONF_SPI0_SCLK_OFFSET, val);

  val = MCSPI_read_reg(control_module_base, CONF_SPI0_D0_OFFSET);
  val &= ~CONF_MODULE_PIN_MMODE(0x07);
  MCSPI_write_reg(control_module_base, CONF_SPI0_D0_OFFSET, val);

  val = MCSPI_read_reg(control_module_base, CONF_SPI0_D1_OFFSET);
  val &= ~CONF_MODULE_PIN_MMODE(0x07);
  MCSPI_write_reg(control_module_base, CONF_SPI0_D1_OFFSET, val);

  val = MCSPI_read_reg(control_module_base, CONF_SPI0_CS0_OFFSET);
  val &= ~CONF_MODULE_PIN_MMODE(0x07);
  MCSPI_write_reg(control_module_base, CONF_SPI0_CS0_OFFSET, val);

  val = MCSPI_read_reg(control_module_base, CONF_SPI0_CS1_OFFSET);
  val &= ~CONF_MODULE_PIN_MMODE(0x07);
  MCSPI_write_reg(control_module_base, CONF_SPI0_CS1_OFFSET, val);

  iounmap(control_module_base);
  return 0;
}

//this should be called before using any kind of registers of SPI module
int clock_start_stop(bool st_sp)
{
  void __iomem *clock_base_addr;
  clock_base_addr = (void __iomem *)ioremap(CM_PER_START, CM_PER_SIZE);
  if(IS_ERR(clock_base_addr))
  {
    DEBUG_ALERT("%s: Open: Cannot get access to SPI clock region. Aborting.. \n", DEVICE_NAME);
    return -(PTR_ERR(clock_base_addr));
  }

  if(st_sp)
    enable_clock(clock_base_addr, CM_PER_SPI0_CLKCTRL);
  else
    disable_clock(clock_base_addr, CM_PER_SPI0_CLKCTRL);

  iounmap(clock_base_addr);
  return 0;
}


/*..............................................................................
*    @brief The LKM initialization function
*    @params: void
*    @return returns 0 if successful
 .............................................................................*/
static int __init MCSPI_init(void){

   DEBUG_ALERT("%s: Initializing... \n", DEVICE_NAME);

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(MAJOR_NUMBER, DEVICE_NAME, &fops);
   if (majorNumber<0){
      DEBUG_ALERT("%s: failed to register a major number\n", DEVICE_NAME);
      return majorNumber;
   }

   DEBUG_NORM("%s: Registered correctly with major number %d\n" ,DEVICE_NAME, majorNumber);

   // Register the device class
   MCSPI_Class = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(MCSPI_Class)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      DEBUG_ALERT("%s: Failed to register device class\n", DEVICE_NAME);
      return PTR_ERR(MCSPI_Class);          // Correct way to return an error on a pointer
   }

   DEBUG_NORM("%s: device class registered correctly\n", DEVICE_NAME);

   // Register the device driver
   MCSPI_Device = device_create(MCSPI_Class, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(MCSPI_Device)){               // Clean up if there is an error
      class_destroy(MCSPI_Class);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      DEBUG_ALERT("%s: Failed to create the device\n", DEVICE_NAME);
      return PTR_ERR(MCSPI_Device);
   }
   DEBUG_NORM("%s: device class created correctly\n", DEVICE_NAME); // Made it! device was initialized

   mutex_init(&MCSPI_mutex);
   return 0;
}

/*..............................................................................
*    @brief The LKM cleanup function
*    @params: void
*    @return returns 0 if successful
 .............................................................................*/
static void __exit MCSPI_exit(void){
   mutex_destroy(&MCSPI_mutex);
   device_destroy(MCSPI_Class, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(MCSPI_Class);                          // unregister the device class
   class_destroy(MCSPI_Class);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   DEBUG_ALERT("%s: Driver unloaded\n", DEVICE_NAME);
}

/*
//this is yet to be formulated and included
static int MCSPI_probe(struct platform_device *pdev)
{
  return 0;
}

//this is yet to be formulated and included
static int MCSPI_remove(struct platform_device *pdev)
{
  return 0;
}

//this is yet to be formulated and included
static struct platform_driver MCSPI_driver = {
  .driver = {
    .name = DEVICE_NAME,
    .owner = THIS_MODULE,
  },
  .probe = MCSPI_probe,
  .remove = MCSPI_remove,
};
*/


/*..............................................................................
*    @brief The device open function that is called each time the device is opened
*           - Sets up the clock for the MCSPI module
*           - maps the physical registers to the virtual kernel space
*           - Configures the MCSPI module
*           - Sets the mux mode
*           - Enable the MCSPI module
*    @param: inodep A pointer to an inode object (defined in linux/fs.h)
*            filep A pointer to a file object (defined in linux/fs.h)
*    @return: if any error occurs, the returns error or else 0
 .............................................................................*/
static int MCSPI_open(struct inode *inodep, struct file *filep){

  int err_val = 0;
  data->device = &mcspi;
  data->device_id = MKDEV(majorNumber, 0);
  data->numberOpens++;
  filep->private_data = data;

  //cannot open more than 1 file for the SPI driver (for now)
  if(data->numberOpens>1)
   return -EUSERS;

  //enable the clock and check for errors.
  err_val = clock_start_stop(1);
  if(err_val < 0)
    return err_val;


  res=request_mem_region(MCSPI0_START, MCSPI0_ADDR_SIZE, "MCSPI0 register space");
  if(res==NULL)
  {
    DEBUG_ALERT("%s: Open: Couldn't acquire the memory region\n", DEVICE_NAME);
    //return -EBUSY;
  }

  data->device->base_addr = (void __iomem *)ioremap_nocache(MCSPI0_START, MCSPI0_ADDR_SIZE);

  if (IS_ERR(data->device->base_addr)){
    DEBUG_NORM("%s: Open: IO mem remap unsuccessful. Error Code: %ld \n ", DEVICE_NAME, PTR_ERR(data->device->base_addr));
    return -PTR_ERR(data->device->base_addr);
  }

  DEBUG_NORM("%s: Open: IO mem remap successful(0x%08lx)\n ", DEVICE_NAME, (unsigned long)data->device->base_addr);

  if(MCSPI_configure(data->device))
  {
    DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
    return -EBUSY;
  }

  MCSPI_mux_mode_set();

  MCSPI_enable(data->device, 1);

  DEBUG_NORM("%s: Open: Device enabled\n", DEVICE_NAME);
  DEBUG_NORM("%s: Open: Configuration of device successful\n", DEVICE_NAME);
  DEBUG_ALERT("%s: Open: Device opened successfully\n", DEVICE_NAME);
   return nonseekable_open(inodep, filep);
}


/*..............................................................................
 *  @Brief: This function is called whenever device is being read from user space
 *          i.e. data is being sent from the device to the user.
 *  @Params: filep: A pointer to a file object (defined in linux/fs.h)
 *           buffer: Pointer to the buffer to which this function writes the data
 *                   len: The length of the message copied to buffer
 *                   offset: The offset if required
 *  @Return: Error value or 0
 .............................................................................*/
static ssize_t MCSPI_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset){
   int error_count = 0;

   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   memset(message, '\0', size_of_message + 1);
   size_of_message = 0;

   if (error_count==0){            // if true then have success
      DEBUG_NORM("%s: Sent %d characters to the user\n", DEVICE_NAME, size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      DEBUG_ALERT("%s: Failed to send %d characters to the user\n", DEVICE_NAME, error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}


/*..............................................................................
 *  @Brief: This function is called whenever the device is being written to from
 *         user space i.e. data is sent to the device from the user. The data is
 *         copied to the message[] array and then sent for further processing.
 *  @Parameters: filep: A pointer to a file object
 *              buffer: Buffer containing the string to write to the device
 *              len: The length of the array of data (buffer)
 *              offset: The offset if required
 *  @Return: Error value or 0
 .............................................................................*/
static ssize_t MCSPI_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset){

   char message[len];
   int error_count=0;
   struct MCSPI_data *data = (struct MCSPI_data *)filep->private_data;
   struct MCSPI *mcspi = data->device;

   //get the data from user to kernel space
   error_count = copy_from_user(message, buffer, len);

   if(error_count)
	    DEBUG_ALERT("String was too long, could not copy %d cahracters\n", error_count);

   if( MCSPI_send_data_poll(mcspi, message, len-error_count) < 0)
   {
     DEBUG_ALERT("%s: Write: Timeout in sending data\n", DEVICE_NAME);
   }

   DEBUG_NORM("%s: Received %zu characters from the user\n", DEVICE_NAME, len);
   return len;
}


/*..............................................................................
 *   @brief: The device release function that is called whenever the device is
 *           closed/released by the userspace program
 *   @param: inodep: A pointer to an inode object (defined in linux/fs.h)
 *           filep: A pointer to a file object (defined in linux/fs.h)
 *   @return
 .............................................................................*/
static int MCSPI_release(struct inode *inodep, struct file *filep){
   struct MCSPI_data *data = (struct MCSPI_data *)filep->private_data;
   struct MCSPI *mcspi = data->device;

   //reduce the number of times this is opened
   data->numberOpens--;

   MCSPI_enable(mcspi, 0);

   //unmap the mapped memory address
   iounmap(mcspi->base_addr);

   //stop the clock to the SPI0 module
   clock_start_stop(0);

   //Unlock the mutex for the next process which open()s this device
   mutex_unlock(&MCSPI_mutex);

   DEBUG_ALERT("%s: Device successfully closed\n",  DEVICE_NAME);
   return 0;
}


/*..............................................................................
 *   @brief: The ioctl function used to send command to the device.
 *   @param: filep: A pointer to a file object (defined in linux/fs.h)
 *           command: the command being sent to the driver. (All commands are
 *                    listed in the MCSPI_ioctl.h file
 *           arg: the argument to be sent with the command
 *   @return error code/return value for the command
 .............................................................................*/
long MCSPI_ioctl(struct file *filep, unsigned int command, unsigned long arg)
{
  struct MCSPI_data *mcspi_data = (struct MCSPI_data *)filep->private_data;
  struct MCSPI *mcspi = mcspi_data->device;

  if (_IOC_TYPE(command) != MCSPI_MAGIC_NUMBER) return -ENOTTY;
  if (_IOC_NR(command) > MAX_IOCTL_NUMBER) return -ENOTTY;

  switch(command)
  {
    case MCSPI_MODE_SET :
                         if(arg == MCSPI_MODULCTRL_MASTER  ||  arg == MCSPI_MODULCTRL_SLAVE)
                         {
                           mcspi->role = arg;
                           if(MCSPI_configure(mcspi))
                           {
                             DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                             return -EBUSY;
                           }
                           DEBUG_NORM("%s: IOCTL: MCSPI_MODE_SET: %ld\n", DEVICE_NAME, arg);
                         }
                         MCSPI_enable(mcspi, 1);
                         return 0;
                         break;


    case MCSPI_MODE_GET:
                         if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                          return -EFAULT;
                         put_user(mcspi->role, (__u32 __user *)arg);
                         DEBUG_NORM("%s: IOCTL: MCSPI_MODE requested\n", DEVICE_NAME);
                         break;


    case MCSPI_POL_SET :
                         if(arg == MCSPI_CHCONF_POL_ACTIVE_HIGH  ||
                            arg == MCSPI_CHCONF_POL_ACTIVE_LOW)
                         {
                           mcspi->polarity = MCSPI_CHCONF_POL_ACTIVE_HIGH;
                           if(MCSPI_configure(mcspi))
                           {
                             DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                             return -EBUSY;
                           }
                           DEBUG_NORM("%s: IOCTL: MCSPI_POL_SET: %ld\n", DEVICE_NAME, arg);
                         }
                         MCSPI_enable(mcspi, 1);
                         return 0;
                         break;


    case MCSPI_POL_GET :
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->polarity, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_POL requested\n", DEVICE_NAME);
                          break;


    case MCSPI_PHA_SET:
                          if(arg == MCSPI_CHCONF_PHA_ODD ||
                             arg == MCSPI_CHCONF_PHA_EVEN )
                          {
                            mcspi->phase = arg;
                            if(MCSPI_configure(mcspi))
                            {
                              DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                              return -EBUSY;
                            }
                            DEBUG_NORM("%s: IOCTL: MCSPI_PHA_SET: %ld\n", DEVICE_NAME, arg);
                          }
                          MCSPI_enable(mcspi, 1);
                          return 0;
                          break;


    case MCSPI_PHA_GET :
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->phase, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_PHA requested\n", DEVICE_NAME);
                          break;


    case MCSPI_PIN_CONFIG_SET:
                          if(arg == MCSPI_D0_IN_D1_OUT ||
                             arg == MCSPI_D1_IN_D0_OUT)
                          {
                            mcspi->pin_direction = arg;
                            if(MCSPI_configure(mcspi))
                            {
                              DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                              return -EBUSY;
                            }
                            DEBUG_NORM("%s: IOCTL: MCSPI_PIN_DIRECTION: %ld\n", DEVICE_NAME, arg);
                          }
                          MCSPI_enable(mcspi, 1);
                          return 0;
                          break;


    case MCSPI_PIN_CONFIG_GET:
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->pin_direction, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_PIN_DIRECTION requested\n", DEVICE_NAME);
                          break;


    case MCSPI_CLKD_SET:
                          if(arg >= CLK_1  && arg <=CLK_32768)
                          {
                            mcspi->clock_div = arg;
                            if(MCSPI_configure(mcspi))
                            {
                              DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                              return -EBUSY;
                            }
                            DEBUG_NORM("%s: IOCTL: MCSPI_CLKD: %ld\n", DEVICE_NAME, arg);
                          }
                          MCSPI_enable(mcspi, 1);
                          return 0;
                          break;


    case MCSPI_CLKD_GET:
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->clock_div, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_CLKD requested\n", DEVICE_NAME);
                          break;


    case MCSPI_CS_SET :
                          if(arg == MCSPI_CS_SENSITIVE_DISABLED ||
                             arg == MCSPI_CS_SENSITIVE_ENABLED)
                          {
                             mcspi->CS_sensitive = arg;
                             if(MCSPI_configure(mcspi))
                             {
                               DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                               return -EBUSY;
                             }
                             DEBUG_NORM("%s: IOCTL: MCSPI_CS: %ld\n", DEVICE_NAME, arg);
                          }
                          MCSPI_enable(mcspi, 1);
                          return 0;
                          break;


    case MCSPI_CS_GET:
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->CS_sensitive, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_CS_SENSITIVE requested\n", DEVICE_NAME);
                          break;


    case MCSPI_TRM_SET :
                          if(arg == MCSPI_TRM_TX || arg == MCSPI_TRM_RX || arg == MCSPI_TRM_TX_RX)
                          {
                             mcspi->tx_rx = arg;
                             if(MCSPI_configure(mcspi))
                             {
                               DEBUG_ALERT("%s: Open: configuration failed. (Check logs for more info)\n", DEVICE_NAME);
                               return -EBUSY;
                             }
                             DEBUG_NORM("%s: IOCTL: MCSPI_TX_RX: %ld\n", DEVICE_NAME, arg);
                          }
                          MCSPI_enable(mcspi, 1);
                          return 0;
                          break;


    case MCSPI_TRM_GET  :
                          if(!access_ok(VERIFY_WRITE, (void __user *)arg, sizeof(u32)))
                            return -EFAULT;
                          put_user(mcspi->tx_rx, (__u32 __user *)arg);
                          DEBUG_NORM("%s: IOCTL: MCSPI_TX_RX requested\n", DEVICE_NAME);
                          break;

    default: return -ENOTTY;
  }

  //access_ok (error then -EFAULT) to check with __put_user or __get_user  ot just pu_user or get_user instead of copy_from_user
  // since we are using predefined sizes of data to be transfered from/to user space
  return 0;//-ENOTTY;   //according to the POSIX standard instead of -EINVAL
}

/*..............................................................................
 *  @brief A module must use the module_init() module_exit() macros from
 *         linux/init.h, which identify the initialization function at insertion
 *         time and the cleanup function (as listed above)
 */
module_init(MCSPI_init);
module_exit(MCSPI_exit);
