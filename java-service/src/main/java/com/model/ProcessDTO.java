package com.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

@Data
public class ProcessDTO {
    /** Process ID */
    private int     pid;

    /** Full command-line (spaces instead of nulls). Empty ⇒ kernel thread. */
    private String  cmd;

    /** Executable name, e.g. "bash" */
    private String  comm;

    /** Process state (e.g. 'R', 'S', 'D') */
    private char    state;

    /** CPU usage (%) since last snapshot */
    private double  cpuPercent;

    /** RAM usage (%) at this snapshot */
    private double  ramPercent;

    /** Nice value */
    private int     nice;

    /** Username (owner of the process) */
    private String  username;

    /** Priority (kernel “prio” field) */
    private int     prio;

    /** Virtual memory in KiB */
    private long    virt;

    /** Resident memory (in KiB) */
    private long    res;

    /** Shared memory (in KiB) */
    private long    shared;

    /** How many seconds this process has been running */
    private double  upTime;

      /** 1 if this is a sleeper thread, 0 otherwise */
    @JsonProperty("is_sleeper")
    private int isSleeper;
}
