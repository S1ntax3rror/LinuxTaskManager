// src/main/java/com/osproject/ltm/service/MockProcessService.java
package com.service;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import org.springframework.stereotype.Service;
import com.model.*;

@Service
public class MockProcessService implements ProcessService {
  private final Random rnd = new Random();

  @Override
  public List<ProcessDTO> fetchProcesses() {
    List<ProcessDTO> procs = new ArrayList<>();
    for (int i = 1; i <= 10; i++) {
      ProcessDTO p = new ProcessDTO();
      p.setPid(1000 + i);
      p.setComm("mock-" + i);
      p.setState('R');
      p.setCpuPercent(rnd.nextDouble() * 50);
      p.setRamPercent(rnd.nextDouble() * 80);
      p.setTimestampMs(System.currentTimeMillis());
      procs.add(p);
    }
    return procs;
  }

  @Override
  public void sendSignal(int pid, String cmd) {
    System.out.printf("[MOCK] signal pid=%d cmd=%s%n", pid, cmd);
  }
}
