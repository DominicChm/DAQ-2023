/**
 * How many ticks can be queued between samplers and writers
 * If logs are failing with QUEUE_FULL, this should be increased.
 *
 * Slow SD cards might need a larger queue
 */
#define DLF_QUEUE_SIZE 64

#define DLF_MIN_SAMPLE_BUFFER 512