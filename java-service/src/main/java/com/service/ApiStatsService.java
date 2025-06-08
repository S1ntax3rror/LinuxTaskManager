// ApiStatsService.java
package com.service;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

@Service
public class ApiStatsService {
    // RestTemplate to call our C‑daemon endpoints
    private final RestTemplate rest;
    // Base URL for the C‑daemon API (e.g. http://localhost:9000/api)
    private final String baseUrl;

    public ApiStatsService(RestTemplate rest,
                           @Value("${external.api.url}") String baseUrl) {
        this.rest = rest;
        this.baseUrl = baseUrl;
    }

    // Fetch network statistics JSON from C‑daemon
    public String fetchNetworkStats() {
        return rest.getForObject(baseUrl + "/stats/network", String.class);
    }

    // Fetch disk statistics JSON from C‑daemon
    public String fetchDiskStats() {
        return rest.getForObject(baseUrl + "/stats/disk", String.class);
    }

    // Fetch CPU+Memory stats JSON from C‑daemon
    public String fetchCpuMemStats() {
        return rest.getForObject(baseUrl + "/cpu_mem", String.class);
    }

    // Fetch general system stats (loadavg, memory, CPU%) JSON
    public String fetchGeneralStats() {
        return rest.getForObject(baseUrl + "/stats/general", String.class);
    }
}
