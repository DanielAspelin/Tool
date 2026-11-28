# SPDX-License-Identifier: GPL-3.0-only
function(totals, operation, records, bytes) {
    stopifnot(is.numeric(totals), length(totals) == 8L,
              all(is.finite(totals)), all(totals >= 0),
              operation %in% seq_len(6L),
              records %in% 0:32, bytes %in% 0:4096,
              operation == 4L || (records == 0L && bytes == 0L))
    totals[operation] <- totals[operation] + 1
    totals[7:8] <- totals[7:8] + c(records, bytes)
    totals
}
