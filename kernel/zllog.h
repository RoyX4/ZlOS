/* zllog.h - the bounded RAM flight-recorder seam.
 *
 * Callers never perform USB I/O.  They append small records here; zllog.c
 * decides when the specifically prepared ZLLOG partition may be touched.
 * Keeping that split is what prevents diagnostics from becoming input lag.
 */
#ifndef ZL_ZLLOG_H
#define ZL_ZLLOG_H

enum zllog_subsystem {
    ZLLOG_SUB_BOOT    = 1,
    ZLLOG_SUB_INPUT   = 2,
    ZLLOG_SUB_FRAME   = 3,
    ZLLOG_SUB_USB     = 4,
    ZLLOG_SUB_STORAGE = 5,
    ZLLOG_SUB_KERNEL  = 6,
    ZLLOG_SUB_CPU     = 7,
    ZLLOG_SUB_IRQ     = 8,
    ZLLOG_SUB_DRIVER  = 9,
    ZLLOG_SUB_FS      = 10,
    ZLLOG_SUB_SYSCALL = 11,
    ZLLOG_SUB_SCHED   = 12,
    ZLLOG_SUB_MEMORY  = 13,
    ZLLOG_SUB_NET     = 14,
    ZLLOG_SUB_DISPLAY = 15,
    ZLLOG_SUB_PERF    = 16,
    ZLLOG_SUB_LAB     = 17
};

enum zllog_event_code {
    ZLLOG_EV_BOOT_START       = 1,
    ZLLOG_EV_BOOT_MILESTONE   = 2,
    ZLLOG_EV_STORAGE_READY    = 3,
    ZLLOG_EV_STORAGE_REFUSED  = 4,
    ZLLOG_EV_STORAGE_XHCI     = 5,
    ZLLOG_EV_INPUT_BATCH      = 10,
    ZLLOG_EV_INPUT_EVENT      = 11,
    ZLLOG_EV_INPUT_DROP       = 12,
    ZLLOG_EV_FRAME            = 20,
    ZLLOG_EV_FRAME_LATE       = 21,
    ZLLOG_EV_FLUSH_ERROR      = 30,
    ZLLOG_EV_TEXT             = 31,

    /* Stable cross-subsystem catalogue. Values 40+ are deliberately generic:
     * a command in NVMe and a command in xHCI have the same lifecycle shape,
     * while subsystem + numeric detail still names the concrete producer. */
    ZLLOG_EV_DRIVER_STATE     = 40,
    ZLLOG_EV_COMMAND_SUBMIT   = 41,
    ZLLOG_EV_COMMAND_COMPLETE = 42,
    ZLLOG_EV_TIMEOUT          = 43,
    ZLLOG_EV_FAULT            = 44,
    ZLLOG_EV_PANIC            = 45,
    ZLLOG_EV_SYSCALL_ENTER    = 46,
    ZLLOG_EV_SYSCALL_EXIT     = 47,
    ZLLOG_EV_FS_MUTATION      = 48,
    ZLLOG_EV_DROP             = 49,
    ZLLOG_EV_COUNTER          = 50,
    ZLLOG_EV_SPAN_BEGIN       = 51,
    ZLLOG_EV_SPAN_END         = 52,
    ZLLOG_EV_TRIGGER          = 53,
    ZLLOG_EV_BURST            = 54,
    ZLLOG_EV_MEMORY           = 55,
    ZLLOG_EV_PROCESS          = 56,
    ZLLOG_EV_NET_STATE        = 57,
    ZLLOG_EV_DISPLAY_STATE    = 58,
    ZLLOG_EV_FRAME_EXT        = 59,
    ZLLOG_EV_FRAME_BURST      = 60,
    ZLLOG_EV_RECORDER_DROP    = 61,
    ZLLOG_EV_CHECKPOINT       = 62,
    ZLLOG_EV_POINTER          = 63,
    /* Bounded anomaly snapshot. value0 packs operation in bits 31:8 and phase
     * in bits 7:0 (0=before, 1=at trigger, 2=after recovery); value1/value2
     * are producer-defined raw registers/state words. */
    ZLLOG_EV_SNAPSHOT         = 64,
    /* Per-retained-frame compositor attribution. */
    ZLLOG_EV_FRAME_PAINT      = 65,
    /* Typed audit trail. Lifecycle names durable objects; operation begin and
     * result share an operation id so the host can reconstruct causality even
     * when unrelated CPUs/subsystems interleave records. No user content is
     * stored: IDs, operation codes, byte counts, results and errors only. */
    ZLLOG_EV_LIFECYCLE        = 66,
    ZLLOG_EV_OPERATION_BEGIN  = 67,
    ZLLOG_EV_OPERATION_RESULT = 68
};

enum zllog_object_kind {
    ZLLOG_OBJ_PROCESS = 1,
    ZLLOG_OBJ_APP     = 2,
    ZLLOG_OBJ_WINDOW  = 3,
    ZLLOG_OBJ_FILE    = 4,
    ZLLOG_OBJ_SOCKET  = 5,
    ZLLOG_OBJ_DNS     = 6,
    ZLLOG_OBJ_HTTP    = 7,
    ZLLOG_OBJ_KERNEL  = 8
};

enum zllog_lifecycle_action {
    ZLLOG_LIFE_START   = 1,
    ZLLOG_LIFE_READY   = 2,
    ZLLOG_LIFE_EXIT    = 3,
    ZLLOG_LIFE_FAULT   = 4,
    ZLLOG_LIFE_REFUSED = 5
};

/* Operation ranges are stable and intentionally grouped for host decoding. */
enum zllog_audit_operation {
    ZLLOG_OP_SYSCALL_BASE = 0x1000,

    ZLLOG_OP_FILE_MOUNT = 0x2001,
    ZLLOG_OP_FILE_FORMAT,
    ZLLOG_OP_FILE_FIND,
    ZLLOG_OP_FILE_CREATE,
    ZLLOG_OP_FILE_OPEN,
    ZLLOG_OP_FILE_READ,
    ZLLOG_OP_FILE_WRITE,
    ZLLOG_OP_FILE_CLOSE,
    ZLLOG_OP_FILE_INFO,
    ZLLOG_OP_FILE_DELETE,
    ZLLOG_OP_FILE_RENAME,
    ZLLOG_OP_FILE_SYNC,

    ZLLOG_OP_NET_CONNECT = 0x3001,
    ZLLOG_OP_NET_SEND,
    ZLLOG_OP_NET_RECEIVE,
    ZLLOG_OP_NET_CLOSE,
    ZLLOG_OP_NET_ABORT,
    ZLLOG_OP_DNS_RESOLVE,
    ZLLOG_OP_HTTP_REQUEST,

    ZLLOG_OP_WINDOW_OPEN = 0x4001,
    ZLLOG_OP_WINDOW_PRESENT,
    ZLLOG_OP_WINDOW_POLL,
    ZLLOG_OP_WINDOW_CLOSE,

    ZLLOG_OP_IPC_SEND = 0x5001,
    ZLLOG_OP_IPC_RECEIVE
};

/* Counters are RAM-only increments on hot paths. The normal-context flusher
 * periodically emits changed totals, sample counts and maxima as EV_COUNTER.
 * IDs are stable because the host extractor gives them human names. */
enum zllog_counter_id {
    ZLLOG_C_IRQ_TIMER = 1,
    ZLLOG_C_IRQ_KEYBOARD,
    ZLLOG_C_IRQ_MOUSE,
    ZLLOG_C_IRQ_STRAY,
    ZLLOG_C_XHCI_EVENT,
    ZLLOG_C_XHCI_COMMAND,
    ZLLOG_C_XHCI_TRANSFER,
    ZLLOG_C_MMIO_POLL,
    ZLLOG_C_NVME_COMMAND,
    ZLLOG_C_NVME_POLL,
    ZLLOG_C_NET_RX,
    ZLLOG_C_NET_TX,
    ZLLOG_C_NET_DROP,
    ZLLOG_C_ALLOC,
    ZLLOG_C_FREE,
    ZLLOG_C_ALLOC_BYTES,
    ZLLOG_C_HEAP_REFUSE,
    ZLLOG_C_SCHED_SWITCH,
    ZLLOG_C_SCHED_YIELD,
    ZLLOG_C_SYSCALL,
    ZLLOG_C_FS_READ,
    ZLLOG_C_FS_WRITE,
    ZLLOG_C_FS_MUTATION,
    ZLLOG_C_FRAME_PAINT,
    ZLLOG_C_FRAME_LATE,
    ZLLOG_C_FRAME_LOST,
    ZLLOG_C_RENDERED_PIXELS,
    ZLLOG_C_PRESENT_BYTES,
    ZLLOG_C_INPUT_QUEUE,
    ZLLOG_C_INPUT_DROP,
    ZLLOG_C_RECORDER_OVERWRITE,
    ZLLOG_C_USB_WRITE_BYTES,
    ZLLOG_C_FLUSH_US,
    ZLLOG_COUNTER_LIMIT
};

enum zllog_severity {
    ZLLOG_INFO  = 0,
    ZLLOG_WARN  = 1,
    ZLLOG_ERROR = 2,
    ZLLOG_FATAL = 3
};

enum zllog_snapshot_operation {
    ZLLOG_SNAP_XHCI_WAIT       = 1,
    ZLLOG_SNAP_NVME_CSTS       = 2,
    ZLLOG_SNAP_NVME_COMPLETION = 3,
    ZLLOG_SNAP_GPU_FORCEWAKE   = 4,
    ZLLOG_SNAP_GPU_RING        = 5,
    ZLLOG_SNAP_INTEL_VBLANK    = 6,
    ZLLOG_SNAP_NET_WAIT        = 7,
    ZLLOG_SNAP_TCP_TIMEOUT     = 8,
    ZLLOG_SNAP_XHCI_COMMAND    = 9,
    ZLLOG_SNAP_XHCI_TRANSFER   = 10
};

/* Safe before PCI, timers, heap and storage. */
void zllog_putc(char c);
void zllog_event(unsigned subsystem, unsigned event, unsigned severity,
                 unsigned a, unsigned b, unsigned c);
/* Interrupt/exception-safe producer. It only claims a fixed RAM emergency
 * cell; normal kernel context drains those cells into the ordered journal. */
void zllog_event_irq(unsigned subsystem, unsigned event, unsigned severity,
                     unsigned a, unsigned b, unsigned c);
void zllog_counter_add(unsigned counter, unsigned delta);
void zllog_counter_observe(unsigned counter, unsigned value);
unsigned zllog_span_begin(unsigned subsystem, unsigned operation,
                          unsigned detail);
void zllog_span_end(unsigned subsystem, unsigned span, unsigned status);
void zllog_trigger(unsigned subsystem, unsigned event, unsigned severity,
                   unsigned a, unsigned b, unsigned c);
void zllog_lifecycle(unsigned subsystem, unsigned object_kind,
                     unsigned object_id, unsigned action,
                     unsigned parent_id, unsigned detail);
unsigned zllog_operation_begin(unsigned subsystem, unsigned actor_kind,
                               unsigned actor_id, unsigned operation,
                               unsigned object_id);
void zllog_operation_result(unsigned subsystem, unsigned operation_id,
                            unsigned operation, int result, unsigned error,
                            unsigned detail);
int zllog_burst_active(void);
int zllog_io_active(void);

/* Called by input.c through weak seams so its host-only tests stay standalone. */
void zllog_input_batch(unsigned processed, unsigned depth, unsigned drops);
void zllog_input_event(unsigned type, unsigned code, unsigned depth);
void zllog_pointer_event(unsigned x, unsigned y, unsigned buttons,
                         unsigned depth, unsigned source_tsc,
                         unsigned sequence);
void zllog_frame(unsigned input_us, unsigned tick_us, unsigned compositor_us,
                 unsigned vblank_us, unsigned present_us, unsigned total_us,
                 unsigned flags, unsigned damage_count, unsigned damage_area);
void zllog_frame_observe(unsigned input_us, unsigned tick_us,
                         unsigned compositor_us, unsigned vblank_us,
                         unsigned present_us, unsigned total_us,
                         unsigned flags, unsigned damage_count,
                         unsigned damage_area, unsigned input_to_present_us,
                         unsigned input_sequence, unsigned missed_deadlines,
                         unsigned queue_depth, unsigned present_bytes,
                         unsigned desk_us, unsigned chrome_us,
                         unsigned app_us, unsigned effects_us,
                         unsigned repaint_rects, unsigned repaint_pixels,
                         unsigned window_visits, unsigned app_calls);

/* Storage is attempted only from normal kernel context, never an IRQ. */
int  zllog_mount(void);
int  zllog_ready(void);
int  zllog_flush(void);
int  zllog_complete(void);
void zllog_flush_if_due(void);
void zllog_milestone(unsigned id, unsigned value);

unsigned zllog_buffered(void);
unsigned zllog_dropped(void);
unsigned zllog_last_error(void);

#endif
