// src/main/java/com/OSProject/LinuxTaskManager/model/ProcessDto.java
package com.OSProject.LinuxTaskManager.model;

import lombok.Data;

/**
 * A lightweight view of a process, matching the C struct `trimmed_info`.
 * Fields:
 *   - pid           : process ID
 *   - comm          : executable name
 *   - state         : process state (e.g. 'R', 'S', 'D')
 *   - cpuPercent    : CPU usage since last snapshot
 *   - ramPercent    : RAM usage at snapshot time
 *   - timestampMs   : Unix epoch millis when this snapshot was taken
 */
@Data
public class Ltm_Dto { // naming stands for Linux Task Manager - Data Transfer Object 
    private int    pid;
    private String comm;
    private char   state;
    private double cpuPercent;
    private double ramPercent;
    private long   timestampMs;
}
