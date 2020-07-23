#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sh_diag.h>

extern const struct seq_operations ov5693_proc_op;

static int camera_open(struct inode *inode, struct file *file)
{
        return seq_open(file, &ov5693_proc_op);
}

static const struct file_operations proc_cpuinfo_operations = {
        .open           = camera_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = seq_release,
};


static int __init proc_camerainfo_init(void)
{
        proc_create("sw_camera_info", 0, NULL, &proc_cpuinfo_operations);
        return 0;
}

fs_initcall(proc_camerainfo_init);

