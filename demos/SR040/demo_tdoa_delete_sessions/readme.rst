..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. _sr040-demo-tdoa-delete-sessions:

=======================================================================
 demo_tdoa_delete_sessions
=======================================================================

.. brief:start

This demo showcases to delete all the existing non volatile sessions stored into the sro40 firmware.

.. brief:end

Delete sessions on QN9090-SR040
=======================================================================

Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Sr040 programmed with Ranging firmware


How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Ranging project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_tdoa_delete_sessions``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_tdoa_delete_sessions`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040  natively as::

    demo_tdoa_delete_sessions-SR040.bin


.. only:: nxp

    Delete sessions on S32K-SR040
    =======================================================================

    Prerequisites
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    - S32Uart firmware running on S32K


    How to Run
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Build the Ranging project for SR040 with the following CMake configurations:

    - ``UWBIOT_Host=PCWindows``

    - ``UWBIOT_OS=FreeRTOS``

    - ``UWBIOT_TML=S32Uart``

    - ``UWBIOT_UWBD=SR040``

    - Project:``demo_tdoa_delete_sessions``
