// src/main/java/com/OSProject/LinuxTaskManager/service/LmtClient.java
package com.OSProject.LinuxTaskManager.service;

import com.OSProject.LinuxTaskManager.model.Ltm_Dto;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.net.StandardProtocolFamily;
import java.net.UnixDomainSocketAddress;
import java.nio.channels.Channels;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.util.List;

/**
 * Encapsulates all communication over the UNIX-domain socket with the C daemon.
 * - fetchProcesses(): connect, read one JSON snapshot, parse into List<Ltm_Dto>
 * - sendCommand()   : connect, write a JSON command string (e.g. kill or renice)
 */
@Service
public class Ltm_Client {


    /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    /** TODO: Path to the UNIX socket (configured in application.yml) */
    /** !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
    @Value("${lmt.socket-path:/tmp/lmt.sock}")
    private String socketPath;

    /** Jackson mapper for JSON ↔ Java conversion */
    private final ObjectMapper mapper = new ObjectMapper();

    /**
     * Fetches the latest process snapshot.
     * @return list of Ltm_Dto parsed from a newline-terminated JSON array
     */
    public List<Ltm_Dto> fetchProcesses() throws Exception {
        // Build a UnixDomainSocketAddress from the configured path
        UnixDomainSocketAddress address = UnixDomainSocketAddress.of(socketPath);

        // Open a socket channel using the UNIX protocol
        try (SocketChannel channel = SocketChannel.open(StandardProtocolFamily.UNIX, address);
             BufferedReader reader = new BufferedReader(Channels.newReader(channel, StandardCharsets.UTF_8))) {

            // Read exactly one line (one JSON array) from the daemon
            String json = reader.readLine();
            // Deserialize into a List<Ltm_Dto>
            return mapper.readValue(json, new TypeReference<List<Ltm_Dto>>() {});
        }
    }

    /**
     * Sends a control command back to the daemon.
     * @param cmdJson a JSON-encoded command (e.g. {"type":"kill","pid":1234})
     */
    public void sendCommand(String cmdJson) throws Exception {
        UnixDomainSocketAddress address = UnixDomainSocketAddress.of(socketPath);

        try (SocketChannel channel = SocketChannel.open(StandardProtocolFamily.UNIX, address);
             BufferedWriter writer = new BufferedWriter(Channels.newWriter(channel, StandardCharsets.UTF_8))) {

            // Write the JSON command (no newline needed; daemon reads until EOF)
            writer.write(cmdJson);
            writer.flush();
        }
    }
}
