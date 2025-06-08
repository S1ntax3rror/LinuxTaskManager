package com.controller;

import com.service.ApiStatsService;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api")
public class StatsController {
    private final ApiStatsService statsSvc;

    public StatsController(ApiStatsService statsSvc) {
        this.statsSvc = statsSvc;
    }

    // Forward GET /api/stats/network → our internal C-daemon
    @GetMapping(value = "/stats/network", produces = MediaType.APPLICATION_JSON_VALUE)
    public String getNetwork() {
        return statsSvc.fetchNetworkStats();
    }

    // Forward GET /api/stats/disk → our internal C-daemon
    @GetMapping(value = "/stats/disk", produces = MediaType.APPLICATION_JSON_VALUE)
    public String getDisk() {
        return statsSvc.fetchDiskStats();
    }

    // Forward GET /api/cpu_mem → our internal C-daemon
    @GetMapping(value = "/cpu_mem", produces = MediaType.APPLICATION_JSON_VALUE)
    public String getCpuMem() {
        return statsSvc.fetchCpuMemStats();
    }

    // Forward GET /api/stats/general → our internal C-daemon
    @GetMapping(value = "/stats/general", produces = MediaType.APPLICATION_JSON_VALUE)
    public String getGeneral() {
        return statsSvc.fetchGeneralStats();
    }
}
