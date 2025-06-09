package com.service;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

@Service
public class ApiStatsService {
    private final RestTemplate rest;
    private final String baseUrl;

    public ApiStatsService(RestTemplate rest,
                           @Value("${external.api.url}") String baseUrl) {
        this.rest    = rest;
        this.baseUrl = baseUrl;
    }

    /**
     * Fetch all stats in one shot from the C-daemon.
     */
    public String fetchAllStats() {
        return rest.getForObject(baseUrl + "/stats/all", String.class);
    }
}
