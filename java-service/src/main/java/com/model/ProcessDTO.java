package com.model;

import lombok.Data;

/**
 * Maps exactly to the C struct `trimmed_info`.
 */
@Data
public class ProcessDTO {
    /** Process ID */
    private int    pid;

    /** Executable name, e.g. "bash" */
    private String comm;

    /** State character, e.g. 'R', 'S', 'D' */
    private char   state;

    /** CPU usage (%) since last snapshot */
    private double cpuPercent;

    /** RAM usage (%) at this snapshot */
    private double ramPercent;

    /** Nice value */
    private int    nice;

    /* ─── NEW FIELDS ─── */

    /** Owner’s username */
    private String username;

    /** Priority (prio) */
    private int    prio;

    /** Virtual memory in KiB */
    private long   virt;

    /** Resident memory in KiB */
    private long   res;

    /** Shared memory in KiB */
    private long   shared;

    /** Full command line including args */
    private String cmd;

    /** Up‐time in seconds */
    private double upTime;
}
