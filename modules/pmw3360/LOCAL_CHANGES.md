# Stock Keyball39 integration

Source: https://github.com/george-norton/zmk-driver-pmw3360
Commit: 0dfa2f0411b795613a35f222f993e45a1e4cc32a
The upstream MIT license and copyright notices are preserved.

The stock sensor board does not wire MOTION. Fix the polling path to allocate
and initialize a real k_work_delayable instead of treating a k_work as one.
Use delayed work for the optional interrupt path too. Skip absent IRQ GPIOs,
configure chip-select before resetting SPI, reject failed reads, and stop
polling if a PMW3360 product ID (0x42) is not detected.

No SROM upload is used, matching the stock QMK default. CPI starts at 500.
SPI is four-wire, not the three-wire PMW3610 wiring of the parent config.
