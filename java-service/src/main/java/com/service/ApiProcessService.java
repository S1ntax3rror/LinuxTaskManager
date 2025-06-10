package com.service;

import com.model.*;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

@Service
public class ApiProcessService {

    private final RestTemplate rest;
    private final String baseUrl;

    public ApiProcessService(RestTemplate rest,
                             @Value("${external.api.url}") String baseUrl) {
        this.rest = rest;
        this.baseUrl = baseUrl;
    }

    public List<ProcessDTO> fetchProcesses() {
        ProcessDTO[] arr = rest.getForObject(baseUrl + "/processes", ProcessDTO[].class);
        return arr == null
             ? Collections.emptyList()
             : Arrays.asList(arr);
    }

    public void sendSignal(int pid, String cmd) {
        SignalRequestDTO req = new SignalRequestDTO();
        req.setCmd(cmd);
        rest.postForLocation(
            String.format("%s/processes/%d/signal", baseUrl, pid),
            req
        );
    }

    public void renice(int pid, int nice) {
        ReniceRequestDTO req = new ReniceRequestDTO();
        req.setNice(nice);
        rest.postForLocation(
            String.format("%s/processes/%d/renice", baseUrl, pid),
            req
        );
    }

    public void limitCpu(int pid, int secs) {
        LimitRequestDTO req = new LimitRequestDTO();
        req.setLimit(secs);
        rest.postForLocation(
            String.format("%s/processes/%d/limit/cpu", baseUrl, pid),
            req
        );
    }

    public void limitRam(int pid, int mb) {
        LimitRequestDTO req = new LimitRequestDTO();
        req.setLimit(mb);
        rest.postForLocation(
            String.format("%s/processes/%d/limit/ram", baseUrl, pid),
            req
        );
    }
}
