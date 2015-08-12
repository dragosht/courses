#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>

#include <linux/mfd/ti_am335x_tscadc.h>
#include <linux/platform_data/ti_am335x_adc.h>


MODULE_DESCRIPTION("AM335x ADC");
MODULE_AUTHOR("Dragos Tarcatu");
MODULE_LICENSE("GPL");


#define SET_ADC_CHANNELS        _IOW('z', 1, int)

#define GET_ADC_CHAN1            _IOR('z', 2, int*)
#define GET_ADC_CHAN2            _IOR('z', 3, int*)
#define GET_ADC_CHAN3            _IOR('z', 4, int*)
#define GET_ADC_CHAN4            _IOR('z', 5, int*)
#define GET_ADC_CHAN5            _IOR('z', 6, int*)
#define GET_ADC_CHAN6            _IOR('z', 7, int*)
#define GET_ADC_CHAN7            _IOR('z', 8, int*)
#define GET_ADC_CHAN8            _IOR('z', 9, int*)


struct am335xadc_dev
{
	struct ti_tscadc_dev	*tscadc_dev;
	int			channels;
	u8			channel_line[8];
	u8			channel_step[8];
	dev_t			dev;
	struct class		*cl;
	struct cdev		cdev;
};

static struct am335xadc_dev adcdev;



static unsigned int am335xadc_readl(unsigned int reg)
{
	return readl(adcdev.tscadc_dev->tscadc_base + reg);
}

static void am335xadc_writel(unsigned int reg, unsigned int value)
{
	writel(value, adcdev.tscadc_dev->tscadc_base + reg);
}

static u32 am335xadc_get_step_mask(void)
{
	u32 step;
	step = ((1 << adcdev.channels) - 1);
	step <<= TOTAL_STEPS - adcdev.channels + 1;
	return step;
}

static void am335xadc_step_config(void)
{
	unsigned int stepconfig;
	int i, steps;

	steps = TOTAL_STEPS - adcdev.channels;
	stepconfig = STEPCONFIG_AVG_16 | STEPCONFIG_FIFO1;

	for (i = 0; i < adcdev.channels; i++) {
		int chan;
		chan = adcdev.channel_line[i];
		am335xadc_writel(REG_STEPCONFIG(steps),
				stepconfig | STEPCONFIG_INP(chan));
		am335xadc_writel(REG_STEPDELAY(steps),
				STEPCONFIG_OPENDLY);
		adcdev.channel_step[i] = steps;
		steps++;
	}
}

static int am335xadc_read_signal(int channel)
{
	u32 step, step_en;
	unsigned int fifo1count, read, stepid;
	unsigned long timeout;
	int i, map_val, found;

	timeout = jiffies + usecs_to_jiffies(IDLE_TIMEOUT * adcdev.channels);
	step_en = am335xadc_get_step_mask();
	step = UINT_MAX;
	found = 0;

	am335x_tsc_se_set(adcdev.tscadc_dev, step_en);

	while (am335xadc_readl(REG_ADCFSM) & SEQ_STATUS) {
		if (time_after(jiffies, timeout))
			return -EAGAIN;
	}

	map_val = channel + TOTAL_CHANNELS;

	for (i = 0; i < ARRAY_SIZE(adcdev.channel_step); i++) {
		if (channel == adcdev.channel_line[i]) {
			step = adcdev.channel_step[i];
			break;
		}
	}

	if (step == UINT_MAX)
		return -EINVAL;

	fifo1count = am335xadc_readl(REG_FIFO1CNT);
	for (i = 0; i < fifo1count; i++) {
		read = am335xadc_readl(REG_FIFO1);
		stepid = read & FIFOREAD_CHNLID_MASK;
		stepid = stepid >> 0x10;

		channel = stepid - TOTAL_CHANNELS;
		if (stepid == map_val) {
			read = read & FIFOREAD_DATA_MASK;
			found = 1;
			printk(KERN_INFO "Read channel: %d value: %d\n", channel, read);
		}
	}

	if (!found) {
		return -EBUSY;
	}

	return read;
}



static ssize_t am335xadc_read(struct file *file, char __user *user_buffer, size_t size, loff_t *offset)
{
	return -EPERM;
}

static ssize_t am335xadc_write(struct file *file, const char __user *user_buffer, size_t size, loff_t *offset)
{
	return -EPERM;
}

static int am335xadc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int am335xadc_close(struct inode *inode, struct file *file)
{
	return 0;
}

static long am335xadc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct am335xadc_data data;
	int channel	= 0;
	int value	= 0;

	if (cmd == SET_ADC_CHANNELS) {
		if (arg > 8) {
			return -EINVAL;
		}
		adcdev.channels = arg;
		return 0;
	}

	switch(cmd) {
	case GET_ADC_CHAN1:
		channel = 1;
		break;
	case GET_ADC_CHAN2:
		channel = 2;
		break;
	case GET_ADC_CHAN3:
		channel = 3;
		break;
	case GET_ADC_CHAN4:
		channel = 4;
		break;
	case GET_ADC_CHAN5:
		channel = 5;
		break;
	case GET_ADC_CHAN6:
		channel = 6;
		break;
	case GET_ADC_CHAN7:
		channel = 7;
		break;
	case GET_ADC_CHAN8:
		channel = 8;
		break;
	default:
		return -EINVAL;
	}

	value = am335xadc_read_signal(channel);
	if (copy_to_user((int*) arg, value, sizeof(int))) {
		return -EFAULT;
	}

	return 0;
}


static int am335x_tscadc_probe(struct platform_device *pdev)
{
	struct mfd_tscadc_board	*pdata = NULL;

	adcdev.tscadc_dev = ti_tscadc_dev_get(pdev);
	pdata = adcdev.tscadc_dev->dev->platform_data;

	if (pdata)
		adcdev.channels = pdata->adc_init->adc_channels;
	else
		adcdev.channels = 0;

	am335xadc_step_config();

	return 0;
}

static int am335x_tscadc_remove(struct platform_device *pdev)
{
	u32 step_en;
	step_en = am335xadc_get_step_mask();
	am335x_tsc_se_clr(adcdev.tscadc_dev, step_en);
	return 0;
}

static struct platform_driver am335x_driver = {
	.probe = am335x_tscadc_probe,
	.remove = am335x_tscadc_remove,
	.driver = {
		.name = "tiadc",
		.owner = THIS_MODULE,
	},
};

static struct file_operations fops = {
	.owner		= THIS_MODULE,
	.open		= am335xadc_open,
	.read		= am335xadc_read,
	.write		= am335xadc_write,
	.release	= am335xadc_close,
	.unlocked_ioctl = am335xadc_ioctl,
};

static int am335xadc_init(void)
{
	int status;
	status = platform_driver_probe(&am335x_driver, &am335x_tscadc_probe);
	if (status < 0)
	{
		printk(KERN_ALERT "ADC platform driver probe failed\n");
		return -1;
	}

	status = alloc_chrdev_region(&adcdev.dev, 0, 1, "am335xadc");
	if (status < 0)
		goto err_chrdev_alloc;

	if ((adcdev.cl = class_create(THIS_MODULE, "chardev")) == NULL)
		goto err_class_create;

	if (device_create(adcdev.cl, NULL, adcdev.dev, NULL, "am335xadc") == NULL)
		goto err_device_create;

	cdev_init(&adcdev.cdev, &fops);

	status = cdev_add(&adcdev.cdev, adcdev.dev, 1);
	if (status < 0)
		goto err_cdev_add;

	return 0;

err_cdev_add:
	printk(KERN_ALERT  "ADC char device add failed\n");
	device_destroy(adcdev.cl, adcdev.dev);

err_device_create:
	printk(KERN_ALERT "ADC device creation failed\n");
	class_destroy(adcdev.cl);

err_class_create:
	printk(KERN_ALERT "ADC class creation failed\n");
	unregister_chrdev_region(adcdev.dev, 1);

err_chrdev_alloc:
	printk(KERN_ALERT "ADC device registration failed\n");
	platform_driver_unregister(&am335x_driver);
	return -1;
}

static void am335xadc_exit(void)
{
	device_destroy(adcdev.cl, adcdev.dev);
	class_destroy(adcdev.cl);
	unregister_chrdev_region(adcdev.dev, 1);
	platform_driver_unregister(&am335x_driver);
	cdev_del(&adcdev.cdev);
	printk(KERN_INFO "ADC driver unregistered\n");
}

module_init(am335xadc_init);
module_exit(am335xadc_exit);

