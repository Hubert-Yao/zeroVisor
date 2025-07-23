#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>

#define ZV_LOG_RING_SIZE 1024
#define ZV_LOG_MSG_LEN 256
#define ZV_LOG_TAG_LEN 16

enum log_level {
    LOG_NONE = 0,
    LOG_NORMAL,
    LOG_DEBUG,
    LOG_DETAIL,
};

struct zv_log_entry {
    ktime_t timestamp;
    pid_t pid;
    enum log_level level;
    char tag[ZV_LOG_TAG_LEN];
    char msg[ZV_LOG_MSG_LEN];
};

struct zv_log_ring_buffer {
    struct zv_log_entry buffer[ZV_LOG_RING_SIZE];
    size_t head;
    size_t tail;
    spinlock_t lock;
    enum log_level max_level;
};

void zv_log_write(enum log_level level, const char *tag, const char *fmt, ...);
void zv_log_error(int error_code);
void zv_log_init(void);
void zv_log_exit(void);


