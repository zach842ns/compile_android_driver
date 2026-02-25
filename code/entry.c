#include <linux/module.h>
#include <linux/tty.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include "comm.h"
#include "memory.h"
#include "process.h"

#define zru_RANDOM_SUFFIX_LEN 8
static char zru_device_name[32] = {0};

/* 生成完整的设备名 */
static void generate_zru_device_name(void)
{
	int i;
	unsigned char random_bytes[zru_RANDOM_SUFFIX_LEN];
	
	/* 获取真正的随机字节 */
	get_random_bytes(random_bytes, zru_RANDOM_SUFFIX_LEN);
	
	/* 组合设备名: zru + 随机后缀 */
	zru_device_name[0] = 'z';
	zru_device_name[1] = 'r';
	zru_device_name[2] = 'u';
	
	/* 生成随机后缀，使用小写字母 */
	for (i = 0; i < zru_RANDOM_SUFFIX_LEN; i++) {
		/* 将随机字节映射到 'a'-'z' */
		zru_device_name[3 + i] = 'a' + (random_bytes[i] % 26);
	}
	zru_device_name[3 + zru_RANDOM_SUFFIX_LEN] = '\0';
	
	printk(KERN_INFO "zru: generated random device name: %s\n", zru_device_name);
}

#define DEVICE_NAME zru_device_name

int dispatch_open(struct inode *node, struct file *file)
{
	return 0;
}

int dispatch_close(struct inode *node, struct file *file)
{
	return 0;
}

long dispatch_ioctl(struct file *const file, unsigned int const cmd, unsigned long const arg)
{
	static COPY_MEMORY cm;
	static MODULE_BASE mb;
	static char key[0x100] = {0};
	static char name[0x100] = {0};
	static bool is_verified = false;

	if (cmd == OP_INIT_KEY && !is_verified)
	{
		if (copy_from_user(key, (void __user *)arg, sizeof(key) - 1) != 0)
		{
			return -1;
		}
	}
	switch (cmd)
	{
	case OP_READ_MEM:
	{
		if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0)
		{
			return -1;
		}
		if (read_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) == false)
		{
			return -1;
		}
		break;
	}
	case OP_WRITE_MEM:
	{
		if (copy_from_user(&cm, (void __user *)arg, sizeof(cm)) != 0)
		{
			return -1;
		}
		if (write_process_memory(cm.pid, cm.addr, cm.buffer, cm.size) == false)
		{
			return -1;
		}
		break;
	}
	case OP_MODULE_BASE:
	{
		if (copy_from_user(&mb, (void __user *)arg, sizeof(mb)) != 0 || copy_from_user(name, (void __user *)mb.name, sizeof(name) - 1) != 0)
		{
			return -1;
		}
		mb.base = getModuleBase(mb.pid, name);
		if (copy_to_user((void __user *)arg, &mb, sizeof(mb)) != 0)
		{
			return -1;
		}
		break;
	}
	default:
		break;
	}
	return 0;
}

struct file_operations dispatch_functions = {
	.owner = THIS_MODULE,
	.open = dispatch_open,
	.release = dispatch_close,
	.unlocked_ioctl = dispatch_ioctl,
};

static struct miscdevice misc;

int __init driver_entry(void)
{
	int ret;
	
	/* 生成随机设备名 */
	generate_zru_device_name();
	
	/* 初始化miscdevice结构体 */
	misc.minor = MISC_DYNAMIC_MINOR;
	misc.name = DEVICE_NAME;
	misc.fops = &dispatch_functions;
	
	ret = misc_register(&misc);
	if (ret == 0) {
		printk(KERN_INFO "zru: device registered as /dev/%s\n", DEVICE_NAME);
	} else {
		printk(KERN_ERR "zru: failed to register device\n");
	}
	return ret;
}

void __exit driver_unload(void)
{
	printk(KERN_INFO "zru: device /dev/%s unregistered\n", DEVICE_NAME);
	misc_deregister(&misc);
}

module_init(driver_entry);
module_exit(driver_unload);

MODULE_DESCRIPTION("Tg t.me/Zachnhyyd");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("秋冬");