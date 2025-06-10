package com.model;

import lombok.Data;

/**
 * Request body for POST /api/processes/{pid}/renice
 */
@Data
public class ReniceRequestDTO {
    private int nice;
}
