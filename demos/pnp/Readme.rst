..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.


.. _pnp-fw:

=======================================================================
 QN9090 Plug-n-Play FW
=======================================================================

This firmware is used as a PnP FW for QN9090 to run applications from PC.
PC would send commands over UART to the PnP firmware, which would be forwarded
to SR040 and the responses and notifications are returned to PC.

Pre-built firmware is present in :file:`binaries/QN9090_SR040_TR3`
directory.

The PnP FW also supports SR040 FW update over SWUP. In this case, SR040 FW
would be updated before starting any UART tasks. While the FW is being
updated, we should not send any command over UART. See next section to
understand how to determine if FW is being updated or PnP can be used by
PC application.


How to Use
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Flash the FW to QN9090 demo board.

During FW update blue LED would be ON. During this time the user should
not run any application from PC. Green LED is used to determine if FW
version was same or we need an update. In case the FW version is not same
as on SR040, green LED would blink before starting the update.
After FW update process is completed or skipped if the FW version was same,
blue LED would turn OFF.

.. note:: User should run PC application only when blue LED is OFF.

On PC, set environment variable ``UWBIOT_ENV_COM``
to the QN9090 UART *COMPORT* and execute the application.
