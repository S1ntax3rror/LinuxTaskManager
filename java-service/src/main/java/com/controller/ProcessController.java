package com.controller;

import com.model.*;
import com.service.ApiProcessService;
import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/processes")
public class ProcessController {

    private final ApiProcessService svc;

    public ProcessController(ApiProcessService svc) {
        this.svc = svc;
    }

    @GetMapping
    public List<ProcessDTO> all() {
        return svc.fetchProcesses();
    }

    @PostMapping("/{pid}/signal")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void signal(
            @PathVariable int pid,
            @RequestBody(required = false) SignalRequestDTO body
    ) {
        // Default to "KILL" if no body or empty cmd
        String cmd = "KILL";
        if (body != null && body.getCmd() != null && !body.getCmd().trim().isEmpty()) {
            cmd = body.getCmd().trim();
        }

        // Attempt to forward to C-daemon. If C-daemon returns 4xx/5xx or is down,
        // let those exceptions bubble up so the front-end sees a network error.
        svc.sendSignal(pid, cmd);
    }


    @PostMapping("/{pid}/renice")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void renice(@PathVariable int pid,
                       @RequestBody ReniceRequestDTO body) {
        svc.renice(pid, body.getNice());
    }

    @PostMapping("/{pid}/limit/cpu")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void limitCpu(@PathVariable int pid,
                         @RequestBody LimitRequestDTO body) {
        svc.limitCpu(pid, body.getLimit());
    }

    @PostMapping("/{pid}/limit/ram")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void limitRam(@PathVariable int pid,
                         @RequestBody LimitRequestDTO body) {
        svc.limitRam(pid, body.getLimit());
    }
}
