..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. include:: /docs/src/uwbiot_ver.rst.txt

.. _sr040-demo-swup-update-fw:


=======================================================================
 demo_swup_update_fw
=======================================================================

.. brief:start

This examples updates to the latest SR040 FW over SWUP.   If same version
is found on the target device, it skips the FW Upgrade.

Current version of FW is ``v0.8.4``.

.. brief:end


Software Update Program
=======================================================================

:ref:`swup`

Running Swup Update FW Demo on QN9090-SR040
=======================================================================

This demo is used to demonstrate Software update on SR040 natively and
using QN9090 as controller. We will send data packages through SPI
to SR040 .

Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Packages updated in :file:`SwupPkgFW.h`

How to build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the FW Update project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_swup_update_fw``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_swup_update_fw`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040  natively as::

    demo_swup_update_fw-sr040.bin


Logs: Success and failure.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See :numref:`sr040-demo-swup-update-dsp` :ref:`sr040-demo-swup-update-dsp`.
Since DSP update and FW Update are done over SWUP, the logs and their action is
exactly the same.

.. only:: nxp

    Running Swup Update FW Demo on S32K-SR040
    =======================================================================

    This demo is used to demonstrate FW update on SR040 from PC and
    using S32K as controller. We will send data packages through UART
    to S32K which will forward them to SR040.

    Prerequisites
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    - S32Uart firmware running on S32K
    - Packages updated in :file:`SwupPkgFW.h`


    How to build
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Compile the SWUP example with the following CMake configurations:

    - ``UWBIOT_Host=PCWindows``

    - ``UWBIOT_TML=S32Uart``

    - ``UWBIOT_UWBD=SR040``

    - Project: ``demo_swup_update_fw``



    How to Run
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    The project ``demo_swup_update_fw`` has package compiled for SR040
    located at :file:`binaries/PCWindows/` in the project and can be
    run for SR040 as::

        demo_swup_update_fw.exe
