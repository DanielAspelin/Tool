# SPDX-License-Identifier: GPL-3.0-only
function(counts, operation, success) {
    stopifnot(is.numeric(counts), length(counts) == 4L,
              all(is.finite(counts) & counts >= 0),
              operation %in% 1:2, success %in% 0:1)
    counts[operation] <- counts[operation] + 1
    outcome <- if (success == 1L) 3L else 4L
    counts[outcome] <- counts[outcome] + 1
    counts
}
