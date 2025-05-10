// java-service/src/main/java/com/OSProject/LinuxTaskManager/service/MockProcessService.java
package com.OSProject.LinuxTaskManager.service;

import com.OSProject.LinuxTaskManager.model.Ltm_Dto;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

@Service
@Profile("dev")
public class MockProcessService implements ProcessService {
    private final Random rnd = new Random();

    @Override
    public List<Ltm_Dto> fetchProcesses() {
        List<Ltm_Dto> procs = new ArrayList<>();
        for (int i = 1; i <= 10; i++) {
            Ltm_Dto dto = new Ltm_Dto();
            dto.setPid(1000 + i);
            dto.setComm("process-" + i);
            dto.setState('R');
            dto.setCpuPercent(rnd.nextDouble() * 50);
            dto.setRamPercent(rnd.nextDouble() * 80);
            dto.setTimestampMs(System.currentTimeMillis());
            procs.add(dto);
        }
        return procs;
    }

    @Override
    public void sendCommand(String cmdJson) {
        System.out.println("[MOCK] sendCommand: " + cmdJson);
    }
}
