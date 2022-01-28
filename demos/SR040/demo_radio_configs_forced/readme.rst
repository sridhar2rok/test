..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.


.. _sr040-demo-radio-configs:

=======================================================================
 demo_radio_configs_forced
=======================================================================

.. brief:start

This is used to program Radio Configs to SR040 Example over UCI Interface.

It uses proprietory UCI Commands to do this programming.
.. brief:end


Update Radio config using QN9090-SR040
=======================================================================

How to Build
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the Radio Configs project for SR040 Using QN9090 MCUXpresso Project :

- Project:  ``QN9090_SR040_TR3``
- Source:   ``demo_radio_configs_forced``

Refer :ref:`qn9090-McuXpresso-project`.

How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The project ``demo_radio_configs_forced`` has package compiled for SR040 located at
:file:`binaries/QN9090_SR040_TR3/`
in the project and can be run for SR040  natively as::

    demo_radio_configs_forced-SR040.bin

Log (Success)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

At the end of program execution, log message like this must be seen::

    RECV =   5 > 4E11000100
    SEND = 136 > 2E110084010000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    RECV =   5 > 4E11000100
    SEND = 136 > 2E1100840180008000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000103913001000E8730000240800000000000000000000000000000000000000000000000000000000000000000000000000000000000001230400
    RECV =   5 > 4E11000100
    SEND =   7 > 2E110003027BCE
    RECV =   5 > 4E11000100
    APP     :INFO :Done!
    APP     :INFO :Finished /opt/_ddm/uwbiot/src/uwbiot-top/demos/SR040/demo_radio_configs_forced/main.c : Succes!

If such a log is not seen, re-run the program.
