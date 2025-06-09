package com.controller;

import com.service.ApiStatsService;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api")
public class StatsController {
    private final ApiStatsService statsSvc;

    public StatsController(ApiStatsService statsSvc) {
        this.statsSvc = statsSvc;
    }

    /**
     * Proxy GET /api/stats/all → C-daemon /api/stats/all
     */
    @GetMapping(value = "/stats/all", produces = MediaType.APPLICATION_JSON_VALUE)
    public String getAllStats() {
        return statsSvc.fetchAllStats();
    }
}
