..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr040-demo-tracker-sr040:

=======================================================================
 demo_tracker_sr040
=======================================================================

.. brief:start

This demo showcases App ranging with SR040 configured as a Controller - Initiator
and Helios configured as a Controllee - Responder.

1.SWUP Update: SWUP update is done only if there is a FW mismatch is detected between existing
  FW in device and FW to be updated.
2.DSTWR P2P Ranging

For details on Peer-to-Peer ranging sequence and configuration details
for Helios and SR040, refer to :ref:`p2p-ranging`.

.. brief:end

App Ranging on QN9090-SR040
=======================================================================

Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Helios programmed with Ranging firmware, configured as a Controllee - Responder


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Ranging project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_tracker_sr040``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_tracker_sr040`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040  natively as::

    demo_tracker_sr040-SR040.bin


.. only:: nxp

    App Ranging on S32K-SR040
    =======================================================================

    Prerequisites
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    - S32Uart firmware running on S32K
    - Helios programmed with Ranging firmware, configured as a Controllee - Responder


    How to Run
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Build the Ranging project for SR040 with the following CMake configurations:

    - ``UWBIOT_Host=PCWindows``

    - ``UWBIOT_OS=FreeRTOS``

    - ``UWBIOT_TML=S32Uart``

    - ``UWBIOT_UWBD=SR040``

    - Project:``demo_tracker_sr040``
