#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h> /// // cdev_add()/cdev_del()
#include <linux/fs.h> /// chrdev file_operations
#include <linux/slab.h>		/// kmalloc()
#include <asm/uaccess.h> ///這裡包含了kernel space空間與user space進行數據交換時的函數
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h> ///msleep
#include <linux/interrupt.h>

#include "ioc_hw5.h"

///當用戶利用類架構加載模塊時，在標準輸出設備和系統日誌上會顯
///示一個壞模塊的出錯訊息。為了消除這條訊息，用戶需要為
///MODULE_LICENSE()宏增加一個示例
MODULE_LICENSE("GPL");
#define DEBUG_ENABLE 0
#define IRQ_NUM  1

#define DMA_BUFSIZE 64
#define DMASTUIDADDR 0x0          /// student ID's address               // ioctl : set and printk value
#define DMARWOKADDR  0x4          /// RW function complete               // ioctl : set and printk value
#define DMAIOCOKADDR 0x8          /// ioctl function complete            // ioctl : set and printk value
#define DMAIRQOKADDR 0xc          /// ISR function complete              // ioctl : set and printk value
#define DMACOUNTADDR 0x10         /// interrupt count function complete  // ISR : set value, exit_module : printk value
#define DMAANSADDR   0x14         /// computation answer                 // work routine : set value, read: printk value
#define DMAREADABLEADDR 0x18      /// READABLE variable for synchronize  // ioctl : check value, write : check value
#define DMABLOCKADDR 0x1c         /// Blocking or Non-Blocking IO        // ioctl: set and printk value, write: check value
#define DMAOPCODEADDR 0x20        /// data.a opcode    //char            // write: set value, work routine: get value
#define DMAOPERANDBADDR 0x21      /// data.b operand1                    // write: set value, work routine: get value
#define DMAOPERANDCADDR 0x25      /// data.c operand2  //short           // write: set value, work routine: get value

void *dma_buf;

static int dev_major;
static int dev_minor;
static struct cdev *dev_cdevp = NULL;
/// 核心內部使用struct cdev型別的結構來代表字元裝置。
/// 要讓核心能提調你的驅動程式提供的作業方法，你必須配置並註冊一或多個struct cdev



/// The 6 in & out function in module to operate dma_buf
/// out function is used to output data to dma buffer.
/// in function is used to input data from dma buffer.
/// 'b', 'w', 'l' are data size
/// char=b -> 1byte ;  short=w-->1 short word (2 bytes); int=l --> 1 long word（4 bytes）
void myoutb(unsigned char data, unsigned short int port){
    memcpy(dma_buf+port, &data, sizeof(unsigned char));
    ///*(dma_buf+port)=data;
    
    ///*(unsigned char *)(dma_buf+port) = data;
}

void myoutw(unsigned short data, unsigned short int port){
    memcpy( dma_buf+port, &data, sizeof(unsigned short));
    ///*(dma_buf+port)= data;
    
    ///*(unsigned short *)(dma_buf+port) = data;
}

void myoutl(unsigned int data, unsigned short int port){
    ///printk("OS_HW5:%s():port=%d, data=%d, \n",__FUNCTION__,port, data);
    memcpy( dma_buf+port, &data, sizeof(unsigned int));
    ///*(dma_buf+port)= data;
    
    
    ///*(unsigned int *)(dma_buf+port) = data;	
}


unsigned char myinb(unsigned short int port){
    unsigned char returnValue;
    memcpy(&returnValue , dma_buf+port, sizeof(unsigned char));
    
   return returnValue;
   
  /// return *(volatile unsigned char*)(dma_buf+port);    
}

unsigned short myinw(unsigned short int port){
    unsigned short returnValue ;
    memcpy(&returnValue , dma_buf+port, sizeof(unsigned short));
    ///returnValue = *(dma_buf+port);
   return returnValue;
   
   ///return *(volatile unsigned short*)(dma_buf+port);
}

unsigned int myinl(unsigned short int port){
    unsigned int returnValue;
    memcpy(&returnValue , dma_buf+port, sizeof(unsigned int));
    ///returnValue = *(dma_buf+port);
   return returnValue;
   
   ///return *(volatile unsigned int*)(dma_buf+port);
}



 ///ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);
 /// read只是把answer傳到user space，
 /// 然後要把answer和readable清成0，下次write才不會有問題。
 static ssize_t drv_read(struct file *filp, char *buf, size_t count, loff_t *ppos)
 {
    unsigned int computeAns;
    ssize_t result = 0;

    # if DEBUG_ENABLE
    printk("OS_HW5:%s(): ###debug### count=%zu\n",__FUNCTION__,count);
    # endif

    computeAns = myinl( DMAANSADDR);
    printk("OS_HW5:%s(): ans = %d\n",__FUNCTION__,computeAns);

    if (copy_to_user(buf, &computeAns, count) < 0) {
        printk("OS_HW5:%s(): ##Error## ans = %d, copy_to_user failed.\n",__FUNCTION__,computeAns);
        result = -EFAULT;
        goto out;
    }
    result = count;

out:
    /// reset ans & readable for next computing.
    myoutl(0, DMAANSADDR);
    myoutl(0, DMAREADABLEADDR);
    return result;
 }


 int prime(int base, short nth){
    int fnd=0;
    int i, num, isPrime;

    num = base;
    while(fnd != nth) {
        isPrime=1;
        num++;
        for(i=2;i<=num/2;i++) {
            if(num%i == 0) {
                isPrime=0;
                break;
            }
        }

        if(isPrime) {
            fnd++;
        }
    }
    return num;
}


 static void arithmetic_routine(struct work_struct *ws){


    int ans;
    struct dataIn {
        char a;
        int b;
        short c;
    };

    struct dataIn data;

    ///printk("OS_HW5:%s(): on cpu:%d, pname:%s\n",__func__, smp_processor_id(), current->comm);


    data.a = myinb( DMAOPCODEADDR);
    data.b = myinl(DMAOPERANDBADDR);
    data.c = myinw( DMAOPERANDCADDR);

    switch(data.a) {
        case '+':
            ans=data.b+data.c;
            break;
        case '-':
            ans=data.b-data.c;
            break;
        case '*':
            ans=data.b*data.c;
            break;
        case '/':
            ans=data.b/data.c;
            break;
        case 'p':
            ans = prime(data.b, data.c);
            break;
        default:
            ans=0;
    }

    printk("OS_HW5:%s(): %d %c %hd = %d \n",__FUNCTION__, data.b, data.a,data.c, ans);

    /// key write back to dma
    myoutl(ans, DMAANSADDR);
    myoutl(1, DMAREADABLEADDR);

 }



 /// ssize_t(*write) (struct file *, const char __user *, size_t, loff_t *);
 /// ssize_t x = ...;
 /// printf("%zu\n", x);  // prints as unsigned decimal
 /// printf("%zx\n", x);  // prints as hex
 /// printf("%zd\n", x);  // prints as signed decimal
 static ssize_t drv_write(struct file *filp, const char *buf, size_t count, loff_t *ppos)
 {
    char *data;
    ssize_t result;
    unsigned int blockMode, readable;
    struct work_struct *arithmetic_work;

    typedef struct {
        char a;
        int b;
        short c;
    } DataIn;


    DataIn* dataIn;


    result = 0;
    ///printk("OS_HW5:%s():device write, count=%zu \n",__FUNCTION__,count);
    data = kmalloc(sizeof(char) * count, GFP_KERNEL);

    if (!data) {
        return -ENOMEM;
    }

    if (copy_from_user(data, buf, count) < 0) {
        printk("OS_HW5:%s(): ##Error## copy_from_user failed.\n",__FUNCTION__);
        result = -EFAULT;
        goto out;
    }


     dataIn = (DataIn*)data;


    # if DEBUG_ENABLE
    printk("OS_HW5:%s(): ###debug### count=%zu, dataIn.a=%c, dataIn.b=%d, dataIn.c=%hd, \n",__FUNCTION__,count,dataIn->a ,dataIn->b,dataIn->c );
    # endif

    myoutb( dataIn->a , DMAOPCODEADDR);
    myoutl( dataIn->b , DMAOPERANDBADDR);
    myoutw(  dataIn->c ,DMAOPERANDCADDR);


     /// for event , Use INIT_WORK() and schedule_work() to queue work to system queue
     /// schedule_work - put work task in kernel-global  workqueue ex: queue_work(system_wq, work);
     /// work queu 的原理是延後處理工作時，排進 wait queue 的等待隊伍之中，
     /// 一個 work queue 有一個對應的 kernel daemon，會在適當時機以 kernel daemon
     /// 處理這些延後的工作，而 kernel daemon 是一隻在 user context 執行的程式。
     printk("OS_HW5:%s():queue work\n",__FUNCTION__);
     arithmetic_work =  kmalloc(sizeof(typeof(*arithmetic_work)), GFP_KERNEL);
     INIT_WORK(arithmetic_work, arithmetic_routine);
     schedule_work(arithmetic_work);

     blockMode = myinl(DMABLOCKADDR);

     if (blockMode ==1){
        /// blocking mode, implement blocking action!!
        printk("OS_HW5:%s():block \n",__FUNCTION__);
        readable = myinl( DMAREADABLEADDR);
        while (readable ==0){
                msleep(1000);
                readable = myinl( DMAREADABLEADDR);
        }
     }




out:
    kfree(data);
    return result;
 }

 static int drv_open(struct inode *inode, struct file *filp)
 {
    printk("OS_HW5:%s():device open\n",__FUNCTION__);
    return 0;
 }


 /// ioctl --> static int drv_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
 /// unlocked_ioctl --> 1.沒有 struct inode *inode  2.return long
 /// long(*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
 /// long(*compat_ioctl) (struct file *, unsigned int, unsigned long);
 static long drv_ioctl( struct file *filp, unsigned int cmd, unsigned long args)
 {
    int err = 0, result = 0;
    int value =0;

    ///printk("OS_HW5:%s():device ioctl\n",__FUNCTION__);

    if (_IOC_TYPE(cmd) != HW5_IOC_MAGIC)  /// 用來解碼一個命令的宏定義
        return -ENOTTY;
    if (_IOC_NR(cmd) > HW5_IOC_MAXNR)
        return -ENOTTY;

    if (_IOC_DIR(cmd) & _IOC_READ) {
        err = !access_ok(VERIFY_WRITE, (void __user*)args, _IOC_SIZE(cmd));
    } else if (_IOC_DIR(cmd) & _IOC_WRITE) {
        err = !access_ok(VERIFY_READ, (void __user *)args, _IOC_SIZE(cmd));
    }

    if (err)
        return -EFAULT;
    /// get_user: user space -> kernel space ; success return0, fail return negative integer
    /// put_user: kernel space -> user space
    switch (cmd) {
       case HW5_IOCSETSTUID:
            /// don't need call access_ok() again. using __get_user().
            result = __get_user(value, (int __user *)args);
            myoutl(value,DMASTUIDADDR);
            printk("OS_HW5:%s(): My STUID is = %d\n", __FUNCTION__, value);
            break;
       case HW5_IOCSETRWOK:
            /// don't need call access_ok() again. using __get_user().
            result = __get_user(value, (int __user *)args);
            myoutl(value,DMARWOKADDR);
            printk("OS_HW5:%s(): RW OK\n", __FUNCTION__);
            break;
       case HW5_IOCSETIOCOK:
            /// don't need call access_ok() again. using __get_user().
            result = __get_user(value, (int __user *)args);
            myoutl(value,DMAIOCOKADDR);
            printk("OS_HW5:%s(): IOC OK\n", __FUNCTION__);
            break;
       case HW5_IOCSETIRQOK:
            /// don't need call access_ok() again. using __get_user().
            result = __get_user(value, (int __user *)args);
            myoutl(value,DMAIRQOKADDR);
            printk("OS_HW5:%s(): IRQ OK\n", __FUNCTION__);
            break;

       case HW5_IOCSETBLOCK:
            /// don't need call access_ok() again. using __get_user().
            result = __get_user(value, (int __user *)args);
            myoutl(value,DMABLOCKADDR);
            if (value == 1){
                printk("OS_HW5:%s(): Blocking IO\n", __FUNCTION__);
            }else{ /// value =0
                printk("OS_HW5:%s(): Non-Blocking IO\n", __FUNCTION__);
            }
            break;
       case HW5_IOCWAITREADABLE:
            value = myinl( DMAREADABLEADDR);
            while (value ==0){
                msleep(1000);
                value = myinl( DMAREADABLEADDR);
            }
            printk("OS_HW5:%s(): wait readable %d\n", __FUNCTION__, value);
            result = __put_user(value, (int __user *)args);

            break;
       default: /* redundant, as cmd was checked against MAXNR */
            return -ENOTTY;
    }


    return result;

 }

 static int drv_release(struct inode *inode, struct file *filp)
 {
    printk("OS_HW5:%s():device close\n",__FUNCTION__);
    return 0;
 }

 struct file_operations dev_fops =
 {
    owner:              THIS_MODULE,
    read:               drv_read,
    write:              drv_write,
    unlocked_ioctl:     drv_ioctl,
    open:               drv_open,
    release:            drv_release,
 };

static void initDMA(void){
    myoutl(0, DMASTUIDADDR);
    myoutl(0, DMARWOKADDR);
    myoutl(0, DMAIOCOKADDR);
    myoutl(0, DMAIRQOKADDR);
    myoutl(0, DMACOUNTADDR);
    myoutl(0, DMAANSADDR);
    myoutl(0, DMAREADABLEADDR);
    myoutl(0, DMABLOCKADDR);
    myoutb('0', DMAOPCODEADDR);
    myoutl(0, DMAOPERANDBADDR);
    myoutw( 0, DMAOPERANDCADDR);
}



irqreturn_t keyboard_interrupt(int irq, void *dev_id){
  unsigned int count;
  count = myinl( DMACOUNTADDR);
  ++count ;
  ///printk("IRQ number is %d, count=%d\n", irq,count);
  myoutl( count , DMACOUNTADDR);

  return IRQ_HANDLED;
}

static int __init init_modules(void)
{
    dev_t dev;
    int result;

    printk("OS_HW5:%s():................Start.................\n",__FUNCTION__);


    printk("OS_HW5:%s():request_irq %d return %d \n",__FUNCTION__,IRQ_NUM, 0);

    /// register irq into ISR
    /// irq：IRQ號碼。
    /// handler：interrupt handler，return的資料型態是irq_handler_t，
    /// 如果它return值是IRQ_HANDLED，表示成功處理完成，但如果return值是IRQ_NONE，表示處理失敗。
    /// flags：僅列出一些，如下: IRQF_DISABLED，IRQF_TRIGGER_RISING，IRQF_TRIGGER_FALLING，
    ///        IRQF_TRIGGER_HIGH，IRQF_TRIGGER_LOWIRQF_SHARED
    /// name：用於識別使用此IRQ的裝置名稱，例如cat /proc/interrupts會列出IRQ號碼與裝置名稱。
    /// dev_id：如果IRQ被很多devices共享，表示共用同一個handler，此資料結構會餵給handler，
    /// 用於識別IRQ正被哪一個device使用，並作對應處理。
/*
*
* IRQF_DISABLED - keep irqs disabled when calling the action handler
* IRQF_SAMPLE_RANDOM - irq is used to feed the random generator
* IRQF_SHARED - allow sharing the irq among several devices
* IRQF_PROBE_SHARED - set by callers when they expect sharing mismatches to occur
* IRQF_TIMER - Flag to mark this interrupt as timer interrupt
* IRQF_PERCPU - Interrupt is per cpu
* IRQF_NOBALANCING - Flag to exclude this interrupt from irq balancing
* IRQF_IRQPOLL - Interrupt is used for polling (only the interrupt that is
*                registered first in an shared interrupt is considered for
*                performance reasons)

 IRQF_DISABLED           0x00000020
 IRQF_SAMPLE_RANDOM      0x00000040
 IRQF_SHARED             0x00000080
 IRQF_PROBE_SHARED       0x00000100
 IRQF_TIMER              0x00000200
 IRQF_PERCPU             0x00000400
 IRQF_NOBALANCING        0x00000800
 IRQF_IRQPOLL            0x00001000
 */

    result = request_irq(IRQ_NUM, keyboard_interrupt, IRQF_SHARED, "keyboard_IRQ_Count_device",  (void *)&keyboard_interrupt);

    if (result<0) {
        printk("request_irq() failed (%d)\n", result);
    }



    /// //自動給主編號
    result = alloc_chrdev_region(&dev, 0, 1, "mydev");
    /// 第一個參數:僅供輸出的參數,當配置成功,此參數持有配置的裝備編號
    /// 第二個參數:你想申請的第一個次編號 (通常是0)
    /// 第三個參數:申請連續裝置編號的總數
    /// 第四個參數:出現在/proc/devices與sysfs的名稱 (2.6以上 sysfs /sys/module/..)
    /// 回傳值: 0表示成功 負值表示失敗

    if (result<0) {
        printk("OS_HW5:%s():can't alloc chrdev\n",__FUNCTION__ );
        return result;
    }

    dev_major = MAJOR(dev);
    dev_minor = MINOR(dev);
    printk("OS_HW5:%s():register chrdev(%d,%d)\n", __FUNCTION__ , dev_major, dev_minor);


    /// 用kzalloc申请内存的时候， 效果等同于先是用 kmalloc() 申请空间 ,
    /// 然后用 memset() 来初始化 ,所有申请的元素都被初始化为 0.
    dev_cdevp = kmalloc(sizeof(struct cdev), GFP_KERNEL);
    if (dev_cdevp == NULL) {
        printk("OS_HW5:%s():kmalloc *dev_cdevp failed\n",__FUNCTION__);
        goto failed;
    }

    ///use cdev_init() at module init to bind cdev and file_ operations
    cdev_init(dev_cdevp, &dev_fops);
    dev_cdevp->owner = THIS_MODULE;

    result = cdev_add(dev_cdevp, MKDEV(dev_major, dev_minor), 1); /// 添加character device
    if (result < 0) {
        printk("OS_HW5:%s():add chr dev failed\n",__FUNCTION__ );
        goto failed;
    }

    dma_buf = kmalloc(DMA_BUFSIZE, GFP_KERNEL);
    if (dma_buf == NULL) {
        printk("OS_HW5:%s():kzalloc *dma_buf failed\n",__FUNCTION__);
        goto failed;
    }else{
        printk("OS_HW5:%s():allocate dma buffer\n", __FUNCTION__);
    }

    initDMA();

    return 0;

failed:
    if (dev_cdevp) {
        kfree(dev_cdevp);
        dev_cdevp = NULL;
    }
    return 0;
}

# if DEBUG_ENABLE
static void validateDMA(void){

    unsigned int studID = myinl( DMASTUIDADDR);
    unsigned int rwFunc = myinl( DMARWOKADDR);
    unsigned int iocFunc = myinl( DMAIOCOKADDR);
    unsigned int irqFunc = myinl( DMAIRQOKADDR);
    unsigned int interruptCount = myinl( DMACOUNTADDR);
    unsigned int computeAns = myinl( DMAANSADDR);
    unsigned int readable = myinl( DMAREADABLEADDR);
    unsigned int blockingIO= myinl( DMABLOCKADDR);
    unsigned char opCode = myinb( DMAOPCODEADDR);
    unsigned int operand1 = myinl(DMAOPERANDBADDR);
    unsigned int short operand2 = myinw( DMAOPERANDCADDR);
    printk("OS_HW5:%s():studID=%d, rwFunc=%d, iocFunc=%d, irqFunc=%d, interruptCount=%d, computeAns=%d, readable=%d, blockingIO=%d, opCode=%c, operand1=%d, operand2=%hd \n",
             __FUNCTION__,studID,rwFunc,iocFunc,irqFunc,interruptCount,computeAns,readable,blockingIO,opCode,operand1,operand2);

}
# endif

static void exit_modules(void)
{
    dev_t dev;
	
# if DEBUG_ENABLE	
    validateDMA();
# endif

    free_irq(IRQ_NUM, (void *)&keyboard_interrupt );

    dev = MKDEV(dev_major, dev_minor);
    if (dev_cdevp) {
        cdev_del(dev_cdevp);
        kfree(dev_cdevp);
    }

    printk("OS_HW5:%s():interrupt count=%d\n", __FUNCTION__,myinl( DMACOUNTADDR));

    if (dma_buf != NULL){
        kfree(dma_buf);
        printk("OS_HW5:%s():free dma buffer\n", __FUNCTION__);
    }

    unregister_chrdev_region(dev, 1);
    printk("OS_HW5:%s():unregister chrdev\n", __FUNCTION__);
    printk("OS_HW5:%s():................End.................\n",__FUNCTION__);
}


module_init(init_modules);
module_exit(exit_modules);
