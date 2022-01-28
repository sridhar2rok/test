..
    Copyright 2020 NXP

    This software is owned or controlled by NXP and may only be used
    strictly in accordance with the applicable license terms.  By expressly
    accepting such terms or by downloading, installing, activating and/or
    otherwise using the software, you are agreeing that you have read, and
    that you agree to comply with and are bound by, such license terms.  If
    you do not agree to be bound by the applicable license terms, then you
    may not retain, install, activate or otherwise use the software.

.. highlight:: shell


.. _demo-device-calibration-py:

=======================================================================
 demo_device_calibration_py
=======================================================================

This python script is to perform device calibration by setting trim values
for ``TX_POWER_DIFF``, ``FREQ_DIFF`` and ``ANTENNA_DELAY`` and starting
Continuous Wave test mode on QN9090-SR040.

We communicate with QN9090 PnP firmware by sending UCI
data packets over UART and receiving responses and notifications.

To calibrate ``TX_POWER_DIFF`` and ``FREQ_DIFF``, we can use the output
of CW Test mode and measure the power and frequency at which spectrum
is seen. To calibrate ``ANTENNA_DELAY``, we need to start ranging and
measure the distance against a calibrated device.

Refer to :ref:`testmode-config` and :ref:`ranging-config` for details
on application configurations for Test modes and ranging.



Prerequisites
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- PnP Firmware on QN9090. See :ref:`pnp-fw`


Trim Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Trim values for ``TX_POWER_DIFF``, ``FREQ_DIFF`` and ``ANTENNA_DELAY`` can be
configured in the python file :file:`demo_device_calibration.py` as:

.. literalinclude:: demo_device_calibration.py
    :language: python
    :start-after: # doc:start:trim-values
    :end-before: # doc:end:trim-values


For more information on device calibration, refer :numref:`trim-params`
:ref:`trim-params`.



How to Run
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Set environment variable ``UWBIOT_ENV_COM`` to QN9090 PnP UART port:

.. literalinclude:: pnp_core.py
    :language: python
    :start-after: # doc:start:comport
    :end-before: # doc:end:comport


Run :file:`demo_device_calibration.py` as::

    python demo_device_calibration.py

This script will write the trim values and reset the device.

#) To calibrate ``TX_POWER_DIFF``, ``FREQ_DIFF`` and ``TX_ADAPTIVE_POWER_CALC``,
   run :file:`demo_test_mode.py` as::

    python demo_test_mode.py

   This script will start Test mode.
   Based on output tone, trim parameters ``TX_POWER_DIFF``, ``FREQ_DIFF`` and
   ``TX_ADAPTIVE_POWER_CALC`` can be fine tuned to calibrate the device.


#) To calibrate ``ANTENNA_DELAY``, run :file:`demo_ranging.py` as::

    python demo_ranging.py


   .. note:: Make sure you run ranging against a calibrated device.

   Based on the distance received in ranging, ``ANTENNA_DELAY`` can
   be fine tuned to calibrate the device.


.. _testmode-config:

Application configuration for Test Modes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

TX Mode configurations::

    Channel number    => 9
    TX_POWER          => 0x00 (max)
    Slot type         => SP3
    Delay             => 0
    EVENT_COUNTER_MAX => 0x14
    TX_CYCLE_TIME     => 1000us

RX Mode configurations::

    Channel number    => 9
    TX_POWER          => 0x00 (max)
    Slot type         => SP3
    Delay             => 0
    EVENT_COUNTER_MAX => 60
    TIME_OUT          => 0

CW Mode configurations::

    Channel number    => 9
    TX_POWER          => 0x00 (max)


Also see :numref:`sr040-test-modes` :ref:`sr040-test-modes`.

At the end of RX Modes, you should see summary like this::

    ########################################
     RX Stats Summary
    ########################################
    ntfCount = 89
    OK                                       =  86
    TIMEOUT                                  =   3
    STS_FAILED                               =   0
    TOA_FAILED                               =   0
    PHY_DECODING_FAILED                      =   0
    ****************************************
    NUM_FRAMES                               =  89
    ATTENUATION                              =   0
    RX_TIMEOUTS                              =   3
    RX_DECODING_FAILS                        =   0
    RX_TOA_FAILS                             =   0
    RX_STS_FAILS                             =   0
    RX_PSDU_FAILS                            =   0
    RX_ERRORS_TOTAL                          =   3
    PER_PERCENT                              = 3.370787
    ########################################


.. note:: This report shows contains failures just for demonstration.
    A good report must have all pass notifications.


.. _phy-log-parsing:

PHY Log parsing and PER measurement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Python script :file:`demo_parse_per.py` can be used to parse PHY Logs
generated from :numref:`demo-sr040-test-modes` :ref:`demo-sr040-test-modes`.
It will print out the same statistics as mentioned above.

Run the script as::

    python demo_parse_per.py uwb_per_log_COMXX.log

where, ``uwb_per_log_COMXX.log`` is the path to input log file generated
based on the COMPORT *COMXX*.


.. _ranging-config:

Application configuration for Ranging
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Application configurations used for ranging are::

    RANGING_METHOD        => DS-TWR
    DEVICE_TYPE           => Controller
    DEVICE_ROLE           => Initiator
    MULTI_NODE_MODE       => Unicast
    RFRAME_CONFIG         => STS follows SFD, PPDU has no PHR or PSDU
    DEVICE_MAC_ADDRESS    => 0x1111
    DST_MAC_ADDRESS       => 0x2222
    SLOTS_PER_RR          => 24 slots
    RANGING_INTERVAL      => 192 ms
    SFD_ID                => BPRF
    TX_POWER_ID           => +14dB
    NUMBER_OF_ANCHORS     => 1




.. On successful execution, you should be able to see these log::

..     NXPUCIR <=
..       62 00 00 37
..     00 44 33 22 11 00 c0 00 01 00 00 00 00 00 00 00 00 00 00 01 22 22 00 00 d0 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
..     ***** Ranging Detected ****
..     ToF:3792 - AoA:0.000000 ; avg_ToF:3792 - avg_AoA:0.000000 LoS
..     NXPUCIR <=
..       62 00 00 37
..     01 44 33 22 11 00 c0 00 01 00 00 00 00 00 00 00 00 00 00 01 22 22 00 00 d3 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
..     ***** Ranging Detected ****
..     ToF:3795 - AoA:0.000000 ; avg_ToF:3793 - avg_AoA:0.000000 LoS
..     NXPUCIR <=
..       62 00 00 37
..     02 44 33 22 11 00 c0 00 01 00 00 00 00 00 00 00 00 00 00 01 22 22 00 00 d6 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
..     ***** Ranging Detected ****
..     ToF:3798 - AoA:0.000000 ; avg_ToF:3795 - avg_AoA:0.000000 LoS
