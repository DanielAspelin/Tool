# SPDX-License-Identifier: GPL-3.0-only
function(slots, live, completed, failed) {
    stopifnot(slots >= 2L, slots <= 4L, live >= 0L, live <= slots,
              completed >= 0L, failed >= 0L)
    total <- completed + failed
    c(100 * live / slots, if (total == 0) 0 else 100 * completed / total)
}
