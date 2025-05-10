// java-service/src/main/java/com/OSProject/LinuxTaskManager/controller/ProcessController.java
package com.OSProject.LinuxTaskManager.controller;

import com.OSProject.LinuxTaskManager.model.Ltm_Dto;
import com.OSProject.LinuxTaskManager.service.ProcessService;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.server.ResponseStatusException;

import java.util.List;

@RestController
@RequestMapping("/api/processes")
public class ProcessController {
    private final ProcessService svc;

    public ProcessController(ProcessService svc) {
        this.svc = svc;
    }

    @GetMapping
    public List<Ltm_Dto> all() throws Exception {
        return svc.fetchProcesses();
    }

    @GetMapping("/{pid}")
    public Ltm_Dto one(@PathVariable int pid) throws Exception {
        return svc.fetchProcesses().stream()
                  .filter(p -> p.getPid() == pid)
                  .findFirst()
                  .orElseThrow(() -> new ResponseStatusException(HttpStatus.NOT_FOUND));
    }

    @PostMapping("/{pid}/signal")
    public void signal(@PathVariable int pid,
                       @RequestParam("cmd") String cmd) throws Exception {
        String json = String.format("{\"pid\":%d,\"cmd\":\"%s\"}", pid, cmd);
        svc.sendCommand(json);
    }
}
