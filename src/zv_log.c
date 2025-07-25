#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include <linux/module.h>     
#include <linux/kernel.h>     
#include <linux/string.h>

#include "../include/zv_log.h"

EXPORT_SYMBOL(zv_log_write);
EXPORT_SYMBOL(zv_log_error);

EXPORT_SYMBOL(zv_log_init);
EXPORT_SYMBOL(zv_log_exit);

static struct zv_log_ring_buffer zv_log_buf;

static const char *log_level_str[] = {
    "NONE", "NORMAL", "DEBUG", "DETAIL"
};

void zv_log_write(
    enum log_level level,
    const char *tag,
    const char *fmt,
    ...
) {
    unsigned long flags;
    struct zv_log_entry *entry;
    va_list args;

    if(level > zv_log_buf.max_level) return;

    // get spin_lock
    spin_lock_irqsave(&zv_log_buf.lock, flags);

    entry = &zv_log_buf.buffer[zv_log_buf.head];
    entry->timestamp = ktime_get();
    entry->pid = current->pid;
    entry->level = level;
    strlcpy(entry->tag, tag, ZV_LOG_TAG_LEN);

    va_start(args, fmt);
    vsnprintf(entry->msg, ZV_LOG_MSG_LEN, fmt, args);
    va_end(args);

    zv_log_buf.head = (zv_log_buf.head + 1) % ZV_LOG_RING_SIZE;

    // if buffer is full
    if (zv_log_buf.head == zv_log_buf.tail)
        zv_log_buf.tail = (zv_log_buf.tail + 1) % ZV_LOG_RING_SIZE;

    spin_unlock_irqrestore(&zv_log_buf.lock, flags);
} 

void zv_log_error(int error_code) {
    zv_log_write(LOG_NONE, "Error", "Error Code: ", error_code);
}

static int zv_log_proc_show(struct seq_file *msg, void *v) {
    size_t i;
    unsigned long flags;

    spin_lock_irqsave(&zv_log_buf.lock, flags);
    i = zv_log_buf.tail;

    while(i != zv_log_buf.head) {
        struct zv_log_entry *entry = &zv_log_buf.buffer[i];

        seq_printf(msg, "[%llu ms] PID: %d [%-6s] [%-6s]: %s \n",
                // ktime_to_ns(entry->timestamp),
                ktime_to_ms(entry->timestamp),
                entry->pid,
                log_level_str[entry->level],
                entry->tag,
                entry->msg);

        i = (i + 1) % ZV_LOG_RING_SIZE;
    }

    spin_unlock_irqrestore(&zv_log_buf.lock, flags);

    return 0;
}

static int zv_log_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, zv_log_proc_show, NULL);
}

static const struct proc_ops zv_log_proc_fops = {
    .proc_open    = zv_log_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

// INIT 
void zv_log_init(void) {
    spin_lock_init(&zv_log_buf.lock);
    zv_log_buf.head = 0;
    zv_log_buf.tail = 0;
    zv_log_buf.max_level = LOG_DETAIL;

    proc_create("zv_log", 0444, NULL, &zv_log_proc_fops);

    zv_log_write(LOG_NORMAL, "init", "zeroVisor log system initialized");
}

// EXIT
void zv_log_exit(void) {
    remove_proc_entry("zv_log", NULL);
}



