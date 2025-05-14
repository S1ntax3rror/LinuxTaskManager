// src/main/java/com/OSProject/LinuxTaskManager/service/ProcessService.java
package com.service;

import com.model.*;
import java.util.List;

/**
 * Abstraction for fetching snapshots and sending commands.
 */
public interface ProcessService {
    List<ProcessDTO> fetchProcesses();
    void sendSignal(int pid, String cmd);
  }