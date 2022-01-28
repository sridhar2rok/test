..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.


.. _sr040-demo-swup-device-info:

=======================================================================
 demo_swup_device_info
=======================================================================


.. brief:start

This examples Uses SWUP interface and prints device
information.

It is mandatory to power-cycle device after running this example.

.. brief:end

Software Update Program
=======================================================================

See :numref:`swup` :ref:`swup`

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_swup_device_info`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040 using this precompiled binary.

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
    APP     :INFO :Finished /opt/_ddm/uwbiot
