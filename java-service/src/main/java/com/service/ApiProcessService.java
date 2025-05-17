// java-service/src/main/java/com/service/ApiProcessService.java
package com.service;

import java.util.*;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import com.model.*;

@Service
public class ApiProcessService {
  private final RestTemplate rest;
  private final String baseUrl;

  public ApiProcessService(RestTemplate rest,
                           @Value("${external.api.url}") String baseUrl) {
    this.rest    = rest;
    this.baseUrl = baseUrl;
  }

  public List<ProcessDTO> fetchProcesses() {
    ProcessDTO[] arr = rest.getForObject(baseUrl + "/processes",
                                         ProcessDTO[].class);
    return arr == null
         ? Collections.emptyList()
         : Arrays.asList(arr);
  }

  public void sendSignal(int pid, String cmd) {
    SignalRequest req = new SignalRequest();
    req.setCmd(cmd);
    rest.postForLocation(
      String.format("%s/processes/%d/signal", baseUrl, pid),
      req
    );
  }
}
