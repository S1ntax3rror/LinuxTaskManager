package com.model;

import lombok.Data;

/**
 * Request body for POST /api/processes/{pid}/renice
 */
@Data
public class ReniceRequest {
    private int nice;
}
