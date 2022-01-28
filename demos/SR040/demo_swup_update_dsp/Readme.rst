..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.


.. _sr040-demo-swup-update-dsp:

=======================================================================
 demo_swup_update_dsp
=======================================================================


.. brief:start

This examples updates to the latest DSP Image over SWUP.   If same version
is found on the target device, it skips the DSP Upgrade.

Current version of DSP is ``2.19.3``

.. brief:end

Software Update Program
=======================================================================

:ref:`swup`

Running Swup Update DSP Demo on QN9090-SR040
=======================================================================

This demo is used to demonstrate Software update on SR040 natively and
using QN9090 as controller. We will send data packages through SPI
to SR040 .

Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- Packages updated in :file:`SwupPkgDSP.h`

How to build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the DSP Update project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_swup_update_dsp``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_swup_update_dsp`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040  natively as::

    demo_swup_update_dsp-SR040.bin

Logs: On success
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If everything is fine, at the end you will get message like this::

    SWUP    :INFO :TransferComponent #268
    SWUP    :INFO :TransferComponent #269
    SWUP    :INFO :TransferComponent #270
    SWUP    :INFO :TransferComponent #271
    SWUP    :INFO :TransferComponent #272
    SWUP    :INFO :VerifyAll
    SWUP    :INFO :FinishUpdate
    APP     :INFO :Execution completed
    APP     :INFO :Application passed
    APP     :INFO :Finished /opt/_ddm/uwbiot/src/uwbiot-top/demos/SR040/demo_swup_update_dsp/main.c : Succes!

Logs: On success (Same package)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If no update is needed, a message like this will be shown.  This is also success. ::

    RECV =   5 > 6001000101
    SEND =   8 > 2004000401010101
    RECV =   6 > 400400020000
    SEND =   9 > 2004000501E9024600
    RECV =   6 > 400400020000
    SEND =   9 > 2004000501EA02C800
    RECV =   6 > 400400020000
    SEND =   4 > 20020000
    RECV =  60 > 4002003800010834A0020101A108302E302E33610000E3085352303430000000E403000700E5021600E6080000000000000000E703021303E802042A
    APP     :WARN :Same package version found, exiting
    APP     :INFO :Execution completed
    APP     :INFO :Application passed
    APP     :INFO :Finished /opt/_ddm/uwbiot/src/uwbiot-top/demos/SR040/demo_swup_update_dsp/main.c : Succes!

Logs: Wrong Keys (Failure)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you use Engineering FW Update on Production Key / vice-versa, a failure message like this is shown ::

    SWUP    :INFO :TransferManifest #0
    SWUP    :INFO :TransferManifest #1
    SWUP    :INFO :TransferManifest #2
    SWUP    :INFO :TransferManifest #3
    SWUP    :INFO :StartUpdate
    APP     :INFO :Execution completed
    APP     :ERROR:Application failed
    APP     :ERROR:Finished /opt/_ddm/uwbiot/src/uwbiot-top/demos/SR040/demo_swup_update_dsp/main.c : Failed!

As a solution, use FW with right key.


Logs: Incomplete previous download (Failure)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If previous SWUP Update was interrupted, it is mandatory that previous SWUP Update is fully completed.
Until and unless that interrupted packages is not downloaded, new SWUP update will be REJECTED, as shown in
logs below.  The only solution is to first complete the previous package::

    SWUP    :INFO :VerifyAll
    APP     :INFO :Execution completed
    APP     :ERROR:Application failed
    APP     :ERROR:Finished /opt/_ddm/uwbiot/src/uwbiot-top/demos/SR040/demo_swup_update_dsp/main.c : Failed!


.. only:: nxp

    Running Swup Update DSP Demo on S32K-SR040
    =======================================================================

    This demo is used to demonstrate DSP update on SR040 from PC and
    using S32K as controller. We will send data packages through UART
    to S32K which will forward them to SR040.

    Prerequisites
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    - S32Uart firmware running on S32K
    - Packages updated in :file:`SwupPkgDSP.h`


    How to build
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    Compile the SWUP example with the following CMake configurations:

    - ``UWBIOT_Host=PCWindows``

    - ``UWBIOT_TML=S32Uart``

    - ``UWBIOT_UWBD=SR040``

    - Project: ``demo_swup_update_dsp``



    How to Run
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    The project ``demo_swup_update_dsp`` has package compiled for SR040
    located at :file:`binaries/PCWindows/` in the project and can be
    run for SR040 as::

        demo_swup_update_dsp.exe
