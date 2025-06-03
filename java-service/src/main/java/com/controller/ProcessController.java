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
    public void signal(@PathVariable int pid,
                       @RequestBody SignalRequest body) {
        svc.sendSignal(pid, body.getCmd());
    }

    @PostMapping("/{pid}/renice")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void renice(@PathVariable int pid,
                       @RequestBody ReniceRequest body) {
        svc.renice(pid, body.getNice());
    }

    @PostMapping("/{pid}/limit/cpu")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void limitCpu(@PathVariable int pid,
                         @RequestBody LimitRequest body) {
        svc.limitCpu(pid, body.getLimit());
    }

    @PostMapping("/{pid}/limit/ram")
    @ResponseStatus(HttpStatus.NO_CONTENT)
    public void limitRam(@PathVariable int pid,
                         @RequestBody LimitRequest body) {
        svc.limitRam(pid, body.getLimit());
    }
}
