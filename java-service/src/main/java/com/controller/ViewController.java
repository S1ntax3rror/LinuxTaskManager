// java-service/src/main/java/com/OSProject/LinuxTaskManager/controller/ViewController.java
package com.controller;

import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;

@Controller
public class ViewController {
    @GetMapping("/")
    public String index() {
        return "index";
    }
}
