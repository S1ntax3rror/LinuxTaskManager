package com.OSProject.LinuxTaskManager.service;

import com.OSProject.LinuxTaskManager.model.Ltm_Dto;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Profile;
import org.springframework.stereotype.Service;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.net.StandardProtocolFamily;
import java.net.UnixDomainSocketAddress;
import java.nio.channels.Channels;
import java.nio.channels.SocketChannel;
import java.nio.charset.StandardCharsets;
import java.util.List;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;

/**
 * Real implementation talking over a UNIX-domain socket.
 * Active in all profiles except "dev".
 */
@Service
@Profile("!dev")
public class Ltm_Client implements ProcessService {

    @Value("${ltm.socket-path}")
    private String socketPath;

    private final ObjectMapper mapper = new ObjectMapper();

    /**
     * Fetches the latest process snapshot over the UNIX socket.
     */
    @Override
    public List<Ltm_Dto> fetchProcesses() throws Exception {
        UnixDomainSocketAddress address = UnixDomainSocketAddress.of(socketPath);

        // 1) Open the channel for UNIX sockets
        try (SocketChannel channel = SocketChannel.open(StandardProtocolFamily.UNIX)) {
            // 2) Connect to the address
            channel.connect(address);

            // 3) Wrap it in a Reader and read one JSON line
            try (BufferedReader reader = new BufferedReader(
                     Channels.newReader(channel, StandardCharsets.UTF_8))) {

                String json = reader.readLine();
                // 4) Deserialize into our DTO list
                return mapper.readValue(json, new TypeReference<List<Ltm_Dto>>() {});
            }
        }
    }

    /**
     * Sends a JSON-encoded command back to the daemon.
     */
    @Override
    public void sendCommand(String cmdJson) throws Exception {
        UnixDomainSocketAddress address = UnixDomainSocketAddress.of(socketPath);

        try (SocketChannel channel = SocketChannel.open(StandardProtocolFamily.UNIX)) {
            channel.connect(address);

            try (BufferedWriter writer = new BufferedWriter(
                     Channels.newWriter(channel, StandardCharsets.UTF_8))) {

                writer.write(cmdJson);
                writer.flush();
            }
        }
    }
}
