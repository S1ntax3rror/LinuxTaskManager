// java-service/src/main/java/com/OSProject/LinuxTaskManager/controller/ProcessController.java
package com.controller;

import com.model.*;
import com.service.*;
import org.springframework.http.*;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/processes")
public class ProcessController {

  private final ProcessService svc;
  public ProcessController(ProcessService svc) { this.svc = svc; }

  @GetMapping
  public List<ProcessDTO> all() {
    return svc.fetchProcesses();
  }

  @PostMapping("/{pid}/signal")
  @ResponseStatus(HttpStatus.NO_CONTENT)
  public void signal(@PathVariable int pid,
                     @RequestBody SignalRequest body) {
    svc.sendSignal(pid, body.getCmd());
  }
}