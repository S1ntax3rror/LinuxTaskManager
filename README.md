# LinuxTaskManager

A complete, modular Linux process monitor & manager with C core, JSON daemon, Spring Boot REST API, and Thymeleaf web UI.

## Features
- **C core library**: parses `/proc` into `proc_stat` structs  
- **Background daemon**: polls core and serves JSON over a UNIX socket  
- **Spring Boot REST API**: exposes process data to the web front end  
- **Thymeleaf Web UI**: dashboard, filtering, and controls in the browser

## Dependencies and Prerequisites

### C Libraries
- **libmicrohttpd** (embedded HTTP server)  
- **libcjson** (JSON parsing/serialization)  
- **POSIX APIs** (e.g. `<sys/time.h>`, `<signal.h>`, `<sys/resource.h>`)

### Java / Spring Dependencies
*(managed via Maven `pom.xml`)*  
- **Spring Boot Starter Web** (REST API & Thymeleaf)  
- **Spring Boot Starter Thymeleaf**  
- **Jackson** (JSON serialization)

### Prerequisites to run our projects are:
- **gcc**, **make** (C toolchain)  
- **Maven**  
- **Java 17+** (OpenJDK or Oracle JDK)

## How to run it:
Inside the root folder OS_TeamProject/LinuxTaskManager/ run following command to initiate all make files:
- make all

After that navigate to the c-daemon folder and start the server with this command:
- ./http_server 

The last step is to launch the frontend. To do this navigate to the package java-service and make sure that you have mvn installed on your machine. Then run following command in the terminal:

- mvn spring-boot:run -Dspring-boot.run.profiles=api

The "mvn" command invokes the maven framework which uses the projects pom.xml to figure out what plugins and dependencies we are using. The spring-boot:run section compiles the code and sets up a classpath including all dependencies and launches the application. The "-Dspring-boot.run.profiles=api" tells it which profile to run which is configured in the "application.api.yml file.

