/* telemetry.h - optional producer side of the always-on flight recorder.
 *
 * Kernel builds resolve these weak references to zllog.c. Standalone host
 * harnesses may omit zllog entirely and keep testing the shipping source.
 * Producers never perform storage I/O through this interface. */
#ifndef ZL_TELEMETRY_H
#define ZL_TELEMETRY_H

#include "zllog.h"

extern void zllog_event(unsigned, unsigned, unsigned,
                        unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_event_irq(unsigned, unsigned, unsigned,
                            unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_counter_add(unsigned, unsigned) __attribute__((weak));
extern void zllog_counter_observe(unsigned, unsigned) __attribute__((weak));
extern unsigned zllog_span_begin(unsigned, unsigned, unsigned)
    __attribute__((weak));
extern void zllog_span_end(unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_trigger(unsigned, unsigned, unsigned,
                          unsigned, unsigned, unsigned) __attribute__((weak));
extern void zllog_lifecycle(unsigned, unsigned, unsigned, unsigned,
                            unsigned, unsigned) __attribute__((weak));
extern unsigned zllog_operation_begin(unsigned, unsigned, unsigned, unsigned,
                                      unsigned) __attribute__((weak));
extern void zllog_operation_result(unsigned, unsigned, unsigned, int,
                                   unsigned, unsigned) __attribute__((weak));
extern int zllog_burst_active(void) __attribute__((weak));
extern int zllog_io_active(void) __attribute__((weak));

static inline int zlt_suppressed(void)
{
    return zllog_io_active && zllog_io_active();
}

static inline void zlt_event(unsigned subsystem, unsigned event,
                             unsigned severity, unsigned a, unsigned b,
                             unsigned c)
{
    if (zlt_suppressed()) {
        /* Recorder USB work must not recursively re-enter the normal ring,
         * but hiding its own timeout/failure is worse. Defer WARN+ boundaries
         * through the IRQ-safe RAM lane; normal context drains it after I/O. */
        if (severity >= ZLLOG_WARN && zllog_event_irq)
            zllog_event_irq(subsystem, event, severity, a, b, c);
    } else if (zllog_event) {
        zllog_event(subsystem, event, severity, a, b, c);
    }
}

static inline void zlt_irq_event(unsigned subsystem, unsigned event,
                                 unsigned severity, unsigned a, unsigned b,
                                 unsigned c)
{
    if (zllog_event_irq)
        zllog_event_irq(subsystem, event, severity, a, b, c);
}

static inline void zlt_count(unsigned counter, unsigned delta)
{
    if (!zlt_suppressed() && zllog_counter_add) zllog_counter_add(counter, delta);
}

static inline void zlt_observe(unsigned counter, unsigned value)
{
    if (!zlt_suppressed() && zllog_counter_observe)
        zllog_counter_observe(counter, value);
}

static inline unsigned zlt_span_begin(unsigned subsystem, unsigned operation,
                                      unsigned detail)
{
    return !zlt_suppressed() && zllog_span_begin
        ? zllog_span_begin(subsystem, operation, detail) : 0;
}

static inline void zlt_span_end(unsigned subsystem, unsigned span,
                                unsigned status)
{
    if (span && zllog_span_end) zllog_span_end(subsystem, span, status);
}

static inline void zlt_trigger(unsigned subsystem, unsigned event,
                               unsigned severity, unsigned a, unsigned b,
                               unsigned c)
{
    if (zlt_suppressed()) {
        if (zllog_event_irq)
            zllog_event_irq(subsystem, event, severity, a, b, c);
        return;
    }
    if (zllog_trigger) zllog_trigger(subsystem, event, severity, a, b, c);
    else zlt_event(subsystem, event, severity, a, b, c);
}

static inline void zlt_snapshot(unsigned subsystem, unsigned operation,
                                unsigned phase, unsigned raw0, unsigned raw1)
{
    zlt_event(subsystem, ZLLOG_EV_SNAPSHOT, ZLLOG_INFO,
              (operation << 8) | (phase & 0xffu), raw0, raw1);
}

static inline void zlt_lifecycle(unsigned subsystem, unsigned object_kind,
                                 unsigned object_id, unsigned action,
                                 unsigned parent_id, unsigned detail)
{
    if (!zlt_suppressed() && zllog_lifecycle)
        zllog_lifecycle(subsystem, object_kind, object_id, action,
                        parent_id, detail);
}

static inline unsigned zlt_operation_begin(unsigned subsystem,
                                            unsigned actor_kind,
                                            unsigned actor_id,
                                            unsigned operation,
                                            unsigned object_id)
{
    return !zlt_suppressed() && zllog_operation_begin
        ? zllog_operation_begin(subsystem, actor_kind, actor_id,
                                operation, object_id) : 0;
}

static inline void zlt_operation_result(unsigned subsystem,
                                        unsigned operation_id,
                                        unsigned operation, int result,
                                        unsigned error, unsigned detail)
{
    if (operation_id && zllog_operation_result)
        zllog_operation_result(subsystem, operation_id, operation, result,
                               error, detail);
}

#endif
