// src/main/java/com/OSProject/LinuxTaskManager/service/ProcessService.java
package com.OSProject.LinuxTaskManager.service;

import com.OSProject.LinuxTaskManager.model.Ltm_Dto;
import java.util.List;

/**
 * Abstraction for fetching snapshots and sending commands.
 */
public interface ProcessService {
    List<Ltm_Dto> fetchProcesses() throws Exception;
    void sendCommand(String cmdJson) throws Exception;
}
