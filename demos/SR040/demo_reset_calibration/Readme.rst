..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.


.. _calibration-app:

=======================================================================
 demo_reset_calibration
=======================================================================

This demo is used to reset the trim parameters ``TX_POWER_DIFF``, ``FREQ_DIFF``
and ``ANTENNA_DELAY``.

The device needs to be in ``READY`` state to apply any trim value.
Trim values are stored in an NVM and applied only on device boot.
So after updating any of these values, we need to reboot the device for
these configs to apply. At device boot, if the CRC of stored values is
not valid, all values are set to default.
The difference is always calculated as received value - expected value.


How to build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Compile the example for SR040 with the following CMake configurations:

- Project: ``demo_reset_calibration``

- ``UWBIOT_Host=QN9090_SR040_TR3``

- ``UWBIOT_TML=SPI``

- ``UWBIOT_UWBD=SR040``


To compile for PC based build, compile with the following CMake configurations:

- Project: ``demo_reset_calibration``

- ``UWBIOT_Host=PCWindows``

- ``UWBIOT_TML=S32Uart``

- ``UWBIOT_UWBD=SR040``



How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To run on QN9090, flash the compiled binary on to QN9090 demo board and
reset the board.

To run using ``UWBIOT_Host=PCWindows``, run as::

    set UWBIOT_ENV_COM=COMxx
    demo_reset_calibration.exe

where *COMxx* is the comport of S32K.
