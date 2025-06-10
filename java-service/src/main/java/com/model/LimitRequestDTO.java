package com.model;

import lombok.Data;

/**
 * Request body for CPU/RAM limits.
 * For CPU: seconds; for RAM: megabytes.
 */
@Data
public class LimitRequestDTO {
    private int limit;
}
