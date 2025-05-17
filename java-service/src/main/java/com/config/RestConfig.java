// src/main/java/com/OSProject/LinuxTaskManager/config/RestConfig.java
package com.config;

import org.springframework.boot.web.client.RestTemplateBuilder;
import org.springframework.context.annotation.*;
import org.springframework.web.client.RestTemplate;

@Configuration
public class RestConfig {
  @Bean
  public RestTemplate restTemplate(RestTemplateBuilder b) {
    return b.build();
  }
}
