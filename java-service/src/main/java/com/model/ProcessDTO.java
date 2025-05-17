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
    private int    nice;     // <— new
}
