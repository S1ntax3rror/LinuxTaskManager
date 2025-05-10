// java-service/src/main/java/com/OSProject/LinuxTaskManager/model/Ltm_Dto.java
package com.OSProject.LinuxTaskManager.model;

import lombok.Data;

/**
 * Maps exactly to the C struct `trimmed_info`.
 * Each instance represents one process snapshot.
 */
@Data
public class Ltm_Dto {
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

    /** Timestamp (ms since Unix epoch) when snapshot was taken */
    private long   timestampMs;
}
